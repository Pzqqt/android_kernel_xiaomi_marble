/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
#include "dfs_etsi_precac.h"
#include "dfs_process_radar_found_ind.h"
#include "target_if.h"
#include "wlan_dfs_init_deinit_api.h"

/* Given a bandwidth, find the number of subchannels in that bandwidth */
#define N_SUBCHS_FOR_BANDWIDTH(_bw) ((_bw) / MIN_DFS_SUBCHAN_BW)

void dfs_zero_cac_reset(struct wlan_dfs *dfs)
{
	dfs->dfs_precac_timeout_override = -1;
	dfs->dfs_precac_primary_freq = 0;
	dfs->dfs_precac_secondary_freq = 0;
}

void dfs_zero_cac_timer_detach(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_free(&dfs_soc_obj->dfs_precac_timer);
}

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

/* dfs_descend_precac_tree() - Descend into the precac BSTree based on the
 *                             channel provided. If the channel is less than
 *                             given node's channel, descend left, else right.
 * @node:    Precac BSTree node.
 * @channel: Channel whose node is to be found.
 *
 * Return: the next precac_tree_node (left child or right child of current node).
 */
static struct precac_tree_node *
dfs_descend_precac_tree(struct precac_tree_node *node,
			uint8_t channel)
{
	if (!node)
		return NULL;

	if (channel < node->ch_ieee)
		return node->left_child;
	else
		return node->right_child;
}

void dfs_find_chwidth_and_center_chan(struct wlan_dfs *dfs,
				      enum phy_ch_width *chwidth,
				      uint8_t *primary_chan_ieee,
				      uint8_t *secondary_chan_ieee)
{
	struct dfs_channel *curchan = dfs->dfs_curchan;

	if (!curchan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "curchan is NULL");
		return;
	}

	if (primary_chan_ieee)
		*primary_chan_ieee = curchan->dfs_ch_vhtop_ch_freq_seg1;
	if (WLAN_IS_CHAN_MODE_20(curchan)) {
		*chwidth = CH_WIDTH_20MHZ;
	} else if (WLAN_IS_CHAN_MODE_40(curchan)) {
		*chwidth = CH_WIDTH_40MHZ;
	} else if (WLAN_IS_CHAN_MODE_80(curchan)) {
		*chwidth = CH_WIDTH_80MHZ;
	} else if (WLAN_IS_CHAN_MODE_80_80(curchan)) {
		*chwidth = CH_WIDTH_80P80MHZ;
		if (secondary_chan_ieee)
			*secondary_chan_ieee =
				curchan->dfs_ch_vhtop_ch_freq_seg2;
	} else if (WLAN_IS_CHAN_MODE_160(curchan)) {
		*chwidth = CH_WIDTH_160MHZ;
		if (primary_chan_ieee)
			*primary_chan_ieee =
				curchan->dfs_ch_vhtop_ch_freq_seg2;
	}
}

/* dfs_find_cac_status_for_chan() - Find CAC-done status for the channel in the
 *                                  precac Binary Search Tree (BSTree).
 *				    Return true if CAC done, else false.
 * @dfs_precac_entry: Precac entry which has the root of the precac BSTree.
 * @chan_ieee:        IEEE channel. This ieee channel is the center of a
 *                    20/40/80 MHz channel and the center channel is unique
 *                    irrespective of the bandwidth(20/40/80 MHz).
 *
 * Note: For each tree node of a level, the number of CACed subchannels is
 * the total number of leaf nodes of the sub tree for the node, which are
 * CACed.
 * That is, at a level (1..n, n being the depth of the BSTree), the maximum
 * number of CACed subchannels is: Number of subchannels of root / level.
 * Num_subchannels = Num_subchannels_of_root_level / level.
 *
 * Return: true if CAC done on channel, else false.
 */
static bool dfs_find_cac_status_for_chan(struct dfs_precac_entry *precac_entry,
					 uint8_t chan_ieee)
{
	struct precac_tree_node *node = precac_entry->tree_root;
	uint8_t n_cur_lvl_subchs = N_SUBCHANS_FOR_80BW;

	while (node) {
		if (node->ch_ieee == chan_ieee)
			return (node->n_caced_subchs == n_cur_lvl_subchs) ?
				true : false;

		n_cur_lvl_subchs /= 2;
		node = dfs_descend_precac_tree(node, chan_ieee);
	}

	return false;
}

#define VHT80_OFFSET 6
#define IS_WITHIN_RANGE(_A, _B, _C)  \
	(((_A) >= ((_B)-(_C))) && ((_A) <= ((_B)+(_C))))
bool dfs_is_precac_done_on_ht20_40_80_chan(struct wlan_dfs *dfs,
					   uint8_t chan)
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
			/* Find if the VHT80 ieee_chan is in this precac_list */
			if (IS_WITHIN_RANGE(chan, precac_entry->vht80_ch_ieee,
					    VHT80_OFFSET)) {
				ret_val = dfs_find_cac_status_for_chan(
						precac_entry, chan);
				break;
			}
		}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS, "ch_ieee = %u cac_done = %d",
		  chan, ret_val);

	return ret_val;
}

bool dfs_is_precac_done_on_ht8080_ht160_chan(struct wlan_dfs *dfs,
					     struct dfs_channel *chan)
{
	bool ret_val = 0, primary_found = 0;
	u_int8_t pri_chan, sec_chan = 0;

	pri_chan = chan->dfs_ch_vhtop_ch_freq_seg1;
	if (WLAN_IS_CHAN_MODE_160(chan)) {
		if (chan->dfs_ch_ieee < chan->dfs_ch_vhtop_ch_freq_seg2)
			sec_chan = chan->dfs_ch_vhtop_ch_freq_seg1 +
				   VHT160_IEEE_FREQ_DIFF;
		else
			sec_chan = chan->dfs_ch_vhtop_ch_freq_seg1 -
				   VHT160_IEEE_FREQ_DIFF;
	} else
		sec_chan = chan->dfs_ch_vhtop_ch_freq_seg2;

	/* Check if primary is DFS then search */
	if (WLAN_IS_CHAN_DFS(chan))
		primary_found = dfs_is_precac_done_on_ht20_40_80_chan(dfs,
								      pri_chan);
	else
		primary_found = 1;

	/* Check if secondary DFS then search */
	if (WLAN_IS_CHAN_DFS_CFREQ2(chan) && primary_found) {
		ret_val = dfs_is_precac_done_on_ht20_40_80_chan(dfs, sec_chan);
	} else {
		if (primary_found)
			ret_val = 1;
	}
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"seg1_ieee = %u seg2_ieee = %u ret_val = %d",
		pri_chan,
		sec_chan,
		ret_val);

	return ret_val;
}

bool dfs_is_precac_done(struct wlan_dfs *dfs, struct dfs_channel *chan)
{
	bool ret_val = 0;
	uint8_t pri_chan = chan->dfs_ch_vhtop_ch_freq_seg1;

	if (WLAN_IS_CHAN_MODE_20(chan) ||
	    WLAN_IS_CHAN_MODE_40(chan) ||
	    WLAN_IS_CHAN_MODE_80(chan)) {
		ret_val = dfs_is_precac_done_on_ht20_40_80_chan(dfs,
								pri_chan);
	} else if (WLAN_IS_CHAN_MODE_80_80(chan) ||
		   WLAN_IS_CHAN_MODE_160(chan)) {
		ret_val = dfs_is_precac_done_on_ht8080_ht160_chan(dfs, chan);
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS, "precac_done_status = %d", ret_val);
	return ret_val;
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_find_pdev_for_agile_precac(struct wlan_objmgr_pdev *pdev,
				    uint8_t *cur_precac_dfs_index)
{
	struct wlan_dfs *dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_objmgr_psoc *psoc;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	psoc = wlan_pdev_get_psoc(pdev);

	dfs_soc_obj = dfs->dfs_soc_obj;

	*cur_precac_dfs_index =
	   (dfs_soc_obj->cur_precac_dfs_index + 1) % dfs_soc_obj->num_dfs_privs;
}

#define DFS_160MHZ_SECSEG_CHAN_OFFSET 8
void dfs_prepare_agile_precac_chan(struct wlan_dfs *dfs)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_dfs *temp_dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint8_t ch_freq = 0;
	uint8_t cur_dfs_idx = 0;
	uint8_t vhtop_ch_freq_seg1, vhtop_ch_freq_seg2;
	int i;
	struct dfs_agile_cac_params adfs_param;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_soc_obj = dfs->dfs_soc_obj;

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);

	pdev = dfs->dfs_pdev_obj;

	for (i = 0; i < dfs_soc_obj->num_dfs_privs; i++) {
		dfs_find_pdev_for_agile_precac(pdev, &cur_dfs_idx);
		dfs_soc_obj->cur_precac_dfs_index = cur_dfs_idx;
		temp_dfs = dfs_soc_obj->dfs_priv[cur_dfs_idx].dfs;
		pdev = temp_dfs->dfs_pdev_obj;
		if (!dfs_soc_obj->dfs_priv[cur_dfs_idx].agile_precac_active)
			continue;

		vhtop_ch_freq_seg1 =
			temp_dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg1;
		vhtop_ch_freq_seg2 =
			temp_dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2;
		if (WLAN_IS_CHAN_MODE_160(temp_dfs->dfs_curchan)) {
			if (vhtop_ch_freq_seg2 < vhtop_ch_freq_seg1)
				vhtop_ch_freq_seg2 -=
					DFS_160MHZ_SECSEG_CHAN_OFFSET;
			else
				vhtop_ch_freq_seg2 +=
					DFS_160MHZ_SECSEG_CHAN_OFFSET;
		}
		dfs_get_ieeechan_for_agilecac(temp_dfs,
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
		adfs_param.precac_chan = ch_freq;
		adfs_param.precac_chwidth = temp_dfs->dfs_precac_chwidth;
		dfs_start_agile_precac_timer(temp_dfs,
					     dfs->dfs_soc_obj->ocac_status,
					     &adfs_param);
		qdf_info("%s : %d ADFS channel set request sent for pdev: %pK ch_freq: %d",
			 __func__, __LINE__, pdev, ch_freq);
		if (dfs_tx_ops && dfs_tx_ops->dfs_agile_ch_cfg_cmd)
			dfs_tx_ops->dfs_agile_ch_cfg_cmd(pdev, &adfs_param);
		else
			dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"dfs_tx_ops=%pK", dfs_tx_ops);
	} else {
		dfs->dfs_soc_obj->precac_state_started = false;
		qdf_info("No channels in preCAC required list");
	}
}
#endif

/* dfs_is_tree_node_marked_as_cac() - Check if preCAC BSTree node is
 * marked as CAC.
 * @root: Pointer to root node of the preCAC BSTree.
 * @channel: 20MHz channel to be checked if marked as CAC done already.
 *
 * Return: True if already marked, else false.
 */
static bool
dfs_is_tree_node_marked_as_cac(struct precac_tree_node *root,
				  uint8_t channel)
{
	struct precac_tree_node *curr_node = root;

	while (curr_node) {
		if (!curr_node->n_caced_subchs)
			return false;
		if (curr_node->ch_ieee == channel)
			return curr_node->n_caced_subchs;
		curr_node = dfs_descend_precac_tree(curr_node, channel);
	}
	return false;
}

/* dfs_mark_tree_node_as_cac_done() - Mark the preCAC BSTree node as CAC done.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac_list entry pointer.
 * @channel:      IEEE channel to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_mark_tree_node_as_cac_done(struct wlan_dfs *dfs,
			       struct dfs_precac_entry *precac_entry,
			       uint8_t channel)
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
	if (dfs_is_tree_node_marked_as_cac(curr_node, channel))
		return;

	while (curr_node) {
		 if (curr_node->n_caced_subchs <
		     N_SUBCHS_FOR_BANDWIDTH(curr_node->bandwidth))
			curr_node->n_caced_subchs++;
		curr_node = dfs_descend_precac_tree(curr_node, channel);
	}
}

/* dfs_unmark_tree_node_as_cac_done() - Unmark the preCAC BSTree node as CAC
 *                                      done.
 * @precac_entry: Precac_list entry pointer.
 * @channel:      IEEE channel to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_unmark_tree_node_as_cac_done(struct dfs_precac_entry *precac_entry,
				 uint8_t channel)
{
	struct precac_tree_node *curr_node = precac_entry->tree_root;

	while (curr_node) {
		if (curr_node->n_caced_subchs)
			curr_node->n_caced_subchs--;
		else
			return;
		curr_node = dfs_descend_precac_tree(curr_node, channel);
	}
}

void dfs_mark_precac_done(struct wlan_dfs *dfs,
			  uint8_t pri_ch_ieee,
			  uint8_t sec_ch_ieee,
			  enum phy_ch_width ch_width)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	uint8_t channels[NUM_CHANNELS_160MHZ];
	uint8_t i, nchannels = 0;

	if (!pri_ch_ieee)
		return;
	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		nchannels = 1;
		channels[0] = pri_ch_ieee;
		break;
	case CH_WIDTH_40MHZ:
		nchannels = 2;
		channels[0] = pri_ch_ieee - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[1] = pri_ch_ieee + DFS_5GHZ_NEXT_CHAN_OFFSET;
		break;
	case CH_WIDTH_80MHZ:
		nchannels = 4;
		channels[0] = pri_ch_ieee - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[1] = pri_ch_ieee - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[2] = pri_ch_ieee + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[3] = pri_ch_ieee + DFS_5GHZ_2ND_CHAN_OFFSET;
		break;
	case CH_WIDTH_80P80MHZ:
		nchannels = 8;
		channels[0] = pri_ch_ieee - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[1] = pri_ch_ieee - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[2] = pri_ch_ieee + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[3] = pri_ch_ieee + DFS_5GHZ_2ND_CHAN_OFFSET;
		/* secondary channels */
		channels[4] = sec_ch_ieee - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[5] = sec_ch_ieee - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[6] = sec_ch_ieee + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[7] = sec_ch_ieee + DFS_5GHZ_2ND_CHAN_OFFSET;
		break;
	case CH_WIDTH_160MHZ:
		nchannels = 8;
		channels[0] = pri_ch_ieee - DFS_5GHZ_4TH_CHAN_OFFSET;
		channels[1] = pri_ch_ieee - DFS_5GHZ_3RD_CHAN_OFFSET;
		channels[2] = pri_ch_ieee - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[3] = pri_ch_ieee - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[4] = pri_ch_ieee + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[5] = pri_ch_ieee + DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[6] = pri_ch_ieee + DFS_5GHZ_3RD_CHAN_OFFSET;
		channels[7] = pri_ch_ieee + DFS_5GHZ_4TH_CHAN_OFFSET;
		break;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "invalid channel width");
		return;
	}

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
					    precac_entry->vht80_ch_ieee,
					    VHT80_OFFSET)) {
				dfs_mark_tree_node_as_cac_done(dfs,
							       precac_entry,
							       channels[i]);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
}

/* dfs_mark_tree_node_as_nol() - Mark the preCAC BSTree node as NOL.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac_list entry pointer.
 * @channel:      IEEE channel to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_mark_tree_node_as_nol(struct wlan_dfs *dfs,
			  struct dfs_precac_entry *precac_entry,
			  uint8_t channel)
{
	struct precac_tree_node *curr_node;

	if (!precac_entry->tree_root) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Precac tree root pointer is NULL!");
		return;
	}
	curr_node = precac_entry->tree_root;
	while (curr_node) {
		if (curr_node->n_nol_subchs <
		    N_SUBCHS_FOR_BANDWIDTH(curr_node->bandwidth)) {
			curr_node->n_nol_subchs++;
		} else {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Radarfound on an already marked NOL channel!");
			return;
		}
		if (channel == curr_node->ch_ieee) {
			if (curr_node->n_caced_subchs) {
				/* remove cac done status for this node
				 * and it's parents, since this node
				 * now requires cac (after NOL expiry)
				 */
				dfs_unmark_tree_node_as_cac_done(precac_entry,
								 channel);
			}
		}
		curr_node = dfs_descend_precac_tree(curr_node, channel);
	}
}

/* dfs_unmark_tree_node_as_nol() - Unmark the preCAC BSTree node as NOL.
 * @dfs:          Pointer to WLAN DFS structure.
 * @precac_entry: Precac_list entry pointer.
 * @channel:      IEEE channel to be marked.
 *
 * Note: The input channel is always of 20MHz bandwidth.
 */
static void
dfs_unmark_tree_node_as_nol(struct wlan_dfs *dfs,
			    struct dfs_precac_entry *precac_entry,
			    uint8_t channel)
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
		curr_node = dfs_descend_precac_tree(curr_node, channel);
	}
}

void dfs_unmark_precac_nol(struct wlan_dfs *dfs, uint8_t channel)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	uint8_t pri_ch_ieee = 0, chwidth_80 = DFS_CHWIDTH_80_VAL;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		TAILQ_FOREACH_SAFE(precac_entry, &dfs->dfs_precac_list,
				   pe_list, tmp_precac_entry) {
			if (IS_WITHIN_RANGE(channel,
					    precac_entry->vht80_ch_ieee,
					    VHT80_OFFSET)) {
				dfs_unmark_tree_node_as_nol(dfs, precac_entry,
							    channel);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	/* If preCAC / agile CAC is not running, restart the timer
	 * to check if the NOL expired channels can be CACed again.
	 */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "NOL expired for channel %u, trying to start preCAC",
		 channel);
	if (!dfs->dfs_soc_obj->dfs_precac_timer_running) {
		if (dfs->dfs_precac_enable) {
			if (dfs_is_ap_cac_timer_running(dfs)) {
				dfs->dfs_defer_precac_channel_change = 1;
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					  "Primary CAC is running, deferred"
					  );
			} else if (WLAN_IS_CHAN_11AC_VHT80(dfs->dfs_curchan)) {
				pri_ch_ieee = dfs->dfs_curchan->
						dfs_ch_vhtop_ch_freq_seg1;

				/* Check if there is a new channel to preCAC
				 * and only then do vdev restart.
				 */
				if (!dfs_get_ieeechan_for_precac(dfs,
								 pri_ch_ieee,
								 0,
								 chwidth_80))
					return;
				dfs_mlme_channel_change_by_precac(
						dfs->dfs_pdev_obj);
			}
		} else if (dfs->dfs_agile_precac_enable &&
			   !dfs->dfs_soc_obj->precac_state_started) {
			/* precac_state_started will be set to false if
			 * agile CAC is not begun for any channels or
			 * all channels were CACed. If it's not set, defer
			 * changing the current Agile CAC channel.
			 */
			dfs_prepare_agile_precac_chan(dfs);
		}
	}
}

void dfs_mark_precac_nol(struct wlan_dfs *dfs,
			 uint8_t is_radar_found_on_secondary_seg,
			 uint8_t detector_id,
			 uint8_t *channels,
			 uint8_t num_channels)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	struct wlan_objmgr_psoc *psoc;
	uint8_t i;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_soc_obj = dfs->dfs_soc_obj;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "is_radar_found_on_secondary_seg = %u subchannel_marking = %u detector_id = %u",
		  is_radar_found_on_secondary_seg,
		  dfs->dfs_use_nol_subchannel_marking,
		  detector_id);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "agile detector ieee = %u primary_ieee = %u secondary_ieee = %u",
		  dfs->dfs_agile_precac_freq,
		  dfs->dfs_precac_secondary_freq,
		  dfs->dfs_precac_primary_freq);

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
			if (IS_WITHIN_RANGE(channels[i],
					    precac_entry->vht80_ch_ieee,
					    VHT80_OFFSET)) {
				dfs_mark_tree_node_as_nol(dfs,
							  precac_entry,
							  channels[i]);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	/* PreCAC timer is not running, no need to restart preCAC */
	if (!dfs_soc_obj->dfs_precac_timer_running)
		return;

	if (dfs->dfs_precac_enable) {
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
	} else if (dfs->dfs_agile_precac_enable) {
		/* If preCAC is not running on the DFS where radar is detected,
		 * no need to configure agile channel.
		 * Return from this function.
		 */
		if (!(dfs_soc_obj->cur_precac_dfs_index == dfs->dfs_psoc_idx)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "preCAC not running on radarfound DFS idx=%d",
				  dfs->dfs_psoc_idx);
			return;
		}

		qdf_timer_sync_cancel(&dfs_soc_obj->dfs_precac_timer);
		dfs_soc_obj->dfs_precac_timer_running = 0;
		/*
		 * If radar is found on agile engine, change the channel here
		 * since primary channel change will not be triggered.
		 * If radar is found on primary detector, let agile
		 * channel change be triggered after start response.
		 * Set precac_state_started to false to indicate preCAC is not
		 * running.
		 */
		if (detector_id == AGILE_DETECTOR_ID)
			dfs_prepare_agile_precac_chan(dfs);
		else
			dfs_soc_obj->precac_state_started = false;
	}
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_process_ocac_complete(struct wlan_objmgr_pdev *pdev,
			       uint32_t ocac_status,
			       uint32_t center_freq)
{
	struct wlan_dfs *dfs = NULL;
	struct dfs_agile_cac_params adfs_param;

	dfs = wlan_pdev_get_dfs_obj(pdev);

	/* STOP TIMER irrespective of status */
	utils_dfs_cancel_precac_timer(pdev);
	if (ocac_status == OCAC_RESET) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer reset, Sending Agile chan set command");
		dfs_prepare_agile_precac_chan(dfs);
	} else if (ocac_status == OCAC_CANCEL) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer abort, agile precac stopped");
	} else if (ocac_status == OCAC_SUCCESS) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer Completed for agile freq: %d",
			  center_freq);
		/*
		 * TRIGGER agile precac timer with 0sec timeout
		 * with ocac_status 0 for old pdev
		 */
		adfs_param.precac_chan = center_freq;
		adfs_param.precac_chwidth = dfs->dfs_precac_chwidth;
		dfs_start_agile_precac_timer(dfs,
					     ocac_status,
					     &adfs_param);
	} else {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS, "Error Unknown");
	}
}
#endif

bool dfs_is_precac_timer_running(struct wlan_dfs *dfs)
{
	return dfs->dfs_soc_obj->dfs_precac_timer_running ? true : false;
}

#define VHT80_IEEE_FREQ_OFFSET 6
void dfs_find_precac_secondary_vht80_chan(struct wlan_dfs *dfs,
		struct dfs_channel *chan)
{
	uint8_t first_primary_dfs_ch_ieee;

	first_primary_dfs_ch_ieee =
		dfs->dfs_precac_secondary_freq - VHT80_IEEE_FREQ_OFFSET;

	dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
			first_primary_dfs_ch_ieee, 0,
			WLAN_PHYMODE_11AC_VHT80,
			&(chan->dfs_ch_freq),
			&(chan->dfs_ch_flags),
			&(chan->dfs_ch_flagext),
			&(chan->dfs_ch_ieee),
			&(chan->dfs_ch_vhtop_ch_freq_seg1),
			&(chan->dfs_ch_vhtop_ch_freq_seg2));
}

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
void dfs_precac_csa(struct wlan_dfs *dfs)
{
	/*
	 * Save current chan into intermediate chan, so that
	 * next time a DFS channel needs preCAC, there is no channel switch
	 * until preCAC finishes.
	 */
	dfs->dfs_precac_inter_chan = dfs->dfs_autoswitch_des_chan;
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Use %d as intermediate channel for further channel changes",
		  dfs->dfs_precac_inter_chan);

	if (NULL != global_dfs_to_mlme.mlme_precac_chan_change_csa)
		global_dfs_to_mlme.mlme_precac_chan_change_csa(dfs->dfs_pdev_obj,
							       dfs->dfs_autoswitch_des_chan,
							       dfs->dfs_autoswitch_des_mode);
	dfs->dfs_autoswitch_des_chan = 0;
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
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
		dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					    dfs->dfs_autoswitch_des_chan, 0,
					    dfs->dfs_autoswitch_des_mode,
					    &chan.dfs_ch_freq,
					    &chan.dfs_ch_flags,
					    &chan.dfs_ch_flagext,
					    &chan.dfs_ch_ieee,
					    &chan.dfs_ch_vhtop_ch_freq_seg1,
					    &chan.dfs_ch_vhtop_ch_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Channel %d not found for mode %d",
			dfs->dfs_autoswitch_des_chan,
			dfs->dfs_autoswitch_des_mode);
		return false;
	}
	/*
	 * If desired channel is in precac done list,
	 * Change channel to desired channel using CSA.
	 */
	if (dfs->dfs_autoswitch_des_chan && dfs_is_precac_done(dfs, &chan)) {
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

	OS_GET_TIMER_ARG(dfs_soc_obj, struct dfs_soc_priv_obj *);

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_precac_dfs_index].dfs;
	dfs_soc_obj->dfs_precac_timer_running = 0;

	if (!dfs->dfs_agile_precac_enable) {
		/*
		 * Remove the HT80 freq from the precac-required-list
		 * and add it to the precac-done-list
		 */
		dfs_mark_precac_done(dfs, dfs->dfs_precac_secondary_freq, 0,
				     dfs->dfs_precac_chwidth);
		current_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Pre-cac expired, Precac Secondary chan %u curr time %d",
			  dfs->dfs_precac_secondary_freq,
			  (current_time) / 1000);
		/*
		 * Do vdev restart so that we can change
		 * the secondary VHT80 channel.
		 */

		/* check if CAC done on home channel */
		is_cac_done_on_des_chan = dfs_precac_check_home_chan_change(dfs);
		if (!is_cac_done_on_des_chan) {
			/*
			 * Use same home channel, only change preCAC channel.
			 */

			/*
			 * TO BE DONE xxx : Need to lock the channel change.
			 */
			dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
		}
	} else {
		current_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Pre-cac expired, Agile Precac chan %u curr time %d",
			 dfs->dfs_agile_precac_freq,
			 current_time / 1000);
		if (dfs_soc_obj->ocac_status == OCAC_SUCCESS) {
			dfs_soc_obj->ocac_status = OCAC_RESET;
			dfs_mark_precac_done(dfs,
					     dfs->dfs_agile_precac_freq,
					     0,
					     dfs->dfs_precac_chwidth);
		}
		/* check if CAC done on home channel */
		is_cac_done_on_des_chan = dfs_precac_check_home_chan_change(dfs);
		if (!is_cac_done_on_des_chan) {
			/*
			 * Use same home channel, only change preCAC channel.
			 */

			/*
			 * TO BE DONE xxx : Need to lock the channel change.
			 */
			dfs_prepare_agile_precac_chan(dfs);
		}
	}
}

void dfs_zero_cac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	dfs_soc_obj->precac_state_started = false;
	qdf_timer_init(NULL, &dfs_soc_obj->dfs_precac_timer,
		       dfs_precac_timeout,
		       (void *)dfs_soc_obj,
		       QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_zero_cac_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_precac_timeout_override = -1;
	PRECAC_LIST_LOCK_CREATE(dfs);
}

/* dfs_init_precac_tree_node() - Initialise the preCAC BSTree node with the
 *                               provided values.
 * @node:      Precac_tree_node to be filled.
 * @chan:      IEEE channel value.
 * @bandwidth: Bandwidth of the channel.
 */
static inline void dfs_init_precac_tree_node(struct precac_tree_node *node,
					     int chan,
					     uint8_t bandwidth)
{
	node->left_child = NULL;
	node->right_child = NULL;
	node->ch_ieee = (uint8_t)chan;
	node->n_caced_subchs = 0;
	node->n_nol_subchs = 0;
	node->n_valid_subchs = N_SUBCHS_FOR_BANDWIDTH(bandwidth);
	node->bandwidth = bandwidth;
}

/* dfs_insert_node_into_bstree() - Insert a new preCAC BSTree node.
 * @root:      The preCAC BSTree root pointer.
 * @chan:      IEEE channel value of the new node.
 * @bandwidth: Bandwidth of the channel.
 *
 * Return: EOK if new node is allocated, else return ENOMEM.
 */
static QDF_STATUS
dfs_insert_node_into_bstree(struct precac_tree_node **root,
			    int chan,
			    uint8_t bandwidth)
{
	struct precac_tree_node *new_node = NULL;
	struct precac_tree_node *curr_node, *prev_node = NULL;
	QDF_STATUS status = EOK;

	new_node = qdf_mem_malloc(sizeof(*new_node));
	if (!new_node)
		return -ENOMEM;
	dfs_init_precac_tree_node(new_node, chan, bandwidth);

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
		curr_node = dfs_descend_precac_tree(curr_node, chan);
	}

	/* Add to the leaf node */
	if (chan < prev_node->ch_ieee)
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
static QDF_STATUS
dfs_create_precac_tree(struct wlan_dfs *dfs,
		       struct dfs_precac_entry *precac_entry,
		       uint8_t ch_ieee)
{
	struct precac_tree_node *root = NULL;
	int chan, i, bandwidth = DFS_CHWIDTH_80_VAL;
	QDF_STATUS status = EOK;
	static const int initial_and_next_offsets[TREE_DEPTH][N_OFFSETS] = {
		{INITIAL_80_CHAN_OFFSET, NEXT_80_CHAN_OFFSET},
		{INITIAL_40_CHAN_OFFSET, NEXT_40_CHAN_OFFSET},
		{INITIAL_20_CHAN_OFFSET, NEXT_20_CHAN_OFFSET}
	};

	for (i = 0; i < TREE_DEPTH; i++) {
		/* In offset array,
		 * column 0 is initial chan offset,
		 * column 1 is next chan offset.
		 * Boundary offset is initial offset and next offset
		 * of root level (since root level can have only 1 node)
		 */
		int offset = initial_and_next_offsets[i][START_INDEX];
		int step = initial_and_next_offsets[i][STEP_INDEX];
		uint8_t top_lvl_step = NEXT_80_CHAN_OFFSET;
		int boundary_offset = offset + top_lvl_step;

		for (; offset < boundary_offset; offset += step) {
			chan = (int)ch_ieee + offset;
			status = dfs_insert_node_into_bstree(&root,
							     chan,
							     bandwidth);
			if (status)
				return status;
		}
		bandwidth /= 2;
	}

	precac_entry->tree_root = root;
	return status;
}

void dfs_init_precac_list(struct wlan_dfs *dfs)
{
	u_int i;
	uint8_t found;
	struct dfs_precac_entry *tmp_precac_entry;
	int nchans = 0;
	QDF_STATUS status;

	/* Right now, only ETSI domain supports preCAC. Check if current
	 * DFS domain is ETSI and only then build the preCAC list.
	 */
	if (utils_get_dfsdomain(dfs->dfs_pdev_obj) != DFS_ETSI_DOMAIN)
		return;

	/*
	 * We need to prepare list of uniq VHT80 center frequencies. But at the
	 * beginning we do not know how many uniq frequencies are present.
	 * Therefore, we calculate the MAX size and allocate a temporary
	 * list/array. However we fill the temporary array with uniq frequencies
	 * and copy the uniq list of frequencies to the final list with exact
	 * size.
	 */
	TAILQ_INIT(&dfs->dfs_precac_list);
	dfs_mlme_get_dfs_ch_nchans(dfs->dfs_pdev_obj, &nchans);

	PRECAC_LIST_LOCK(dfs);
	/* Fill the  precac_list with unique elements */
	for (i = 0; i < nchans; i++) {
		struct dfs_channel *ichan = NULL, lc;
		uint8_t pri_cntr_chan = 0;

		ichan = &lc;
		dfs_mlme_get_dfs_ch_channels(dfs->dfs_pdev_obj,
				&(ichan->dfs_ch_freq),
				&(ichan->dfs_ch_flags),
				&(ichan->dfs_ch_flagext),
				&(ichan->dfs_ch_ieee),
				&(ichan->dfs_ch_vhtop_ch_freq_seg1),
				&(ichan->dfs_ch_vhtop_ch_freq_seg2),
				i);
		pri_cntr_chan = ichan->dfs_ch_vhtop_ch_freq_seg1;

		if (WLAN_IS_CHAN_11AC_VHT80(ichan) &&
				WLAN_IS_CHAN_DFS(ichan)) {
			found = 0;
			TAILQ_FOREACH(tmp_precac_entry,
					&dfs->dfs_precac_list,
					pe_list) {
				if (tmp_precac_entry->vht80_ch_ieee ==
						pri_cntr_chan) {
					found = 1;
					break;
				}
			}
			if (!found && pri_cntr_chan) {
				struct dfs_precac_entry *precac_entry;

				precac_entry = qdf_mem_malloc(
					sizeof(*precac_entry));
				if (!precac_entry) {
					dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
						"entry alloc fail for : %d", i);
					continue;
				}
				precac_entry->vht80_ch_ieee = pri_cntr_chan;
				precac_entry->dfs = dfs;
				status = dfs_create_precac_tree(dfs,
								precac_entry,
								pri_cntr_chan);
				if (status) {
					dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
						"tree_node alloc failed");
					continue;
				}
				TAILQ_INSERT_TAIL(
						&dfs->dfs_precac_list,
						precac_entry, pe_list);
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Print the list of VHT80 frequencies from linked list");
	TAILQ_FOREACH(tmp_precac_entry,
			&dfs->dfs_precac_list,
			pe_list)
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "freq=%u",
				tmp_precac_entry->vht80_ch_ieee);
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

	dfs_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							    WLAN_UMAC_COMP_DFS);
	dfs->dfs_psoc_idx = dfs_soc_obj->num_dfs_privs;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "dfs->dfs_psoc_idx: %d ", dfs->dfs_psoc_idx);
	dfs_soc_obj->dfs_priv[dfs_soc_obj->num_dfs_privs].dfs = dfs;
	dfs_soc_obj->num_dfs_privs++;
	dfs->dfs_soc_obj = dfs_soc_obj;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_soc_obj->num_dfs_privs: %d ",
		 dfs_soc_obj->num_dfs_privs);
}
#endif

void dfs_zero_cac_detach(struct wlan_dfs *dfs)
{
	dfs_deinit_precac_list(dfs);
	PRECAC_LIST_LOCK_DESTROY(dfs);
}

/* dfs_is_cac_needed_for_bst_node() - For a requested bandwidth, find
 *                                    if the current preCAC BSTree node needs
 *                                    CAC.
 * @node:          Node to be checked.
 * @req_bandwidth: bandwidth of channel requested.
 *
 * Return: TRUE/FALSE.
 * Return true if there exists a channel of the requested bandwidth
 * for the node which is not CAC done, else false.
 */
static bool
dfs_is_cac_needed_for_bst_node(struct precac_tree_node *node,
			       uint8_t req_bandwidth)
{
	uint8_t n_subchs_for_req_bw, non_nol_subchs;

	if (!node)
		return false;

	/* Find the number of subchannels for the requested bandwidth */
	n_subchs_for_req_bw = N_SUBCHS_FOR_BANDWIDTH(req_bandwidth);
	non_nol_subchs = node->n_valid_subchs - node->n_nol_subchs;

	/* Return false if,
	 * 1. Number of non-NOL subchannels in the current node is less than
	 *    the requested number of subchannels.
	 * 2. All the subchannels of the node are CAC done.
	 * 3. If the number CAC done subchannels + NOL subchannels in the
	 *    current node is equal to number of valid subchannels in the node.
	 * else, return true.
	 */
	if ((non_nol_subchs < n_subchs_for_req_bw) ||
	    (node->n_caced_subchs == node->n_valid_subchs) ||
	    (node->n_caced_subchs + node->n_nol_subchs == node->n_valid_subchs))
		return false;

	return true;
}

/* dfs_find_ieee_ch_from_precac_tree() - from the given preCAC tree, find a IEEE
 *                                       channel of the given bandwidth which
 *                                       is valid and needs CAC.
 * @root:   PreCAC BSTree root pointer.
 * @req_bw: Bandwidth of channel requested.
 *
 * Return: IEEE channel number.
 * Return a valid IEEE value which needs CAC for the given bandwidth, else
 * return 0.
 */
static uint8_t
dfs_find_ieee_ch_from_precac_tree(struct precac_tree_node *root,
				  uint8_t req_bw)
{
	struct precac_tree_node *curr_node;

	if (!dfs_is_cac_needed_for_bst_node(root, req_bw))
		return 0;

	curr_node = root;
	while (curr_node) {
		if (curr_node->bandwidth == req_bw) {
			/* find if current node in valid state (req.) */
			if (dfs_is_cac_needed_for_bst_node(curr_node, req_bw))
				return curr_node->ch_ieee;
			else
				return 0;
		}

		/* Find if we need to go to left or right subtree.
		 * Note: If both are available, go to left.
		 */
		if (!dfs_is_cac_needed_for_bst_node(curr_node->left_child,
						    req_bw))
			curr_node = curr_node->right_child;
		else
			curr_node = curr_node->left_child;
	}
	/* If requested bandwidth is invalid, return 0 here */
	return 0;
}

uint8_t dfs_get_ieeechan_for_precac(struct wlan_dfs *dfs,
				    uint8_t exclude_pri_ch_ieee,
				    uint8_t exclude_sec_ch_ieee,
				    uint8_t bandwidth)
{
	struct dfs_precac_entry *precac_entry;
	struct precac_tree_node *root = NULL;
	uint8_t ieee_chan = 0;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "current operating channel(s) to be excluded = [%u] [%u]",
		 exclude_pri_ch_ieee,
		 exclude_sec_ch_ieee);

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_list)) {
		TAILQ_FOREACH(precac_entry, &dfs->dfs_precac_list,
			      pe_list) {
			root = precac_entry->tree_root;
			ieee_chan =
				dfs_find_ieee_ch_from_precac_tree(root,
								  bandwidth);
			if (ieee_chan &&
			    (ieee_chan != exclude_pri_ch_ieee) &&
			    (ieee_chan != exclude_sec_ch_ieee))
				break;
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "Channel picked for preCAC = %u",
		 ieee_chan);

	return ieee_chan;
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
#define FIND_IF_OVERLAP_WITH_WEATHER_RANGE(first_ch, last_ch) \
((first_ch <= WEATHER_CHAN_END) && (WEATHER_CHAN_START <= last_ch))

/* dfs_is_precac_on_weather_channel() - Given a channel number, find if
 * it's a weather radar channel.
 * @dfs: Pointer to WLAN_DFS structure.
 * @chwidth: PreCAC channel width enum.
 * @precac_chan: Channel for preCAC.
 *
 * Based on the precac_width, find the first and last subchannels of the given
 * preCAC channel and check if this range overlaps with weather channel range.
 *
 * Return: True if weather channel, else false.
 */
static bool dfs_is_precac_on_weather_channel(struct wlan_dfs *dfs,
					     enum phy_ch_width chwidth,
					     uint8_t precac_chan)
{
	uint8_t first_subch, last_subch;

	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		first_subch = precac_chan;
		last_subch = precac_chan;
		break;
	case CH_WIDTH_40MHZ:
		first_subch = precac_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		last_subch = precac_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
		break;
	case CH_WIDTH_80MHZ:
		first_subch = precac_chan - DFS_5GHZ_2ND_CHAN_OFFSET;
		last_subch = precac_chan + DFS_5GHZ_2ND_CHAN_OFFSET;
		break;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Precac channel width invalid!");
		return false;
	}
	return FIND_IF_OVERLAP_WITH_WEATHER_RANGE(first_subch, last_subch);
}

void dfs_start_agile_precac_timer(struct wlan_dfs *dfs,
				  uint8_t ocac_status,
				  struct dfs_agile_cac_params *adfs_param)
{
	uint8_t precac_chan = adfs_param->precac_chan;
	enum phy_ch_width chwidth = adfs_param->precac_chwidth;
	uint32_t min_precac_timeout, max_precac_timeout;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs_soc_obj->dfs_precac_timer_running = 1;

	if (ocac_status == OCAC_SUCCESS) {
		dfs_soc_obj->ocac_status = OCAC_SUCCESS;
		min_precac_timeout = 0;
		max_precac_timeout = 0;
	} else {
		/* Find the minimum and maximum precac timeout. */
		max_precac_timeout = MAX_PRECAC_DURATION;
		if (dfs->dfs_precac_timeout_override != -1) {
			min_precac_timeout =
				dfs->dfs_precac_timeout_override * 1000;
		} else if (dfs_is_precac_on_weather_channel(dfs,
							    chwidth,
							    precac_chan)) {
			min_precac_timeout = MIN_WEATHER_PRECAC_DURATION;
			max_precac_timeout = MAX_WEATHER_PRECAC_DURATION;
		} else {
			min_precac_timeout = MIN_PRECAC_DURATION;
		}
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "precactimeout = %d ms", (min_precac_timeout));
	qdf_timer_mod(&dfs_soc_obj->dfs_precac_timer, min_precac_timeout);
	adfs_param->min_precac_timeout = min_precac_timeout;
	adfs_param->max_precac_timeout = max_precac_timeout;
}
#endif

void dfs_start_precac_timer(struct wlan_dfs *dfs,
			    uint8_t precac_chan)
{
	struct dfs_channel *ichan, lc;
	uint8_t first_primary_dfs_ch_ieee;
	int primary_cac_timeout;
	int secondary_cac_timeout;
	int precac_timeout;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_precac_dfs_index].dfs;
#define EXTRA_TIME_IN_SEC 5
	dfs_soc_obj->dfs_precac_timer_running = 1;

	/*
	 * Get the first primary ieee chan in the HT80 band and find the channel
	 * pointer.
	 */
	first_primary_dfs_ch_ieee = precac_chan - VHT80_IEEE_FREQ_OFFSET;

	primary_cac_timeout = dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
			dfs->dfs_curchan->dfs_ch_freq,
			dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
			dfs->dfs_curchan->dfs_ch_flags);

	ichan = &lc;
	dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
			first_primary_dfs_ch_ieee, 0,
			WLAN_PHYMODE_11AC_VHT80,
			&(ichan->dfs_ch_freq),
			&(ichan->dfs_ch_flags),
			&(ichan->dfs_ch_flagext),
			&(ichan->dfs_ch_ieee),
			&(ichan->dfs_ch_vhtop_ch_freq_seg1),
			&(ichan->dfs_ch_vhtop_ch_freq_seg2));

	secondary_cac_timeout = (dfs->dfs_precac_timeout_override != -1) ?
		dfs->dfs_precac_timeout_override :
		dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
				ichan->dfs_ch_freq,
				ichan->dfs_ch_vhtop_ch_freq_seg2,
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
	 * that etsi_precac/cac is done on primary segment.
	 */
	if (WLAN_IS_CHAN_DFS(dfs->dfs_curchan) &&
	    !dfs_is_etsi_precac_done(dfs) &&
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
#define MAX_PREFIX_CHAR 20
static void dfs_print_node_data(struct wlan_dfs *dfs,
				struct precac_tree_node *node)
{
	char prefix[MAX_PREFIX_CHAR] = "";
	char prev_line_prefix[MAX_PREFIX_CHAR] = "";
	char inv[4] = "inv";

	switch (node->bandwidth) {
	case DFS_CHWIDTH_80_VAL:
		break;
	case DFS_CHWIDTH_40_VAL:
		qdf_str_lcopy(prev_line_prefix, "|", MAX_PREFIX_CHAR);
		qdf_str_lcopy(prefix, "|------- ", MAX_PREFIX_CHAR);
		break;
	case DFS_CHWIDTH_20_VAL:
		qdf_str_lcopy(prev_line_prefix, "|        |", MAX_PREFIX_CHAR);
		qdf_str_lcopy(prefix, "|        |------- ", MAX_PREFIX_CHAR);
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

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
void dfs_set_precac_preferred_channel(struct wlan_dfs *dfs,
				      struct dfs_channel *chan, uint8_t mode)
{
	bool found = false;
	uint8_t freq_160_sec = 0;
	struct dfs_precac_entry *precac_entry;

	if (dfs_is_precac_timer_running(dfs) &&
	    WLAN_IS_CHAN_MODE_80(chan) &&
	    (dfs->dfs_precac_secondary_freq == chan->dfs_ch_freq)) {
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
			if (precac_entry->vht80_ch_ieee ==
			    chan->dfs_ch_vhtop_ch_freq_seg1) {
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
		if (chan->dfs_ch_ieee < chan->dfs_ch_vhtop_ch_freq_seg2)
			freq_160_sec = chan->dfs_ch_vhtop_ch_freq_seg1 +
				       VHT160_IEEE_FREQ_DIFF;
		else
			freq_160_sec = chan->dfs_ch_vhtop_ch_freq_seg1 -
				       VHT160_IEEE_FREQ_DIFF;

		found = false;
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_list, pe_list) {
			if (precac_entry->vht80_ch_ieee ==
			    freq_160_sec) {
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
dfs_decide_precac_preferred_chan(struct wlan_dfs *dfs,
				 uint8_t *pref_chan,
				 enum wlan_phymode mode)
{
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					*pref_chan, 0,
					mode,
					&chan.dfs_ch_freq,
					&chan.dfs_ch_flags,
					&chan.dfs_ch_flagext,
					&chan.dfs_ch_ieee,
					&chan.dfs_ch_vhtop_ch_freq_seg1,
					&chan.dfs_ch_vhtop_ch_freq_seg2))
		return false;
	if (!dfs->dfs_precac_inter_chan)
		return false;

	/*
	 * If precac is done on this channel use it, else use a intermediate
	 * non-DFS channel and trigger a precac on this channel.
	 */
	if ((WLAN_IS_CHAN_DFS(&chan) ||
	    (WLAN_IS_CHAN_MODE_160(&chan) &&
	     WLAN_IS_CHAN_DFS_CFREQ2(&chan))) &&
	    !dfs_is_precac_done(dfs, &chan)) {
		dfs_set_precac_preferred_channel(dfs, &chan, mode);
		dfs->dfs_autoswitch_des_chan = *pref_chan;
		dfs->dfs_autoswitch_des_mode = mode;
		*pref_chan = dfs->dfs_precac_inter_chan;
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "des_chan=%d, des_mode=%d. Current operating channel=%d",
			  dfs->dfs_autoswitch_des_chan,
			  dfs->dfs_autoswitch_des_mode,
			  *pref_chan);
		return true;
	}

	dfs->dfs_precac_inter_chan = chan.dfs_ch_ieee;
	return false;
}

enum precac_chan_state
dfs_get_precac_chan_state(struct wlan_dfs *dfs, uint8_t precac_chan)
{
	struct dfs_channel chan;
	struct dfs_precac_entry *tmp_precac_entry;
	struct precac_tree_node *root = NULL;
	enum precac_chan_state ret = PRECAC_ERR;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					precac_chan, 0,
					WLAN_PHYMODE_11AC_VHT80,
					&chan.dfs_ch_freq,
					&chan.dfs_ch_flags,
					&chan.dfs_ch_flagext,
					&chan.dfs_ch_ieee,
					&chan.dfs_ch_vhtop_ch_freq_seg1,
					&chan.dfs_ch_vhtop_ch_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Intermediate channel not found");
		return PRECAC_ERR;
	}

	if (!WLAN_IS_CHAN_DFS(&chan)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"[%d] Not a DFS channel", precac_chan);
		return PRECAC_ERR;
	}

	PRECAC_LIST_LOCK(dfs);
	if (dfs_is_precac_timer_running(dfs)) {
		tmp_precac_entry = TAILQ_FIRST(&dfs->dfs_precac_list);
		if (tmp_precac_entry && (tmp_precac_entry->vht80_ch_ieee ==
			chan.dfs_ch_vhtop_ch_freq_seg1)) {
			ret = PRECAC_NOW;
			goto end;
		}
	}

	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_precac_list, pe_list) {
		if (tmp_precac_entry->vht80_ch_ieee ==
		    chan.dfs_ch_vhtop_ch_freq_seg1) {
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

/* dfs_find_agile_width() - Given a channel width enum, find the corresponding
 *                          translation for Agile channel width.
 *                          Translation schema of different operating modes:
 *                          20 -> 20, 40 -> 40, (80 & 160 & 80_80) -> 80.
 * @dfs:     Pointer to WLAN DFS structure.
 * @chwidth: Channel width enum.
 *
 * Return: The translated channel width enum.
 */
static enum phy_ch_width
dfs_find_agile_width(struct wlan_dfs *dfs, enum phy_ch_width chwidth)
{
	switch (chwidth) {
	case CH_WIDTH_20MHZ:
		return CH_WIDTH_20MHZ;
	case CH_WIDTH_40MHZ:
		return CH_WIDTH_40MHZ;
	case CH_WIDTH_80MHZ:
	case CH_WIDTH_80P80MHZ:
	case CH_WIDTH_160MHZ:
		return CH_WIDTH_80MHZ;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "Invalid chwidth enum!");
		return CH_WIDTH_INVALID;
	}
}

void dfs_get_ieeechan_for_agilecac(struct wlan_dfs *dfs,
				   uint8_t *ch_ieee,
				   uint8_t pri_ch_ieee,
				   uint8_t sec_ch_ieee)
{
	uint8_t ieee_chan, chwidth_val;
	enum phy_ch_width chwidth = CH_WIDTH_INVALID;

	/*
	 * Agile detector's band of operation depends on current pdev.
	 * Find the current channel's width and apply the translate rules
	 * to find the Agile detector bandwidth.
	 * Translate rules (all numbers are in MHz) from current pdev's width
	 * to Agile detector's width:
	 * 20 - 20, 40 - 40, 80 - 80, 160 - 80, 160 (non contiguous) - 80.
	 */
	dfs_find_chwidth_and_center_chan(dfs, &chwidth, NULL, NULL);
	dfs->dfs_precac_chwidth = dfs_find_agile_width(dfs, chwidth);
	if (dfs->dfs_precac_chwidth == CH_WIDTH_INVALID) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "cannot start agile CAC!");
		return;
	}
	/* Find chwidth value for the given enum */
	chwidth_val = dfs_translate_chwidth_enum2val(dfs,
						     dfs->dfs_precac_chwidth);

	dfs->dfs_soc_obj->ocac_status = OCAC_RESET;
	ieee_chan = dfs_get_ieeechan_for_precac(dfs,
						pri_ch_ieee,
						sec_ch_ieee,
						chwidth_val);
	if (ieee_chan)
		dfs->dfs_agile_precac_freq = ieee_chan;
	else
		dfs->dfs_agile_precac_freq = 0;

	*ch_ieee = dfs->dfs_agile_precac_freq;
}
#endif

void dfs_find_vht80_chan_for_precac(struct wlan_dfs *dfs,
				    uint32_t chan_mode,
				    uint8_t ch_freq_seg1,
				    uint32_t *cfreq1,
				    uint32_t *cfreq2,
				    uint32_t *phy_mode,
				    bool *dfs_set_cfreq2,
				    bool *set_agile)
{
	uint8_t ieee_freq;
	uint8_t chwidth_val = DFS_CHWIDTH_80_VAL;

	if (chan_mode != WLAN_PHYMODE_11AC_VHT80)
		return;

	dfs->dfs_precac_chwidth = CH_WIDTH_80MHZ;
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "precac_secondary_freq = %u precac_running = %u",
		  dfs->dfs_precac_secondary_freq,
		  dfs->dfs_soc_obj->dfs_precac_timer_running);

	/*
	 * If Pre-CAC is enabled then find a center frequency for
	 * the secondary VHT80 and Change the mode to
	 * VHT80_80 or VHT160.
	 */
	if (dfs->dfs_precac_enable) {
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
			if (ch_freq_seg1 ==
					dfs->dfs_precac_secondary_freq)
				ieee_freq =
				dfs_get_ieeechan_for_precac(dfs,
							    ch_freq_seg1,
							    0,
							    chwidth_val);
			else
				ieee_freq = dfs->dfs_precac_secondary_freq;
		} else
			ieee_freq = dfs_get_ieeechan_for_precac(dfs,
								ch_freq_seg1,
								0,
								chwidth_val);
		if (ieee_freq) {
			if (ieee_freq == (ch_freq_seg1 +
					  VHT160_IEEE_FREQ_DIFF)) {
				/*
				 * Override the HW channel mode to
				 * VHT160
				 */
				uint8_t ieee_160_cfreq;

				ieee_160_cfreq = (ieee_freq + ch_freq_seg1) / 2;
				chan_mode = WLAN_PHYMODE_11AC_VHT160;
				*cfreq1 = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
							    ch_freq_seg1,
							    WLAN_CHAN_5GHZ);
				*cfreq2 = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
							    ieee_160_cfreq,
							    WLAN_CHAN_5GHZ);
			} else {
				/*
				 * Override the HW channel mode to
				 * VHT80_80.
				 */
				chan_mode = WLAN_PHYMODE_11AC_VHT80_80;
				*cfreq2 = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
							    ieee_freq,
							    WLAN_CHAN_5GHZ);
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
			if (dfs->dfs_precac_primary_freq == ch_freq_seg1)
				*set_agile = true;
			else
				*set_agile = false;

			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "cfreq1 = %u cfreq2 = %u ieee_freq = %u mode = %u set_agile = %d",
				  *cfreq1, *cfreq2, ieee_freq,
				  chan_mode, *set_agile);

			dfs->dfs_precac_secondary_freq = ieee_freq;
			dfs->dfs_precac_primary_freq = ch_freq_seg1;
			/* Start the pre_cac_timer */
			dfs_start_precac_timer(dfs,
					       dfs->dfs_precac_secondary_freq);
		} else {
			dfs->dfs_precac_secondary_freq = 0;
		} /* End of if(ieee_freq) */
	} /* End of if(dfs->dfs_precac_enable) */
}

void dfs_set_precac_enable(struct wlan_dfs *dfs, uint32_t value)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tx_ops;
	uint32_t target_type;
	struct target_psoc_info *tgt_hdl;
	struct tgt_info *info;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		dfs->dfs_precac_enable = 0;
		dfs->dfs_agile_precac_enable = 0;
		return;
	}

	tx_ops = &psoc->soc_cb.tx_ops.target_tx_ops;
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

	if ((1 == value) && tx_ops->tgt_is_tgt_type_qca9984(target_type) &&
	    (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)) {
		dfs->dfs_precac_enable = value;
	} else if ((1 == value) && (info->wlan_res_cfg.agile_capability == 1) &&
		(utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)) {
		dfs->dfs_agile_precac_enable = value;
	} else {
		dfs->dfs_agile_precac_enable = 0;
		dfs->dfs_precac_enable = 0;
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "preCAC disabled");
	}

	if (dfs_is_precac_timer_running(dfs)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Precac flag changed. Cancel the precac timer");
		dfs_cancel_precac_timer(dfs);
		dfs->dfs_soc_obj->precac_state_started = 0;
	}
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_agile_precac_start(struct wlan_dfs *dfs)
{
	struct dfs_agile_cac_params adfs_param;
	uint8_t ocac_status = 0;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	uint8_t cur_dfs_idx;

	dfs_soc_obj = dfs->dfs_soc_obj;

	qdf_info("%s : %d agile_precac_started: %d",
		 __func__, __LINE__,
		dfs_soc_obj->precac_state_started);

	if (!dfs_soc_obj->precac_state_started)
		dfs_soc_obj->cur_precac_dfs_index = dfs->dfs_psoc_idx;

	cur_dfs_idx = dfs_soc_obj->cur_precac_dfs_index;
	dfs_soc_obj->dfs_priv[cur_dfs_idx].agile_precac_active = true;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 " setting true to cur_precac_dfs_index = %d, dfs: %pK",
		 dfs_soc_obj->cur_precac_dfs_index,
		 dfs->dfs_soc_obj->dfs_priv[cur_dfs_idx].dfs);

	if (!dfs->dfs_soc_obj->precac_state_started) {
		/*
		 * Initiate first call to start preCAC here, for channel as 0,
		 * and ocac_status as 0
		 */
		adfs_param.precac_chan = 0;
		adfs_param.precac_chwidth = CH_WIDTH_INVALID;
		qdf_info("%s : %d Initiated agile precac",
			 __func__, __LINE__);
		dfs->dfs_soc_obj->precac_state_started = true;
		dfs_start_agile_precac_timer(dfs, ocac_status, &adfs_param);
	}
}
#endif

uint32_t dfs_get_precac_enable(struct wlan_dfs *dfs)
{
	return dfs->dfs_precac_enable;
}

bool dfs_get_agile_precac_enable(struct wlan_dfs *dfs)
{
	return dfs->dfs_agile_precac_enable;
}

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
int32_t dfs_set_precac_intermediate_chan(struct wlan_dfs *dfs, uint32_t value)
{
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					value, 0,
					WLAN_PHYMODE_11AC_VHT80,
					&chan.dfs_ch_freq,
					&chan.dfs_ch_flags,
					&chan.dfs_ch_flagext,
					&chan.dfs_ch_ieee,
					&chan.dfs_ch_vhtop_ch_freq_seg1,
					&chan.dfs_ch_vhtop_ch_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Intermediate channel not found");
		return -EINVAL;
	}

	/*Intermediate channel should be non-DFS channel*/
	if (!WLAN_IS_CHAN_DFS(&chan)) {
		dfs->dfs_precac_inter_chan = value;
	} else {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "intermediate channel %s",
			(chan.dfs_ch_ieee == value) ?
			"should not be DFS channel" : "is invalid");
		dfs->dfs_precac_inter_chan = 0;
		return -EINVAL;
	}

	return 0;
}

uint32_t dfs_get_precac_intermediate_chan(struct wlan_dfs *dfs)
{
	return dfs->dfs_precac_inter_chan;
}
#endif

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_reset_agile_config(struct dfs_soc_priv_obj *dfs_soc)
{
	dfs_soc->cur_precac_dfs_index = PCAC_DFS_INDEX_ZERO;
	dfs_soc->dfs_precac_timer_running = PCAC_TIMER_NOT_RUNNING;
	dfs_soc->precac_state_started = PRECAC_NOT_STARTED;
	dfs_soc->ocac_status = OCAC_SUCCESS;
}
#endif
