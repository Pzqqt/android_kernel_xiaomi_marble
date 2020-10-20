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
 * DOC: This file has ZERO CAC DFS functions.
 * Abstract:- Operation in a DFS channel requires CAC that adds additional
 * delay as well as loss of connection even when CSA is used. ETSI allows
 * pre-CAC, i.e. performing CAC at a convenient time and using that channel
 * later. Once Pre-CAC is done in a channel, it is no longer required to
 * perform a CAC in the channel before TX/RX as long as radar is not found in
 * it or we reset or restart the device.
 *
 * Design:-
 * When Zero-CAC is enabled and the current regulatory domain is ETSI,
 * a Binary Search Forest (BSForest) is initialized and maintained, indexed by
 * DFS IEEE channels of different bandwidths (20/40/80 MHz).
 *
 * The structure of precac BSForest is:
 *   1). A preCAC list of 80MHz channels which contains the Binary Search Tree
 *       (BSTree) root pointer.
 *   2). The BSTree consists of nodes of different IEEEs of different
 *       bandwidths (80/40/20 MHz) of that 80MHz channel in the list.
 *
 * Each Binary Search Tree (BSTree) node has a unique IEEE channel and
 * three values that indicate three statuses (Channel valid / CAC Done /
 * Channel in NOL) of the subchannels of the node.
 *
 * A sample Precac BSForest:
 *
 * List HEAD --------> 58 -------------> 106 --------------> 122
 *                      |                 |                   |
 *                     58                106                 122
 *                     /\                / \                 / \
 *                    /  \              /   \               /   \
 *                   /    \            /     \             /     \
 *                  /      \          /       \           /       \
 *                 54      62       102       110       118       126
 *                 /\      /\       / \       / \       / \       / \
 *                /  \    /  \     /   \     /   \     /   \     /   \
 *               52  56  60  64  100  104  108  112  116  120  124   128
 *
 * Consider the BSTree 106, where all subchannels of 106HT80 are available
 * in the regulatory (100, 104, 108, 112 are valid channels) and 100HT20 is
 * preCAC done and 104HT20 is in NOL. The BSTree would look like:
 *
 *                               _________
 *                              |   | | | |
 *                              |106|4|1|1|
 *                              |___|_|_|_|
 *                                 _/ \_
 *                               _/     \_
 *                             _/         \_
 *                           _/             \_
 *                         _/                 \_
 *                 _______/_                   _\_______
 *                |   | | | |                 |   | | | |
 *                |102|2|1|1|                 |110|2|0|0|
 *                |___|_|_|_|                 |___|_|_|_|
 *                    / \                           / \
 *                   /   \                         /   \
 *                  /     \                       /     \
 *                 /       \                     /       \
 *          ______/__     __\______       ______/__     __\______
 *         |   | | | |   |   | | | |     |   | | | |   |   | | | |
 *         |100|1|1|0|   |104|1|0|1|     |108|1|0|0|   |112|1|0|0|
 *         |___|_|_|_|   |___|_|_|_|     |___|_|_|_|   |___|_|_|_|
 *
 *
 *  Syntax of each node:
 *      _______________________________
 *     |      |       |          |     |
 *     | IEEE | Valid | CAC done | NOL |
 *     |______|_______|__________|_____|
 *
 * where,
 * IEEE     - Unique IEEE channel that the node represents.
 * Valid    - Number of valid subchannels of the node (for the current country).
 * CAC done - Number of subchannels of the node that are CAC
 *            (primary or preCAC) done.
 * NOL      - Number of subchannels of the node in NOL.
 *
 * PreCAC (legacy chipsets. e.g. QCA9984):
 *   The pre-CAC is done in a RADIO that has  VHT80_80 capable radio where the
 *   primary and secondary HT80s can be programmed independently with two
 *   different HT80 channels.
 *   The bandwidth of preCAC channels are always 80MHz.
 *
 * Agile CAC (e.g. Hawkeye V2):
 *   The Agile CAC is done in a chipset that has a separate Agile detector,
 *   which can perform Rx on the channel provided by stealing the chains
 *   from one of the primary pdevs.
 *   Note: This impliess that the bandwidth of the Agile detector is always
 *   the same as the pdev it is attached to.
 *   The bandwidth of Agile CAC channels may vary from 20/40/80 MHz.
 *
 * Operations on preCAC list:
 *
 *  Initialize the list:
 *    To initialize the preCAC list,
 *      1. Find a 80MHz DFS channel.
 *      2. Add an entry to the list with this channel as index and create
 *         a BSTree for this channel. This is done by level order insertion
 *         where the channel for each node is determined by adding the
 *         respective level offsets to the 80MHz channel.
 *      3. Repeat step 1 & 2 until no 80MHz DFS channels are found.
 *
 *  Remove the list:
 *   To remove the preCAC list,
 *      1. Iterate through the list and for every entry,
 *         a). Convert the tree into a left child only list, removing the
 *             root node on the way. O(n) deletion.
 *         b). Remove the preCAC list entry.
 *
 *   Algorithm to convert the tree to a left child only list:
 *     1. Find the leftmost leaf node of the BSTree.
 *     2. Set current node as root node.
 *     3. If current node has right child, add right child of current node
 *        as left child of leftmost leaf.
 *     4. Update the leftmost leaf.
 *     5. Update current node to left child and remove the node.
 *     6. Repeat steps 3 to 5 till current node is NULL.
 *
 *  Print the list:
 *   To print the contents of the preCAC list,
 *    1. Iterate through the list and for every entry,
 *       a) Perform a morris preorder traversal (iterative and O(n)) and
 *          for every node, print the Channel IEEE and CAC and NOL values.
 *          Use the level information to create a tree(3) command like
 *          structure for printing each nodes of the BSTree.
 *
 *   A sample BSTree print output:
 *
 *        A                  A(C,N)
 *       / \                 |
 *      B   C                |------- B(C,N)
 *     / \                   |        |
 *    D   E                  |        |------- D(C,N)
 *                           |        |
 *                           |        |------- E(C,N)
 *                           |
 *                           |------- E(C,N)
 *
 *    Where C is number of CACed subchannels, and N is number of
 *    NOL subchannels.
 *
 *  Find a channel to preCAC/Agile CAC:
 *   1. Given a requested bandwidth (80MHz always in case of preCAC, XMHz in
 *      case of Agile CAC where X is the current operating bandwidth of the
 *      pdev the detector is attached to), iterate through the preCAC list.
 *   2. For every entry, find if there a valid channel that is not in NOL
 *      and not in CAC done and is of the requested mode.
 *   3. If such channel exists and is not equal to the current operating
 *      channel, then return the channel. Else, go to the next entry.
 *
 *  Find if the channel is preCAC done:
 *   1. Given a IEEE channel, go through the preCAC list and find the entry
 *      which has the channel provided.
 *   2. Traverse through the BSTree and check if the channel's CACed
 *      subchannels value is equal to the number of subchannels of that level.
 *   3. If the above condition is true, return 1, else 0.
 *
 *  Mark the channel as CAC done:
 *   1. Given a channel, find all the subchannels.
 *   2. For every subchannel, iterate through the list, and find the entry
 *      the channel belongs to.
 *   3. Traverse through the BSTree of this entry, going to left or right
 *      child based on the channel IEEE.
 *   4. Increment the CACed subchannels count (by 1) along the way till
 *      (and including) the node that contains the subchannel that
 *      was searched for.
 *
 *  Unmark the channel as CAC done:
 *   1. Given a 20MHz channel, iterate through the list, and find the entry
 *      the channel belongs to.
 *   2. Traverse through the BSTree of this entry, going to left or right
 *      child based on the channel IEEE.
 *   3. Decrement the CACed subchannels count (by 1) along the way till
 *      (and including) the node that contains the subchannel that
 *      was searched for.
 *
 *  Mark the channel as NOL:
 *   1. Given a 20MHz channel, iterate through the list, and find the entry
 *      the channel belongs to.
 *   3. Traverse through the BSTree of this entry, going to left or right
 *      child based on the channel IEEE.
 *   4. Increment the NOL subchannels count (by 1) along the way till
 *      (and including) the node that contains the subchannel that
 *      was searched for.
 *   5. If the subchannel node's CAC subchannels value is non-zero, unmark
 *      the channel as CAC done.
 *
 *  Unmark the channel as NOL:
 *   1. Given a 20MHz channel, iterate through the list, and find the entry
 *      the channel belongs to.
 *   3. Traverse through the BSTree of this entry, going to left or right
 *      child based on the channel IEEE.
 *   4. Decrement the NOL subchannels count (by 1) along the way till
 *      (and including) the node that contains the subchannel that
 *      was searched for.
 *
 * New RadarTool commands:-
 * 1)radartool -i wifi[X] bangradar 1 (where 1 is the segment ID)
 * It simulates RADAR from the secondary HT80 when the
 * secondary HT80 is doing pre-CAC. If secondary is not
 * doing any pre-CAC then this command has no effect.
 * 2)radartool -i wifi[X] showPreCACLists
 * It shows all the pre-CAC Lists' contents.
 *
 * New iwpriv commands:-
 * 1)iwpriv wifi[X] preCACEn 0/1
 * This command enables/disables the zero-cac-DFS.
 * 2)iwpriv wifi[X] pCACTimeout <timeout>
 * Override the pCACTimeout.
 *
 * FAQ(Frequently Asked Questions):-
 * 1)
 * Question:
 *    Why was the separate HT80 preCAC NOL timer removed?
 * Answer:
 *    In previous design, the channels that were preCACed were always
 *    80MHz channels. Since NOL timers were maintained for 20MHz channels,
 *    a separate preCAC NOL timer was created to modularize and move
 *    lists accordingly at the expiry of the timer.
 *    With the current support of 20/40/80MHz preCAC channels, and
 *    the introduction of subchannel marking, the existing NOL timer
 *    can be used to mark the preCAC lists aswell.
 */

#include "dfs_zero_cac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_utils_api.h"
#include "dfs_internal.h"
#include "dfs_process_radar_found_ind.h"
#include "target_if.h"
#include "wlan_dfs_init_deinit_api.h"

/* Given a bandwidth, find the number of subchannels in that bandwidth */
#define N_SUBCHS_FOR_BANDWIDTH(_bw) ((_bw) / MIN_DFS_SUBCHAN_BW)

#define DFS_160MHZ_SECSEG_CHAN_OFFSET 40

 /*dfs_zero_cac_reset() - Reset zero cac variables.
  *@dfs: Pointer to wlan_dfs
  */
void dfs_zero_cac_reset(struct wlan_dfs *dfs)
{
	dfs->dfs_precac_timeout_override = -1;
	dfs->dfs_precac_primary_freq_mhz = 0;
	dfs->dfs_precac_secondary_freq_mhz = 0;
}

#if defined(ATH_SUPPORT_ZERO_CAC_DFS) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_timer_detach(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_free(&dfs_soc_obj->dfs_precac_timer);
}
#endif

int dfs_override_precac_timeout(struct wlan_dfs *dfs, int precac_timeout)
{
	if (!dfs)
		return -EIO;

	dfs->dfs_precac_timeout_override = precac_timeout;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "PreCAC timeout is now %s (%d)",
		(precac_timeout == -1) ? "default" : "overridden",
		precac_timeout);

	return 0;
}

int dfs_get_override_precac_timeout(struct wlan_dfs *dfs, int *precac_timeout)
{
	if (!dfs)
		return -EIO;

	(*precac_timeout) = dfs->dfs_precac_timeout_override;

	return 0;
}

bool dfs_is_legacy_precac_enabled(struct wlan_dfs *dfs)
{
	return dfs->dfs_legacy_precac_ucfg;
}

#ifdef QCA_SUPPORT_AGILE_DFS
bool dfs_is_agile_precac_enabled(struct wlan_dfs *dfs)
{
	return (dfs->dfs_agile_precac_ucfg && dfs->dfs_fw_adfs_support_non_160);
}
#endif

/* dfs_descend_precac_tree_for_freq() - Descend into the precac BSTree based on
 *                             the channel provided. If the channel is less than
 *                             given node's channel, descend left, else right.
 * @node:    Precac BSTree node.
 * @chan_freq: Channel freq whose node is to be found.
 *
 * Return: the next precac_tree_node (left child or right child of
 * current node).
 */

static struct precac_tree_node *
dfs_descend_precac_tree_for_freq(struct precac_tree_node *node,
				 uint16_t chan_freq)
{
	if (!node)
		return NULL;

	if (chan_freq < node->ch_freq)
		return node->left_child;
	else
		return node->right_child;
}

/* dfs_find_curchwidth_and_center_chan_for_freq() - Find channel width and
 *                                                  center channelfrequency.
 * @dfs: Pointer to wlan_dfs.
 * @chwidth: Pointer to phy_ch_width.
 * @primary_chan_freq: Pointer to primary channel.
 * @secondary_chan_freq: Pointer to secondary channel.
 */
void
dfs_find_curchwidth_and_center_chan_for_freq(struct wlan_dfs *dfs,
					     enum phy_ch_width *chwidth,
					     qdf_freq_t *primary_chan_freq,
					     qdf_freq_t *secondary_chan_freq)
{
	struct dfs_channel *curchan = dfs->dfs_curchan;

	if (!curchan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "curchan is NULL");
		return;
	}

	if (primary_chan_freq)
		*primary_chan_freq = curchan->dfs_ch_mhz_freq_seg1;
	if (WLAN_IS_CHAN_MODE_20(curchan)) {
		*chwidth = CH_WIDTH_20MHZ;
	} else if (WLAN_IS_CHAN_MODE_40(curchan)) {
		*chwidth = CH_WIDTH_40MHZ;
	} else if (WLAN_IS_CHAN_MODE_80(curchan)) {
		*chwidth = CH_WIDTH_80MHZ;
	} else if (WLAN_IS_CHAN_MODE_80_80(curchan)) {
		*chwidth = CH_WIDTH_80P80MHZ;
		if (secondary_chan_freq)
			*secondary_chan_freq =
				curchan->dfs_ch_mhz_freq_seg2;
	} else if (WLAN_IS_CHAN_MODE_160(curchan)) {
		*chwidth = CH_WIDTH_160MHZ;
		if (primary_chan_freq)
			*primary_chan_freq =
				curchan->dfs_ch_mhz_freq_seg2;
	}
}

/* dfs_find_cac_status_for_chan_for_freq() - Find CAC-done status for the
 *                                  channel in the precac Binary Search Tree.
 *				    Return true if CAC done, else false.
 * @dfs_precac_entry: Precac entry which has the root of the precac BSTree.
 * @chan_freq:        IEEE channel freq. This is the center of a
 *                    20/40/80/160/165 MHz channel and the center channel is
 *                    unique irrespective of the bandwidth
 *                    (20/40/80/160/165 MHz).
 */
static bool
dfs_find_cac_status_for_chan_for_freq(struct dfs_precac_entry *precac_entry,
				      uint16_t chan_freq)
{
	struct precac_tree_node *node = precac_entry->tree_root;
	uint8_t n_cur_lvl_subchs = node->n_valid_subchs;

	while (node) {
		if (node->ch_freq == chan_freq)
			return (node->n_caced_subchs == n_cur_lvl_subchs) ?
				true : false;

		n_cur_lvl_subchs /= 2;
		node = dfs_descend_precac_tree_for_freq(node, chan_freq);
	}

	return false;
}

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

/*
 * dfs_is_precac_done_on_ht20_40_80_chan_for_freq() - Find if preCAC is done
 * for the given frequency.
 * @dfs: Pointer to wlan_dfs.
 * @chan_freq: Channel frequency in MHZ.
 */
bool
dfs_is_precac_done_on_ht20_40_80_160_165_chan_for_freq(struct wlan_dfs *dfs,
						       uint16_t chan_freq)
{
	struct dfs_precac_entry *precac_entry;
	bool ret_val = 0;

	/*
	 * A is within B-C and B+C
	 * (B-C) <= A <= (B+C)
	 */
	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list))
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_list,
			      pe_list) {
			/* Find if the channel frequency is
			 * in this precac_list.
			 */
			if (IS_WITHIN_RANGE_STRICT(chan_freq,
					precac_entry->center_ch_freq,
					(precac_entry->bw/2))) {
				ret_val = dfs_find_cac_status_for_chan_for_freq(
						precac_entry, chan_freq);
				break;
			}
		}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS, "ch_freq = %u cac_done = %d",
		  chan_freq, ret_val);

	return ret_val;
}

/*
 * dfs_is_precac_done_on_ht8080_chan - Find if preCAC is done
 * for the given frequency.
 * @dfs: Pointer to wlan_dfs.
 * @chan: Pointer to dfs_channel.
 */
bool dfs_is_precac_done_on_ht8080_chan(struct wlan_dfs *dfs,
				       struct dfs_channel *chan)
{
	bool ret_val = 0, primary_found = 0;
	uint16_t cfreq1, cfreq2 = 0;

	cfreq1 = chan->dfs_ch_mhz_freq_seg1;
	cfreq2 = chan->dfs_ch_mhz_freq_seg2;

	/* Check if primary is DFS then search */
	if (WLAN_IS_CHAN_DFS(chan))
		primary_found =
			dfs_is_precac_done_on_ht20_40_80_160_165_chan_for_freq(
				dfs,
				cfreq1);
	else
		primary_found = 1;

	/* Check if secondary DFS then search */
	if (WLAN_IS_CHAN_DFS_CFREQ2(chan) && primary_found) {
		ret_val =
			dfs_is_precac_done_on_ht20_40_80_160_165_chan_for_freq(
				dfs,
				cfreq2);

	} else {
		if (primary_found)
			ret_val = 1;
	}
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "seg1_freq = %u seg2_freq = %u ret_val = %d",
		  cfreq1,
		  cfreq2,
		  ret_val);

	return ret_val;
}

/*
 * dfs_is_precac_done() - Verify if preCAC is done.
 * @dfs: Pointer to wlan_dfs.
 * @chan: Pointer to dfs_channel.
 */
bool dfs_is_precac_done(struct wlan_dfs *dfs, struct dfs_channel *chan)
{
	bool ret_val = 0;
	uint16_t cfreq;

	if (!WLAN_IS_CHAN_5GHZ(chan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Channel %d not a 5GHz channel",
			  chan->dfs_ch_ieee);
		return 0;
	}

	if (WLAN_IS_CHAN_MODE_160(chan))
		cfreq = chan->dfs_ch_mhz_freq_seg2;
	else if (WLAN_IS_CHAN_MODE_165(dfs, chan))
		cfreq = RESTRICTED_80P80_CHAN_CENTER_FREQ;
	else
	    /* Center frequency of 20/40/80MHz is given
	     * by dfs_ch_mhz_freq_seg1.
	     */
		cfreq = chan->dfs_ch_mhz_freq_seg1;

	if (WLAN_IS_CHAN_MODE_20(chan) ||
	    WLAN_IS_CHAN_MODE_40(chan) ||
	    WLAN_IS_CHAN_MODE_80(chan) ||
	    WLAN_IS_CHAN_MODE_160(chan) ||
	    WLAN_IS_CHAN_MODE_165(dfs, chan)) {
		ret_val =
		    dfs_is_precac_done_on_ht20_40_80_160_165_chan_for_freq(
				dfs,
				cfreq);
	} else if (WLAN_IS_CHAN_MODE_80_80(chan)) {
		ret_val = dfs_is_precac_done_on_ht8080_chan(dfs, chan);
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS, "precac_done_status = %d", ret_val);
	return ret_val;
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_find_pdev_for_agile_precac(struct wlan_objmgr_pdev *pdev,
				    uint8_t *cur_agile_dfs_index)
{
	struct wlan_dfs *dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_objmgr_psoc *psoc;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	psoc = wlan_pdev_get_psoc(pdev);

	dfs_soc_obj = dfs->dfs_soc_obj;

	*cur_agile_dfs_index =
	   (dfs_soc_obj->cur_agile_dfs_index + 1) % dfs_soc_obj->num_dfs_privs;
}

/*
 * dfs_fill_adfs_chan_params() - Fill the ADFS FW params.
 * @dfs: Pointer to wlan_dfs.
 * @adfs_param: Pointer to struct dfs_agile_cac_params.
 * @ch_freq: Frequency in MHZ to be programmed to the agile detector.
 */
static void dfs_fill_adfs_chan_params(struct wlan_dfs *dfs,
				      struct dfs_agile_cac_params *adfs_param)
{
	qdf_freq_t ch_freq = dfs->dfs_agile_precac_freq_mhz;

	adfs_param->precac_center_freq_1 =
		(ch_freq == RESTRICTED_80P80_CHAN_CENTER_FREQ) ?
		(RESTRICTED_80P80_LEFT_80_CENTER_FREQ) : ch_freq;
	adfs_param->precac_center_freq_2 =
		(ch_freq == RESTRICTED_80P80_CHAN_CENTER_FREQ) ?
		(RESTRICTED_80P80_RIGHT_80_CENTER_FREQ) : 0;
	adfs_param->precac_chan = utils_dfs_freq_to_chan(ch_freq);
	adfs_param->precac_chwidth = dfs->dfs_precac_chwidth;
}

/* dfs_agile_precac_cleanup() - Reset parameters of wlan_dfs.
 *
 * @dfs: Pointer to struct wlan_dfs.
 */
static void dfs_agile_precac_cleanup(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	qdf_timer_sync_cancel(&dfs_soc_obj->dfs_precac_timer);
	dfs_soc_obj->dfs_precac_timer_running = 0;
	dfs_soc_obj->precac_state_started = 0;
	dfs->dfs_agile_precac_freq_mhz = 0;
	dfs->dfs_precac_chwidth = CH_WIDTH_INVALID;
	dfs_soc_obj->cur_agile_dfs_index = DFS_PSOC_NO_IDX;
}

/*
 * dfs_prepare_agile_precac_chan() - Prepare an agile channel for preCAC.
 * @dfs: Pointer to wlan_dfs.
 *
 * Return Type: void
 */
void  dfs_prepare_agile_precac_chan(struct wlan_dfs *dfs, bool *is_chan_found)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_dfs *temp_dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint16_t ch_freq = 0;
	uint8_t cur_agile_dfs_idx = 0;
	uint16_t vhtop_ch_freq_seg1, vhtop_ch_freq_seg2;
	int i;
	struct dfs_agile_cac_params adfs_param;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_soc_obj = dfs->dfs_soc_obj;

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);

	pdev = dfs->dfs_pdev_obj;

	for (i = 0; i < dfs_soc_obj->num_dfs_privs; i++) {
		dfs_find_pdev_for_agile_precac(pdev, &cur_agile_dfs_idx);
		dfs_soc_obj->cur_agile_dfs_index = cur_agile_dfs_idx;
		temp_dfs = dfs_soc_obj->dfs_priv[cur_agile_dfs_idx].dfs;
		pdev = temp_dfs->dfs_pdev_obj;
		if (!dfs_soc_obj->dfs_priv[cur_agile_dfs_idx].agile_precac_active)
			continue;

		vhtop_ch_freq_seg1 =
			temp_dfs->dfs_curchan->dfs_ch_mhz_freq_seg1;
		vhtop_ch_freq_seg2 =
			temp_dfs->dfs_curchan->dfs_ch_mhz_freq_seg2;
		if (WLAN_IS_CHAN_MODE_160(temp_dfs->dfs_curchan)) {
			if (vhtop_ch_freq_seg2 < vhtop_ch_freq_seg1)
				vhtop_ch_freq_seg2 -=
					DFS_160MHZ_SECSEG_CHAN_OFFSET;
			else
				vhtop_ch_freq_seg2 +=
					DFS_160MHZ_SECSEG_CHAN_OFFSET;
		}
		dfs_set_agilecac_chan_for_freq(temp_dfs,
					       &ch_freq,
					       vhtop_ch_freq_seg1,
					       vhtop_ch_freq_seg2);

		if (!ch_freq) {
			qdf_info(" %s : %d No preCAC required channels left in current pdev: %pK",
				 __func__, __LINE__, pdev);
			continue;
		} else {
			break;
		}
	}

	if (ch_freq) {
		dfs_fill_adfs_chan_params(temp_dfs, &adfs_param);
		dfs_start_agile_precac_timer(temp_dfs,
					     dfs->dfs_soc_obj->ocac_status,
					     &adfs_param);
		qdf_info("%s : %d ADFS channel set request sent for pdev: %pK ch_freq: %d",
			 __func__, __LINE__, pdev, ch_freq);

		if (dfs_tx_ops && dfs_tx_ops->dfs_agile_ch_cfg_cmd)
			dfs_tx_ops->dfs_agile_ch_cfg_cmd(pdev,
							 &adfs_param);
		else
			dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"dfs_tx_ops=%pK", dfs_tx_ops);
		*is_chan_found = true;
	} else {
		dfs_agile_precac_cleanup(dfs);
		*is_chan_found = false;
	}
}
#endif

/* dfs_is_tree_node_marked_as_cac_for_freq() - Check if preCAC BSTree node is
 * marked as CAC.
 * @root: Pointer to root node of the preCAC BSTree.
 * @freq: 20MHz channel to be checked if marked as CAC done already.
 *
 * Return: True if already marked, else false.
 */
static bool
dfs_is_tree_node_marked_as_cac_for_freq(struct precac_tree_node *root,
					uint16_t freq)
{
	struct precac_tree_node *curr_node = root;

	while (curr_node) {
		if (!curr_node->n_caced_subchs)
			return false;
		if (curr_node->ch_freq == freq)
			return curr_node->n_caced_subchs;
		curr_node = dfs_descend_precac_tree_for_freq(curr_node,
							     freq);
	}
	return false;
}

/* dfs_mark_tree_node_as_cac_done_for_freq() - Mark the preCAC BSTree node as
 * CAC done.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac_list entry pointer.
 * @chan_freq:    IEEE channel freq to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_mark_tree_node_as_cac_done_for_freq(struct wlan_dfs *dfs,
					struct dfs_precac_entry *precac_entry,
					uint16_t chan_freq)
{
	struct precac_tree_node *curr_node;

	if (!precac_entry->tree_root) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Precac tree root pointer is NULL!");
		return;
	}

	curr_node = precac_entry->tree_root;
	/**
	 * Check if the channel is already marked and return if true.
	 * This will happen in scenarios like the following:
	 * preCAC is running on channel 128 in HT20 mode (note: 124 is already
	 * marked. Now if the mode is switched to HT40, preCAC is restarted
	 * and the new channel picked for preCAC is 126 HT40. Here, 124
	 * will be already marked since it was completed in HT20 mode.
	 * This may happen for any mode switches (20<->40<->80 MHz).
	 */
	if (dfs_is_tree_node_marked_as_cac_for_freq(curr_node, chan_freq))
		return;

	while (curr_node) {
		/* Update the current node's CACed subchannels count only
		 * if it's less than maximum subchannels, else return.
		 */
		if (curr_node->n_caced_subchs <
		    N_SUBCHS_FOR_BANDWIDTH(curr_node->bandwidth))
			curr_node->n_caced_subchs++;
		curr_node = dfs_descend_precac_tree_for_freq(curr_node,
							     chan_freq);
	}
}

/* dfs_unmark_tree_node_as_cac_done_for_freq() - Unmark the preCAC BSTree
 *                                       node as CAC done.
 * @precac_entry: Precac_list entry pointer.
 * @chan_freq:    IEEE channel freq to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_unmark_tree_node_as_cac_done_for_freq(struct dfs_precac_entry
					  *precac_entry, uint16_t chan_freq)
{
	struct precac_tree_node *curr_node = precac_entry->tree_root;

	while (curr_node) {
		if (curr_node->n_caced_subchs)
			curr_node->n_caced_subchs--;
		else
			return;
		curr_node = dfs_descend_precac_tree_for_freq(curr_node,
							     chan_freq);
	}
}

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
static uint8_t
dfs_find_subchannels_for_center_freq(qdf_freq_t pri_center_freq,
				     qdf_freq_t sec_center_freq,
				     enum phy_ch_width ch_width,
				     qdf_freq_t *channels)
{
	uint8_t nchannels = 0;

	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		nchannels = 1;
		channels[0] = pri_center_freq;
		break;
	case CH_WIDTH_40MHZ:
		nchannels = 2;
		channels[0] = pri_center_freq - DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[1] = pri_center_freq + DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		break;
	case CH_WIDTH_80MHZ:
		nchannels = 4;
		channels[0] = pri_center_freq - DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		channels[1] = pri_center_freq - DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[2] = pri_center_freq + DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[3] = pri_center_freq + DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		break;
	case CH_WIDTH_80P80MHZ:
		nchannels = 8;
		channels[0] = pri_center_freq - DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		channels[1] = pri_center_freq - DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[2] = pri_center_freq + DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[3] = pri_center_freq + DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		/* secondary channels */
		channels[4] = sec_center_freq - DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		channels[5] = sec_center_freq - DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[6] = sec_center_freq + DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[7] = sec_center_freq + DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		break;
	case CH_WIDTH_160MHZ:
		nchannels = 8;
		channels[0] = pri_center_freq - DFS_5GHZ_4TH_CHAN_FREQ_OFFSET;
		channels[1] = pri_center_freq - DFS_5GHZ_3RD_CHAN_FREQ_OFFSET;
		channels[2] = pri_center_freq - DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		channels[3] = pri_center_freq - DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[4] = pri_center_freq + DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		channels[5] = pri_center_freq + DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		channels[6] = pri_center_freq + DFS_5GHZ_3RD_CHAN_FREQ_OFFSET;
		channels[7] = pri_center_freq + DFS_5GHZ_4TH_CHAN_FREQ_OFFSET;
		break;
	default:
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS, "invalid channel width");
		break;
	}

	return nchannels;
}

#ifdef QCA_SUPPORT_ADFS_RCAC
/**
 * dfs_unmark_rcac_done() - Unmark the CAC done channels from the RCAC list.
 * @dfs: Pointer to wlan_dfs object.
 */
static void dfs_unmark_rcac_done(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	qdf_freq_t channels[NUM_CHANNELS_160MHZ];
	uint8_t i, nchannels = 0;
	qdf_freq_t pri_ch_freq =
		dfs->dfs_rcac_param.rcac_ch_params.mhz_freq_seg0;
	qdf_freq_t sec_ch_freq =
		dfs->dfs_rcac_param.rcac_ch_params.mhz_freq_seg1;
	enum phy_ch_width ch_width =
		dfs->dfs_rcac_param.rcac_ch_params.ch_width;

	if (ch_width == CH_WIDTH_160MHZ) {
		pri_ch_freq = sec_ch_freq;
		sec_ch_freq = 0;
	}

	if (!pri_ch_freq)
		return;

	nchannels = dfs_find_subchannels_for_center_freq(pri_ch_freq,
							 sec_ch_freq,
							 ch_width,
							 channels);
	if (!nchannels)
		return;

	PRECAC_LIST_LOCK(dfs);
	if (TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		PRECAC_LIST_UNLOCK(dfs);
		return;
	}
	for (i = 0; i < nchannels; i++) {
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_precac_list,
				   pe_list,
				   tmp_precac_entry) {
			if (IS_WITHIN_RANGE(channels[i],
					    precac_entry->center_ch_freq,
					    VHT160_FREQ_OFFSET)) {
				dfs_unmark_tree_node_as_cac_done_for_freq
					(precac_entry, channels[i]);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
}
#else
static inline void dfs_unmark_rcac_done(struct wlan_dfs *dfs)
{
}
#endif

/*
 * dfs_mark_precac_done_for_freq() - Mark a frequency as preCAC done.
 * @dfs: Pointer to wlan_dfs.
 * @pri_ch_freq: Primary 80MHZ center frequency.
 * @sec_ch_freq: Secondary 80MHZ center frequency.
 * @ch_width: Channel width.
 */
void dfs_mark_precac_done_for_freq(struct wlan_dfs *dfs,
				   uint16_t pri_ch_freq,
				   uint16_t sec_ch_freq,
				   enum phy_ch_width ch_width)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	uint16_t channels[NUM_CHANNELS_160MHZ];
	uint8_t i, nchannels = 0;

	if (!pri_ch_freq)
		return;

	nchannels = dfs_find_subchannels_for_center_freq(pri_ch_freq,
							 sec_ch_freq,
							 ch_width,
							 channels);
	if (!nchannels)
		return;

	PRECAC_LIST_LOCK(dfs);
	if (TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		PRECAC_LIST_UNLOCK(dfs);
		return;
	}
	for (i = 0; i < nchannels; i++) {
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_precac_list,
				   pe_list,
				   tmp_precac_entry) {
			if (IS_WITHIN_RANGE_STRICT(channels[i],
					precac_entry->center_ch_freq,
					(precac_entry->bw/2))) {
				dfs_mark_tree_node_as_cac_done_for_freq
					(dfs, precac_entry, channels[i]);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
}

/* dfs_mark_tree_node_as_nol_for_freq() - Mark the preCAC BSTree node as NOL.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac_list entry pointer.
 * @freq:         IEEE channel freq to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_mark_tree_node_as_nol_for_freq(struct wlan_dfs *dfs,
				   struct dfs_precac_entry *pcac,
				   uint16_t freq)
{
	struct precac_tree_node *curr_node;

	if (!pcac->tree_root) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Precac tree root pointer is NULL!");
		return;
	}
	curr_node = pcac->tree_root;
	while (curr_node) {
		if (curr_node->n_nol_subchs <
		    N_SUBCHS_FOR_BANDWIDTH(curr_node->bandwidth)) {
			curr_node->n_nol_subchs++;
		} else {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Radarfound on an already marked NOL channel!");
			return;
		}
		if (freq == curr_node->ch_freq) {
			if (curr_node->n_caced_subchs) {
				/* remove cac done status for this node
				 * and it's parents, since this node
				 * now requires cac (after NOL expiry)
				 */
				dfs_unmark_tree_node_as_cac_done_for_freq(pcac,
									  freq);
			}
		}
		curr_node = dfs_descend_precac_tree_for_freq(curr_node,
							     freq);
	}
}

/* dfs_unmark_tree_node_as_nol_for_freq() - Unmark the preCAC BSTree node as
 * NOL.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac_list entry pointer.
 * @chan_freq      IEEE channel freq to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */

static void
dfs_unmark_tree_node_as_nol_for_freq(struct wlan_dfs *dfs,
				     struct dfs_precac_entry *precac_entry,
				     uint16_t chan_freq)
{
	struct precac_tree_node *curr_node;

	if (!precac_entry->tree_root) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Precac tree root pointer is NULL!");
		return;
	}
	curr_node = precac_entry->tree_root;
	while (curr_node) {
		if (curr_node->n_nol_subchs)
			curr_node->n_nol_subchs--;
		else
			return;
		curr_node = dfs_descend_precac_tree_for_freq(curr_node,
							     chan_freq);
	}
}

/*
 * dfs_unmark_precac_nol_for_freq() - Unmark a channel frequency as NOL.
 * @dfs: Pointer to wlan_dfs.
 * @chan_freq: Channel frequency in MHZ.
 */
void dfs_unmark_precac_nol_for_freq(struct wlan_dfs *dfs, uint16_t chan_freq)
{
	struct dfs_precac_entry *pcac_entry = NULL,
				*tmp_precac_entry = NULL;
	uint16_t pri_ch_freq = 0, chwidth_80 = DFS_CHWIDTH_80_VAL;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		TAILQ_FOREACH_SAFE(pcac_entry, &dfs->dfs_precac_list,
				   pe_list, tmp_precac_entry) {
			if (IS_WITHIN_RANGE_STRICT(chan_freq,
					pcac_entry->center_ch_freq,
					(pcac_entry->bw/2))) {
				dfs_unmark_tree_node_as_nol_for_freq(dfs,
								     pcac_entry,
								     chan_freq);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	/* If preCAC / agile CAC is not running, restart the timer
	 * to check if the NOL expired channels can be CACed again.
	 */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "NOL expired for chan_freq %u, trying to start preCAC",
		 chan_freq);
	if (!dfs->dfs_soc_obj->dfs_precac_timer_running) {
		if (dfs_is_legacy_precac_enabled(dfs)) {
			if (dfs_is_ap_cac_timer_running(dfs)) {
				dfs->dfs_defer_precac_channel_change = 1;
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					  "Primary CAC is running, deferred"
					  );
			} else if (WLAN_IS_CHAN_11AC_VHT80(dfs->dfs_curchan)) {
				pri_ch_freq = dfs->dfs_curchan->
						dfs_ch_mhz_freq_seg1;

				/* Check if there is a new channel to preCAC
				 * and only then do vdev restart.
				 */
				if (!dfs_get_ieeechan_for_precac_for_freq
				    (dfs, pri_ch_freq, 0, chwidth_80))
					return;
				dfs_mlme_channel_change_by_precac(
						dfs->dfs_pdev_obj);
			}
		}
	}
}

/*
 * dfs_mark_precac_nol_for_freq() - Mark a channel as preCAC NOL.
 * @dfs: Pointer to wlan_dfs.
 * @is_radar_found_on_secondary_seg: Flag to indicate second segment radar.
 * @detector_id: Detector ID.
 * @freq_list: frequency list.
 * @num_channels: Number of channels.
 */
void dfs_mark_precac_nol_for_freq(struct wlan_dfs *dfs,
				  uint8_t is_radar_found_on_secondary_seg,
				  uint8_t detector_id,
				  uint16_t *freq_lst,
				  uint8_t num_channels)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	struct wlan_objmgr_psoc *psoc;
	uint8_t i;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct wlan_objmgr_pdev *pdev;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "is_radar_found_on_secondary_seg = %u subchannel_marking = %u detector_id = %u",
		  is_radar_found_on_secondary_seg,
		  dfs->dfs_use_nol_subchannel_marking,
		  detector_id);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "agile detector freq = %u primary_freq = %u secondary_freq = %u",
		  dfs->dfs_agile_precac_freq_mhz,
		  dfs->dfs_precac_secondary_freq_mhz,
		  dfs->dfs_precac_primary_freq_mhz);

	/*
	 * Even if radar found on primary, we need to move
	 * the channel from precac-required-list and precac-done-list
	 * to precac-nol-list.
	 */
	PRECAC_LIST_LOCK(dfs);
	if (TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		PRECAC_LIST_UNLOCK(dfs);
		return;
	}
	for (i = 0; i < num_channels; i++) {
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_precac_list,
				   pe_list,
				   tmp_precac_entry) {
			if (IS_WITHIN_RANGE_STRICT(freq_lst[i],
					precac_entry->center_ch_freq,
					(precac_entry->bw/2))) {
				dfs_mark_tree_node_as_nol_for_freq(dfs,
								   precac_entry,
								   freq_lst[i]);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_soc_obj = dfs->dfs_soc_obj;

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	pdev = dfs->dfs_pdev_obj;
	/* PreCAC timer is not running, no need to restart preCAC */
	if (!dfs_soc_obj->dfs_precac_timer_running)
		return;

	if (dfs_is_legacy_precac_enabled(dfs)) {
		qdf_timer_sync_cancel(&dfs_soc_obj->dfs_precac_timer);
		dfs_soc_obj->dfs_precac_timer_running = 0;
		/*
		 * If radar is found on primary channel, no need to
		 * restart VAP's channels since channel change will happen
		 * after RANDOM channel selection anyway.
		 */
		if (is_radar_found_on_secondary_seg) {
			/*
			 * Change the channel
			 * case 1:-  No  VHT80 channel for precac is available
			 * so bring it back to VHT80.
			 * case 2:-  pick a new VHT80 channel for precac.
			 */
			if (dfs_is_ap_cac_timer_running(dfs)) {
				dfs->dfs_defer_precac_channel_change = 1;
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					  "Primary CAC is running, defer the channel change"
					  );
			} else {
				dfs_mlme_channel_change_by_precac(
						dfs->dfs_pdev_obj);
			}
		}
	}
}

#ifdef QCA_SUPPORT_AGILE_DFS
#ifdef CONFIG_CHAN_FREQ_API
/* Checks the Host side agile configurations. ie if agile channel
 * is configured as 5730MHz and the agile channel width is 80P80/165MHz.
 */
#define IS_HOST_AGILE_CURCHAN_165MHZ(_x) \
	((_x)->dfs_agile_precac_freq_mhz == \
	 RESTRICTED_80P80_CHAN_CENTER_FREQ) && \
	((_x)->dfs_precac_chwidth == CH_WIDTH_80P80MHZ)

/* Checks if the FW Agile operation was on the restricited 80P80MHz,
 * by comparing the center frequency 1 with 5690MHz, center frequency 2
 * with 5775MHz and the channel width was 80P80/165MHz.
 */
#define IS_OCAC_EVENT_ON_165_MHZ_CHAN(_x, _y, _z) \
	((_x) == RESTRICTED_80P80_LEFT_80_CENTER_FREQ) && \
	((_y) == RESTRICTED_80P80_RIGHT_80_CENTER_FREQ) && \
	((_z) == CH_WIDTH_80P80MHZ)

/*
 * dfs_is_ocac_complete_event_for_cur_agile_chan() - Check if the OCAC
 * completion event from FW is received for the currently configured agile
 * channel in host.
 *
 * @dfs: Pointer to dfs structure.
 * @center_freq_mhz1: Center frequency of the band when the precac width is
 * 20/40/80/160MHz and center frequency of the left 80MHz in case of restricted
 * 80P80/165MHz.
 * @center_freq_mhz2: Center frequency of the right 80MHz in case of restricted
 * 80P80/165MHz. It is zero for other channel widths.
 * @chwidth: Agile channel width for which the completion event is received.
 *
 * return: True if the channel on which OCAC completion event received is same
 * as currently configured agile channel in host. False otherwise.
 */
static bool
dfs_is_ocac_complete_event_for_cur_agile_chan(struct wlan_dfs *dfs,
					      uint32_t center_freq_mhz1,
					      uint32_t center_freq_mhz2,
					      enum phy_ch_width chwidth)
{
    if (IS_HOST_AGILE_CURCHAN_165MHZ(dfs) &&
	IS_OCAC_EVENT_ON_165_MHZ_CHAN(center_freq_mhz1,
				      center_freq_mhz2,
				      chwidth))
	return true;
    else if (dfs->dfs_agile_precac_freq_mhz == center_freq_mhz1)
	return true;
    else
	return false;
}

/*
 * dfs_process_ocac_complete() - Process OCAC Complete eventid.
 * @pdev: Pointer to wlan_objmgr_pdev.
 * @ocac_status: OCAC Status.
 * @center_freq_mhz1: Center frequency of the band when the precac width is
 * 20/40/80/160MHz and center frequency of the left 80MHz in case of restricted
 * 80P80/165MHz.
 * @center_freq_mhz2: Center frequency of the right 80MHz in case of restricted
 * 80P80/165MHz. It is zero for other channel widths.
 * @chwidth: Agile channel width for which the completion event is received.
 */
#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_process_ocac_complete(struct wlan_objmgr_pdev *pdev,
			       uint32_t ocac_status,
			       uint32_t center_freq_mhz1,
			       uint32_t center_freq_mhz2,
			       enum phy_ch_width chwidth)
{
	struct wlan_dfs *dfs = NULL;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	dfs_soc_obj = dfs->dfs_soc_obj;

	/* When the FW sends a delayed OCAC completion status, Host might
	 * have changed the precac channel already before an OCAC
	 * completion event is received. So the OCAC completion status
	 * should be validated if it is on the currently configured agile
	 * channel.
	 */

	/* Assume the previous agile channel was 64 (20Mhz) and current
	 * agile channel is 100(20Mhz), if the event from the FW is for
	 * previously configured agile channel 64(20Mhz) then Host ignores
	 * the event.
	 */
	if (!dfs_is_ocac_complete_event_for_cur_agile_chan(dfs,
							  center_freq_mhz1,
							  center_freq_mhz2,
							  chwidth)) {
	    dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
		    "OCAC completion event is received on a different channel %d %d that is not the current Agile channel %d",
		    center_freq_mhz1,
		    center_freq_mhz2,
		    dfs->dfs_agile_precac_freq_mhz);
	    return;
	}

	if (ocac_status == OCAC_RESET) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer reset, Sending Agile chan set command");
	} else if (ocac_status == OCAC_CANCEL) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer abort, agile precac stopped");
	} else if (ocac_status == OCAC_SUCCESS) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer Completed for agile freq: %d %d",
			  center_freq_mhz1,
			  center_freq_mhz2);
	} else {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS, "Error Unknown");
	}

	dfs_soc_obj->ocac_status = ocac_status;
	dfs_agile_sm_deliver_evt(dfs_soc_obj,
				 DFS_AGILE_SM_EV_AGILE_DONE,
				 0, (void *)dfs);
}
#endif
#endif
#endif

bool dfs_is_precac_timer_running(struct wlan_dfs *dfs)
{
	return dfs->dfs_soc_obj->dfs_precac_timer_running ? true : false;
}

/*
 * dfs_find_precac_secondary_vht80_chan() - Find preCAC secondary VHT80 channel.
 * @dfs: Pointer to wlan_dfs.
 * @chan: Pointer to dfs_channel.
 */
#define VHT80_FREQ_OFFSET 30
void dfs_find_precac_secondary_vht80_chan(struct wlan_dfs *dfs,
					  struct dfs_channel *chan)
{
	uint8_t first_primary_dfs_ch_freq;

	first_primary_dfs_ch_freq =
		dfs->dfs_precac_secondary_freq_mhz - VHT80_FREQ_OFFSET;

	dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					  first_primary_dfs_ch_freq, 0,
					  WLAN_PHYMODE_11AC_VHT80,
					  &chan->dfs_ch_freq,
					  &chan->dfs_ch_flags,
					  &chan->dfs_ch_flagext,
					  &chan->dfs_ch_ieee,
					  &chan->dfs_ch_vhtop_ch_freq_seg1,
					  &chan->dfs_ch_vhtop_ch_freq_seg2,
					  &chan->dfs_ch_mhz_freq_seg1,
					  &chan->dfs_ch_mhz_freq_seg2);
}

/*
 * dfs_precac_csa() - Intitiate CSA for preCAC channel switch.
 * @dfs: pointer to wlan_dfs.
 */
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
void dfs_precac_csa(struct wlan_dfs *dfs)
{
	/*
	 * Save current chan into intermediate chan, so that
	 * next time a DFS channel needs preCAC, there is no channel switch
	 * until preCAC finishes.
	 */
	dfs->dfs_precac_inter_chan_freq = dfs->dfs_autoswitch_chan->dfs_ch_freq;
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Use %d as intermediate channel for further channel changes",
		  dfs->dfs_precac_inter_chan_freq);

	if (global_dfs_to_mlme.mlme_precac_chan_change_csa_for_freq)
		global_dfs_to_mlme.mlme_precac_chan_change_csa_for_freq
			(dfs->dfs_pdev_obj,
			 dfs->dfs_autoswitch_chan->dfs_ch_freq,
			 dfs->dfs_autoswitch_des_mode);
	qdf_mem_free(dfs->dfs_autoswitch_chan);
	dfs->dfs_autoswitch_chan = NULL;
}
#endif

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
static bool dfs_precac_check_home_chan_change(struct wlan_dfs *dfs)
{
	struct dfs_channel *chan = dfs->dfs_autoswitch_chan;

	if (chan && dfs_is_precac_done(dfs, chan)) {
		dfs_precac_csa(dfs);
		dfs->dfs_soc_obj->precac_state_started = false;
		return true;
	}
	return false;
}
#else
static inline bool dfs_precac_check_home_chan_change(struct wlan_dfs *dfs)
{
	return false;
}
#endif

/* dfs_mark_adfs_chan_as_cac_done()- Mark the ADFS CAC completed channel as
 *                                   CAC done in the precac tree.
 * @dfs: Pointer to struct wlan_dfs.
 */
static void dfs_mark_adfs_chan_as_cac_done(struct wlan_dfs *dfs)
{
	qdf_freq_t pri_chan_freq, sec_chan_freq;
	enum phy_ch_width chan_width;

	if (dfs->dfs_agile_precac_freq_mhz ==
		RESTRICTED_80P80_CHAN_CENTER_FREQ) {
		pri_chan_freq = RESTRICTED_80P80_LEFT_80_CENTER_FREQ;
		sec_chan_freq = RESTRICTED_80P80_RIGHT_80_CENTER_FREQ;
		chan_width = CH_WIDTH_80P80MHZ;
	} else {
		pri_chan_freq = dfs->dfs_agile_precac_freq_mhz;
		sec_chan_freq = 0;
		chan_width =  dfs->dfs_precac_chwidth;
	}
	dfs_mark_precac_done_for_freq(dfs, pri_chan_freq, sec_chan_freq,
				      chan_width);
}

/**
 * dfs_precac_timeout() - Precac timeout.
 *
 * Removes the channel from precac_required list and adds it to the
 * precac_done_list. Triggers a precac channel change.
 */
static os_timer_func(dfs_precac_timeout)
{
	struct wlan_dfs *dfs = NULL;
	struct dfs_soc_priv_obj *dfs_soc_obj = NULL;
	uint32_t current_time;
	bool is_cac_done_on_des_chan = false;
	uint16_t dfs_pcac_cfreq2 = 0;

	OS_GET_TIMER_ARG(dfs_soc_obj, struct dfs_soc_priv_obj *);

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_agile_dfs_index].dfs;
	dfs_soc_obj->dfs_precac_timer_running = 0;

	if (dfs_is_legacy_precac_enabled(dfs)) {
		/*
		 * Remove the HT80 freq from the precac-required-list
		 * and add it to the precac-done-list
		 */
		dfs_pcac_cfreq2 = dfs->dfs_precac_secondary_freq_mhz;
		dfs_mark_precac_done_for_freq(dfs,
					      dfs_pcac_cfreq2, 0,
					      dfs->dfs_precac_chwidth);
		current_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Pre-cac expired, Precac Secondary chan %u curr time %d",
			  dfs_pcac_cfreq2,
			  (current_time) / 1000);
		/*
		 * Do vdev restart so that we can change
		 * the secondary VHT80 channel.
		 */

		/* check if CAC done on home channel */
		is_cac_done_on_des_chan =
			dfs_precac_check_home_chan_change(dfs);
		if (!is_cac_done_on_des_chan) {
			/*
			 * Use same home channel, only change preCAC channel.
			 */

			/*
			 * TO BE DONE xxx : Need to lock the channel change.
			 */
			dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
		}
	} else if (dfs_is_agile_precac_enabled(dfs)) {
	    current_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
	    dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		     "Pre-cac expired, Agile Precac chan %u curr time %d",
		     dfs->dfs_agile_precac_freq_mhz,
		     current_time / 1000);
	    dfs_agile_sm_deliver_evt(dfs_soc_obj,
				     DFS_AGILE_SM_EV_AGILE_DONE,
				     0, (void *)dfs);
	}
}

#if defined(ATH_SUPPORT_ZERO_CAC_DFS) && !defined(QCA_MCL_DFS_SUPPORT)
void dfs_zero_cac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	dfs_soc_obj->precac_state_started = false;
	qdf_timer_init(NULL, &dfs_soc_obj->dfs_precac_timer,
		       dfs_precac_timeout,
		       (void *)dfs_soc_obj,
		       QDF_TIMER_TYPE_WAKE_APPS);
}
#endif

void dfs_zero_cac_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_precac_timeout_override = -1;
	PRECAC_LIST_LOCK_CREATE(dfs);
	if (dfs_is_true_160mhz_supported(dfs))
		dfs->dfs_agile_detector_id = AGILE_DETECTOR_ID_TRUE_160MHZ;
	else
		dfs->dfs_agile_detector_id = AGILE_DETECTOR_ID_80P80;
}

/* dfs_init_precac_tree_node() - Initialise the preCAC BSTree node with the
 *                               provided values.
 * @node:      Precac_tree_node to be filled.
 * @freq:      IEEE channel freq value.
 * @bandwidth: Bandwidth of the channel.
 * @depth:     Depth of the tree. The depth of the tree when the root is 160MHz
 *             channel is 4, 80MHz is 3, 40MHz is 2 and 20MHz is 1.
 */
static inline void
dfs_init_precac_tree_node_for_freq(struct precac_tree_node *node,
				   uint16_t freq,
				   uint8_t bandwidth,
				   uint8_t depth)
{
	node->left_child = NULL;
	node->right_child = NULL;
	node->ch_freq = freq;
	node->ch_ieee = utils_dfs_freq_to_chan(freq);
	node->n_caced_subchs = 0;
	node->n_nol_subchs = 0;
	node->n_valid_subchs = N_SUBCHS_FOR_BANDWIDTH(bandwidth);
	node->bandwidth = bandwidth;
	node->depth = depth;

}

/* dfs_insert_node_into_bstree_for_freq() - Insert a new preCAC BSTree node.
 * @root:      The preCAC BSTree root pointer.
 * @chan:      IEEE freq of the new node.
 * @bandwidth: Bandwidth of the channel.
 * @depth:     Depth of the tree. The depth of the tree when the root is 160MHz
 *             channel is 4, 80MHz is 3, 40MHz is 2 and 20MHz is 1.
 *
 * Return: EOK if new node is allocated, else return ENOMEM.
 */
static QDF_STATUS
dfs_insert_node_into_bstree_for_freq(struct precac_tree_node **root,
				     uint16_t chan_freq,
				     uint8_t bandwidth,
				     uint8_t depth)
{
	struct precac_tree_node *new_node = NULL;
	struct precac_tree_node *curr_node, *prev_node = NULL;
	QDF_STATUS status = EOK;

	new_node = qdf_mem_malloc(sizeof(*new_node));
	if (!new_node)
		return -ENOMEM;
	dfs_init_precac_tree_node_for_freq(new_node,
					   chan_freq,
					   bandwidth,
					   depth);

	/* If root node is null, assign the newly allocated node
	 * to this node and return.
	 */
	if (!(*root)) {
		*root = new_node;
		return status;
	}

	curr_node = *root;
	/* Find the leaf node which will be the new node's parent */
	while (curr_node) {
		prev_node = curr_node;
		curr_node = dfs_descend_precac_tree_for_freq(curr_node,
							     chan_freq);
	}

	/* Add to the leaf node */
	if (chan_freq < prev_node->ch_freq)
		prev_node->left_child = new_node;
	else
		prev_node->right_child = new_node;

	return status;
}

/* dfs fill_precac_tree_for_entry() - Fill precac entry tree (level insertion).
 * @dfs:     WLAN DFS structure
 * @ch_ieee: root_node ieee channel.
 *
 * Since every node at a tree level is equally spaced (fixed BW for a level),
 * insertion of tree nodes are level order insertion.
 * For each depth starting from root depth (0),
 *       1. start from initial chan offset and fill node with ch_ieee
 *          provided and this offset.
 *       2. increment offset with next chan offset and fill node
 *       3. repeat step 2 till boundary offset is reached.
 *
 * If the above sequence is not maintained, the tree will not be balanced
 * as expected and would require rebalancing. Hence maintain the above
 * sequence for insertion.
 *
 */

#define N_OFFSETS 2
#define START_INDEX 0
#define STEP_INDEX 1


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

/* dfs_create_precac_tree_for_freq() - Fill precac entry tree (level insertion).
 * @dfs:       WLAN DFS structure
 * @ch_freq:   root_node freq.
 * @root:      Pointer to the node that will be filled and inserted as tree
 *             root.
 * @bandwidth: Bandwidth value of the root.
 */
static QDF_STATUS
dfs_create_precac_tree_for_freq(struct wlan_dfs *dfs,
				uint16_t ch_freq,
				struct precac_tree_node **root,
				int bandwidth)
{
	int chan_freq, i;
	QDF_STATUS status = EOK;
	struct precac_tree_offset_for_different_bw current_mode;
	uint8_t top_lvl_step;
	bool is_node_part_of_165_tree = false;

	if (ch_freq == RESTRICTED_80P80_LEFT_80_CENTER_FREQ ||
	    ch_freq == RESTRICTED_80P80_RIGHT_80_CENTER_FREQ)
		is_node_part_of_165_tree = true;

	switch (bandwidth) {
	case DFS_CHWIDTH_160_VAL:
			current_mode = offset160;
			break;
	case DFS_CHWIDTH_80_VAL:
			current_mode = offset80;
			break;
	case DFS_CHWIDTH_40_VAL:
			current_mode = offset40;
			break;
	case DFS_CHWIDTH_20_VAL:
			current_mode = offset20;
			break;
	default:
			current_mode = default_offset;
			break;
	}
	top_lvl_step = current_mode.initial_and_next_offsets[0][1];
	for (i = 0; i < current_mode.tree_depth; i++) {
		/* In offset array,
		 * column 0 is initial chan offset,
		 * column 1 is next chan offset.
		 * Boundary offset is initial offset and next offset
		 * of root level (since root level can have only 1 node)
		 */
		int offset =
		    current_mode.initial_and_next_offsets[i][START_INDEX];
		int step = current_mode.initial_and_next_offsets[i][STEP_INDEX];
		int boundary_offset = offset + top_lvl_step;
		uint8_t depth = is_node_part_of_165_tree ? i + 1 : i;

		for (; offset < boundary_offset; offset += step) {
			chan_freq = (int)ch_freq + offset;
			status =
			    dfs_insert_node_into_bstree_for_freq(root,
								 chan_freq,
								 bandwidth,
								 depth);
			if (status)
				return status;
		}
		bandwidth /= 2;
	}

	return status;
}

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

/* dfs_calculate_bw_for_same_pri_ch() - When the primary channel is a new
 * channel initialize the center channel frequency and bandwidth and when the
 * primary is same as previous channel update the center frequency and the
 * bandwith.
 * @dfs:               WLAN DFS structure
 * @dfs_max_bw_info:   Structure to store precac tree root channel's
 * information.
 * @index:             The index that is manipulated.
 * @ichan:             The DFS channel structure that holds the primary channel
 *                     number, center frquency and channel bandwidth.
 * @delimiter:         Band gap in MHz from the current primary channel to next
 *                     primary channel.
 */
static void
dfs_calculate_bw_for_same_pri_ch(struct wlan_dfs *dfs,
				 struct dfs_channel_bw *dfs_max_bw_info,
				 int index,
				 struct dfs_channel *ichan,
				 int *delimiter)
{
	uint8_t temp_bw = 0;
	uint16_t tmp_center_freq;

	dfs_max_bw_info[index].dfs_pri_ch_freq = ichan->dfs_ch_freq;
	tmp_center_freq = ichan->dfs_ch_mhz_freq_seg1;

	if (WLAN_IS_CHAN_MODE_20(ichan)) {
		temp_bw = DFS_CHWIDTH_20_VAL;
	} else if (WLAN_IS_CHAN_MODE_40(ichan)) {
		temp_bw = DFS_CHWIDTH_40_VAL;
	} else if (WLAN_IS_CHAN_MODE_80(ichan) ||
		   WLAN_IS_CHAN_MODE_80_80(ichan)) {
		temp_bw = DFS_CHWIDTH_80_VAL;
	if (dfs_is_restricted_80p80mhz_supported(dfs) &&
	    WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(ichan) &&
	    (ichan->dfs_ch_vhtop_ch_freq_seg1 ==
	     RESTRICTED_80P80_LEFT_80_CENTER_CHAN) &&
	    (ichan->dfs_ch_vhtop_ch_freq_seg2 ==
	     RESTRICTED_80P80_RIGHT_80_CENTER_CHAN)) {
		temp_bw = DFS_CHWIDTH_165_VAL;
		tmp_center_freq = RESTRICTED_80P80_CHAN_CENTER_FREQ;
		}
	} else if (WLAN_IS_CHAN_MODE_160(ichan)) {
		temp_bw = DFS_CHWIDTH_160_VAL;
		tmp_center_freq = ichan->dfs_ch_mhz_freq_seg2;
	}
	if (temp_bw > dfs_max_bw_info[index].dfs_max_bw) {
		dfs_max_bw_info[index].dfs_max_bw = temp_bw;
		*delimiter = dfs_max_bw_info[index].dfs_pri_ch_freq +
			     dfs_max_bw_info[index].dfs_max_bw;
		dfs_max_bw_info[index].dfs_center_ch_freq = tmp_center_freq;
	}
}

/* dfs_fill_max_bw_for_chan() - Finds unique precac tree node in the channel
 * list and stores the primary channel frequency, maximum bandwidth and the
 * center frequency. The algorithm is based on the data structure ic_channels
 * where the channels are organized as 36HT20, 36HT40, 36HT80,... and so on..
 * @dfs:               WLAN DFS structure
 * @dfs_max_bw_info:   Structure to store precac tree root channel's
 * information.
 * @num_precac_roots:  Number of unique.
 */
static void dfs_fill_max_bw_for_chan(struct wlan_dfs *dfs,
				     struct dfs_channel_bw *dfs_max_bw_info,
				     int *num_precac_roots)
{
	int i;
	int n_total_chans = 0;
	int  n_chanseg_found = 0;
	int prev_ch_freq = 0;
	int delimiter = 0;

	dfs_mlme_get_dfs_ch_nchans(dfs->dfs_pdev_obj, &n_total_chans);
	for (i = 0; i < n_total_chans; i++) {
		struct dfs_channel *ichan = NULL, lc;
		/* The array index of the bandwidth list that needs to be
		 * updated.
		 */
		int index_to_update;

		ichan = &lc;
		dfs_mlme_get_dfs_channels_for_freq
			(dfs->dfs_pdev_obj,
			 &ichan->dfs_ch_freq,
			 &ichan->dfs_ch_flags,
			 &ichan->dfs_ch_flagext,
			 &ichan->dfs_ch_ieee,
			 &ichan->dfs_ch_vhtop_ch_freq_seg1,
			 &ichan->dfs_ch_vhtop_ch_freq_seg2,
			 &ichan->dfs_ch_mhz_freq_seg1,
			 &ichan->dfs_ch_mhz_freq_seg2,
			 i);
		if (!WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(ichan))
			continue;
		if (ichan->dfs_ch_freq == prev_ch_freq) {
			/* When the primary channels are common for consecutive
			 * channels, for example 36HT20, 36HT40, 36HT80,...,
			 * only the center frequecy and the bandwidth have to be
			 * updated.
			 */
			index_to_update = n_chanseg_found - 1;
			dfs_calculate_bw_for_same_pri_ch(dfs,
							 dfs_max_bw_info,
							 index_to_update,
							 ichan,
							 &delimiter);
		} else if (ichan->dfs_ch_freq < delimiter) {
			continue;
		} else {
			prev_ch_freq = ichan->dfs_ch_freq;
			/* When the primary channels are unique and consecutive
			 * like 149HT20, 153HT20, 157HT20,..., the new element
			 * has to be initialized here.
			 */
			index_to_update = n_chanseg_found;
			dfs_calculate_bw_for_same_pri_ch(dfs,
							 dfs_max_bw_info,
							 n_chanseg_found,
							 ichan,
							 &delimiter);
			n_chanseg_found++;
		}
	}
	*num_precac_roots = n_chanseg_found;
	for (i = 0; i < *num_precac_roots; i++)
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "index = %d pri: %d centr: %d bw: %d",
			  i,
			  dfs_max_bw_info[i].dfs_pri_ch_freq,
			  dfs_max_bw_info[i].dfs_center_ch_freq,
			  dfs_max_bw_info[i].dfs_max_bw);
}

static QDF_STATUS
dfs_precac_create_precac_entry(struct wlan_dfs *dfs,
			       struct dfs_precac_entry *precac_entry,
			       struct dfs_channel_bw *dfs_max_bw_info,
			       int index)
{
	QDF_STATUS status;
	uint16_t precac_center_freq =
	    dfs_max_bw_info[index].dfs_center_ch_freq;

	precac_entry->center_ch_freq = precac_center_freq;
	precac_entry->center_ch_ieee =
	utils_dfs_freq_to_chan(precac_center_freq);
	precac_entry->bw = dfs_max_bw_info[index].dfs_max_bw;
	/* non_dfs_subch_count will be updated once the channels are marked. */
	precac_entry->non_dfs_subch_count = 0;
	precac_entry->dfs = dfs;
	status =
	    dfs_create_precac_tree_for_freq(dfs,
					    precac_entry->center_ch_freq,
					    &precac_entry->tree_root,
					    precac_entry->bw);
	if (status)
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "PreCAC entry for channel %d not created",
			  precac_entry->center_ch_ieee);
	else
	    TAILQ_INSERT_TAIL(&dfs->dfs_precac_list, precac_entry, pe_list);

	return status;
}

static QDF_STATUS
dfs_precac_create_165mhz_precac_entry(struct wlan_dfs *dfs,
				      struct dfs_precac_entry *precac_entry)
{
	QDF_STATUS status;

	precac_entry->center_ch_freq =
		RESTRICTED_80P80_CHAN_CENTER_FREQ;
	precac_entry->center_ch_ieee =
		utils_dfs_freq_to_chan(precac_entry->center_ch_freq);
	precac_entry->bw = DFS_CHWIDTH_160_VAL;
	/* non_dfs_subch_count will be updated once the channels are marked. */
	precac_entry->non_dfs_subch_count = 0;
	precac_entry->dfs = dfs;
	dfs_insert_node_into_bstree_for_freq(&precac_entry->tree_root,
					     RESTRICTED_80P80_CHAN_CENTER_FREQ,
					     DFS_CHWIDTH_160_VAL,
					     DEPTH_160_ROOT);
	status =
		dfs_create_precac_tree_for_freq
		(dfs,
					RESTRICTED_80P80_LEFT_80_CENTER_FREQ,
					&precac_entry->tree_root->left_child,
					DFS_CHWIDTH_80_VAL);
	if (!status)
		status =
		    dfs_create_precac_tree_for_freq(
			    dfs,
			    RESTRICTED_80P80_RIGHT_80_CENTER_FREQ,
			    &precac_entry->tree_root->right_child,
			    DFS_CHWIDTH_80_VAL);
	TAILQ_INSERT_TAIL(
			&dfs->dfs_precac_list,
			precac_entry, pe_list);
	return status;
}

/**
 * dfs_update_non_dfs_subchannel_count() - API to update the preCAC entry
 * with the given non DFS subchannel count.
 * @dfs:       Pointer to DFS object.
 * @frequency: Frequency whose corresponding preCAC entry needs to be updated.
 * @count:     Non DFS subchannel count for the preCAC entry.
 */
static void
dfs_update_non_dfs_subchannel_count(struct wlan_dfs *dfs,
				    qdf_freq_t frequency,
				    uint8_t count)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;

	PRECAC_LIST_LOCK(dfs);
	TAILQ_FOREACH_SAFE(precac_entry,
			   &dfs->dfs_precac_list,
			   pe_list,
			   tmp_precac_entry) {
		if (IS_WITHIN_RANGE_STRICT(frequency,
					   precac_entry->center_ch_freq,
					   (precac_entry->bw/2))) {
			precac_entry->non_dfs_subch_count = count;
			break;
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
}

static void
dfs_mark_non_dfs_as_precac_done(struct wlan_dfs *dfs,
				uint16_t dfs_pri_ch_freq,
				enum wlan_phymode mode)
{
	struct dfs_channel *ichan, lc;

	ichan = &lc;
	dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					  dfs_pri_ch_freq,
					  0,
					  mode,
					  &ichan->dfs_ch_freq,
					  &ichan->dfs_ch_flags,
					  &ichan->dfs_ch_flagext,
					  &ichan->dfs_ch_ieee,
					  &ichan->dfs_ch_vhtop_ch_freq_seg1,
					  &ichan->dfs_ch_vhtop_ch_freq_seg2,
					  &ichan->dfs_ch_mhz_freq_seg1,
					  &ichan->dfs_ch_mhz_freq_seg2);
	if (!WLAN_IS_CHAN_DFS(ichan)) {
		PRECAC_LIST_UNLOCK(dfs);
		dfs_mark_precac_done_for_freq(dfs,
					      ichan->dfs_ch_mhz_freq_seg1,
					      0,
					      CH_WIDTH_80MHZ);
		dfs_update_non_dfs_subchannel_count(dfs,
						    ichan->dfs_ch_mhz_freq_seg1,
						    N_SUBCHANS_FOR_80BW);
		PRECAC_LIST_LOCK(dfs);
	} else if (!WLAN_IS_CHAN_DFS_CFREQ2(ichan)) {
		PRECAC_LIST_UNLOCK(dfs);
		dfs_mark_precac_done_for_freq(dfs,
					      ichan->dfs_ch_mhz_freq_seg2,
					      0,
					      CH_WIDTH_80MHZ);
		dfs_update_non_dfs_subchannel_count(dfs,
						    ichan->dfs_ch_mhz_freq_seg2,
						    N_SUBCHANS_FOR_80BW);
		PRECAC_LIST_LOCK(dfs);
	}
}

/*
 * dfs_init_precac_list() - Initialize preCAC lists.
 * @dfs: Pointer to wlan_dfs.
 */
void dfs_init_precac_list(struct wlan_dfs *dfs)
{
	u_int i;
	uint8_t found;
	struct dfs_precac_entry *tmp_precac_entry;
	int nchans = 0;
	QDF_STATUS status;
	struct dfs_channel_bw *dfs_max_bw_info;
	int num_precac_roots;

	/*
	 * We need to prepare list of uniquee center frequencies of maximum
	 * possible bandwidths. But at the beginning we do not know how many
	 * unique frequencies are present. Therefore, we calculate the MAX size
	 * and allocate a temporary list/array. However we fill the temporary
	 * array with unique frequencies and copy the unique list of frequencies
	 * to the final list with exact size.
	 */
	dfs_mlme_get_dfs_ch_nchans(dfs->dfs_pdev_obj, &nchans);
	dfs_max_bw_info = qdf_mem_malloc(nchans *
		sizeof(struct dfs_channel_bw));
	if (!dfs_max_bw_info) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"memory allocation failed");
		return;
	}
	dfs_fill_max_bw_for_chan(dfs, dfs_max_bw_info, &num_precac_roots);

	TAILQ_INIT(&dfs->dfs_precac_list);

	PRECAC_LIST_LOCK(dfs);
	for (i = 0; i < num_precac_roots; i++) {
		uint16_t pri_chan_cfreq = dfs_max_bw_info[i].dfs_center_ch_freq;

		found = 0;
		TAILQ_FOREACH(tmp_precac_entry,
			      &dfs->dfs_precac_list,
			      pe_list) {
			if (tmp_precac_entry->center_ch_freq ==
					pri_chan_cfreq) {
				found = 1;
				break;
			}
		}
		if (!found && pri_chan_cfreq) {
			struct dfs_precac_entry *precac_entry;

			precac_entry =
				qdf_mem_malloc(sizeof(*precac_entry));
			if (!precac_entry) {
				dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"entry alloc fail for : %d", i);
				continue;
			}
			if (dfs_max_bw_info[i].dfs_max_bw ==
				DFS_CHWIDTH_165_VAL) {
				status = dfs_precac_create_165mhz_precac_entry(
						dfs,
						precac_entry);
				if (status) {
					dfs_debug(dfs,
						  WLAN_DEBUG_DFS,
						  "PreCAC entry for channel 146 not created");
					continue;
				}
			    /* The restricted 80p80 or the 165MHz channel might
			     * have a non DFS part with center frequency 5775.
			     * Mark the non DFS portion as precac done.
			     */
			    dfs_mark_non_dfs_as_precac_done(
				    dfs,
				    dfs_max_bw_info[i].dfs_pri_ch_freq,
				    WLAN_PHYMODE_11AC_VHT80_80);
			} else {
			    status =
				dfs_precac_create_precac_entry(dfs,
							       precac_entry,
							       dfs_max_bw_info,
							       i);
			if (status)
				continue;
			/* Some channels like 36HT160 might have a non DFS
			 * part. Mark the non DFS portion as precac done.
			 */
			dfs_mark_non_dfs_as_precac_done(
				dfs,
				dfs_max_bw_info[i].dfs_pri_ch_freq,
				WLAN_PHYMODE_11AC_VHT160);
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
	qdf_mem_free(dfs_max_bw_info);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Print the list of PreCAC ieee chan from linked list");
	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_precac_list,
		      pe_list) {
	    uint8_t ch_ieee, bw;

	    ch_ieee = utils_dfs_freq_to_chan(tmp_precac_entry->center_ch_freq);
	    bw = tmp_precac_entry->bw;
	    dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "ieee=%u bw=%u", ch_ieee, bw);
	}
}

/* dfs_find_leftmost_leaf_of_precac_tree() - Find the leftmost leaf node of
 *                                           BSTree rooted by the given node.
 * @node: PreCAC BSTree node whose leftmost leaf is required.
 *
 * Return: Pointer of struct precac_tree_node.
 */
static inline struct precac_tree_node *
dfs_find_leftmost_leaf_of_precac_tree(struct precac_tree_node *node)
{
	if (!node)
		return NULL;

	while (node->left_child)
		node = node->left_child;

	return node;
}

/*
 * dfs_free_precac_tree_nodes() - Free the tree nodes starting from
 *                                the root node.
 *                                NOTE: This changes tree structure, hence
 *                                caller should be in a lock.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac list entry whose BSTree is to be freed.
 *
 * Consider the below Binary tree,
 *
 *                         A
 *                        / \
 *                       B   C
 *                        \
 *                         D
 *
 * Steps for freeing this tree,
 *
 * 1. Find the leftmost leaf node of the Binary Tree.
 * 2. Set current node as root node.
 * 3. If current node has right child, add right child of current node as left
 *    child of leftmost leaf.
 * 4. Update the leftmost leaf.
 * 5. Update current node to left child and remove the node.
 * 6. Repeat steps 3 to 5 till current node is NULL.
 *
 * The above Binary Tree structure during the afore mentioned steps:
 *
 *    A            A
 *   / \          /
 *  B   C  -->    B     -->    B     -->    B   -->   C   -->  D   -->  .
 *   \           / \          / \          /         /
 *    D         C   D        C   D        C         D
 *                                       /
 *                                      D
 *
 */

static void dfs_free_precac_tree_nodes(struct wlan_dfs *dfs,
				       struct dfs_precac_entry *precac_entry)
{
	struct precac_tree_node *root_node, *left_most_leaf, *prev_root_node;

	root_node = precac_entry->tree_root;
	if (!root_node) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "tree root is null");
		return;
	}

	/* Find leftmost leaf node */
	left_most_leaf = root_node;
	left_most_leaf = dfs_find_leftmost_leaf_of_precac_tree(left_most_leaf);
	if (!left_most_leaf) {
		/* should've been caught in previous check, assert here */
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Could not find leaf, deletion failed! Asserting");
		QDF_ASSERT(0);
		return;
	}

	while (root_node) {
		if (root_node->right_child) {
			/* Add the right subtree as the left child of the
			 * leftmost leaf
			 */
			left_most_leaf->left_child = root_node->right_child;
			/* Update left most leaf */
			left_most_leaf = dfs_find_leftmost_leaf_of_precac_tree(
						left_most_leaf);
			if (!left_most_leaf) {
				dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"Could not find leaf, deletion failed");
				QDF_ASSERT(0);
				return;
			}
		}
		/* Free current node */
		prev_root_node = root_node;
		root_node = root_node->left_child;
		qdf_mem_free(prev_root_node);
	}
}

void dfs_deinit_precac_list(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *tmp_precac_entry, *precac_entry;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Free the list of VHT80 frequencies from linked list");
	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_precac_list,
				   pe_list, tmp_precac_entry) {
			dfs_free_precac_tree_nodes(dfs, precac_entry);
			TAILQ_REMOVE(&dfs->dfs_precac_list,
				     precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}
	PRECAC_LIST_UNLOCK(dfs);

}

#if defined(QCA_SUPPORT_AGILE_DFS) || defined(ATH_SUPPORT_ZERO_CAC_DFS)
void dfs_agile_soc_obj_init(struct wlan_dfs *dfs,
			    struct wlan_objmgr_psoc *psoc)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs->dfs_psoc_idx = dfs_soc_obj->num_dfs_privs;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "dfs->dfs_psoc_idx: %d ", dfs->dfs_psoc_idx);
	dfs_soc_obj->dfs_priv[dfs_soc_obj->num_dfs_privs].dfs = dfs;
	dfs_soc_obj->num_dfs_privs++;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_soc_obj->num_dfs_privs: %d ",
		 dfs_soc_obj->num_dfs_privs);
}
#endif

void dfs_zero_cac_detach(struct wlan_dfs *dfs)
{
	dfs_deinit_precac_list(dfs);
	PRECAC_LIST_LOCK_DESTROY(dfs);
}

/**
 * dfs_is_pcac_required_for_freq() - Find if given frequency is preCAC required.
 * @node: Pointer to the preCAC tree Node in which the frequency is present.
 * @freq: Frequency to be checked.
 *
 * Return: False if the frequency is not fully CAC done or in NOL, else true.
 */
static bool
dfs_is_pcac_required_for_freq(struct precac_tree_node *node, uint16_t freq)
{
	while (node) {
		if (node->ch_freq == freq) {
			if ((node->n_caced_subchs ==
			     N_SUBCHS_FOR_BANDWIDTH(node->bandwidth)) ||
			     (node->n_nol_subchs))
				return false;
			else
				return true;
		}
		node = dfs_descend_precac_tree_for_freq(node, freq);
	}
	return false;
}

#define DFS_160MHZ_SECSEG_CHAN_FREQ_OFFSET 40
/**
 * dfs_get_num_cur_subchans_in_node_freq() - Get number of excluded channels
 *                                           inside the current node.
 * @dfs:  Pointer to wlan_dfs structure.
 * @node: Node to be checked.
 *
 * Return: uint8_t.
 * Return the number of excluded (current operating channels in CAC) that are in
 * the given tree node range.
 */
static uint8_t
dfs_get_num_cur_subchans_in_node_freq(struct wlan_dfs *dfs,
				      struct precac_tree_node *node)
{
	uint16_t exclude_pri_ch_freq, exclude_sec_ch_freq;
	uint8_t chwidth_val = DFS_CHWIDTH_80_VAL;
	uint8_t n_exclude_subchs = 0;

	exclude_pri_ch_freq =
		dfs->dfs_curchan->dfs_ch_mhz_freq_seg1;
	exclude_sec_ch_freq =
		dfs->dfs_curchan->dfs_ch_mhz_freq_seg2;
	if (WLAN_IS_CHAN_MODE_160(dfs->dfs_curchan)) {
		if (exclude_sec_ch_freq < exclude_pri_ch_freq)
			exclude_sec_ch_freq -=
				DFS_160MHZ_SECSEG_CHAN_OFFSET;
		else
			exclude_sec_ch_freq +=
				DFS_160MHZ_SECSEG_CHAN_OFFSET;
	}

	if (WLAN_IS_CHAN_MODE_20(dfs->dfs_curchan))
		chwidth_val = DFS_CHWIDTH_20_VAL;
	else if (WLAN_IS_CHAN_MODE_40(dfs->dfs_curchan))
		chwidth_val = DFS_CHWIDTH_40_VAL;

	/* Check if the channel is a subset of the tree node and if it's
	 * currently in CAC period. This is to avoid excluding channels twice,
	 * one below and one in the already CACed channels exclusion (in the
	 * caller API). */
	if (IS_WITHIN_RANGE(exclude_pri_ch_freq,
			   node->ch_freq,
			   (node->bandwidth / 2)) &&
	   dfs_is_pcac_required_for_freq(node, exclude_pri_ch_freq))
		n_exclude_subchs += N_SUBCHS_FOR_BANDWIDTH(chwidth_val);
	if (IS_WITHIN_RANGE(exclude_sec_ch_freq,
			   node->ch_freq,
			   (node->bandwidth / 2)) &&
	   dfs_is_pcac_required_for_freq(node, exclude_sec_ch_freq))
		n_exclude_subchs += N_SUBCHS_FOR_BANDWIDTH(chwidth_val);
	return n_exclude_subchs;
}

/* dfs_is_cac_needed_for_bst_node_for_freq() - For a requested bandwidth, find
 *                                             if the current preCAC BSTree
 *                                             node needs CAC.
 * @dfs:           Pointer to wlan_dfs struct.
 * @node:          Node to be checked.
 * @req_bandwidth: bandwidth of channel requested.
 *
 * Return: TRUE/FALSE.
 * Return true if there exists a channel of the requested bandwidth
 * for the node which is not CAC done, else false.
 */
static bool
dfs_is_cac_needed_for_bst_node_for_freq(struct wlan_dfs *dfs,
					struct precac_tree_node *node,
					uint8_t req_bandwidth)
{
	uint8_t n_subchs_for_req_bw, n_allowed_subchs, n_excluded_subchs;

	if (!node)
		return false;

	/* Find the number of subchannels for the requested bandwidth */
	n_excluded_subchs = dfs_get_num_cur_subchans_in_node_freq(dfs, node);
	n_subchs_for_req_bw = N_SUBCHS_FOR_BANDWIDTH(req_bandwidth);
	n_allowed_subchs = node->n_valid_subchs -
			(node->n_nol_subchs + n_excluded_subchs);

	/* Return false if,
	 * 1. Number of allowed subchannels (all subchannels other than
	 *    current operating sub-channels and NOL sub-channels) in the
	 *    current node is less than the requested number of subchannels.
	 * 3. If the number CAC done subchannels + NOL subchannels + current
	 *    operating subchannels in the current node is equal to number of
	 *    valid subchannels in the node.
	 * else, return true.
	 */
	if ((n_allowed_subchs < n_subchs_for_req_bw) ||
	    ((node->n_caced_subchs + node->n_nol_subchs + n_excluded_subchs) ==
	     node->n_valid_subchs))
		return false;

	return true;
}

/* dfs_find_ieee_ch_from_precac_tree_for_freq() - from the given preCAC tree,
 *                                       find a IEEE freq of the given bandwidth
 *                                       which is valid and needs CAC.
 * @root:   PreCAC BSTree root pointer.
 * @req_bw: Bandwidth of channel requested.
 *
 * Return: IEEE channel frequency.
 * Return a valid freq value which needs CAC for the given bandwidth, else
 * return 0.
 */
static uint16_t
dfs_find_ieee_ch_from_precac_tree_for_freq(struct wlan_dfs *dfs,
					   struct precac_tree_node *root,
					   uint8_t req_bw)
{
	struct precac_tree_node *curr_node;

	if (!dfs_is_cac_needed_for_bst_node_for_freq(dfs, root, req_bw))
		return 0;

	curr_node = root;
	while (curr_node) {
		if (curr_node->bandwidth == req_bw) {
			/* find if current node in valid state (req.) */
			if (dfs_is_cac_needed_for_bst_node_for_freq(dfs,
								    curr_node,
								    req_bw))
				return curr_node->ch_freq;
			else
				return 0;
		}

		/* Find if we need to go to left or right subtree.
		 * Note: If both are available, go to left.
		 */
		if (!dfs_is_cac_needed_for_bst_node_for_freq(
				dfs,
				curr_node->left_child,
				req_bw))
			curr_node = curr_node->right_child;
		else
			curr_node = curr_node->left_child;
	}
	/* If requested bandwidth is invalid, return 0 here */
	return 0;
}

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
#ifdef CONFIG_CHAN_FREQ_API
/**
 * dfs_find_precac_state_of_node() - Find the preCAC state of the given channel.
 * @channel: Channel whose preCAC state is to be found.
 * @precac_entry: PreCAC entry where the channel exists.
 *
 * Return, enum value of type precac_chan_state.
 */
static enum precac_chan_state
dfs_find_precac_state_of_node(qdf_freq_t channel,
			      struct dfs_precac_entry *precac_entry)
{
	struct precac_tree_node *node = precac_entry->tree_root;

	while (node) {
		if (node->ch_freq == channel) {
			if (node->n_nol_subchs)
				return PRECAC_NOL;
			if (node->n_caced_subchs ==
			    N_SUBCHS_FOR_BANDWIDTH(node->bandwidth))
				return PRECAC_DONE;
			return PRECAC_REQUIRED;
		}
		node = dfs_descend_precac_tree_for_freq(node, channel);
	}
	return PRECAC_ERR;
}

/**
 * dfs_configure_deschan_for_precac() - API to prioritize user configured
 * channel for preCAC.
 *
 * @dfs: Pointer to DFS of wlan_dfs structure.
 * Return: frequency of type qdf_freq_t if configured, else 0.
 */
static qdf_freq_t
dfs_configure_deschan_for_precac(struct wlan_dfs *dfs)
{
	struct dfs_channel *deschan = dfs->dfs_autoswitch_chan;
	qdf_freq_t channels[2];
	uint8_t i, nchannels;
	struct dfs_precac_entry *tmp_precac_entry;
	enum precac_chan_state precac_state = PRECAC_ERR;

	if (!deschan)
		return 0;

	if (dfs->dfs_precac_chwidth == CH_WIDTH_160MHZ &&
	    WLAN_IS_CHAN_MODE_160(dfs->dfs_autoswitch_chan)) {
		channels[0] = deschan->dfs_ch_mhz_freq_seg2;
		channels[1] = 0;
		nchannels = 1;
	} else {
		/* The InterCAC feature is enabled only for 80MHz or 160MHz.
		 * Hence split preferred channel into two 80MHz channels.
		 */
		channels[0] = deschan->dfs_ch_mhz_freq_seg1;
		channels[1] = deschan->dfs_ch_mhz_freq_seg2;
		if (WLAN_IS_CHAN_MODE_160(dfs->dfs_autoswitch_chan)) {
			if (deschan->dfs_ch_freq >
			    deschan->dfs_ch_mhz_freq_seg2)
				channels[1] -= DFS_160MHZ_SECSEG_CHAN_OFFSET;
			else
				channels[1] += DFS_160MHZ_SECSEG_CHAN_OFFSET;
		}
		nchannels = 2;
	}

	for (i = 0; i < nchannels; i++) {
		PRECAC_LIST_LOCK(dfs);
		TAILQ_FOREACH(tmp_precac_entry,
			      &dfs->dfs_precac_list,
			      pe_list) {
			if (IS_WITHIN_RANGE(channels[i],
					    tmp_precac_entry->center_ch_freq,
					    tmp_precac_entry->bw)) {
				precac_state = dfs_find_precac_state_of_node(
						channels[i],
						tmp_precac_entry);
				if (precac_state == PRECAC_REQUIRED) {
					PRECAC_LIST_UNLOCK(dfs);
					return channels[i];
				}
			}
		}
		PRECAC_LIST_UNLOCK(dfs);
	}

	return 0;
}
#endif
#else
static inline qdf_freq_t
dfs_configure_deschan_for_precac(struct wlan_dfs *dfs)
{
	return 0;
}
#endif

/*
 * dfs_get_ieeechan_for_precac_for_freq() - Get chan frequency for preCAC.
 * @dfs: Pointer to wlan_dfs.
 * @exclude_pri_ch_freq: Primary frequency to be excluded.
 * @exclude_sec_ch_freq: Secondary freqeuncy to be excluded.
 * @bandwidth: Bandwidth.
 */
uint16_t dfs_get_ieeechan_for_precac_for_freq(struct wlan_dfs *dfs,
					      uint16_t exclude_pri_ch_freq,
					      uint16_t exclude_sec_ch_freq,
					      uint8_t bw)
{
	struct dfs_precac_entry *precac_entry;
	struct precac_tree_node *root = NULL;
	uint16_t ieee_chan_freq = 0;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "current operating channel(s) to be excluded = [%u] [%u]",
		 exclude_pri_ch_freq,
		 exclude_sec_ch_freq);

	/* If interCAC is enabled, prioritize the desired channel first before
	 * using the normal logic to find a channel for preCAC. */
	ieee_chan_freq = dfs_configure_deschan_for_precac(dfs);

	if (ieee_chan_freq)
		goto exit;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		TAILQ_FOREACH(precac_entry, &dfs->dfs_precac_list,
			      pe_list) {
			root = precac_entry->tree_root;
			ieee_chan_freq =
				dfs_find_ieee_ch_from_precac_tree_for_freq(dfs,
									   root,
									   bw);
			if (ieee_chan_freq)
				break;
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

exit:
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "Channel picked for preCAC = %u",
		 ieee_chan_freq);

	return ieee_chan_freq;
}

void dfs_cancel_precac_timer(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	qdf_timer_sync_cancel(&dfs_soc_obj->dfs_precac_timer);
	dfs_soc_obj->dfs_precac_timer_running = 0;
}

#ifdef QCA_SUPPORT_AGILE_DFS
/* FIND_IF_OVERLAP_WITH_WEATHER_RANGE() - Find if the given channel range
 * overlaps with the weather channel range.
 * @first_ch: First subchannel of the channel range.
 * @last_ch:  Last subchannel of the channel range.
 *
 * Algorithm:
 * If the first channel of given range is left of last weather channel
 * and if the last channel of given range is right of the first weather channel,
 * return true, else false.
 */
#define FIND_IF_OVERLAP_WITH_WEATHER_FREQ_RANGE(first_ch_freq, last_ch_freq) \
((first_ch_freq <= WEATHER_CHAN_END_FREQ) && (last_ch_freq >= \
					      WEATHER_CHAN_START_FREQ))

/* dfs_is_pcac_on_weather_channel_for_freq() - Given a channel number, find if
 * it's a weather radar channel.
 * @dfs: Pointer to WLAN_DFS structure.
 * @chwidth: PreCAC channel width enum.
 * @precac_freq: preCAC freq.
 *
 * Based on the precac_width, find the first and last subchannels of the given
 * preCAC channel and check if this range overlaps with weather channel range.
 *
 * Return: True if weather channel, else false.
 */
static bool dfs_is_pcac_on_weather_channel_for_freq(struct wlan_dfs *dfs,
						    enum phy_ch_width chwidth,
						    uint16_t precac_freq)
{
	uint16_t first_subch, last_subch;

	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		first_subch = precac_freq;
		last_subch = precac_freq;
		break;
	case CH_WIDTH_40MHZ:
		first_subch = precac_freq - DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		last_subch = precac_freq + DFS_5GHZ_NEXT_CHAN_FREQ_OFFSET;
		break;
	case CH_WIDTH_80MHZ:
		first_subch = precac_freq - DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		last_subch = precac_freq + DFS_5GHZ_2ND_CHAN_FREQ_OFFSET;
		break;
	case CH_WIDTH_160MHZ:
		first_subch = precac_freq - DFS_5GHZ_4TH_CHAN_FREQ_OFFSET;
		last_subch = precac_freq + DFS_5GHZ_4TH_CHAN_FREQ_OFFSET;
		break;
	case CH_WIDTH_80P80MHZ:
		/* The restricted 80P80MHz channel or the 165MHz channel
		 * does not include any of the weather radar channels.
		 * Even though other 80P80 channels might include the weather
		 * radar channels, it is not currently possible for Agile
		 * detector to operate in a 80P80MHz channel except in the
		 * restricted 80P80MHz channel.
		 */
		return false;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Precac channel width invalid!");
		return false;
	}
	return FIND_IF_OVERLAP_WITH_WEATHER_FREQ_RANGE(first_subch, last_subch);
}

/*
 * dfs_start_agile_precac_timer() - Start Agile preCAC timer.
 * @dfs: pointer to wlan_dfs.
 * @ocac_status: OCAC Status.
 * @adfs_param: Pointer to ADFS params.
 */
#define EXTRA_TIME_IN_MS 2000
void dfs_start_agile_precac_timer(struct wlan_dfs *dfs,
				  uint8_t ocac_status,
				  struct dfs_agile_cac_params *adfs_param)
{
	uint16_t pcacfreq = adfs_param->precac_center_freq_1;
	enum phy_ch_width chwidth = adfs_param->precac_chwidth;
	uint32_t min_precac_timeout, max_precac_timeout;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs_soc_obj->dfs_precac_timer_running = 1;

	/* Find the minimum and maximum precac timeout. */
	max_precac_timeout = MAX_PRECAC_DURATION;
	if (dfs->dfs_precac_timeout_override != -1) {
		min_precac_timeout =
			dfs->dfs_precac_timeout_override * 1000;
	} else if (dfs_is_pcac_on_weather_channel_for_freq(dfs,
							   chwidth,
							   pcacfreq)) {
		min_precac_timeout = MIN_WEATHER_PRECAC_DURATION;
		max_precac_timeout = MAX_WEATHER_PRECAC_DURATION;
	} else {
		min_precac_timeout = MIN_PRECAC_DURATION;
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "precactimeout = %d ms", (min_precac_timeout));
	/* Add the preCAC timeout in the params to be sent to FW. */
	adfs_param->min_precac_timeout = min_precac_timeout;
	adfs_param->max_precac_timeout = max_precac_timeout;
	/* For preCAC, in which the FW has to run a timer of a finite amount of
	 * time, set the mode to QUICK_OCAC_MODE.
	 */
	adfs_param->ocac_mode = QUICK_OCAC_MODE;
	/* Increase the preCAC timeout in HOST by 2 seconds to avoid
	 * FW OCAC completion event and HOST timer firing at same time. */
	if (min_precac_timeout)
		min_precac_timeout += EXTRA_TIME_IN_MS;
	qdf_timer_mod(&dfs_soc_obj->dfs_precac_timer, min_precac_timeout);
}
#endif

/*
 * dfs_start_precac_timer_for_freq() - Start preCAC timer.
 * @dfs: pointer to wlan_dfs.
 * @precac_chan_freq: PreCAC channel frequency
 */
void dfs_start_precac_timer_for_freq(struct wlan_dfs *dfs,
				     uint16_t precac_chan_freq)
{
	struct dfs_channel *ichan, lc, *curchan;
	uint16_t first_primary_dfs_ch_freq;
	int primary_cac_timeout;
	int secondary_cac_timeout;
	int precac_timeout;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs_soc_obj->cur_agile_dfs_index = dfs->dfs_psoc_idx;
	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_agile_dfs_index].dfs;
#define EXTRA_TIME_IN_SEC 5
	dfs_soc_obj->dfs_precac_timer_running = 1;

	/*
	 * Get the first primary ieee chan in the HT80 band and find the channel
	 * pointer.
	 */
	curchan = dfs->dfs_curchan;
	first_primary_dfs_ch_freq = precac_chan_freq - VHT80_FREQ_OFFSET;

	primary_cac_timeout =
	    dfs_mlme_get_cac_timeout_for_freq(dfs->dfs_pdev_obj,
					      curchan->dfs_ch_freq,
					      curchan->dfs_ch_mhz_freq_seg2,
					      curchan->dfs_ch_flags);

	ichan = &lc;
	dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					  first_primary_dfs_ch_freq, 0,
					  WLAN_PHYMODE_11AC_VHT80,
					  &ichan->dfs_ch_freq,
					  &ichan->dfs_ch_flags,
					  &ichan->dfs_ch_flagext,
					  &ichan->dfs_ch_ieee,
					  &ichan->dfs_ch_vhtop_ch_freq_seg1,
					  &ichan->dfs_ch_vhtop_ch_freq_seg2,
					  &ichan->dfs_ch_mhz_freq_seg1,
					  &ichan->dfs_ch_mhz_freq_seg2);

	secondary_cac_timeout = (dfs->dfs_precac_timeout_override != -1) ?
		dfs->dfs_precac_timeout_override :
		dfs_mlme_get_cac_timeout_for_freq(dfs->dfs_pdev_obj,
						  ichan->dfs_ch_freq,
						  ichan->dfs_ch_mhz_freq_seg2,
						  ichan->dfs_ch_flags);

	/*
	 * EXTRA time is needed so that if CAC and PreCAC is running
	 * simultaneously, PreCAC expiry function may be called before CAC
	 * expiry and PreCAC expiry does a channel change (vdev_restart) the
	 * restart response calls CAC_start function(ieee80211_dfs_cac_start)
	 * which cancels any previous CAC timer and starts a new CAC again.
	 * So CAC expiry does not happen and moreover a new CAC is started.
	 * Therefore do not disturb the CAC by channel restart (vdev_restart).
	 *
	 * If CAC/preCAC was already completed on primary, then we do not need
	 * to calculate which CAC timeout is maximum.
	 * For example: If primary's CAC is 600 seconds and secondary's CAC
	 * is 60 seconds then maximum gives 600 seconds which is not needed
	 * if CAC/preCAC was already completed on primary. It is to be noted
	 * that precac/cac is done on primary segment.
	 */
	if (WLAN_IS_CHAN_DFS(dfs->dfs_curchan) &&
	    !dfs_is_precac_done(dfs, dfs->dfs_curchan))
		precac_timeout = QDF_MAX(primary_cac_timeout,
					 secondary_cac_timeout) +
				 EXTRA_TIME_IN_SEC;
	else
		precac_timeout = secondary_cac_timeout + EXTRA_TIME_IN_SEC;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"precactimeout = %d", (precac_timeout)*1000);
	qdf_timer_mod(&dfs_soc_obj->dfs_precac_timer, (precac_timeout) * 1000);
}

/* dfs_print_node_data() - Print the precac tree node data.
 * @dfs:  Pointer to WLAN DFS structure.
 * @node: Precac tree node pointer.
 *
 * Sample print for below tree:
 *
 *      A                  A(C,N)
 *     / \                 |
 *    B   C                |------- B(C,N)
 *   / \                   |        |
 *  D   E                  |        |------- D(C,N)
 *                         |        |
 *                         |        |------- E(C,N)
 *                         |
 *                         |------- E(C,N)
 *
 * Where C is number of CACed subchannels, and N is number of NOL subchannels.
 * For each node, the prefix and previous line prefix to be printed will be
 * based on the level (and by our logic, bandwidth) of the current node.
 *
 */
#define MAX_PREFIX_CHAR 28
/*Retaining IEEE to print node data */
static void dfs_print_node_data(struct wlan_dfs *dfs,
				struct precac_tree_node *node)
{
	char prefix[MAX_PREFIX_CHAR] = "";
	char prev_line_prefix[MAX_PREFIX_CHAR] = "";
	char inv[4] = "inv";

	switch (node->depth) {
	case DEPTH_160_ROOT:
		break;
	case DEPTH_80_ROOT:
		qdf_str_lcopy(prev_line_prefix, "|", MAX_PREFIX_CHAR);
		qdf_str_lcopy(prefix, "|------- ", MAX_PREFIX_CHAR);
		break;
	case DEPTH_40_ROOT:
		qdf_str_lcopy(prev_line_prefix, "|        |", MAX_PREFIX_CHAR);
		qdf_str_lcopy(prefix, "|        |------- ", MAX_PREFIX_CHAR);
		break;
	case DEPTH_20_ROOT:
		qdf_str_lcopy(prev_line_prefix,
			      "|        |        |",
			      MAX_PREFIX_CHAR);
		qdf_str_lcopy(prefix,
			      "|        |        |------- ",
			      MAX_PREFIX_CHAR);
		break;
	default:
		return;
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "%s", prev_line_prefix);
	/* if current node is not a valid ic channel, print invalid */
	if (node->n_valid_subchs != N_SUBCHS_FOR_BANDWIDTH(node->bandwidth))
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "%s%s", prefix, inv);
	else
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "%s%u(%u,%u)",
			 prefix,
			 node->ch_ieee,
			 node->n_caced_subchs,
			 node->n_nol_subchs);
}

/* dfs_print_precac_tree_nodes() - print the precac tree nodes using
 *                                 preorder traversal. (Root-Left-Right)
 * @dfs:          WLAN DFS structure.
 * @precac_entry: A single entry in the precac list.
 *
 * Algorithm used - Morris preorder tree traversal (iterative).
 *
 * Description of Algorithm:
 * Consider the below tree, Preorder sequence (A, B, D, E, C)
 *
 *                         A
 *                        / \
 *                       B   C
 *                      / \
 *                     D   E
 *
 * What is inorder predecessor?
 *
 *    For a given node, the inorder predecessor of the node is
 *    the rightmost node of the left subtree.
 *    For e.g.: In our tree, E is the predecessor of A,
 *    D is the predecessor of B.
 *
 * Steps for Algorithm:
 * Starting from the root node as the current node,
 * 1) If there is no left child, print current node data and go to right child.
 * 2) If the left child exists,
 *    2.1) Find the inorder predecessor of the current node.
 *    2.2) If the predecessor's right child is
 *         2.2.1) NULL, then
 *                A) Print current node.
 *                B) Make the predecessor's right child as the current node.
 *                C) Go to left child.
 *         2.2.2) Current node, then
 *                A) Make the predecessor's right child as NULL.
 *                B) Go to the right child.
 * 3) Repeat 1 & 2 till current node is NULL.
 *
 * The above Binary Tree structure during the afore mentioned steps:
 * Note: Nodes with '[]' are printed.
 *
 *     A         [A]         [A]         [A]        [A]        [A]       [A]
 *    / \        /|\         /|\         /|\        /|\        / \       / \
 *   B   C -->  B | C --> [B] | C --> [B] | C --> [B]| C --> [B]  C --> [B] [C]
 *  / \        / \|       // \|       // \|       / \|       / \        / \
 * D   E      D   E      D    E      [D]  E     [D]  E     [D] [E]    [D] [E]
 *
 */
static void dfs_print_precac_tree_nodes(struct wlan_dfs *dfs,
					struct dfs_precac_entry *precac_entry)
{
	struct precac_tree_node *root = precac_entry->tree_root;
	struct precac_tree_node *curr_node, *inorder_predecessor;

	if (!root)
		return;
	curr_node = root;
	while (curr_node) {
		if (!curr_node->left_child) {
			dfs_print_node_data(dfs, curr_node);
			curr_node = curr_node->right_child;
		} else {
			/* Find the right most leaf node of the left subtree. */
			inorder_predecessor = curr_node->left_child;
			while (inorder_predecessor->right_child &&
			       inorder_predecessor->right_child != curr_node)
				inorder_predecessor =
					inorder_predecessor->right_child;

			/* If the right most child of left subtree already
			 * is linked to current node. We have traversed
			 * left subtree. Remove the link and go to right
			 * subtree
			 */
			if (inorder_predecessor->right_child == curr_node) {
				inorder_predecessor->right_child = NULL;
				curr_node = curr_node->right_child;
			} else {
			/* Print current node data, make current node
			 * as predecessor's right child, and move to left child.
			 */
				dfs_print_node_data(dfs, curr_node);
				inorder_predecessor->right_child = curr_node;
				curr_node = curr_node->left_child;
			}
		}
	}
}

/**
 * dfs_is_precac_completed_count_non_zero() - API to find if the preCAC
 * completed channels count is zero/non_zero.
 * @dfs: Pointer to DFS object.
 *
 * Return true, if there exists atleast one node/subchannel in the preCAC list
 * that is CAC done, else return false.
 */
static bool
dfs_is_precac_completed_count_non_zero(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *precac_entry = NULL;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_list,
			      pe_list) {
			/* Find if the tree root has any preCAC channels
			 * that is CAC done.
			 */
			if (!precac_entry->tree_root->n_caced_subchs)
				continue;
			if (abs(precac_entry->tree_root->n_caced_subchs -
			    precac_entry->non_dfs_subch_count)) {
				PRECAC_LIST_UNLOCK(dfs);
				return true;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	return false;
}

#ifdef ATH_SUPPORT_ZERO_CAC_DFS
enum precac_status_for_chan
dfs_precac_status_for_channel(struct wlan_dfs *dfs,
			      struct dfs_channel *deschan)
{
	if (!dfs_is_precac_completed_count_non_zero(dfs))
		return DFS_NO_PRECAC_COMPLETED_CHANS;

	if (dfs_is_precac_done(dfs, deschan))
		return DFS_PRECAC_COMPLETED_CHAN;

	return DFS_PRECAC_REQUIRED_CHAN;
}
#endif

void dfs_print_precaclists(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *tmp_precac_entry;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	PRECAC_LIST_LOCK(dfs);

	/* Print the Pre-CAC required List */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Precac status of all nodes in the list:");
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "NOTE: Syntax for each node: <ch_ieee>(<CAC>,<NOL>)");
	TAILQ_FOREACH(tmp_precac_entry,
			&dfs->dfs_precac_list,
			pe_list) {
		dfs_print_precac_tree_nodes(dfs, tmp_precac_entry);
	}
	PRECAC_LIST_UNLOCK(dfs);
}

void dfs_reset_precaclists(struct wlan_dfs *dfs)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Reset precaclist of VHT80 frequencies");
	dfs_deinit_precac_list(dfs);
	dfs_init_precac_list(dfs);
}

void dfs_reset_precac_lists(struct wlan_dfs *dfs)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}
	dfs_reset_precaclists(dfs);
}

/*
 * dfs_set_precac_preferred_channel() - Set preCAC preferred channel.
 * @dfs: Pointer to wlan_dfs.
 * @chan: Pointer to dfs_channel.
 * @mode: Wireless mode of channel.
 */
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
void dfs_set_precac_preferred_channel(struct wlan_dfs *dfs,
				      struct dfs_channel *chan, uint8_t mode)
{
	bool found = false;
	uint16_t freq_160_sec_mhz = 0;
	struct dfs_precac_entry *precac_entry;

	if (dfs_is_precac_timer_running(dfs) &&
	    WLAN_IS_CHAN_MODE_80(chan) &&
	    (dfs->dfs_precac_secondary_freq_mhz == chan->dfs_ch_freq)) {
		return;
	}

	/* Remove and insert into head, so that the user configured channel
	 * is picked first for preCAC.
	 */
	PRECAC_LIST_LOCK(dfs);
	if (WLAN_IS_CHAN_DFS(chan) &&
	    !TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_list, pe_list) {
			if (precac_entry->vht80_ch_freq ==
			    chan->dfs_ch_mhz_freq_seg1) {
				found = true;
				TAILQ_REMOVE(&dfs->dfs_precac_list,
					     precac_entry, pe_list);
				TAILQ_INSERT_HEAD(&dfs->dfs_precac_list,
						  precac_entry, pe_list);
				break;
			}
		}
	}

	if (WLAN_IS_CHAN_MODE_160(chan) && WLAN_IS_CHAN_DFS(chan) &&
	    !TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		if (chan->dfs_ch_freq < chan->dfs_ch_mhz_freq_seg2)
			freq_160_sec_mhz = chan->dfs_ch_mhz_freq_seg1 +
				VHT160_FREQ_DIFF;
		else
			freq_160_sec_mhz = chan->dfs_ch_mhz_freq_seg1 -
				VHT160_FREQ_DIFF;

		found = false;
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_list, pe_list) {
			if (precac_entry->vht80_ch_freq ==
			    freq_160_sec_mhz) {
				found = true;
				TAILQ_REMOVE(&dfs->dfs_precac_list,
					     precac_entry, pe_list);
				TAILQ_INSERT_HEAD(&dfs->dfs_precac_list,
						  precac_entry, pe_list);
				break;
			}
		}
	}

	PRECAC_LIST_UNLOCK(dfs);

	if (!found) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"frequency not found in precac list");
		return;
	}
}

bool
dfs_decide_precac_preferred_chan_for_freq(struct wlan_dfs *dfs,
					  uint16_t *pref_chan_freq,
					  enum wlan_phymode mode)
{
	struct dfs_channel *chan;

	chan = qdf_mem_malloc(sizeof(struct dfs_channel));

	if (!chan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "malloc failed");
		return false;
	}

	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					      *pref_chan_freq, 0,
					      mode,
					      &chan->dfs_ch_freq,
					      &chan->dfs_ch_flags,
					      &chan->dfs_ch_flagext,
					      &chan->dfs_ch_ieee,
					      &chan->dfs_ch_vhtop_ch_freq_seg1,
					      &chan->dfs_ch_vhtop_ch_freq_seg2,
					      &chan->dfs_ch_mhz_freq_seg1,
					      &chan->dfs_ch_mhz_freq_seg2))
		goto exit;
	if (!dfs->dfs_precac_inter_chan_freq)
		goto exit;

	/*
	 * If precac is done on this channel use it, else use a intermediate
	 * non-DFS channel and trigger a precac on this channel.
	 */
	if ((WLAN_IS_CHAN_DFS(chan) ||
	     (WLAN_IS_CHAN_MODE_160(chan) &&
	      WLAN_IS_CHAN_DFS_CFREQ2(chan))) &&
	    !dfs_is_precac_done(dfs, chan)) {
		dfs->dfs_autoswitch_des_mode = mode;
		*pref_chan_freq = dfs->dfs_precac_inter_chan_freq;
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "des_chan=%d, des_mode=%d. Current operating channel=%d",
			  chan->dfs_ch_freq,
			  dfs->dfs_autoswitch_des_mode,
			  *pref_chan_freq);
		dfs->dfs_autoswitch_chan = chan;
		return true;
	}

	/* Since preCAC is completed on the user configured preferred channel,
	 * make this channel the future intermediate channel.
	 */
	dfs->dfs_precac_inter_chan_freq = chan->dfs_ch_freq;
exit:
	qdf_mem_free(chan);
	return false;
}

enum precac_chan_state
dfs_get_precac_chan_state_for_freq(struct wlan_dfs *dfs, uint16_t pcac_freq)
{
	struct dfs_channel chan;
	struct dfs_precac_entry *tmp_precac_entry;
	struct precac_tree_node *root = NULL;
	enum precac_chan_state ret = PRECAC_ERR;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					      pcac_freq, 0,
					      WLAN_PHYMODE_11AC_VHT80,
					      &chan.dfs_ch_freq,
					      &chan.dfs_ch_flags,
					      &chan.dfs_ch_flagext,
					      &chan.dfs_ch_ieee,
					      &chan.dfs_ch_vhtop_ch_freq_seg1,
					      &chan.dfs_ch_vhtop_ch_freq_seg2,
					      &chan.dfs_ch_mhz_freq_seg1,
					      &chan.dfs_ch_mhz_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Intermediate channel not found");
		return PRECAC_ERR;
	}

	if (!WLAN_IS_CHAN_DFS(&chan)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"[%d] Not a DFS channel", pcac_freq);
		return PRECAC_ERR;
	}

	PRECAC_LIST_LOCK(dfs);
	if (dfs_is_precac_timer_running(dfs)) {
		tmp_precac_entry = TAILQ_FIRST(&dfs->dfs_precac_list);
		if (tmp_precac_entry && (tmp_precac_entry->vht80_ch_freq ==
					chan.dfs_ch_mhz_freq_seg1)) {
			ret = PRECAC_NOW;
			goto end;
		}
	}

	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_precac_list, pe_list) {
		if (tmp_precac_entry->vht80_ch_freq ==
				chan.dfs_ch_mhz_freq_seg1) {
			root = tmp_precac_entry->tree_root;
			if (root->n_nol_subchs)
				ret = PRECAC_NOL;
			else if (root->n_caced_subchs ==
					N_SUBCHS_FOR_BANDWIDTH(root->bandwidth))
				ret = PRECAC_DONE;
			else
				ret = PRECAC_REQUIRED;
			goto end;
		}
	}
end:
	PRECAC_LIST_UNLOCK(dfs);
	return ret;
}
#endif

#ifdef QCA_SUPPORT_AGILE_DFS
/* dfs_translate_chwidth_enum2val() - Translate the given channel width enum
 *                                    to it's value.
 * @dfs:     Pointer to WLAN DFS structure.
 * @chwidth: Channel width enum of the pdev's current channel.
 *
 * Return: The Bandwidth value for the given channel width enum.
 */
static uint8_t
dfs_translate_chwidth_enum2val(struct wlan_dfs *dfs,
			       enum phy_ch_width chwidth)
{
	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		return DFS_CHWIDTH_20_VAL;
	case CH_WIDTH_40MHZ:
		return DFS_CHWIDTH_40_VAL;
	case CH_WIDTH_80MHZ:
	case CH_WIDTH_80P80MHZ:
		return DFS_CHWIDTH_80_VAL;
	case CH_WIDTH_160MHZ:
		return DFS_CHWIDTH_160_VAL;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "cannot find mode!");
		return 0;
	}
}

/* dfs_map_to_agile_width() - Given a channel width enum, find the corresponding
 *                            translation for Agile channel width.
 *                            Translation schema of different operating modes:
 *                            20 -> 20, 40 -> 40, (80 & 160 & 80_80) -> 80.
 * @dfs:     Pointer to WLAN DFS structure.
 * @chwidth: Channel width enum.
 *
 * Return: The translated channel width enum.
 */
static enum phy_ch_width
dfs_map_to_agile_width(struct wlan_dfs *dfs, enum phy_ch_width chwidth)
{
	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		return CH_WIDTH_20MHZ;
	case CH_WIDTH_40MHZ:
		return CH_WIDTH_40MHZ;
	case CH_WIDTH_80MHZ:
		return CH_WIDTH_80MHZ;
	case CH_WIDTH_80P80MHZ:
	case CH_WIDTH_160MHZ:
		if (dfs_is_true_160mhz_supported(dfs) ||
		    dfs_is_restricted_80p80mhz_supported(dfs))
			return CH_WIDTH_160MHZ;
		return CH_WIDTH_80MHZ;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "Invalid chwidth enum!");
		return CH_WIDTH_INVALID;
	}
}

#ifdef QCA_SUPPORT_ADFS_RCAC

/* dfs_fill_des_rcac_chan_params() - Fill ch_params from dfs current channel.
 *                                   This ch_params is used to determine a
 *                                   Rolling CAC frequency invoking DFS Random
 *                                   selection algorithm.
 * @dfs: Pointer to wlan_dfs structure.
 * @ch_params: Pointer to ch_params structure.
 * @des_chwidth: Desired channel width.
 */
static void dfs_fill_des_rcac_chan_params(struct wlan_dfs *dfs,
					  struct ch_params *ch_params,
					  enum phy_ch_width des_chwidth)
{
	struct dfs_channel *chan = dfs->dfs_curchan;

	ch_params->ch_width = des_chwidth;
	ch_params->center_freq_seg0 = chan->dfs_ch_vhtop_ch_freq_seg1;
	ch_params->center_freq_seg1 = chan->dfs_ch_vhtop_ch_freq_seg2;
	ch_params->mhz_freq_seg0 = chan->dfs_ch_mhz_freq_seg1;
	ch_params->mhz_freq_seg1 = chan->dfs_ch_mhz_freq_seg2;
}


bool dfs_is_agile_rcac_enabled(struct wlan_dfs *dfs)
{
	enum dfs_reg dfsdomain;
	bool rcac_enabled = false;

	dfsdomain = utils_get_dfsdomain(dfs->dfs_pdev_obj);
	if ((dfsdomain == DFS_FCC_REGION ||
	     dfsdomain == DFS_MKK_REGION ||
	     dfsdomain == DFS_MKKN_REGION) &&
	    dfs->dfs_agile_rcac_ucfg && dfs->dfs_fw_adfs_support_non_160)
	    rcac_enabled = true;

	return rcac_enabled;
}
#endif

#ifdef QCA_SUPPORT_AGILE_DFS
bool dfs_is_agile_cac_enabled(struct wlan_dfs *dfs)
{
	return (dfs_is_agile_precac_enabled(dfs) ||
		dfs_is_agile_rcac_enabled(dfs));
}
#endif

/* dfs_convert_chwidth_to_wlan_phymode() - Given a channel width, find out the
 *                                         11AXA channel mode.
 * @chwidth: Channel width of type enum phy_ch_width.
 *
 * Return: Converted phymode of type wlan_phymode.
 */
static enum wlan_phymode
dfs_convert_chwidth_to_wlan_phymode(enum phy_ch_width chwidth)
{
	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		return WLAN_PHYMODE_11AXA_HE20;
	case CH_WIDTH_40MHZ:
		return WLAN_PHYMODE_11AXA_HE40;
	case CH_WIDTH_80MHZ:
		return WLAN_PHYMODE_11AXA_HE80;
	case CH_WIDTH_160MHZ:
		return WLAN_PHYMODE_11AXA_HE160;
	case CH_WIDTH_80P80MHZ:
		return WLAN_PHYMODE_11AXA_HE80_80;
	default:
		return WLAN_PHYMODE_MAX;
	}
}

/* dfs_find_dfschan_for_freq() - Given frequency and channel width, find
 *                               compute a dfs channel structure.
 * @dfs: Pointer to struct wlan_dfs.
 * @freq: Frequency in MHZ.
 * @center_freq_seg2: Secondary center frequency in MHZ.
 * @chwidth: Channel width.
 * @chan: Pointer to struct dfs_channel to be filled.
 *
 * Return: QDF_STATUS_SUCCESS if a valid channel pointer exists, else
 *         return status as QDF_STATUS_E_FAILURE.
 */
static QDF_STATUS
dfs_find_dfschan_for_freq(struct wlan_dfs *dfs,
			  qdf_freq_t freq,
			  qdf_freq_t center_freq_seg2,
			  enum phy_ch_width chwidth,
			  struct dfs_channel *chan)
{
	enum wlan_phymode mode;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!freq) {
		dfs_err(dfs, WLAN_DEBUG_DFS_AGILE, "Input freq is 0!!");
		return QDF_STATUS_E_FAILURE;
	}

	mode = dfs_convert_chwidth_to_wlan_phymode(chwidth);

	if (mode == WLAN_PHYMODE_MAX) {
		dfs_err(dfs, WLAN_DEBUG_DFS_AGILE, "Invalid RCAC mode, user "
				"rcac channel invalid!");
		return QDF_STATUS_E_FAILURE;
	}
	status =
	    dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					      freq, center_freq_seg2, mode,
					      &chan->dfs_ch_freq,
					      &chan->dfs_ch_flags,
					      &chan->dfs_ch_flagext,
					      &chan->dfs_ch_ieee,
					      &chan->dfs_ch_vhtop_ch_freq_seg1,
					      &chan->dfs_ch_vhtop_ch_freq_seg2,
					      &chan->dfs_ch_mhz_freq_seg1,
					      &chan->dfs_ch_mhz_freq_seg2);

	return status;
}

/* dfs_compute_agile_chan_width() - Compute agile detector's channel width
 *                                  and current channel's width.
 *
 * @dfs: Pointer to wlan_dfs structure.
 * @agile_ch_width: Agile channel width to be filled.
 * @cur_ch_width: Current channel width to be filled.
 */
static void
dfs_compute_agile_and_curchan_width(struct wlan_dfs *dfs,
				    enum phy_ch_width *agile_ch_width,
				    enum phy_ch_width *cur_ch_width)
{
	/*
	 * Agile detector's band of operation depends on current pdev.
	 * Find the current channel's width and apply the translate rules
	 * to find the Agile detector bandwidth.
	 * Translate rules (all numbers are in MHz) from current pdev's width
	 * to Agile detector's width:
	 * 20 - 20, 40 - 40, 80 - 80, 160 - 80, 160 (non contiguous) - 80.
	 */
	dfs_find_curchwidth_and_center_chan_for_freq(dfs, cur_ch_width,
						     NULL, NULL);

	/* Check if the FW supports agile DFS when the pdev is operating on
	 * 160 or 80P80MHz bandwidth. This information is stored in the flag
	 * "dfs_fw_adfs_support_160" when the current chainmask is configured.
	 */
	if ((*cur_ch_width == CH_WIDTH_80P80MHZ ||
	     *cur_ch_width == CH_WIDTH_160MHZ) &&
	    (!dfs->dfs_fw_adfs_support_160)) {
	    dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
		    "aDFS during 160MHz operation not supported by target");
	    return;
	}
	*agile_ch_width = dfs_map_to_agile_width(dfs, *cur_ch_width);
}

#ifdef QCA_SUPPORT_ADFS_RCAC

/* dfs_is_subchans_of_rcac_chan_in_nol() - Find out the HT20 subchannels of the
 *                                         given dfs_channel and determine if
 *                                         sub channels are in NOL.
 * @dfs: Pointer to struct wlan_dfs.
 * @rcac_chan: Pointer to struct dfs_channel.
 *
 * Return: true if the channel is in NOL else return false.
 */
static bool
dfs_is_subchans_of_rcac_chan_in_nol(struct wlan_dfs *dfs,
				    struct dfs_channel *rcac_chan)
{
	qdf_freq_t rcac_subchans[NUM_CHANNELS_160MHZ];
	uint8_t n_rcac_sub_chans = 0;
	int i;
	bool is_nol = false;

	n_rcac_sub_chans = dfs_find_dfs_sub_channels_for_freq(dfs, rcac_chan,
							      rcac_subchans);

	for (i = 0; i < n_rcac_sub_chans; i++) {
		if (dfs_is_freq_in_nol(dfs, rcac_subchans[i])) {
			is_nol = true;
			break;
		}
	}
	return is_nol;
}

/* dfs_is_rcac_chan_valid() - Find out if the band identified by the given
 *                            primary channel frequency and the width is
 *                            supported by the agile engine.
 * @dfs: Pointer to struct wlan_dfs.
 * @chwidth: Agile channel width
 * @rcac_freq: Rolling CAC frequency.
 *
 * Return: true if the channel is valid else return false.
 */
static bool
dfs_is_rcac_chan_valid(struct wlan_dfs *dfs, enum phy_ch_width chwidth,
		       qdf_freq_t rcac_freq)
{
	struct dfs_channel rcac_chan;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (chwidth == CH_WIDTH_80P80MHZ) {
		dfs_err(dfs, WLAN_DEBUG_DFS_AGILE,
			"RCAC cannot be started for 80P80MHz with single chan");
		return false;
	}

	qdf_mem_zero(&rcac_chan, sizeof(struct dfs_channel));

	/* 1. Find a valid channel pointer with rcac freq and
	 * agile channel width. If a valid channel pointer does not exists,
	 * return failure.
	 */
	status = dfs_find_dfschan_for_freq(dfs, rcac_freq, 0, chwidth,
					   &rcac_chan);
	if (status != QDF_STATUS_SUCCESS) {
		dfs_err(dfs, WLAN_DEBUG_DFS_AGILE,
			"RCAC Channel %d not found for agile width %d",
			dfs->dfs_agile_rcac_freq_ucfg,
			chwidth);
		return false;
	}

	/* 2. Reject the RCAC channel if it is a subset of the current operating
	 * channel or if the RCAC channel is non-DFS.
	 */
	if (dfs_is_new_chan_subset_of_old_chan(dfs, &rcac_chan,
					       dfs->dfs_curchan)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_AGILE,
			"RCAC Channel %d is either a subset of the current"
			"operating channel or is a non-dfs channel",
			dfs->dfs_agile_rcac_freq_ucfg);
		return false;
	}

	/* 3. Reject the RCAC channel if it has NOL channel as its subset. */
	if (dfs_is_subchans_of_rcac_chan_in_nol(dfs, &rcac_chan)) {
	    dfs_err(dfs, WLAN_DEBUG_DFS_AGILE,
		    "RCAC Channel %d has NOL channels as its subset",
		    dfs->dfs_agile_rcac_freq_ucfg);
		return false;
	}

	return true;
}

/* dfs_save_rcac_ch_params() - Save the RCAC channel's params in DFS.
 *                             It is stored in dfs->dfs_rcac_ch_params.
 *                             This ch_params is used in 80211_dfs_action
 *                             as the next channel after radar detect.
 * @dfs: Pointer to struct wlan_dfs.
 * @rcac_ch_params: Rolling CAC channel parameters.
 * @rcac_freq: Rolling CAC frequency.
 */
static void
dfs_save_rcac_ch_params(struct wlan_dfs *dfs, struct ch_params rcac_ch_params,
			qdf_freq_t rcac_freq)
{
	struct dfs_rcac_params *rcac_param = &dfs->dfs_rcac_param;

	rcac_param->rcac_pri_freq = rcac_freq;
	rcac_param->rcac_ch_params.ch_width = rcac_ch_params.ch_width;
	rcac_param->rcac_ch_params.sec_ch_offset = rcac_ch_params.sec_ch_offset;
	rcac_param->rcac_ch_params.center_freq_seg0 =
			rcac_ch_params.center_freq_seg0;
	rcac_param->rcac_ch_params.center_freq_seg1 =
			rcac_ch_params.center_freq_seg1;
	rcac_param->rcac_ch_params.mhz_freq_seg0 =
			rcac_ch_params.mhz_freq_seg0;
	rcac_param->rcac_ch_params.mhz_freq_seg1 =
			rcac_ch_params.mhz_freq_seg1;
	dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE,
		  "Saved rcac params: prim_freq: %d, width: %d, cfreq0: %d"
		  "cfreq1: %d", rcac_param->rcac_pri_freq,
		  rcac_param->rcac_ch_params.ch_width,
		  rcac_param->rcac_ch_params.center_freq_seg0,
		  rcac_param->rcac_ch_params.center_freq_seg1);
}

/* dfs_find_rcac_chan() - Find out a rolling CAC channel.
 *
 * @dfs: Pointer to struct wlan_dfs.
 * @curchan_chwidth: Current channel width.
 *
 * Return: Rolling CAC frequency in MHZ.
 */
static qdf_freq_t dfs_find_rcac_chan(struct wlan_dfs *dfs,
				     enum phy_ch_width curchan_chwidth,
				     enum phy_ch_width agile_chwidth)
{
	bool is_user_rcac_chan_valid = false;
	qdf_freq_t rcac_freq, rcac_center_freq = 0;
	struct dfs_channel dfs_chan;
	struct ch_params nxt_chan_params;

	qdf_mem_zero(&dfs_chan, sizeof(struct dfs_channel));
	qdf_mem_zero(&nxt_chan_params, sizeof(struct ch_params));

	/* If Rolling CAC is configured, RCAC frequency is the user configured
	 * RCAC frequency or it is found using DFS Random Channel Algorithm.
	 */

	/* Check if user configured RCAC frequency is valid */
	if (dfs->dfs_agile_rcac_freq_ucfg)
		is_user_rcac_chan_valid =
		    dfs_is_rcac_chan_valid(dfs, agile_chwidth,
					   dfs->dfs_agile_rcac_freq_ucfg);

	if (is_user_rcac_chan_valid) {
		rcac_freq = dfs->dfs_agile_rcac_freq_ucfg;
		if (dfs_find_dfschan_for_freq(dfs, rcac_freq, 0,
					      agile_chwidth,
					      &dfs_chan) != QDF_STATUS_SUCCESS)
			goto exit;

		nxt_chan_params.ch_width = agile_chwidth;
		/* Get the ch_params from regulatory. ch_width and rcac_freq
		 * are the input given to fetch other params of struct
		 * ch_params.
		 */
		wlan_reg_set_channel_params_for_freq(dfs->dfs_pdev_obj,
				rcac_freq, 0, &nxt_chan_params);
	} else {
		/* Invoke Random channel selection and select only
		 * DFS channels.
		 */
		uint16_t flags = DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH;

		/* Add Restricted 80p80 enabled bit to the flag so that
		 * random channel selection will fallback to 160MHz and pick
		 * 160MHz channels if the current operating BW is 165MHz.
		 */
		if (WLAN_IS_CHAN_MODE_165(dfs, dfs->dfs_curchan))
			flags |= DFS_RANDOM_CH_FLAG_RESTRICTED_80P80_ENABLED;

		if (!WLAN_IS_CHAN_5GHZ(dfs->dfs_curchan)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE,
				  "Current operating channel not a 5G channel");
			goto exit;
		}

		dfs_fill_des_rcac_chan_params(dfs,
					      &nxt_chan_params,
					      curchan_chwidth);

		/* The current dfs channel width may not be supported by the
		 * agile engine. For example, some chips may support
		 * 160/80+80Mhz mode for its operating channel (Tx/Rx),
		 * however, the agile engine may support up to a maximum of
		 * 80Mhz bandwidth.
		 * Therefore, we need to compute the agile channel width.
		 * The function dfs_compute_agile_chan_width calculated the
		 * agile channel width elsewhere and the agile channel width is
		 * passed to the utils_dfs_get_random_channel_for_freq through
		 * ch_params->ch_width.
		 */
		utils_dfs_get_random_channel_for_freq(dfs->dfs_pdev_obj, flags,
				&nxt_chan_params, NULL,
				&rcac_freq, NULL);

		if (dfs_find_dfschan_for_freq(dfs, rcac_freq,
					      nxt_chan_params.mhz_freq_seg1,
					      nxt_chan_params.ch_width,
					      &dfs_chan) != QDF_STATUS_SUCCESS)
			goto exit;
	}

	/* Store the random channel ch params for future use on
	 * radar detection.
	 */
	dfs_save_rcac_ch_params(dfs, nxt_chan_params, rcac_freq);

	if (!WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(&dfs_chan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE,
			  "Not picking an RCAC channel as the random channel"
			  "cfreq1: %d, cfreq2:%d chosen in non-DFS",
			  dfs_chan.dfs_ch_vhtop_ch_freq_seg1,
			  dfs_chan.dfs_ch_vhtop_ch_freq_seg2);
		return 0;
	}

	if (nxt_chan_params.ch_width != dfs->dfs_precac_chwidth) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE,
			  "Not picking an RCAC channel as next channel"
			  "width: %d is not an agile supported width: %d",
			  nxt_chan_params.ch_width, dfs->dfs_precac_chwidth);
		return 0;
	}
	/* Store the rcac chan params in dfs */
	rcac_center_freq = nxt_chan_params.mhz_freq_seg0;
	switch (nxt_chan_params.ch_width) {
	case CH_WIDTH_160MHZ:
		rcac_center_freq = nxt_chan_params.mhz_freq_seg1;
		break;
	case CH_WIDTH_80P80MHZ:
		if ((rcac_center_freq ==
		     RESTRICTED_80P80_LEFT_80_CENTER_FREQ) ||
		    (rcac_center_freq ==
		     RESTRICTED_80P80_RIGHT_80_CENTER_FREQ))
			rcac_center_freq = RESTRICTED_80P80_CHAN_CENTER_FREQ;
		break;
	default:
		break;
	}

	return rcac_center_freq;
exit:
	qdf_mem_zero(&dfs->dfs_rcac_param, sizeof(struct dfs_rcac_params));
	return 0;
}

#else
static inline qdf_freq_t dfs_find_rcac_chan(struct wlan_dfs *dfs,
					    enum phy_ch_width curchan_chwidth,
					    enum phy_ch_width agile_chwidth)
{
	return 0;
}
#endif

#endif

#ifdef QCA_SUPPORT_AGILE_DFS
/* dfs_find_precac_chan() - Find out a channel to perform preCAC.
 *
 * @dfs: Pointer to struct wlan_dfs.
 * @pri_ch_freq: Primary channel frequency in MHZ.
 * @sec_ch_freq: Secondary channel frequency in MHZ.
 *
 * Return: PreCAC frequency in MHZ.
 */
static qdf_freq_t dfs_find_precac_chan(struct wlan_dfs *dfs,
				       qdf_freq_t pri_ch_freq,
				       qdf_freq_t sec_ch_freq)
{
	/* Convert precac_chwidth to DFS width and find a valid Agile
	 * PreCAC frequency from the preCAC tree.
	 */
	uint8_t chwidth_val;

	/* Find chwidth value for the given enum */
	chwidth_val = dfs_translate_chwidth_enum2val(dfs,
						     dfs->dfs_precac_chwidth);

	dfs->dfs_soc_obj->ocac_status = OCAC_RESET;
	return dfs_get_ieeechan_for_precac_for_freq(dfs,
						    pri_ch_freq,
						    sec_ch_freq,
						    chwidth_val);
}

/*
 * dfs_set_agilecac_chan_for_freq() - Set agile CAC frequency.
 * @dfs: Pointer to wlan_dfs.
 * @ch_freq: Channel frequency in MHZ.
 * @pri_ch_freq: Primary channel frequency.
 * @sec_ch_freq: Secondary channel frequency.
 */
void dfs_set_agilecac_chan_for_freq(struct wlan_dfs *dfs,
				    qdf_freq_t *ch_freq,
				    qdf_freq_t pri_ch_freq,
				    qdf_freq_t sec_ch_freq)
{
	qdf_freq_t ieee_chan_freq;
	enum phy_ch_width agile_chwidth = CH_WIDTH_INVALID;
	enum phy_ch_width curchan_chwidth = CH_WIDTH_INVALID;

	dfs_compute_agile_and_curchan_width(dfs, &agile_chwidth,
					    &curchan_chwidth);
	if (agile_chwidth == CH_WIDTH_INVALID) {
		qdf_info("Cannot start Agile CAC as a valid agile channel width "
			 "could not be found\n");
		return;
	}
	dfs->dfs_precac_chwidth = agile_chwidth;

	if (dfs_is_agile_rcac_enabled(dfs))
		ieee_chan_freq = dfs_find_rcac_chan(dfs, curchan_chwidth,
						    agile_chwidth);
	else
		ieee_chan_freq = dfs_find_precac_chan(dfs, pri_ch_freq,
						      sec_ch_freq);

	dfs->dfs_agile_precac_freq_mhz = ieee_chan_freq;

	/* It was assumed that the bandwidth of the restricted 80p80 channel is
	 * 160MHz to build the precac tree. But when configuring Agile the
	 * channel width should be given as 80p80.
	 */
	if (ieee_chan_freq == RESTRICTED_80P80_CHAN_CENTER_FREQ)
		dfs->dfs_precac_chwidth = CH_WIDTH_80P80MHZ;

	*ch_freq = dfs->dfs_agile_precac_freq_mhz;

	dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE, "Current channel width: %d,"
		  "Agile channel width: %d",
		  curchan_chwidth, agile_chwidth);

	if (!*ch_freq)
		qdf_info("%s: No valid Agile channels available in the current pdev", __func__);
}
#endif


/*
 * dfs_find_vht80_chan_for_precac_for_freq() - Find VHT80 channel for preCAC.
 * @dfs: Pointer to wlan_dfs.
 * @chan_mode: Channel mode.
 * @cfreq_seg1_mhz: VHT80 Segment 1 frequency.
 * @cfreq1: Primary segment center frequency.
 * @cfreq2: Secondary segment center frequency.
 * @phy_mode: Phymode.
 * @dfs_set_cfreq2: Flag to indicate if cfreq2 is set.
 * @set_agile: Flag to set agile.
 */
void dfs_find_vht80_chan_for_precac_for_freq(struct wlan_dfs *dfs,
					     uint32_t chan_mode,
					     uint16_t cfreq_seg1_mhz,
					     uint32_t *cfreq1,
					     uint32_t *cfreq2,
					     uint32_t *phy_mode,
					     bool *dfs_set_cfreq2,
					     bool *set_agile)
{
	uint16_t ieee_freq;
	uint8_t chwidth_val = DFS_CHWIDTH_80_VAL;

	if (chan_mode != WLAN_PHYMODE_11AC_VHT80)
		return;

	dfs->dfs_precac_chwidth = CH_WIDTH_80MHZ;
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "precac_secondary_freq = %u precac_running = %u",
		  dfs->dfs_precac_secondary_freq_mhz,
		  dfs->dfs_soc_obj->dfs_precac_timer_running);

	/*
	 * If Pre-CAC is enabled then find a center frequency for
	 * the secondary VHT80 and Change the mode to
	 * VHT80_80 or VHT160.
	 */
	if (dfs_is_legacy_precac_enabled(dfs)) {
		/*
		 * If precac timer is running then do not change the
		 * secondary channel use the old secondary VHT80
		 * channel. If precac timer is not running then try to
		 * find a new channel from precac-list.
		 */
		if (dfs->dfs_soc_obj->dfs_precac_timer_running) {
			/*
			 * Primary and secondary VHT80 cannot be the
			 * same. Therefore exclude the primary
			 * frequency while getting new channel from
			 * precac-list.
			 */
			if (cfreq_seg1_mhz ==
			    dfs->dfs_precac_secondary_freq_mhz)
				ieee_freq =
					dfs_get_ieeechan_for_precac_for_freq
					(dfs, cfreq_seg1_mhz, 0, chwidth_val);
			else
				ieee_freq = dfs->dfs_precac_secondary_freq_mhz;
		} else
			ieee_freq = dfs_get_ieeechan_for_precac_for_freq
				(dfs, cfreq_seg1_mhz, 0, chwidth_val);
		if (ieee_freq) {
			if (ieee_freq == (cfreq_seg1_mhz +
					  VHT160_FREQ_DIFF)) {
				/*
				 * Override the HW channel mode to
				 * VHT160
				 */
				uint16_t cfreq_160;

				cfreq_160 = (ieee_freq + cfreq_seg1_mhz) / 2;
				chan_mode = WLAN_PHYMODE_11AC_VHT160;
				*cfreq1 = cfreq_seg1_mhz;
				*cfreq2 = cfreq_160;
			} else {
				/*
				 * Override the HW channel mode to
				 * VHT80_80.
				 */
				chan_mode = WLAN_PHYMODE_11AC_VHT80_80;
				*cfreq2 = ieee_freq;
			}
			*phy_mode = lmac_get_phymode_info(dfs->dfs_pdev_obj,
							  chan_mode);
			*dfs_set_cfreq2 = true;

			/*
			 * Finally set the agile flag.
			 * When we want a full calibration of both
			 * primary VHT80 and secondary VHT80 the agile
			 * flag is set to FALSE else set to TRUE. When
			 * a channel is being set for the first time
			 * this flag must be FALSE because first time
			 * the entire channel must be calibrated. All
			 * subsequent times the flag must be set to TRUE
			 * if we are changing only the secondary VHT80.
			 */
			if (dfs->dfs_precac_primary_freq_mhz == cfreq_seg1_mhz)
				*set_agile = true;
			else
				*set_agile = false;

			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "cfreq1 = %u cfreq2 = %u ieee_freq = %u mode = %u set_agile = %d",
				  *cfreq1, *cfreq2, ieee_freq,
				  chan_mode, *set_agile);

			dfs->dfs_precac_secondary_freq_mhz = ieee_freq;
			dfs->dfs_precac_primary_freq_mhz = cfreq_seg1_mhz;
			/* Start the pre_cac_timer */
			dfs_start_precac_timer_for_freq(dfs, ieee_freq);
		} else {
			dfs->dfs_precac_secondary_freq_mhz = 0;
		} /* End of if(ieee_freq) */
	} /* End of if(dfs_is_legacy_precac_enabled(dfs)) */
}

void dfs_set_precac_enable(struct wlan_dfs *dfs, uint32_t value)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tgt_tx_ops;
	uint32_t target_type;
	struct target_psoc_info *tgt_hdl;
	struct tgt_info *info;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		dfs->dfs_legacy_precac_ucfg = 0;
		dfs->dfs_agile_precac_ucfg = 0;
		return;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "tx_ops is NULL");
		return;
	}

	tgt_tx_ops = &tx_ops->target_tx_ops;
	target_type = lmac_get_target_type(dfs->dfs_pdev_obj);

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "target_psoc_info is null");
		return;
	}

	info = (struct tgt_info *)(&tgt_hdl->info);

	if (!info) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "tgt_info is null");
		return;
	}

	/*
	 * If
	 * 1) The chip is CASCADE,
	 * 2) The user has enabled Pre-CAC and
	 * 3) The regdomain the ETSI,
	 * then enable preCAC.
	 *
	 * OR
	 *
	 * If
	 * 1) The chip has agile_capability enabled
	 * 2) The user has enabled Pre-CAC and
	 * 3) The regdomain the ETSI,
	 * then enable Agile preCAC.
	 */

	if ((1 == value) &&
	    (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)) {
		if (tgt_tx_ops->tgt_is_tgt_type_qca9984(target_type))
			dfs->dfs_legacy_precac_ucfg = value;
		else
			dfs->dfs_agile_precac_ucfg = value;
	} else {
		dfs->dfs_agile_precac_ucfg = 0;
		dfs->dfs_legacy_precac_ucfg = 0;
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "preCAC disabled");
	}

	if (dfs_is_precac_timer_running(dfs)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Precac flag changed. Cancel the precac timer");
		if (tgt_tx_ops->tgt_is_tgt_type_qca9984(target_type)) {
			dfs_cancel_precac_timer(dfs);
			dfs->dfs_soc_obj->precac_state_started = 0;
		} else {
#ifdef QCA_SUPPORT_AGILE_DFS
			dfs_agile_sm_deliver_evt(dfs->dfs_soc_obj,
						 DFS_AGILE_SM_EV_AGILE_STOP,
						 0, (void *)dfs);
#endif
		}
	}
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_agile_precac_start(struct wlan_dfs *dfs)
{
	struct dfs_agile_cac_params adfs_param;
	uint8_t ocac_status = 0;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;

	qdf_info("%s : %d agile_precac_started: %d",
		 __func__, __LINE__,
		dfs_soc_obj->precac_state_started);

	dfs_soc_obj->dfs_priv[dfs->dfs_psoc_idx].agile_precac_active = true;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Agile preCAC set to active for dfs_index = %d, dfs: %pK",
		 dfs->dfs_psoc_idx, dfs);

	if (!dfs_soc_obj->precac_state_started) {
		dfs_soc_obj->cur_agile_dfs_index = dfs->dfs_psoc_idx;
		/*
		 * Initiate first call to start preCAC here, for channel as 0,
		 * and ocac_status as 0
		 */
		adfs_param.precac_chan = 0;
		adfs_param.precac_center_freq_1 = 0;
		adfs_param.precac_chwidth = CH_WIDTH_INVALID;
		qdf_info("%s : %d Initiated agile precac",
			 __func__, __LINE__);
		dfs->dfs_soc_obj->precac_state_started = true;
		dfs_start_agile_precac_timer(dfs, ocac_status, &adfs_param);
	}
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/*
 * dfs_set_precac_intermediate_chan() - Set preCAC intermediate channel.
 * @dfs: Pointer to wlan_dfs.
 * @freq: Channel frequency in MHZ.
 */
int32_t dfs_set_precac_intermediate_chan(struct wlan_dfs *dfs, uint32_t freq)
{
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_chan_for_freq(dfs->dfs_pdev_obj,
					      freq, 0,
					      WLAN_PHYMODE_11AC_VHT80,
					      &chan.dfs_ch_freq,
					      &chan.dfs_ch_flags,
					      &chan.dfs_ch_flagext,
					      &chan.dfs_ch_ieee,
					      &chan.dfs_ch_vhtop_ch_freq_seg1,
					      &chan.dfs_ch_vhtop_ch_freq_seg2,
					      &chan.dfs_ch_mhz_freq_seg1,
					      &chan.dfs_ch_mhz_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Intermediate channel not found");
		return -EINVAL;
	}

	/*Intermediate channel should be non-DFS channel*/
	if (!WLAN_IS_CHAN_DFS(&chan)) {
		dfs->dfs_precac_inter_chan_freq = freq;
	} else {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "intermediate channel %s",
			(chan.dfs_ch_freq == freq) ?
			"should not be DFS channel" : "is invalid");
		dfs->dfs_precac_inter_chan_freq = 0;
		return -EINVAL;
	}

	return 0;
}
#endif
/*
 * dfs_get_precac_intermediate_chan() - Get interCAC channel.
 * @dfs: Pointer to wlan_dfs.
 */
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
uint32_t dfs_get_precac_intermediate_chan(struct wlan_dfs *dfs)
{
	return dfs->dfs_precac_inter_chan_freq;
}
#endif
#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_reset_agile_config(struct dfs_soc_priv_obj *dfs_soc)
{
	dfs_soc->cur_agile_dfs_index = PCAC_DFS_INDEX_ZERO;
	dfs_soc->dfs_precac_timer_running = PCAC_TIMER_NOT_RUNNING;
	dfs_soc->precac_state_started = PRECAC_NOT_STARTED;
	dfs_soc->ocac_status = OCAC_SUCCESS;
}

void dfs_set_fw_adfs_support(struct wlan_dfs *dfs,
			     bool fw_adfs_support_160,
			     bool fw_adfs_support_non_160)
{
	dfs->dfs_fw_adfs_support_non_160 = fw_adfs_support_non_160;
	dfs->dfs_fw_adfs_support_160 = fw_adfs_support_160;
}
#endif

void dfs_reinit_precac_lists(struct wlan_dfs *src_dfs,
			     struct wlan_dfs *dest_dfs,
			     uint16_t low_5g_freq,
			     uint16_t high_5g_freq)
{
	struct dfs_precac_entry *tmp_precac_entry, *tmp_precac_entry2;

	/* If the destination DFS is not adhering ETSI (or)
	 * if the source DFS does not have any lists, return (nothing to do).
	 */
	if (utils_get_dfsdomain(dest_dfs->dfs_pdev_obj) != DFS_ETSI_DOMAIN ||
	    TAILQ_EMPTY(&src_dfs->dfs_precac_list))
		return;

	/* If dest_dfs and src_dfs are same it will cause dead_lock. */
	if (dest_dfs == src_dfs)
	       return;

	PRECAC_LIST_LOCK(dest_dfs);
	if (TAILQ_EMPTY(&dest_dfs->dfs_precac_list))
		TAILQ_INIT(&dest_dfs->dfs_precac_list);
	PRECAC_LIST_LOCK(src_dfs);
	TAILQ_FOREACH(tmp_precac_entry,
		      &src_dfs->dfs_precac_list,
		      pe_list) {
		if (low_5g_freq <= tmp_precac_entry->vht80_ch_freq &&
		    high_5g_freq >= tmp_precac_entry->vht80_ch_freq) {
			/* If the destination DFS already have the entries for
			 * some reason, remove them and update with the active
			 * entry in the source DFS list.
			 */
			TAILQ_FOREACH(tmp_precac_entry2,
				      &dest_dfs->dfs_precac_list,
				      pe_list) {
				if (tmp_precac_entry2->vht80_ch_freq ==
				    tmp_precac_entry->vht80_ch_freq)
					TAILQ_REMOVE(&dest_dfs->dfs_precac_list,
						     tmp_precac_entry2,
						     pe_list);
			}
			TAILQ_REMOVE(&src_dfs->dfs_precac_list,
				     tmp_precac_entry,
				     pe_list);
			tmp_precac_entry->dfs = dest_dfs;
			TAILQ_INSERT_TAIL(&dest_dfs->dfs_precac_list,
					  tmp_precac_entry,
					  pe_list);
		}
	}
	PRECAC_LIST_UNLOCK(src_dfs);
	PRECAC_LIST_UNLOCK(dest_dfs);
}

#ifdef QCA_SUPPORT_AGILE_DFS
/* dfs_start_agile_engine() - Prepare ADFS params and program the agile
 *                            engine sending agile config cmd to FW.
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_start_agile_engine(struct wlan_dfs *dfs)
{
	struct dfs_agile_cac_params adfs_param;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;

	/* Fill the RCAC ADFS params and send it to FW.
	 * FW does not use RCAC timeout values for RCAC feature.
	 * FW runs an infinite timer.
	 */
	dfs_fill_adfs_chan_params(dfs, &adfs_param);
	adfs_param.min_precac_timeout = MIN_RCAC_DURATION;
	adfs_param.max_precac_timeout = MAX_RCAC_DURATION;
	adfs_param.ocac_mode = QUICK_RCAC_MODE;

	qdf_info("%s : %d RCAC channel request sent for pdev: %pK "
			 "ch_freq: %d", __func__, __LINE__, dfs->dfs_pdev_obj,
			 dfs->dfs_agile_precac_freq_mhz);

	dfs_tx_ops = wlan_psoc_get_dfs_txops(dfs_soc_obj->psoc);

	if (dfs_tx_ops && dfs_tx_ops->dfs_agile_ch_cfg_cmd)
		dfs_tx_ops->dfs_agile_ch_cfg_cmd(dfs->dfs_pdev_obj,
						 &adfs_param);
	else
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"dfs_tx_ops=%pK", dfs_tx_ops);
}

/**
 * --------------------- ROLLING CAC STATE MACHINE ----------------------
 *
 * Rolling CAC is a feature where in, a separate hardware (Agile detector)
 * will be brought up in a channel that is not the current operating channel
 * and will continue to monitor the channel non-stop, until the next
 * channel change or radar in this RCAC channel.
 *
 * Now if the Rolling CAC channel was radar free for a minimum duration
 * (1 min.) and the device is now switching to this channel, no CAC is required.
 *
 * I.e. let's say the current operating channel is 64 HT80 and we are starting
 * the agile detector in 100 HT80. After a minute of being up in 100 HT80, we
 * switch the radio to 100 HT80. This operating channel change will not
 * require CAC now since the channel was radar free for the last 1 minute,
 * as determined by the agile detector.
 *
 * Introduction of a rolling CAC state machine:
 *
 * To acheive the rolling CAC feature using the agile detector, a trivial
 * state machine is implemented, as represented below:
 *
 *                           _________________
 *                          |                 |
 *            |------------>|       INIT      |<-----------|
 *            |             |_________________|            |
 *            |                      |                     |
 *            |                      |                     |
 *            | [EV_RCAC_STOP]       | [EV_RCAC_START]     | [EV_RCAC_STOP]
 *            | [EV_ADFS_RADAR]      |                     | [EV_ADFS_RADAR]
 *            |                      |                     |
 *            |                      |                     |
 *    ________|________              |             ________|________
 *   |                 |             |----------->|                 |
 *   |    COMPLETE     |                          |     RUNNING     |
 *   |_________________|<-------------------------|_________________|
 *                             [EV_RCAC_DONE]
 *
 *
 *
 * Legend:
 *     _________________
 *    |                 |
 * 1. |   RCAC STATES   |
 *    |_________________|
 *
 * 2. [RCAC_EVENTS]
 *
 *
 * Event triggers and handlers description:
 *
 * EV_RCAC_START:
 *   Posted from vdev response and is handled by all three states.
 *   1. INIT handler:
 *        a. Check if RCAC is already running,
 *           - If yes, do not transition.
 *           - If no, go to step b.
 *        b. Check if a new RCAC channel can be found,
 *           - If no, do not transition.
 *           - If yes, transition to RUNNING.
 *
 * EV_RCAC_STOP:
 *   Posted from last vap down or config disable, handled by RUNNING
 *   and COMPLETE.
 *   1. RUNNING handler:
 *        a. Stop the HOST RCAC timer.
 *        b. Send wmi_adfs_abort_cmd to FW and transition to INIT.
 *   2. COMPLETE handler:
 *        a. Send wmi_adfs_abort_cmd to FW and transition to INIT.
 *
 * EV_ADFS_RADAR:
 *   Posted from radar detection and is handled in RUNNING and COMPLETE.
 *   1. RUNNING handler (same as EV_RCAC_START):
 *        a. Check if RCAC was running for this pdev,
 *           - If yes, transition to INIT and post EV_RCAC_START event.
 *           - If no, ignore.
 *   2. COMPLETE handler (same as EV_RCAC_START):
 *        a. Check if RCAC was running for this pdev,
 *           - If yes, transition to INIT and post EV_RCAC_START event.
 *           - If no, ignore.
 *
 *   Note: EV_ADFS_RADAR works same as EV_RCAC_START event right now, but
 *         will change in future, where, based on user preference, either
 *         a new RCAC channel will be picked (requiring the transition to
 *         INIT like present), or RCAC will be restarted on the same channel.
 *
 * EV_RCAC_DONE:
 *   Posted from host RCAC timer completion and is handled in RUNNING.
 *   1. RUNNING handler:
 *      a. mark RCAC done and transition to COMPLETE.
 *
 * Epilogue:
 *   Rolling CAC state machine is for the entire psoc and since the
 *   agile detector can run for one pdev at a time, sharing of resource is
 *   required.
 *   In case of ETSI preCAC, sharing was done in a round robin fashion where
 *   each pdev runs ADFS for it's channels alternatively. However, in RCAC, the
 *   CAC period is not defined is continuous till the next channel change.
 *
 *   Hence ADFS detector is shared as follows:
 *   1. First come first serve: the pdev that is brought up first, i.e, for
 *      the first vdev response, an RCAC_START is posted and this pdev will
 *      hold the agile detector and run RCAC till it is stopped.
 *   2. Stopping the RCAC can be either by disabling user config "rcac_en 0"
 *      or by bringing down all vaps, or if no channel is available.
 *   3. Once RCAC is stopped for a pdev, it can be started in the other pdev
 *      by restarting it's vap (i.e. a vdev response).
 *
 *   A working sequence of RCAC is as follows:
 *     - Consider that the channel configured during bring up is 52HT80.
 *       1. The First VAP's vdev_start_resp posts an event EV_RCAC_START to the
 *          RCAC state machine.
 *       2. The RCAC state machine which is in INIT state (default) receives the
 *          event, picks a channel to do rolling CAC on, e.g. channel 100HT80.
 *          The SM is then transitioned to RUNNING state.
 *       3. In the entry of RUNNING state, a host timer is started and agile
 *          cfg cmd to FW is sent.
 *       4. When the HOST timer expires, it posts the EV_RCAC_DONE event to
 *          the state machine.
 *       5. EV_RCAC_DONE event received in RUNNING state, transitions the SM
 *          to COMPLETE.
 *       6. In the entry of COMPLETE, the RCAC channel is marked as CAC done
 *          in the precac tree.
 *       7. If radar is detected on primary channel, the new channel is the
 *          RCAC channel (100HT80) which does not require CAC if the preCAC
 *          tree is marked as CAC done.
 *          Before sending vdev_start, an EV_RCAC_STOP is posted
 *          which moves the SM to INIT state clearing all the params and
 *          bringing down the agile detector.
 *          (CAC decisions are taken before).
 *       8. After vdev_resp, another EV_RCAC_START is sent to restart the
 *          RCAC SM with a new RCAC channel if available.
 *
 *   A future enhancement will be triggering RCAC_START at user level.
 */

/**
 * dfs_agile_set_curr_state() - API to set the current state of Agile SM.
 * @dfs_soc_obj: Pointer to DFS soc private object.
 * @state: value of current state.
 *
 * Return: void.
 */
static void dfs_agile_set_curr_state(struct dfs_soc_priv_obj *dfs_soc_obj,
				     enum dfs_agile_sm_state state)
{
	if (state < DFS_AGILE_S_MAX) {
		dfs_soc_obj->dfs_agile_sm_cur_state = state;
	} else {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"DFS RCAC state (%d) is invalid", state);
		QDF_BUG(0);
	}
}

/**
 * dfs_agile_get_curr_state() - API to get current state of Agile SM.
 * @dfs_soc_obj: Pointer to DFS soc private object.
 *
 * Return: current state enum of type, dfs_rcac_sm_state.
 */
static enum dfs_agile_sm_state
dfs_agile_get_curr_state(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	return dfs_soc_obj->dfs_agile_sm_cur_state;
}

/**
 * dfs_rcac_sm_transition_to() - Wrapper API to transition the Agile SM state.
 * @dfs_soc_obj: Pointer to dfs soc private object that hold the SM handle.
 * @state: State to which the SM is transitioning to.
 *
 * Return: void.
 */
static void dfs_agile_sm_transition_to(struct dfs_soc_priv_obj *dfs_soc_obj,
				       enum dfs_agile_sm_state state)
{
	wlan_sm_transition_to(dfs_soc_obj->dfs_agile_sm_hdl, state);
}

/**
 * dfs_agile_sm_deliver_event() - API to post events to Agile SM.
 * @dfs_soc_obj: Pointer to dfs soc private object.
 * @event: Event to be posted to the RCAC SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * Return: QDF_STATUS_SUCCESS on handling the event, else failure.
 *
 * Note: This version of event posting API is not under lock and hence
 * should only be called for posting events within the SM and not be
 * under a dispatcher API without a lock.
 */
static
QDF_STATUS dfs_agile_sm_deliver_event(struct dfs_soc_priv_obj *dfs_soc_obj,
				      enum dfs_agile_sm_evt event,
				      uint16_t event_data_len,
				      void *event_data)
{
	return wlan_sm_dispatch(dfs_soc_obj->dfs_agile_sm_hdl,
				event,
				event_data_len,
				event_data);
}

#ifdef QCA_SUPPORT_ADFS_RCAC
/* dfs_start_agile_rcac_timer() - Start host agile RCAC timer.
 *
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_start_agile_rcac_timer(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;
	uint32_t rcac_timeout = MIN_RCAC_DURATION;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Host RCAC timeout = %d ms", rcac_timeout);

	qdf_timer_mod(&dfs_soc_obj->dfs_rcac_timer,
		      rcac_timeout);
}


/* dfs_stop_agile_rcac_timer() - Cancel the RCAC timer.
 *
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_stop_agile_rcac_timer(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	qdf_timer_sync_cancel(&dfs_soc_obj->dfs_rcac_timer);
}


/**
 * dfs_abort_agile_rcac() - Send abort Agile RCAC to F/W.
 * @dfs: Pointer to struct wlan_dfs.
 */
static void dfs_abort_agile_rcac(struct wlan_dfs *dfs)
{

	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	dfs_stop_agile_rcac_timer(dfs);
	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (dfs_tx_ops && dfs_tx_ops->dfs_ocac_abort_cmd)
		dfs_tx_ops->dfs_ocac_abort_cmd(dfs->dfs_pdev_obj);

	qdf_mem_zero(&dfs->dfs_rcac_param, sizeof(struct dfs_rcac_params));
	dfs->dfs_agile_precac_freq_mhz = 0;
	dfs->dfs_precac_chwidth = CH_WIDTH_INVALID;
	dfs->dfs_soc_obj->cur_agile_dfs_index = DFS_PSOC_NO_IDX;
}
#else
static inline void dfs_abort_agile_rcac(struct wlan_dfs *dfs)
{
}
#endif

/* dfs_abort_agile_precac() - Reset parameters of wlan_dfs and send abort
 * to F/W.
 * @dfs: Pointer to struct wlan_dfs.
 */
static void dfs_abort_agile_precac(struct wlan_dfs *dfs)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);

	dfs_agile_precac_cleanup(dfs);
	/*Send the abort to F/W as well */
	if (dfs_tx_ops && dfs_tx_ops->dfs_ocac_abort_cmd)
		dfs_tx_ops->dfs_ocac_abort_cmd(dfs->dfs_pdev_obj);
}

/**
 * dfs_agile_state_init_entry() - Entry API for INIT state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving to INIT state
 *
 * Return: void
 */
static void dfs_agile_state_init_entry(void *ctx)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;

	dfs_agile_set_curr_state(dfs_soc, DFS_AGILE_S_INIT);
}

/**
 * dfs_agile_state_init_exit() - Exit API for INIT state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving out of INIT state
 *
 * Return: void
 */
static void dfs_agile_state_init_exit(void *ctx)
{
	/* NO OPS */
}

/**
  * dfs_init_agile_start_evt_handler() - Init state start event handler.
  * @dfs: Instance of wlan_dfs structure.
  * @dfs_soc: DFS SoC private object
  *
  * Return : True if PreCAC/RCAC chan is found.
  */
static bool  dfs_init_agile_start_evt_handler(struct wlan_dfs *dfs,
					      struct dfs_soc_priv_obj *dfs_soc)
{
	bool is_chan_found = false;

	/*For RCAC */
	if (dfs_is_agile_rcac_enabled(dfs)) {
		/* Check if feature is enabled for this DFS and if RCAC channel
		 * is valid, if those are true, send appropriate WMIs to FW
		 * and only then transition to the state as follows.
		 */
		dfs_prepare_agile_rcac_channel(dfs, &is_chan_found);
	}
	/*For PreCAC */
	else if (dfs_is_agile_precac_enabled(dfs)) {
		dfs_soc->dfs_priv[dfs->dfs_psoc_idx].agile_precac_active
			= true;
		if (!dfs_soc->precac_state_started &&
		    !dfs_soc->dfs_precac_timer_running) {
			dfs_soc->precac_state_started = true;
			dfs_prepare_agile_precac_chan(dfs, &is_chan_found);
		}
	}

	return is_chan_found;
}

/**
 * dfs_agile_state_init_event() - INIT State event handler
 * @ctx: DFS SoC private object
 * @event: Event posted to the SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * API to handle events in INIT state
 *
 * Return: TRUE:  on handling event
 *         FALSE: on ignoring the event
 */
static bool dfs_agile_state_init_event(void *ctx,
				      uint16_t event,
				      uint16_t event_data_len,
				      void *event_data)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	bool status;
	struct wlan_dfs *dfs;
	bool is_chan_found;

	if (!event_data)
		return false;

	dfs = (struct wlan_dfs *)event_data;

	switch (event) {
	case DFS_AGILE_SM_EV_AGILE_START:

		if (dfs_soc->cur_agile_dfs_index != DFS_PSOC_NO_IDX)
			return true;

		is_chan_found = dfs_init_agile_start_evt_handler(dfs,
								 dfs_soc);
		if (is_chan_found) {
			dfs_soc->cur_agile_dfs_index = dfs->dfs_psoc_idx;
			dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_RUNNING);
		} else {
			/*
			 * This happens when there is no preCAC chan
			 * in any of the radios
			 */
			dfs_agile_precac_cleanup(dfs);
			/* Cleanup and wait */
		}

		status = true;
		break;
	default:
		status = false;
		break;
	}

	return status;
}

/**
 * dfs_agile_state_running_entry() - Entry API for running state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving to running state
 *
 * Return: void
 */
static void dfs_agile_state_running_entry(void *ctx)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	struct wlan_dfs *dfs =
		dfs_soc->dfs_priv[dfs_soc->cur_agile_dfs_index].dfs;

	dfs_agile_set_curr_state(dfs_soc, DFS_AGILE_S_RUNNING);

	/* RCAC */
	if (dfs_is_agile_rcac_enabled(dfs)) {
		dfs_start_agile_rcac_timer(dfs);
		dfs_start_agile_engine(dfs);
	}
}

/**
 * dfs_agile_state_running_exit() - Exit API for RUNNING state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving out of RUNNING state
 *
 * Return: void
 */
static void dfs_agile_state_running_exit(void *ctx)
{
	/* NO OPS */
}

/**
 * dfs_agile_state_running_event() - RUNNING State event handler
 * @ctx: DFS SoC private object
 * @event: Event posted to the SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * API to handle events in RUNNING state
 *
 * Return: TRUE:  on handling event
 *         FALSE: on ignoring the event
 */
static bool dfs_agile_state_running_event(void *ctx,
					 uint16_t event,
					 uint16_t event_data_len,
					 void *event_data)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	bool status;
	struct wlan_dfs *dfs;
	bool is_cac_done_on_des_chan;

	if (!event_data)
		return false;

	dfs = (struct wlan_dfs *)event_data;

	if (dfs->dfs_psoc_idx != dfs_soc->cur_agile_dfs_index)
		return false;

	switch (event) {
	case DFS_AGILE_SM_EV_ADFS_RADAR:
		/* After radar is found on the Agile channel we need to find
		 * a new channel and then start Agile CAC on that.
		 * On receiving the "DFS_AGILE_SM_EV_ADFS_RADAR_FOUND" if
		 * we change the state from [RUNNING] -> [RUNNING] then
		 * [RUNNING] should handle case in which a channel is not found
		 * and bring the state machine back to INIT.
		 * Instead we move the state to INIT and post the event
		 * "DFS_AGILE_SM_EV_AGILE_START" so INIT handles the case of
		 * channel not found and stay in that state.
		 * Abort the existing RCAC and restart from INIT state.
		 */
		if (dfs_is_agile_rcac_enabled(dfs))
			dfs_abort_agile_rcac(dfs);
		else if (dfs_is_agile_precac_enabled(dfs))
			dfs_abort_agile_precac(dfs);

		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		dfs_agile_sm_deliver_event(dfs_soc,
					   DFS_AGILE_SM_EV_AGILE_START,
					   event_data_len,
					   event_data);

		status = true;
		break;
	case DFS_AGILE_SM_EV_AGILE_STOP:
		if (dfs_is_agile_rcac_enabled(dfs))
			dfs_abort_agile_rcac(dfs);
		else if (dfs_is_agile_precac_enabled(dfs))
			dfs_abort_agile_precac(dfs);

		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		status = true;
		break;
	case DFS_AGILE_SM_EV_AGILE_DONE:
		if (dfs_is_agile_precac_enabled(dfs)) {
			if (dfs_soc->ocac_status == OCAC_SUCCESS) {
				dfs_soc->ocac_status = OCAC_RESET;
				dfs_mark_adfs_chan_as_cac_done(dfs);
			}
			dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
			dfs_agile_precac_cleanup(dfs);
			is_cac_done_on_des_chan =
				dfs_precac_check_home_chan_change(dfs);
			if (!is_cac_done_on_des_chan) {
				dfs_agile_sm_deliver_event(dfs_soc,
					DFS_AGILE_SM_EV_AGILE_START,
					event_data_len,
					event_data);
			}
		} else if (dfs_is_agile_rcac_enabled(dfs)) {
			dfs_agile_sm_transition_to(dfs_soc,
						   DFS_AGILE_S_COMPLETE);
		}
		status = true;
	default:
		status = false;
		break;
	}

	return status;
}

/**
 * dfs_agile_state_complete_entry() - Entry API for complete state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving to complete state
 *
 * Return: void
 */
static void dfs_agile_state_complete_entry(void *ctx)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = (struct dfs_soc_priv_obj *)ctx;
	struct wlan_dfs *dfs;

	dfs_agile_set_curr_state(dfs_soc_obj, DFS_AGILE_S_COMPLETE);

	if (!(dfs_soc_obj->cur_agile_dfs_index < WLAN_UMAC_MAX_PDEVS))
		return;

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_agile_dfs_index].dfs;

	/* Mark the RCAC channel as CAC done. */
	dfs_mark_adfs_chan_as_cac_done(dfs);
}

/**
 * dfs_agile_state_complete_exit() - Exit API for complete state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving out of complete state
 *
 * Return: void
 */
static void dfs_agile_state_complete_exit(void *ctx)
{
	/* NO OPs. */
}

/**
 * dfs_agile_state_complete_event() - COMPLETE State event handler
 * @ctx: DFS SoC private object
 * @event: Event posted to the SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * API to handle events in COMPLETE state
 *
 * Return: TRUE:  on handling event
 *         FALSE: on ignoring the event
 */
static bool dfs_agile_state_complete_event(void *ctx,
					  uint16_t event,
					  uint16_t event_data_len,
					  void *event_data)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	bool status;
	struct wlan_dfs *dfs;

	if (!event_data)
		return false;

	dfs = (struct wlan_dfs *)event_data;

	if (dfs->dfs_psoc_idx != dfs_soc->cur_agile_dfs_index)
		return false;

	switch (event) {
	case DFS_AGILE_SM_EV_ADFS_RADAR:
		/* Reset the RCAC done state for this RCAC chan of this dfs.
		 * Unmark the channels for RCAC done before calling abort API as
		 * the abort API invalidates the cur_agile_dfs_index.
		 */
		dfs_unmark_rcac_done(dfs);
		/* Abort the existing RCAC and restart from INIT state. */
		dfs_abort_agile_rcac(dfs);
		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		dfs_agile_sm_deliver_event(dfs_soc,
					  DFS_AGILE_SM_EV_AGILE_START,
					  event_data_len,
					  event_data);
		status = true;
		break;
	case DFS_AGILE_SM_EV_AGILE_STOP:
		/* Reset the RCAC done state for this RCAC chan of this dfs.
		 * Unmark the channels for RCAC done before calling abort API as
		 * the abort API invalidates the cur_agile_dfs_index.
		 */
		dfs_unmark_rcac_done(dfs);
		dfs_abort_agile_rcac(dfs);
		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		status = true;
		break;
	default:
		status = false;
		break;
	}

	return status;
}

static struct wlan_sm_state_info dfs_agile_sm_info[] = {
	{
		(uint8_t)DFS_AGILE_S_INIT,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"INIT",
		dfs_agile_state_init_entry,
		dfs_agile_state_init_exit,
		dfs_agile_state_init_event
	},
	{
		(uint8_t)DFS_AGILE_S_RUNNING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"RUNNING",
		dfs_agile_state_running_entry,
		dfs_agile_state_running_exit,
		dfs_agile_state_running_event
	},
	{
		(uint8_t)DFS_AGILE_S_COMPLETE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"COMPLETE",
		dfs_agile_state_complete_entry,
		dfs_agile_state_complete_exit,
		dfs_agile_state_complete_event
	},
};

static const char *dfs_agile_sm_event_names[] = {
	"EV_AGILE_START",
	"EV_AGILE_STOP",
	"EV_AGILE_DONE",
	"EV_ADFS_RADAR_FOUND",
};

/**
 * dfs_agile_sm_print_state() - API to log the current state.
 * @dfs_soc_obj: Pointer to dfs soc private object.
 *
 * Return: void.
 */
static void dfs_agile_sm_print_state(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	enum dfs_agile_sm_state state;

	state = dfs_agile_get_curr_state(dfs_soc_obj);
	if (!(state < DFS_AGILE_S_MAX))
		return;

	dfs_debug(NULL, WLAN_DEBUG_DFS_AGILE, "->[%s] %s",
		  dfs_soc_obj->dfs_agile_sm_hdl->name,
		  dfs_agile_sm_info[state].name);
}

/**
 * dfs_agile_sm_print_state_event() - API to log the current state and event
 *                                   received.
 * @dfs_soc_obj: Pointer to dfs soc private object.
 * @event: Event posted to RCAC SM.
 *
 * Return: void.
 */
static void dfs_agile_sm_print_state_event(struct dfs_soc_priv_obj *dfs_soc_obj,
					  enum dfs_agile_sm_evt event)
{
	enum dfs_agile_sm_state state;

	state = dfs_agile_get_curr_state(dfs_soc_obj);
	if (!(state < DFS_AGILE_S_MAX))
		return;

	dfs_debug(NULL, WLAN_DEBUG_DFS_AGILE, "[%s]%s, %s",
		  dfs_soc_obj->dfs_agile_sm_hdl->name,
		  dfs_agile_sm_info[state].name,
		  dfs_agile_sm_event_names[event]);
}

QDF_STATUS dfs_agile_sm_deliver_evt(struct dfs_soc_priv_obj *dfs_soc_obj,
				    enum dfs_agile_sm_evt event,
				    uint16_t event_data_len,
				    void *event_data)
{
	enum dfs_agile_sm_state old_state, new_state;
	QDF_STATUS status;

	DFS_AGILE_SM_SPIN_LOCK(dfs_soc_obj);
	old_state = dfs_agile_get_curr_state(dfs_soc_obj);

	/* Print current state and event received */
	dfs_agile_sm_print_state_event(dfs_soc_obj, event);

	status = dfs_agile_sm_deliver_event(dfs_soc_obj, event,
					   event_data_len, event_data);

	new_state = dfs_agile_get_curr_state(dfs_soc_obj);

	/* Print new state after event if transition happens */
	if (old_state != new_state)
		dfs_agile_sm_print_state(dfs_soc_obj);
	DFS_AGILE_SM_SPIN_UNLOCK(dfs_soc_obj);

	return status;
}

QDF_STATUS dfs_agile_sm_create(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	struct wlan_sm *sm;

	sm = wlan_sm_create("DFS_AGILE", dfs_soc_obj,
			    DFS_AGILE_S_INIT,
			    dfs_agile_sm_info,
			    QDF_ARRAY_SIZE(dfs_agile_sm_info),
			    dfs_agile_sm_event_names,
			    QDF_ARRAY_SIZE(dfs_agile_sm_event_names));
	if (!sm) {
		qdf_err("DFS AGILE SM allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	dfs_soc_obj->dfs_agile_sm_hdl = sm;

	qdf_spinlock_create(&dfs_soc_obj->dfs_agile_sm_lock);

	/* Initialize the RCAC DFS index to default (no index). */
	dfs_soc_obj->cur_agile_dfs_index = DFS_PSOC_NO_IDX;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_agile_sm_destroy(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	wlan_sm_delete(dfs_soc_obj->dfs_agile_sm_hdl);
	qdf_spinlock_destroy(&dfs_soc_obj->dfs_agile_sm_lock);

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS dfs_set_rcac_enable(struct wlan_dfs *dfs, bool rcac_en)
{
	if (rcac_en == dfs->dfs_agile_rcac_ucfg) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Rolling CAC: %d is already configured", rcac_en);
		return QDF_STATUS_SUCCESS;
	}
	dfs->dfs_agile_rcac_ucfg = rcac_en;

	/* RCAC config is changed. Reset the preCAC tree. */
	dfs_reset_precac_lists(dfs);

	if (!rcac_en) {
		dfs_agile_sm_deliver_evt(dfs->dfs_soc_obj,
					DFS_AGILE_SM_EV_AGILE_STOP,
					0,
					(void *)dfs);
	}
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "rolling cac is %d", rcac_en);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_get_rcac_enable(struct wlan_dfs *dfs, bool *rcacen)
{
	*rcacen = dfs->dfs_agile_rcac_ucfg;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_set_rcac_freq(struct wlan_dfs *dfs, qdf_freq_t rcac_freq)
{
	if (wlan_reg_is_5ghz_ch_freq(rcac_freq))
		dfs->dfs_agile_rcac_freq_ucfg = rcac_freq;
	else
		dfs->dfs_agile_rcac_freq_ucfg = 0;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,  "rolling cac freq %d",
		 dfs->dfs_agile_rcac_freq_ucfg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_get_rcac_freq(struct wlan_dfs *dfs, qdf_freq_t *rcac_freq)
{
	*rcac_freq = dfs->dfs_agile_rcac_freq_ucfg;

	return QDF_STATUS_SUCCESS;
}

/*
 * Rolling CAC Timer timeout function. Following actions are done
 * on timer expiry:
 * Timer running flag is cleared.
 * If the rolling CAC state is completed, the RCAC freq and its sub-channels
 * are marked as 'CAC Done' in the preCAC tree.
 */
static os_timer_func(dfs_rcac_timeout)
{
	struct wlan_dfs *dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	OS_GET_TIMER_ARG(dfs_soc_obj, struct dfs_soc_priv_obj *);

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_agile_dfs_index].dfs;

	dfs_agile_sm_deliver_evt(dfs_soc_obj,
				DFS_AGILE_SM_EV_AGILE_DONE,
				0,
				(void *)dfs);
}

void dfs_rcac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_init(NULL, &dfs_soc_obj->dfs_rcac_timer,
		       dfs_rcac_timeout,
		       (void *)dfs_soc_obj,
		       QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_rcac_timer_deinit(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_free(&dfs_soc_obj->dfs_rcac_timer);
}

/* dfs_prepare_agile_rcac_channel() - Find a valid Rolling CAC channel if
 *                                    available.
 *
 * @dfs: Pointer to struct wlan_dfs.
 * @is_rcac_chan_available: Flag to indicate if a valid RCAC channel is
 *                          available.
 */
void dfs_prepare_agile_rcac_channel(struct wlan_dfs *dfs,
				    bool *is_rcac_chan_available)
{
	qdf_freq_t rcac_ch_freq = 0;

	/* Find out a valid rcac_ch_freq */
	dfs_set_agilecac_chan_for_freq(dfs, &rcac_ch_freq, 0, 0);

	/* If RCAC channel is available, the caller will start the timer and
	 * send RCAC config to FW. If channel not available, the caller takes
	 * care of sending RCAC abort and moving SM to INIT, resetting the RCAC
	 * variables.
	 */
	*is_rcac_chan_available = rcac_ch_freq ? true : false;
	dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE, "Chosen rcac channel: %d",
		  rcac_ch_freq);
}
#endif
#endif

#if defined(QCA_SUPPORT_AGILE_DFS) || defined(ATH_SUPPORT_ZERO_CAC_DFS) || \
    defined(QCA_SUPPORT_ADFS_RCAC)
QDF_STATUS
dfs_process_radar_ind_on_agile_chan(struct wlan_dfs *dfs,
				    struct radar_found_info *radar_found)
{
	uint32_t freq_center;
	uint32_t radarfound_freq;
	QDF_STATUS status;
	uint8_t num_channels;
	uint16_t freq_list[NUM_CHANNELS_160MHZ];
	uint16_t nol_freq_list[NUM_CHANNELS_160MHZ];
	bool is_radar_source_agile =
		(radar_found->detector_id == dfs_get_agile_detector_id(dfs));

	dfs_compute_radar_found_cfreq(dfs, radar_found, &freq_center);
	radarfound_freq = freq_center + radar_found->freq_offset;
	if (is_radar_source_agile)
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Radar found on Agile detector freq=%d radar freq=%d",
			 freq_center, radarfound_freq);
	else if (radar_found->segment_id == SEG_ID_SECONDARY)
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Radar found on second segment.Radarfound Freq=%d MHz.Secondary Chan cfreq=%d MHz.",
			 radarfound_freq, freq_center);
	utils_dfs_deliver_event(dfs->dfs_pdev_obj, radarfound_freq,
				WLAN_EV_RADAR_DETECTED);
	if (!dfs->dfs_use_nol) {
		dfs_reset_bangradar(dfs);
		dfs_send_csa_to_current_chan(dfs);
		status = QDF_STATUS_SUCCESS;
		goto exit;
	}

	 num_channels = dfs_find_radar_affected_channels(dfs,
			 radar_found,
			 freq_list,
			 freq_center);

	 dfs_reset_bangradar(dfs);

	 status = dfs_radar_add_channel_list_to_nol_for_freq(dfs,
			 freq_list,
			 nol_freq_list,
			 &num_channels);
	 if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"radar event received on invalid channel");
		goto exit;
	 }
	 /*
	  * If precac is running and the radar found in secondary
	  * VHT80 mark the channel as radar and add to NOL list.
	  * Otherwise random channel selection can choose this
	  * channel.
	  */
	 dfs_debug(dfs, WLAN_DEBUG_DFS,
			 "found_on_second=%d is_pre=%d",
			 dfs->is_radar_found_on_secondary_seg,
			 dfs_is_precac_timer_running(dfs));
	 /*
	  * Even if radar found on primary, we need to mark the channel as NOL
	  * in preCAC list. The preCAC list also maintains the current CAC
	  * channels as part of pre-cleared DFS. Hence call the API
	  * to mark channels as NOL irrespective of preCAC being enabled or not.
	  */

	 dfs_debug(dfs, WLAN_DEBUG_DFS,
			 "%s: %d Radar found on dfs detector:%d",
			 __func__, __LINE__, radar_found->detector_id);
	 dfs_mark_precac_nol_for_freq(dfs,
			 dfs->is_radar_found_on_secondary_seg,
			 radar_found->detector_id,
			 nol_freq_list,
			 num_channels);
	if (is_radar_source_agile)
		utils_dfs_agile_sm_deliver_evt(dfs->dfs_pdev_obj,
					       DFS_AGILE_SM_EV_ADFS_RADAR);

exit:
	return status;
}
#endif

#if (defined(QCA_SUPPORT_AGILE_DFS) || defined(QCA_SUPPORT_ADFS_RCAC)) && \
     defined(WLAN_DFS_TRUE_160MHZ_SUPPORT) && defined(WLAN_DFS_FULL_OFFLOAD)
void dfs_translate_radar_params_for_agile_chan(struct wlan_dfs *dfs,
					       struct radar_found_info *r_info)
{
	if (dfs->dfs_precac_chwidth == CH_WIDTH_160MHZ) {
		if (r_info->freq_offset > 0) {
			/*
			 * If the radar hit frequency is right to the center of
			 * 160MHz center frequency, then the segment id should
			 * be secondary segment. The offset frequeny that was
			 * with respect to the 160MHz channel center should be
			 * converted offset frequency based on the right 80MHz
			 * center by subtracting 40MHz on the offset received.
			 */

			r_info->segment_id = SECONDARY_SEG;
			r_info->freq_offset -= DFS_160MHZ_SECOND_SEG_OFFSET;
		} else {
			/*
			 * If the radar hit frequency is left to the center of
			 * 160MHz center frequency, then the segment id should
			 * be primary segment. The offset frequeny that was with
			 * respect to the 160MHz channel center should be
			 * converted into offset frequency based on the left
			 * 80MHz center by adding 40MHz on the offset received.
			 */
			r_info->segment_id = PRIMARY_SEG;
			r_info->freq_offset += DFS_160MHZ_SECOND_SEG_OFFSET;
		}
	} else if (IS_HOST_AGILE_CURCHAN_165MHZ(dfs)) {
		if (r_info->freq_offset > DFS_160MHZ_SECOND_SEG_OFFSET) {
			/*
			 * If the radar hit frequency is on the right 80MHz
			 * segment of the 165MHz channel then the segment id
			 * should be secondary segment id and the offset should
			 * be converted to be based on the right 80MHz center
			 * frequency 5775MHz by subtracting 85MHz.
			 */
			r_info->segment_id = SECONDARY_SEG;
			r_info->freq_offset -= DFS_80P80MHZ_SECOND_SEG_OFFSET;
		}
	}
}
#endif
