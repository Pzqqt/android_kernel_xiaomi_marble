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
 * DOC: This file has Precac Tree structure functions.
 *
 */

#include "../dfs_precac_forest.h"
#include <wlan_dfs_mlme_api.h>
#include <wlan_dfs_utils_api.h>
#include <dfs_process_radar_found_ind.h>
#include <dfs_internal.h>

/* dfs_calculate_bw_for_same_pri_ch() - When the primary channel is a new
 * channel initialize the center channel frequency and bandwidth and when the
 * primary is same as previous channel update the center frequency and the
 * bandwidth.
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
				 uint16_t chan_freq)
{
	if (!node)
		return NULL;

	if (chan_freq < node->ch_freq)
		return node->left_child;
	else
		return node->right_child;
}

/*dfs_insert_node_into_bstree_for_freq() - Insert a new preCAC BSTree node.
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
	 * caller API).
	 */
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

/**
 * dfs_find_precac_state_of_node() - Find the preCAC state of the given channel.
 * @channel: Channel whose preCAC state is to be found.
 * @precac_entry: PreCAC entry where the channel exists.
 *
 * Return, enum value of type precac_chan_state.
 */
enum precac_chan_state
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
	 * using the normal logic to find a channel for preCAC.
	 */
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

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
#ifdef CONFIG_CHAN_FREQ_API
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
#endif

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

/* dfs_mark_adfs_chan_as_cac_done()- Mark the ADFS CAC completed channel as
 *                                   CAC done in the precac tree.
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_mark_adfs_chan_as_cac_done(struct wlan_dfs *dfs)
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

void dfs_reset_precac_lists(struct wlan_dfs *dfs)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}
	dfs_reset_precaclists(dfs);
}

void dfs_reset_precaclists(struct wlan_dfs *dfs)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Reset precaclist of VHT80 frequencies");
	dfs_deinit_precac_list(dfs);
	dfs_init_precac_list(dfs);
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

#ifdef QCA_SUPPORT_ADFS_RCAC
/**
 * dfs_unmark_rcac_done() - Unmark the CAC done channels from the RCAC list.
 * @dfs: Pointer to wlan_dfs object.
 */
void dfs_unmark_rcac_done(struct wlan_dfs *dfs)
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
#endif
