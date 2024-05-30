/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/**
 *  DOC: wlan_mgmt_txrx_rx_reo.c
 *  This file contains mgmt rx re-ordering related function definitions
 */

#include "wlan_mgmt_txrx_rx_reo_i.h"
#include <wlan_mgmt_txrx_rx_reo_tgt_api.h>
#include "wlan_mgmt_txrx_main_i.h"
#include <qdf_util.h>

static struct mgmt_rx_reo_context g_rx_reo_ctx;

#define mgmt_rx_reo_get_context()        (&g_rx_reo_ctx)

#define MGMT_RX_REO_PKT_CTR_HALF_RANGE (0x8000)
#define MGMT_RX_REO_PKT_CTR_FULL_RANGE (MGMT_RX_REO_PKT_CTR_HALF_RANGE << 1)

/**
 * mgmt_rx_reo_compare_pkt_ctrs_gte() - Compare given mgmt packet counters
 * @ctr1: Management packet counter1
 * @ctr2: Management packet counter2
 *
 * We can't directly use the comparison operator here because the counters can
 * overflow. But these counters have a property that the difference between
 * them can never be greater than half the range of the data type.
 * We can make use of this condition to detect which one is actually greater.
 *
 * Return: true if @ctr1 is greater than or equal to @ctr2, else false
 */
static inline bool
mgmt_rx_reo_compare_pkt_ctrs_gte(uint16_t ctr1, uint16_t ctr2)
{
	uint16_t delta = ctr1 - ctr2;

	return delta <= MGMT_RX_REO_PKT_CTR_HALF_RANGE;
}

/**
 * mgmt_rx_reo_subtract_pkt_ctrs() - Subtract given mgmt packet counters
 * @ctr1: Management packet counter1
 * @ctr2: Management packet counter2
 *
 * We can't directly use the subtract operator here because the counters can
 * overflow. But these counters have a property that the difference between
 * them can never be greater than half the range of the data type.
 * We can make use of this condition to detect whichone is actually greater and
 * return the difference accordingly.
 *
 * Return: Difference between @ctr1 and @crt2
 */
static inline int
mgmt_rx_reo_subtract_pkt_ctrs(uint16_t ctr1, uint16_t ctr2)
{
	uint16_t delta = ctr1 - ctr2;

	/**
	 * if delta is greater than half the range (i.e, ctr1 is actually
	 * smaller than ctr2), then the result should be a negative number.
	 * subtracting the entire range should give the correct value.
	 */
	if (delta > MGMT_RX_REO_PKT_CTR_HALF_RANGE)
		return delta - MGMT_RX_REO_PKT_CTR_FULL_RANGE;

	return delta;
}

#define MGMT_RX_REO_GLOBAL_TS_HALF_RANGE (0x80000000)
/**
 * mgmt_rx_reo_compare_global_timestamps_gte()-Compare given global timestamps
 * @ts1: Global timestamp1
 * @ts2: Global timestamp2
 *
 * We can't directly use the comparison operator here because the timestamps can
 * overflow. But these timestamps have a property that the difference between
 * them can never be greater than half the range of the data type.
 * We can make use of this condition to detect which one is actually greater.
 *
 * Return: true if @ts1 is greater than or equal to @ts2, else false
 */
static inline bool
mgmt_rx_reo_compare_global_timestamps_gte(uint32_t ts1, uint32_t ts2)
{
	uint32_t delta = ts1 - ts2;

	return delta <= MGMT_RX_REO_GLOBAL_TS_HALF_RANGE;
}

/**
 * mgmt_rx_reo_is_stale_frame()- API to check whether the given management frame
 * is stale
 * @ts_last_released_frame: pointer to global time stamp of the last frame
 * removed from the reorder list
 * @global_timestamp: global time stamp of the management frame
 *
 * This API checks whether the management frame with global time stamp
 * @global_timestamp is stale. Any frame older than the last frame delivered to
 * upper layer is a stale frame. This could happen when we have to deliver
 * frames out of order due to time out or list size limit. The frames which
 * arrive late at host and with time stamp lesser than the last delivered frame
 * are stale frames and they need to be handled differently.
 *
 * Return: true if the given management frame is stale.
 */
static bool
mgmt_rx_reo_is_stale_frame(
		struct mgmt_rx_reo_global_ts_info *ts_last_released_frame,
		uint32_t global_timestamp)
{
	return ts_last_released_frame->valid &&
	       !mgmt_rx_reo_compare_global_timestamps_gte(global_timestamp,
					ts_last_released_frame->global_ts);
}

#ifndef WLAN_MGMT_RX_REO_SIM_SUPPORT
/**
 * mgmt_rx_reo_get_num_mlo_links() - Get number of MLO HW links from the reo
 * context object
 * @reo_context: Pointer to reo context object
 *
 * Return: On success returns number of MLO HW links. On failure
 * returns MGMT_RX_REO_INVALID_NUM_LINKS.
 */
static int8_t
mgmt_rx_reo_get_num_mlo_links(struct mgmt_rx_reo_context *reo_context) {
	if (!reo_context) {
		mgmt_rx_reo_err("Mgmt reo context is null");
		return MGMT_RX_REO_INVALID_NUM_LINKS;
	}

	return reo_context->num_mlo_links;
}

static QDF_STATUS
mgmt_rx_reo_handle_potential_premature_delivery(
				struct mgmt_rx_reo_context *reo_context,
				uint32_t global_timestamp)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
mgmt_rx_reo_handle_stale_frame(struct mgmt_rx_reo_list *reo_list,
			       struct mgmt_rx_reo_params *reo_params)
{
	return QDF_STATUS_SUCCESS;
}
#else
/**
 * mgmt_rx_reo_sim_get_num_mlo_links() - Get number of MLO HW links from the reo
 * simulation context object
 * @sim_context: Pointer to reo simulation context object
 *
 * Number of MLO links will be equal to number of pdevs in the
 * system. In case of simulation all the pdevs are assumed
 * to have MLO capability.
 *
 * Return: On success returns number of MLO HW links. On failure
 * returns -1.
 */
static int8_t
mgmt_rx_reo_sim_get_num_mlo_links(struct mgmt_rx_reo_sim_context *sim_context)
{
	uint8_t num_mlo_links;

	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt reo simulation context is null");
		return MGMT_RX_REO_INVALID_NUM_LINKS;
	}

	qdf_spin_lock(&sim_context->link_id_to_pdev_map.lock);

	num_mlo_links = sim_context->link_id_to_pdev_map.num_mlo_links;

	qdf_spin_unlock(&sim_context->link_id_to_pdev_map.lock);

	return num_mlo_links;
}

/**
 * mgmt_rx_reo_get_num_mlo_links() - Get number of MLO links from the reo
 * context object
 * @reo_context: Pointer to reo context object
 *
 * Return: On success returns number of MLO HW links. On failure
 * returns -1.
 */
static int8_t
mgmt_rx_reo_get_num_mlo_links(struct mgmt_rx_reo_context *reo_context) {
	if (!reo_context) {
		mgmt_rx_reo_err("Mgmt reo context is null");
		return MGMT_RX_REO_INVALID_NUM_LINKS;
	}

	return mgmt_rx_reo_sim_get_num_mlo_links(&reo_context->sim_context);
}

/**
 * mgmt_rx_reo_sim_get_context() - Helper API to get the management
 * rx reorder simulation context
 *
 * Return: On success returns the pointer to management rx reorder
 * simulation context. On failure returns NULL.
 */
static struct mgmt_rx_reo_sim_context *
mgmt_rx_reo_sim_get_context(void)
{
	struct mgmt_rx_reo_context *reo_context;

	reo_context = mgmt_rx_reo_get_context();
	if (!reo_context) {
		mgmt_rx_reo_err("Mgmt reo context is null");
		return NULL;
	}

	return &reo_context->sim_context;
}

int8_t
mgmt_rx_reo_sim_get_mlo_link_id_from_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct mgmt_rx_reo_sim_context *sim_context;
	int8_t link_id;

	sim_context = mgmt_rx_reo_sim_get_context();
	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt reo simulation context is null");
		return MGMT_RX_REO_INVALID_LINK_ID;
	}

	qdf_spin_lock(&sim_context->link_id_to_pdev_map.lock);

	for (link_id = 0; link_id <
	     sim_context->link_id_to_pdev_map.num_mlo_links; link_id++)
		if (sim_context->link_id_to_pdev_map.map[link_id] == pdev)
			break;

	/* pdev is not found in map */
	if (link_id == sim_context->link_id_to_pdev_map.num_mlo_links)
		link_id = -1;

	qdf_spin_unlock(&sim_context->link_id_to_pdev_map.lock);

	return link_id;
}

struct wlan_objmgr_pdev *
mgmt_rx_reo_sim_get_pdev_from_mlo_link_id(uint8_t mlo_link_id)
{
	struct mgmt_rx_reo_sim_context *sim_context;
	struct wlan_objmgr_pdev *pdev;
	int8_t num_mlo_links;

	sim_context = mgmt_rx_reo_sim_get_context();
	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt reo simulation context is null");
		return NULL;
	}

	num_mlo_links = mgmt_rx_reo_sim_get_num_mlo_links(sim_context);
	if (num_mlo_links <= 0) {
		mgmt_rx_reo_err("invalid number of MLO links %d",
				num_mlo_links);
		return NULL;
	}

	if (mlo_link_id >= num_mlo_links) {
		mgmt_rx_reo_err("Invalid link id %u, total links = %d",
				mlo_link_id, num_mlo_links);
		return NULL;
	}

	qdf_spin_lock(&sim_context->link_id_to_pdev_map.lock);

	pdev = sim_context->link_id_to_pdev_map.map[mlo_link_id];

	qdf_spin_unlock(&sim_context->link_id_to_pdev_map.lock);

	return pdev;
}

/**
 * @mgmt_rx_reo_handle_potential_premature_delivery: Helper API to handle
 * premature delivery.
 * @reo_context: Pointer to reorder list
 * @global_timestamp: Global time stamp of the current management frame
 *
 * Sometimes we have to deliver a management frame to the upper layers even
 * before its wait count reaching zero. This is called premature delivery.
 * Premature delivery could happen due to time out or reorder list overflow.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_handle_potential_premature_delivery(
				struct mgmt_rx_reo_context *reo_context,
				uint32_t global_timestamp)
{
	qdf_list_t stale_frame_list_temp;
	QDF_STATUS status;
	struct mgmt_rx_reo_pending_frame_list_entry *latest_stale_frame = NULL;
	struct mgmt_rx_reo_pending_frame_list_entry *cur_entry;
	struct mgmt_rx_reo_sim_context *sim_context;
	struct mgmt_rx_reo_pending_frame_list *pending_frame_list;
	struct mgmt_rx_reo_stale_frame_list *stale_frame_list;

	if (!reo_context)
		return QDF_STATUS_E_NULL_VALUE;

	sim_context = &reo_context->sim_context;
	pending_frame_list = &sim_context->pending_frame_list;
	stale_frame_list = &sim_context->stale_frame_list;

	qdf_spin_lock(&pending_frame_list->lock);

	qdf_list_for_each(&pending_frame_list->list, cur_entry, node) {
		if (cur_entry->params.global_timestamp == global_timestamp)
			break;

		latest_stale_frame = cur_entry;
	}

	if (latest_stale_frame) {
		qdf_list_create(&stale_frame_list_temp,
				MGMT_RX_REO_SIM_STALE_FRAME_TEMP_LIST_MAX_SIZE);

		status = qdf_list_split(&stale_frame_list_temp,
					&pending_frame_list->list,
					&latest_stale_frame->node);
		if (QDF_IS_STATUS_ERROR(status))
			goto exit_unlock_pending_frame_list;

		qdf_spin_lock(&stale_frame_list->lock);
		status = qdf_list_join(&stale_frame_list->list,
				       &stale_frame_list_temp);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_spin_unlock(&stale_frame_list->lock);
			goto exit_unlock_pending_frame_list;
		}
		qdf_spin_unlock(&stale_frame_list->lock);
	}

	status = QDF_STATUS_SUCCESS;

exit_unlock_pending_frame_list:
	qdf_spin_unlock(&pending_frame_list->lock);

	return status;
}

/**
 * mgmt_rx_reo_sim_remove_frame_from_stale_list() - Removes frame from the
 * stale management frame list
 * @stale_frame_list: pointer to stale management frame list
 * @reo_params: pointer to reo params
 *
 * This API removes frames from the stale management frame list.
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_sim_remove_frame_from_stale_list(
		struct mgmt_rx_reo_stale_frame_list *stale_frame_list,
		const struct mgmt_rx_reo_params *reo_params)
{
	struct mgmt_rx_reo_stale_frame_list_entry *cur_entry;
	struct mgmt_rx_reo_stale_frame_list_entry *matching_entry = NULL;
	QDF_STATUS status;

	if (!stale_frame_list || !reo_params)
		return QDF_STATUS_E_NULL_VALUE;

	qdf_spin_lock(&stale_frame_list->lock);

	/**
	 * Stale frames can come in any order at host. Do a linear search and
	 * remove the matching entry.
	 */
	qdf_list_for_each(&stale_frame_list->list, cur_entry, node) {
		if (cur_entry->params.link_id == reo_params->link_id &&
		    cur_entry->params.mgmt_pkt_ctr == reo_params->mgmt_pkt_ctr &&
		    cur_entry->params.global_timestamp ==
		    reo_params->global_timestamp) {
			matching_entry = cur_entry;
			break;
		}
	}

	if (!matching_entry) {
		qdf_spin_unlock(&stale_frame_list->lock);
		mgmt_rx_reo_err("reo sim failure: absent in stale frame list");
		qdf_assert_always(0);
	}

	status = qdf_list_remove_node(&stale_frame_list->list,
				      &matching_entry->node);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock(&stale_frame_list->lock);
		return status;
	}

	qdf_spin_unlock(&stale_frame_list->lock);

	qdf_mem_free(matching_entry);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_handle_stale_frame() - API to handle stale management frames.
 * @reo_list: Pointer to reorder list
 * @reo_params: Pointer to reo parameters
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_handle_stale_frame(struct mgmt_rx_reo_list *reo_list,
			       struct mgmt_rx_reo_params *reo_params)
{
	QDF_STATUS status;
	struct mgmt_rx_reo_context *reo_context;
	struct mgmt_rx_reo_sim_context *sim_context;

	if (!reo_list || !reo_params)
		return QDF_STATUS_E_NULL_VALUE;

	reo_context = mgmt_rx_reo_get_context_from_reo_list(reo_list);
	if (!reo_context)
		return QDF_STATUS_E_NULL_VALUE;

	sim_context = &reo_context->sim_context;

	status = mgmt_rx_reo_sim_remove_frame_from_stale_list(
				&sim_context->stale_frame_list, reo_params);

	return status;
}
#endif /* WLAN_MGMT_RX_REO_SIM_SUPPORT */

/**
 * mgmt_rx_reo_is_potential_premature_delivery() - Helper API to check
 * whether the current frame getting delivered to upper layer is a premature
 * delivery
 * @release_reason: release reason
 *
 * Return: true for a premature delivery
 */
static bool
mgmt_rx_reo_is_potential_premature_delivery(uint8_t release_reason)
{
	return (release_reason !=
			MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_ZERO_WAIT_COUNT);
}

/**
 * wlan_mgmt_rx_reo_get_priv_object() - Get the pdev private object of
 * MGMT Rx REO module
 * @pdev: pointer to pdev object
 *
 * Return: Pointer to pdev private object of MGMT Rx REO module on success,
 * else NULL
 */
static struct mgmt_rx_reo_pdev_info *
wlan_mgmt_rx_reo_get_priv_object(struct wlan_objmgr_pdev *pdev)
{
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return NULL;
	}

	mgmt_txrx_pdev_ctx = (struct mgmt_txrx_priv_pdev_context *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
						      WLAN_UMAC_COMP_MGMT_TXRX);

	if (!mgmt_txrx_pdev_ctx) {
		mgmt_rx_reo_err("mgmt txrx context is NULL");
		return NULL;
	}

	return mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx;
}

/**
 * wlan_mgmt_rx_reo_algo_calculate_wait_count() - Calculates the number of
 * frames an incoming frame should wait for before it gets delivered.
 * @in_frame_pdev: pdev on which this frame is received
 * @in_frame_params: Pointer to MGMT REO parameters of this frame
 * @num_mlo_links: Number of MLO links
 * @wait_count: Pointer to wait count data structure to fill the calculated
 * wait count
 *
 * Each frame carrys a MGMT pkt number which is local to that link, and a
 * timestamp which is global across all the links. MAC HW and FW also captures
 * the same details of the last frame that they have seen. Host also maintains
 * the details of the last frame it has seen. In total, there are 4 snapshots.
 * 1. MAC HW snapshot - latest frame seen at MAC HW
 * 2. FW forwarded snapshot- latest frame forwarded to the Host
 * 3. FW consumed snapshot - latest frame consumed by the FW
 * 4. Host/FW consumed snapshot - latest frame seen by the Host
 * By using all these snapshots, this function tries to compute the wait count
 * for a given incoming frame on all links.
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
wlan_mgmt_rx_reo_algo_calculate_wait_count(
	struct wlan_objmgr_pdev *in_frame_pdev,
	struct mgmt_rx_reo_params *in_frame_params,
	uint8_t num_mlo_links,
	struct mgmt_rx_reo_wait_count *wait_count)
{
	QDF_STATUS status;
	uint8_t link;
	int8_t in_frame_link;
	int frames_pending, delta_fwd_host;
	uint8_t snapshot_id;
	struct wlan_objmgr_pdev *pdev;
	struct mgmt_rx_reo_pdev_info *rx_reo_pdev_ctx;
	struct mgmt_rx_reo_snapshot *address;
	struct mgmt_rx_reo_snapshot_params snapshot_params
		[MGMT_RX_REO_SHARED_SNAPSHOT_MAX];
	struct mgmt_rx_reo_snapshot_params *mac_hw_ss, *fw_forwarded_ss,
					    *fw_consumed_ss, *host_ss;

	if (!in_frame_params) {
		mgmt_rx_reo_err("MGMT Rx REO params of incoming frame is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!wait_count) {
		mgmt_rx_reo_err("wait count pointer to be filled is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!in_frame_pdev) {
		mgmt_rx_reo_err("pdev is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_assert_always(num_mlo_links <= MGMT_RX_REO_MAX_LINKS);

	/* Get the MLO link ID of incoming frame */
	in_frame_link = wlan_get_mlo_link_id_from_pdev(in_frame_pdev);
	qdf_assert_always(in_frame_link >= 0);

	/* Iterate over all the MLO links */
	for (link = 0; link < num_mlo_links; link++) {
		/* No need wait for any frames on the same link */
		if (link == in_frame_link) {
			frames_pending = 0;
			goto update_pending_frames;
		}

		pdev = wlan_get_pdev_from_mlo_link_id(link);

		rx_reo_pdev_ctx = wlan_mgmt_rx_reo_get_priv_object(pdev);
		if (!rx_reo_pdev_ctx) {
			mgmt_rx_reo_err("Mgmt reo context empty for pdev %pK",
					pdev);
			return QDF_STATUS_E_FAILURE;
		}

		host_ss = &rx_reo_pdev_ctx->host_snapshot;

		mgmt_rx_reo_debug("link_id = %u HOST SS: valid = %u, ctr = %u, ts = %u",
				  link, host_ss->valid, host_ss->mgmt_pkt_ctr,
				  host_ss->global_timestamp);

		/**
		 * Ideally, the incoming frame has to wait for only those frames
		 * (on other links) which meet all the below criterion.
		 * 1. Frame's timestamp is less than incoming frame's
		 * 2. Frame is supposed to be consumed by the Host
		 * 3. Frame is not yet seen by the Host.
		 * We may not be able to compute the exact optimal wait count
		 * because HW/FW provides a limited assist.
		 * This algorithm tries to get the best estimate of wait count
		 * by not waiting for those frames where we have a conclusive
		 * evidence that we don't have to wait for those frames.
		 */

		/**
		 * If this link has already seen a frame whose timestamp is
		 * greater than or equal to incoming frame's timestamp,
		 * then no need to wait for any frames on this link.
		 * If the totalt wait count becomes zero, then the policy on
		 * whether to deliver such a frame to upper layers is handled
		 * separately.
		 */
		if (host_ss->valid &&
		    mgmt_rx_reo_compare_global_timestamps_gte(
				host_ss->global_timestamp,
				in_frame_params->global_timestamp)) {
			frames_pending = 0;
			goto update_pending_frames;
		}

		snapshot_id = 0;
		/* Read all the shared snapshots */
		while (snapshot_id <
			MGMT_RX_REO_SHARED_SNAPSHOT_MAX) {
			address = rx_reo_pdev_ctx->
				   host_target_shared_snapshot[snapshot_id],

			qdf_mem_zero(&snapshot_params[snapshot_id],
				     sizeof(snapshot_params[snapshot_id]));

			status = tgt_mgmt_rx_reo_read_snapshot(
						pdev, address, snapshot_id,
						&snapshot_params[snapshot_id]);

			/* Read operation shouldn't fail */
			if (QDF_IS_STATUS_ERROR(status)) {
				mgmt_rx_reo_err("snapshot(%d) read failed on"
						"link (%d)", snapshot_id, link);
				return status;
			}

			/* If snapshot is valid, save it in the pdev context */
			if (snapshot_params[snapshot_id].valid) {
				rx_reo_pdev_ctx->
				   last_valid_shared_snapshot[snapshot_id] =
				   snapshot_params[snapshot_id];
			}
			snapshot_id++;
		}

		mac_hw_ss = &snapshot_params
				[MGMT_RX_REO_SHARED_SNAPSHOT_MAC_HW];
		fw_forwarded_ss = &snapshot_params
				[MGMT_RX_REO_SHARED_SNAPSHOT_FW_FORWADED];
		fw_consumed_ss = &snapshot_params
				[MGMT_RX_REO_SHARED_SNAPSHOT_FW_CONSUMED];

		mgmt_rx_reo_debug("link_id = %u HW SS: valid = %u, ctr = %u, ts = %u",
				  link, mac_hw_ss->valid,
				  mac_hw_ss->mgmt_pkt_ctr,
				  mac_hw_ss->global_timestamp);
		mgmt_rx_reo_debug("link_id = %u FW forwarded SS: valid = %u, ctr = %u, ts = %u",
				  link, fw_forwarded_ss->valid,
				  fw_forwarded_ss->mgmt_pkt_ctr,
				  fw_forwarded_ss->global_timestamp);
		mgmt_rx_reo_debug("link_id = %u FW consumed SS: valid = %u, ctr = %u, ts = %u",
				  link, fw_consumed_ss->valid,
				  fw_consumed_ss->mgmt_pkt_ctr,
				  fw_consumed_ss->global_timestamp);

		/**
		 * If MAC HW snapshot is invalid, we need to assume the worst
		 * and wait for UINT_MAX frames, but this should not be a
		 * concern because if subsequent frames read a valid snapshot,
		 * the REO algorithm will take care of updating the wait count
		 * of this frame as well.
		 * There may be more optimal ways to handle invalid snapshot
		 * reads.  For e.g., making use of previously read valid
		 * snapshot, but they come with complex logic.
		 * Keeping this simple for now.
		 */
		if (!mac_hw_ss->valid) {
			wait_count->per_link_count[link] = UINT_MAX;
			wait_count->total_count += UINT_MAX;
			mgmt_rx_reo_debug("link_id = %u wait count: per link = 0x%x, total = 0x%llx",
					  link,
					  wait_count->per_link_count[link],
					  wait_count->total_count);
			continue;
		}

		/**
		 * For starters, we only have to wait for the frames that are
		 * seen by MAC HW but not yet seen by Host. The frames which
		 * reach MAC HW later are guaranteed to have a timestamp
		 * greater than incoming frame's timestamp.
		 */
		frames_pending = mgmt_rx_reo_subtract_pkt_ctrs(
					mac_hw_ss->mgmt_pkt_ctr,
					host_ss->mgmt_pkt_ctr);
		qdf_assert_always(frames_pending >= 0);

		if (mgmt_rx_reo_compare_global_timestamps_gte(
					mac_hw_ss->global_timestamp,
					in_frame_params->global_timestamp)) {
			/**
			 * Last frame seen at MAC HW has timestamp greater than
			 * or equal to incoming frame's timestamp. So no need to
			 * wait for that last frame, but we can't conclusively
			 * say anything about timestamp of frames before the
			 * last frame, so try to wait for all of those frames.
			 */
			frames_pending--;
			qdf_assert_always(frames_pending >= 0);

			if (fw_consumed_ss->valid &&
			    mgmt_rx_reo_compare_global_timestamps_gte(
				fw_consumed_ss->global_timestamp,
				in_frame_params->global_timestamp)) {
				/**
				 * Last frame consumed by the FW has timestamp
				 * greater than or equal to incoming frame's.
				 * That means all the frames from
				 * fw_consumed_ss->mgmt_pkt_ctr to
				 * mac_hw->mgmt_pkt_ctr will have timestamp
				 * greater than or equal to incoming frame's and
				 * hence, no need to wait for those frames.
				 * We just need to wait for frames from
				 * host_ss->mgmt_pkt_ctr to
				 * fw_consumed_ss->mgmt_pkt_ctr-1. This is a
				 * better estimate over the above estimate,
				 * so update frames_pending.
				 */
				frames_pending =
				  mgmt_rx_reo_subtract_pkt_ctrs(
				      fw_consumed_ss->mgmt_pkt_ctr,
				      host_ss->mgmt_pkt_ctr) - 1;

				qdf_assert_always(frames_pending >= 0);

				/**
				 * Last frame forwarded to Host has timestamp
				 * less than incoming frame's. That means all
				 * the frames starting from
				 * fw_forwarded_ss->mgmt_pkt_ctr+1 to
				 * fw_consumed_ss->mgmt_pkt_ctr are consumed by
				 * the FW and hence, no need to wait for those
				 * frames. We just need to wait for frames
				 * from host_ss->mgmt_pkt_ctr to
				 * fw_forwarded_ss->mgmt_pkt_ctr. This is a
				 * better estimate over the above estimate,
				 * so update frames_pending.
				 */
				if (fw_forwarded_ss->valid &&
				    !mgmt_rx_reo_compare_global_timestamps_gte(
					fw_forwarded_ss->global_timestamp,
					in_frame_params->global_timestamp)) {
					frames_pending =
					  mgmt_rx_reo_subtract_pkt_ctrs(
					      fw_forwarded_ss->mgmt_pkt_ctr,
					      host_ss->mgmt_pkt_ctr);

					/**
					 * frames_pending can be negative in
					 * cases whene there are no frames
					 * getting forwarded to the Host. No
					 * need to wait for any frames in that
					 * case.
					 */
					if (frames_pending < 0)
						frames_pending = 0;
				}
			}

			/**
			 * Last frame forwarded to Host has timestamp greater
			 * than or equal to incoming frame's. That means all the
			 * frames from fw_forwarded->mgmt_pkt_ctr to
			 * mac_hw->mgmt_pkt_ctr will have timestamp greater than
			 * or equal to incoming frame's and hence, no need to
			 * wait for those frames. We may have to just wait for
			 * frames from host_ss->mgmt_pkt_ctr to
			 * fw_forwarded_ss->mgmt_pkt_ctr-1
			 */
			if (fw_forwarded_ss->valid &&
			    mgmt_rx_reo_compare_global_timestamps_gte(
				fw_forwarded_ss->global_timestamp,
				in_frame_params->global_timestamp)) {
				delta_fwd_host =
				  mgmt_rx_reo_subtract_pkt_ctrs(
				    fw_forwarded_ss->mgmt_pkt_ctr,
				    host_ss->mgmt_pkt_ctr) - 1;

				qdf_assert_always(delta_fwd_host >= 0);

				/**
				 * This will be a better estimate over the one
				 * we computed using mac_hw_ss but this may or
				 * may not be a better estimate over the
				 * one we computed using fw_consumed_ss.
				 * When timestamps of both fw_consumed_ss and
				 * fw_forwarded_ss are greater than incoming
				 * frame's but timestamp of fw_consumed_ss is
				 * smaller than fw_forwarded_ss, then
				 * frames_pending will be smaller than
				 * delta_fwd_host, the reverse will be true in
				 * other cases. Instead of checking for all
				 * those cases, just waiting for the minimum
				 * among these two should be sufficient.
				 */
				frames_pending = qdf_min(frames_pending,
							 delta_fwd_host);
				qdf_assert_always(frames_pending >= 0);
			}
		}

update_pending_frames:
			qdf_assert_always(frames_pending >= 0);

			wait_count->per_link_count[link] = frames_pending;
			wait_count->total_count += frames_pending;

			mgmt_rx_reo_debug("link_id = %u wait count: per link = 0x%x, total = 0x%llx",
					  link,
					  wait_count->per_link_count[link],
					  wait_count->total_count);
	}

	return QDF_STATUS_SUCCESS;
}

/*
 * struct mgmt_rx_reo_list_entry_debug_info - This structure holds the necessary
 * information about a reo list entry for debug purposes.
 * @link_id: link id
 * @mgmt_pkt_ctr: management packet counter
 * @global_timestamp: global time stamp
 * @wait_count: wait count values
 * @status: status of the entry in the list
 * @entry: pointer to reo list entry
 */
struct mgmt_rx_reo_list_entry_debug_info {
	uint8_t link_id;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
	struct mgmt_rx_reo_wait_count wait_count;
	uint32_t status;
	struct mgmt_rx_reo_list_entry *entry;
};

/**
 * mgmt_rx_reo_list_display() - API to print the entries in the reorder list
 * @reo_list: Pointer to reorder list
 * @num_mlo_links: Number of MLO HW links
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_list_display(struct mgmt_rx_reo_list *reo_list,
			 uint8_t num_mlo_links)
{
	uint32_t reo_list_size;
	uint32_t index;
	struct mgmt_rx_reo_list_entry *cur_entry;
	struct mgmt_rx_reo_list_entry_debug_info *debug_info;

	if (!reo_list) {
		mgmt_rx_reo_err("Pointer to reo list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (num_mlo_links == 0 || num_mlo_links > MGMT_RX_REO_MAX_LINKS) {
		mgmt_rx_reo_err("Invalid number of links %u", num_mlo_links);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&reo_list->list_lock);

	reo_list_size = qdf_list_size(&reo_list->list);

	if (reo_list_size == 0) {
		qdf_spin_unlock_bh(&reo_list->list_lock);
		mgmt_rx_reo_debug("Number of entries in the reo list = %u",
				  reo_list_size);
		return QDF_STATUS_SUCCESS;
	}

	debug_info = qdf_mem_malloc_atomic(reo_list_size * sizeof(*debug_info));
	if (!debug_info) {
		qdf_spin_unlock_bh(&reo_list->list_lock);
		mgmt_rx_reo_err("Memory allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	index = 0;
	qdf_list_for_each(&reo_list->list, cur_entry, node) {
		debug_info[index].link_id =
				mgmt_rx_reo_get_link_id(cur_entry->rx_params);
		debug_info[index].mgmt_pkt_ctr =
			mgmt_rx_reo_get_pkt_counter(cur_entry->rx_params);
		debug_info[index].global_timestamp =
				mgmt_rx_reo_get_global_ts(cur_entry->rx_params);
		debug_info[index].wait_count = cur_entry->wait_count;
		debug_info[index].status = cur_entry->status;
		debug_info[index].entry = cur_entry;

		++index;
	}

	qdf_spin_unlock_bh(&reo_list->list_lock);

	mgmt_rx_reo_debug("Reorder list");
	mgmt_rx_reo_debug("##################################################");
	mgmt_rx_reo_debug("Number of entries in the reo list = %u",
			  reo_list_size);
	for (index = 0; index < reo_list_size; index++) {
		uint8_t link_id;

		mgmt_rx_reo_debug("index = %u: link_id = %u, ts = %u, ctr = %u, status = 0x%x, entry = %pK",
				  index, debug_info[index].link_id,
				  debug_info[index].global_timestamp,
				  debug_info[index].mgmt_pkt_ctr,
				  debug_info[index].status,
				  debug_info[index].entry);

		mgmt_rx_reo_debug("Total wait count = 0x%llx",
				  debug_info[index].wait_count.total_count);

		for (link_id = 0; link_id < num_mlo_links; link_id++)
			mgmt_rx_reo_debug("Link id = %u, wait_count = 0x%x",
					  link_id, debug_info[index].wait_count.
					  per_link_count[link_id]);
	}
	mgmt_rx_reo_debug("##################################################");

	qdf_mem_free(debug_info);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_MGMT_RX_REO_DEBUG_SUPPORT
/**
 * mgmt_rx_reo_log_egress_frame_before_delivery() - Log the information about a
 * frame exiting the reorder module. Logging is done before attempting the frame
 * delivery to upper layers.
 * @reo_ctx: management rx reorder context
 * @entry: Pointer to reorder list entry
 * @release_reason: Reason to release the frame to upper layers
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_log_egress_frame_before_delivery(
					struct mgmt_rx_reo_context *reo_ctx,
					struct mgmt_rx_reo_list_entry *entry,
					uint8_t release_reason)
{
	struct reo_egress_debug_info *egress_frame_debug_info;
	struct reo_egress_debug_frame_info *cur_frame_debug_info;

	if (!reo_ctx || !entry)
		return QDF_STATUS_E_NULL_VALUE;

	egress_frame_debug_info = &reo_ctx->egress_frame_debug_info;

	cur_frame_debug_info = &egress_frame_debug_info->debug_info
			[egress_frame_debug_info->next_index];

	cur_frame_debug_info->link_id =
				mgmt_rx_reo_get_link_id(entry->rx_params);
	cur_frame_debug_info->mgmt_pkt_ctr =
				mgmt_rx_reo_get_pkt_counter(entry->rx_params);
	cur_frame_debug_info->global_timestamp =
				mgmt_rx_reo_get_global_ts(entry->rx_params);
	cur_frame_debug_info->wait_count = entry->wait_count;
	cur_frame_debug_info->insertion_ts = entry->insertion_ts;
	cur_frame_debug_info->ingress_timestamp = entry->ingress_timestamp;
	cur_frame_debug_info->egress_begin_timestamp = qdf_get_log_timestamp();
	cur_frame_debug_info->release_reason = release_reason;

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_log_egress_frame_after_delivery() - Log the information about a
 * frame exiting the reorder module. Logging is done after attempting the frame
 * delivery to upper layer.
 * @reo_ctx: management rx reorder context
 * @is_delivered: Flag to indicate whether the frame is delivered to upper
 * layers
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_log_egress_frame_after_delivery(
					struct mgmt_rx_reo_context *reo_ctx,
					bool is_delivered)
{
	struct reo_egress_debug_info *egress_frame_debug_info;
	struct reo_egress_debug_frame_info *cur_frame_debug_info;

	if (!reo_ctx)
		return QDF_STATUS_E_NULL_VALUE;

	egress_frame_debug_info = &reo_ctx->egress_frame_debug_info;

	cur_frame_debug_info = &egress_frame_debug_info->debug_info
			[egress_frame_debug_info->next_index];

	cur_frame_debug_info->is_delivered = is_delivered;
	cur_frame_debug_info->egress_end_timestamp = qdf_get_log_timestamp();

	egress_frame_debug_info->next_index++;
	egress_frame_debug_info->next_index %=
				MGMT_RX_REO_EGRESS_FRAME_DEBUG_ENTRIES_MAX;

	return QDF_STATUS_SUCCESS;
}
#else
/**
 * mgmt_rx_reo_log_egress_frame_before_delivery() - Log the information about a
 * frame exiting the reorder module. Logging is done before attempting the frame
 * delivery to upper layers.
 * @reo_ctx: management rx reorder context
 * @entry: Pointer to reorder list entry
 * @release_reason: Reason to release the frame to upper layers
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_log_egress_frame_before_delivery(
					struct mgmt_rx_reo_context *reo_ctx,
					struct mgmt_rx_reo_list_entry *entry,
					uint8_t release_reason)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_log_egress_frame_after_delivery() - Log the information about a
 * frame exiting the reorder module. Logging is done after attempting the frame
 * delivery to upper layer.
 * @reo_ctx: management rx reorder context
 * @is_delivered: Flag to indicate whether the frame is delivered to upper
 * layers
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_log_egress_frame_after_delivery(
					struct mgmt_rx_reo_context *reo_ctx,
					bool is_delivered)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MGMT_RX_REO_DEBUG_SUPPORT */

/**
 * mgmt_rx_reo_list_entry_get_release_reason() - Helper API to get the reason
 * for releasing the reorder list entry to upper layer.
 * reorder list.
 * @reo_list: Pointer to reorder list
 * @entry: List entry
 *
 * This API expects the caller to acquire the spin lock protecting the reorder
 * list.
 *
 * Return: Reason for releasing the frame.
 */
static uint8_t
mgmt_rx_reo_list_entry_get_release_reason(
		struct mgmt_rx_reo_list *reo_list,
		struct mgmt_rx_reo_list_entry *entry)
{
	uint8_t release_reason = 0;

	if (!reo_list || !entry)
		return 0;

	if (MGMT_RX_REO_LIST_ENTRY_IS_MAX_SIZE_EXCEEDED(entry))
		release_reason |=
		   MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_LIST_MAX_SIZE_EXCEEDED;

	if (!MGMT_RX_REO_LIST_ENTRY_IS_WAITING_FOR_FRAME_ON_OTHER_LINK(entry))
		release_reason |=
			MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_ZERO_WAIT_COUNT;

	if (MGMT_RX_REO_LIST_ENTRY_IS_AGED_OUT(entry))
		release_reason |=
				MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_AGED_OUT;

	if (MGMT_RX_REO_LIST_ENTRY_IS_OLDER_THAN_LATEST_AGED_OUT_FRAME(entry))
		release_reason |=
		MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_OLDER_THAN_AGED_OUT_FRAME;

	return release_reason;
}

/**
 * mgmt_rx_reo_list_entry_send_up() - API to send the frame to the upper layer.
 * @reo_list: Pointer to reorder list
 * @entry: List entry
 *
 * API to send the frame to the upper layer. This API has to be called only
 * for entries which can be released to upper layer. It is the caller's
 * responsibility to ensure that entry can be released (by using API
 * mgmt_rx_reo_list_is_ready_to_send_up_entry). This API is called after
 * acquiring the lock which serializes the frame delivery to the upper layers.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_list_entry_send_up(const struct mgmt_rx_reo_list *reo_list,
			       struct mgmt_rx_reo_list_entry *entry)
{
	uint8_t release_reason;
	uint8_t link_id;
	struct wlan_objmgr_pdev *pdev;
	uint32_t entry_global_ts;
	struct mgmt_rx_reo_global_ts_info *ts_last_delivered_frame;
	QDF_STATUS status;
	struct mgmt_rx_reo_context *reo_context;
	bool is_delivered = false;

	qdf_assert_always(reo_list);
	qdf_assert_always(entry);

	reo_context = mgmt_rx_reo_get_context_from_reo_list(reo_list);
	qdf_assert_always(reo_context);

	link_id = mgmt_rx_reo_get_link_id(entry->rx_params);
	entry_global_ts = mgmt_rx_reo_get_global_ts(entry->rx_params);
	ts_last_delivered_frame = &reo_list->ts_last_delivered_frame;

	release_reason = mgmt_rx_reo_list_entry_get_release_reason(
					reo_list, entry);

	qdf_assert_always(release_reason != 0);

	pdev = wlan_get_pdev_from_mlo_link_id(link_id);
	if (!pdev) {
		mgmt_rx_reo_err("Unable to get pdev corresponding to entry %pK",
				entry);
		return QDF_STATUS_E_FAILURE;
	}

	status = mgmt_rx_reo_log_egress_frame_before_delivery(reo_context,
							      entry,
							      release_reason);
	if (QDF_IS_STATUS_ERROR(status))
		goto exit;

	if (mgmt_rx_reo_is_potential_premature_delivery(release_reason)) {
		status = mgmt_rx_reo_handle_potential_premature_delivery(
						reo_context, entry_global_ts);
		if (QDF_IS_STATUS_ERROR(status))
			goto exit;
	}

	status = wlan_mgmt_txrx_process_rx_frame(pdev, entry->nbuf,
						 entry->rx_params);
	/* Above call frees nbuf and rx_params, make it null explicitly */
	entry->nbuf = NULL;
	entry->rx_params = NULL;

	if (QDF_IS_STATUS_ERROR(status))
		goto exit;

	is_delivered = true;

	status = mgmt_rx_reo_log_egress_frame_after_delivery(reo_context,
							     is_delivered);
	if (QDF_IS_STATUS_ERROR(status))
		goto exit;

	status = QDF_STATUS_SUCCESS;

exit:
	/**
	 * Release the reference taken when the entry is inserted into
	 * the reorder list
	 */
	wlan_objmgr_pdev_release_ref(pdev, WLAN_MGMT_RX_REO_ID);

	return status;
}

/**
 * mgmt_rx_reo_list_is_ready_to_send_up_entry() - API to check whether the
 * list entry can be send to upper layers.
 * @reo_list: Pointer to reorder list
 * @entry: List entry
 *
 * Return: QDF_STATUS
 */
static bool
mgmt_rx_reo_list_is_ready_to_send_up_entry(struct mgmt_rx_reo_list *reo_list,
					   struct mgmt_rx_reo_list_entry *entry)
{
	if (!reo_list || !entry)
		return false;

	return mgmt_rx_reo_list_max_size_exceeded(reo_list) ||
	       !MGMT_RX_REO_LIST_ENTRY_IS_WAITING_FOR_FRAME_ON_OTHER_LINK(
	       entry) || MGMT_RX_REO_LIST_ENTRY_IS_AGED_OUT(entry) ||
	       MGMT_RX_REO_LIST_ENTRY_IS_OLDER_THAN_LATEST_AGED_OUT_FRAME
	       (entry);
}

/**
 * mgmt_rx_reo_list_release_entries() - Release entries from the reorder list
 * @reo_context: Pointer to management Rx reorder context
 *
 * This API releases the entries from the reorder list based on the following
 * conditions.
 *   a) Entries with total wait count equal to 0
 *   b) Entries which are timed out or entries with global time stamp <= global
 *      time stamp of the latest frame which is timed out. We can only release
 *      the entries in the increasing order of the global time stamp.
 *      So all the entries with global time stamp <= global time stamp of the
 *      latest timed out frame has to be released.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_list_release_entries(struct mgmt_rx_reo_context *reo_context)
{
	struct mgmt_rx_reo_list *reo_list;
	QDF_STATUS status;

	if (!reo_context) {
		mgmt_rx_reo_err("reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	reo_list = &reo_context->reo_list;

	qdf_spin_lock(&reo_context->frame_release_lock);

	while (1) {
		struct mgmt_rx_reo_list_entry *first_entry;
		/* TODO yield if release_count > THRESHOLD */
		uint16_t release_count = 0;
		struct mgmt_rx_reo_global_ts_info *ts_last_released_frame =
					&reo_list->ts_last_released_frame;
		uint32_t entry_global_ts;

		qdf_spin_lock_bh(&reo_list->list_lock);

		first_entry = qdf_list_first_entry_or_null(
			&reo_list->list, struct mgmt_rx_reo_list_entry, node);

		if (!first_entry) {
			status = QDF_STATUS_SUCCESS;
			goto exit_unlock_list_lock;
		}

		if (!mgmt_rx_reo_list_is_ready_to_send_up_entry(reo_list,
								first_entry)) {
			status = QDF_STATUS_SUCCESS;
			goto exit_unlock_list_lock;
		}

		if (mgmt_rx_reo_list_max_size_exceeded(reo_list))
			first_entry->status |=
				MGMT_RX_REO_STATUS_LIST_MAX_SIZE_EXCEEDED;

		status = qdf_list_remove_node(&reo_list->list,
					      &first_entry->node);
		if (QDF_IS_STATUS_ERROR(status)) {
			status = QDF_STATUS_E_FAILURE;
			goto exit_unlock_list_lock;
		}

		/**
		 * Last released frame global time stamp is invalid means that
		 * current frame is the first frame to be released to the
		 * upper layer from the reorder list. Blindly update the last
		 * released frame global time stamp to the current frame's
		 * global time stamp and set the valid to true.
		 * If the last released frame global time stamp is valid and
		 * current frame's global time stamp is >= last released frame
		 * global time stamp, deliver the current frame to upper layer
		 * and update the last released frame global time stamp.
		 */
		entry_global_ts =
			mgmt_rx_reo_get_global_ts(first_entry->rx_params);
		if (!ts_last_released_frame->valid ||
		    mgmt_rx_reo_compare_global_timestamps_gte(
			entry_global_ts, ts_last_released_frame->global_ts)) {
			ts_last_released_frame->global_ts = entry_global_ts;
			ts_last_released_frame->valid = true;
		} else {
			/**
			 * This should never happen. All the frames older than
			 * the last frame released from the reorder list will be
			 * discarded at the entry to reorder algorithm itself.
			 */
			qdf_assert_always(0);
		}

		qdf_spin_unlock_bh(&reo_list->list_lock);

		status = mgmt_rx_reo_list_entry_send_up(reo_list,
							first_entry);
		if (QDF_IS_STATUS_ERROR(status)) {
			status = QDF_STATUS_E_FAILURE;
			qdf_mem_free(first_entry);
			goto exit_unlock_frame_release_lock;
		}

		qdf_mem_free(first_entry);
		release_count++;
	}

	status = QDF_STATUS_SUCCESS;
	goto exit_unlock_frame_release_lock;

exit_unlock_list_lock:
	qdf_spin_unlock_bh(&reo_list->list_lock);
exit_unlock_frame_release_lock:
	qdf_spin_unlock(&reo_context->frame_release_lock);

	return status;
}

/**
 * mgmt_rx_reo_list_ageout_timer_handler() - Periodic ageout timer handler
 * @arg: Argument to timer handler
 *
 * This is the handler for periodic ageout timer used to timeout entries in the
 * reorder list.
 *
 * Return: void
 */
static void
mgmt_rx_reo_list_ageout_timer_handler(void *arg)
{
	struct mgmt_rx_reo_list *reo_list = arg;
	struct mgmt_rx_reo_list_entry *cur_entry;
	uint64_t cur_ts;
	QDF_STATUS status;
	struct mgmt_rx_reo_context *reo_context;
	/**
	 * Stores the pointer to the entry in reorder list for the latest aged
	 * out frame. Latest aged out frame is the aged out frame in reorder
	 * list which has the largest global time stamp value.
	 */
	struct mgmt_rx_reo_list_entry *latest_aged_out_entry = NULL;

	qdf_assert_always(reo_list);

	reo_context = mgmt_rx_reo_get_context_from_reo_list(reo_list);
	qdf_assert_always(reo_context);

	qdf_spin_lock_bh(&reo_list->list_lock);

	cur_ts = qdf_get_log_timestamp();

	qdf_list_for_each(&reo_list->list, cur_entry, node) {
		if (cur_ts - cur_entry->insertion_ts >=
		    reo_list->list_entry_timeout_us) {
			latest_aged_out_entry = cur_entry;
			cur_entry->status |= MGMT_RX_REO_STATUS_AGED_OUT;
		}
	}

	if (latest_aged_out_entry) {
		qdf_list_for_each(&reo_list->list, cur_entry, node) {
			if (cur_entry == latest_aged_out_entry)
				break;
			cur_entry->status |= MGMT_RX_REO_STATUS_OLDER_THAN_LATEST_AGED_OUT_FRAME;
		}
	}

	qdf_spin_unlock_bh(&reo_list->list_lock);

	status = mgmt_rx_reo_list_release_entries(reo_context);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to release list entries, status = %d",
				status);
		return;
	}
}

/**
 * mgmt_rx_reo_prepare_list_entry() - Prepare a list entry from the management
 * frame received.
 * @frame_desc: Pointer to the frame descriptor
 * @entry: Pointer to the list entry
 *
 * This API prepares the reorder list entry corresponding to a management frame
 * to be consumed by host. This entry would be inserted at the appropriate
 * position in the reorder list.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_prepare_list_entry(
		const struct mgmt_rx_reo_frame_descriptor *frame_desc,
		struct mgmt_rx_reo_list_entry **entry)
{
	struct mgmt_rx_reo_list_entry *list_entry;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS status;
	uint8_t link_id;

	if (!frame_desc) {
		mgmt_rx_reo_err("frame descriptor is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!entry) {
		mgmt_rx_reo_err("Pointer to list entry is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	link_id = mgmt_rx_reo_get_link_id(frame_desc->rx_params);

	pdev = wlan_get_pdev_from_mlo_link_id(link_id);
	if (!pdev) {
		mgmt_rx_reo_err("pdev corresponding to link %u is null",
				link_id);
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Take the reference when the entry is created for insertion */
	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_MGMT_RX_REO_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to get ref to pdev");
		return QDF_STATUS_E_FAILURE;
	}

	list_entry =  qdf_mem_malloc(sizeof(*list_entry));
	if (!list_entry) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_MGMT_RX_REO_ID);
		mgmt_rx_reo_err("List entry allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	list_entry->nbuf = frame_desc->nbuf;
	list_entry->rx_params = frame_desc->rx_params;
	list_entry->wait_count = frame_desc->wait_count;
	list_entry->status = 0;
	if (list_entry->wait_count.total_count)
		list_entry->status |=
			MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS;

	*entry = list_entry;

	mgmt_rx_reo_debug("New entry to be inserted is %pK", list_entry);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_update_wait_count() - Update the wait count for a frame based
 * on the wait count of a frame received after that on air.
 * @num_mlo_links: Number of MLO links
 * @wait_count_old_frame: Pointer to the wait count structure for the old frame.
 * @wait_count_new_frame: Pointer to the wait count structure for the new frame.
 *
 * This API optimizes the wait count of a frame based on the wait count of
 * a frame received after that on air. Old frame refers to the frame received
 * first on the air and new frame refers to the frame received after that.
 * We use the following fundamental idea. Wait counts for old frames can't be
 * more than wait counts for the new frame. Use this to optimize the wait count
 * for the old frames. Per link wait count of an old frame is minimum of the
 * per link wait count of the old frame and new frame.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_update_wait_count(
		uint8_t num_mlo_links,
		struct mgmt_rx_reo_wait_count *wait_count_old_frame,
		const struct mgmt_rx_reo_wait_count *wait_count_new_frame)
{
	uint8_t link_id;

	qdf_assert_always(wait_count_old_frame);
	qdf_assert_always(wait_count_new_frame);

	qdf_assert_always(num_mlo_links <= MGMT_RX_REO_MAX_LINKS);

	for (link_id = 0; link_id < num_mlo_links; link_id++) {
		if (wait_count_old_frame->per_link_count[link_id]) {
			uint32_t temp_wait_count;
			uint32_t wait_count_diff;

			temp_wait_count =
				wait_count_old_frame->per_link_count[link_id];
			wait_count_old_frame->per_link_count[link_id] =
				qdf_min(wait_count_old_frame->
					per_link_count[link_id],
					wait_count_new_frame->
					per_link_count[link_id]);
			wait_count_diff = temp_wait_count -
				wait_count_old_frame->per_link_count[link_id];

			wait_count_old_frame->total_count -= wait_count_diff;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_update_list() - Modify the reorder list when a frame is received
 * @reo_list: Pointer to reorder list
 * @num_mlo_links: Number of MLO HW links
 * @frame_desc: Pointer to frame descriptor
 * @is_queued: Whether this frame is queued in the REO list
 *
 * API to update the reorder list on every management frame reception.
 * This API does the following things.
 *   a) Update the wait counts for all the frames in the reorder list with
 *      global time stamp <= current frame's global time stamp. We use the
 *      following principle for updating the wait count in this case.
 *      Let A and B be two management frames with global time stamp of A <=
 *      global time stamp of B. Let WAi and WBi be the wait count of A and B
 *      for link i, then WAi <= WBi. Hence we can optimize WAi as
 *      min(WAi, WBi).
 *   b) If the current frame is to be consumed by host, insert it in the
 *      reorder list such that the list is always sorted in the increasing order
 *      of global time stamp. Update the wait count of the current frame based
 *      on the frame next to it in the reorder list (if any).
 *   c) Update the wait count of the frames in the reorder list with global
 *      time stamp > current frame's global time stamp. Let the current frame
 *      belong to link "l". Then link "l"'s wait count can be reduced by one for
 *      all the frames in the reorder list with global time stamp > current
 *      frame's global time stamp.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_update_list(struct mgmt_rx_reo_list *reo_list,
			uint8_t num_mlo_links,
			struct mgmt_rx_reo_frame_descriptor *frame_desc,
			bool *is_queued)
{
	struct mgmt_rx_reo_list_entry *cur_entry;
	struct mgmt_rx_reo_list_entry *least_greater_entry;
	bool least_greater_entry_found = false;
	QDF_STATUS status;
	uint32_t new_frame_global_ts;
	struct mgmt_rx_reo_list_entry *new_entry = NULL;

	if (!is_queued)
		return QDF_STATUS_E_NULL_VALUE;
	*is_queued = false;

	if (!reo_list) {
		mgmt_rx_reo_err("Mgmt Rx reo list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame_desc) {
		mgmt_rx_reo_err("Mgmt frame descriptor is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	new_frame_global_ts = mgmt_rx_reo_get_global_ts(frame_desc->rx_params);

	/* Prepare the list entry before acquiring lock */
	if (frame_desc->type == MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME) {
		status = mgmt_rx_reo_prepare_list_entry(frame_desc, &new_entry);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("Failed to prepare list entry");
			return QDF_STATUS_E_FAILURE;
		}
	}

	qdf_spin_lock_bh(&reo_list->list_lock);

	qdf_list_for_each(&reo_list->list, cur_entry, node) {
		uint32_t cur_entry_global_ts;

		cur_entry_global_ts = mgmt_rx_reo_get_global_ts(
					cur_entry->rx_params);

		if (!mgmt_rx_reo_compare_global_timestamps_gte(
		    new_frame_global_ts, cur_entry_global_ts)) {
			least_greater_entry = cur_entry;
			least_greater_entry_found = true;
			break;
		}

		status = mgmt_rx_reo_update_wait_count(
					num_mlo_links,
					&cur_entry->wait_count,
					&frame_desc->wait_count);
		if (QDF_IS_STATUS_ERROR(status))
			goto error;

		if (cur_entry->wait_count.total_count == 0)
			cur_entry->status &=
			      ~MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS;
	}

	frame_desc->is_stale = false;
	if (mgmt_rx_reo_is_stale_frame(&reo_list->ts_last_released_frame,
				       new_frame_global_ts)) {
		frame_desc->is_stale = true;

		status = mgmt_rx_reo_handle_stale_frame(
				reo_list, frame_desc->rx_params->reo_params);
		if (QDF_IS_STATUS_ERROR(status))
			goto error;
	}

	if (frame_desc->type == MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME &&
	    !frame_desc->is_stale) {
		if (least_greater_entry_found) {
			status = mgmt_rx_reo_update_wait_count(
					num_mlo_links,
					&new_entry->wait_count,
					&least_greater_entry->wait_count);

			if (QDF_IS_STATUS_ERROR(status))
				goto error;

			if (new_entry->wait_count.total_count == 0)
				new_entry->status &=
					~MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS;
		}

		new_entry->insertion_ts = qdf_get_log_timestamp();
		new_entry->ingress_timestamp = frame_desc->ingress_timestamp;

		status = qdf_list_insert_before(&reo_list->list,
						&new_entry->node,
						&least_greater_entry->node);
		if (QDF_IS_STATUS_ERROR(status))
			goto error;

		*is_queued = true;
	}

	cur_entry = least_greater_entry;
	qdf_list_for_each_from(&reo_list->list, cur_entry, node) {
		uint8_t frame_link_id;

		frame_link_id = mgmt_rx_reo_get_link_id(frame_desc->rx_params);
		if (cur_entry->wait_count.per_link_count[frame_link_id]) {
			cur_entry->wait_count.per_link_count[frame_link_id]--;
			cur_entry->wait_count.total_count--;
			if (cur_entry->wait_count.total_count == 0)
				cur_entry->status &=
					~MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS;
		}
	}

	status = QDF_STATUS_SUCCESS;
	goto exit;

error:
	/* Cleanup the entry if it is not queued */
	if (!*is_queued) {
		struct wlan_objmgr_pdev *pdev;
		uint8_t link_id;

		link_id = mgmt_rx_reo_get_link_id(new_entry->rx_params);

		pdev = wlan_get_pdev_from_mlo_link_id(link_id);
		/**
		 * New entry created is not inserted to reorder list, free
		 * the entry and release the reference
		 */
		if (pdev)
			wlan_objmgr_pdev_release_ref(pdev, WLAN_MGMT_RX_REO_ID);
		else
			mgmt_rx_reo_err("Unable to get pdev corresponding to entry %pK",
					new_entry);
		qdf_mem_free(new_entry);
	}

exit:
	qdf_spin_unlock_bh(&reo_list->list_lock);

	if (!*is_queued)
		return status;

	if (frame_desc->type == MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME) {
		if (least_greater_entry_found)
			mgmt_rx_reo_debug("Inserting new entry %pK before %pK",
					  new_entry, least_greater_entry);
		else
			mgmt_rx_reo_debug("Inserting new entry %pK at the tail",
					  new_entry);
	}

	return status;
}

/**
 * mgmt_rx_reo_list_init() - Initialize the management rx-reorder list
 * @reo_list: Pointer to reorder list
 *
 * API to initialize the management rx-reorder list.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_list_init(struct mgmt_rx_reo_list *reo_list)
{
	QDF_STATUS status;

	reo_list->max_list_size = MGMT_RX_REO_LIST_MAX_SIZE;
	reo_list->list_entry_timeout_us = MGMT_RX_REO_LIST_TIMEOUT_US;

	qdf_list_create(&reo_list->list, reo_list->max_list_size);
	qdf_spinlock_create(&reo_list->list_lock);

	status = qdf_timer_init(NULL, &reo_list->ageout_timer,
				mgmt_rx_reo_list_ageout_timer_handler, reo_list,
				QDF_TIMER_TYPE_WAKE_APPS);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to initialize reo list ageout timer");
		return status;
	}

	reo_list->ts_last_delivered_frame.valid = false;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_mgmt_rx_reo_update_host_snapshot() - Update Host snapshot with the MGMT
 * Rx REO parameters.
 * @pdev: pdev extracted from the WMI event
 * @reo_params: MGMT Rx REO parameters received in the WMI event
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
wlan_mgmt_rx_reo_update_host_snapshot(struct wlan_objmgr_pdev *pdev,
				      struct mgmt_rx_reo_params *reo_params)
{
	struct mgmt_rx_reo_pdev_info *rx_reo_pdev_ctx;
	struct mgmt_rx_reo_snapshot_params *host_ss;

	if (!reo_params) {
		mgmt_rx_reo_err("Mgmt Rx REO params are NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	rx_reo_pdev_ctx = wlan_mgmt_rx_reo_get_priv_object(pdev);
	if (!rx_reo_pdev_ctx) {
		mgmt_rx_reo_err("Mgmt Rx REO context empty for pdev %pK", pdev);
		return QDF_STATUS_E_FAILURE;
	}

	/* FW should send valid REO parameters */
	if (!reo_params->valid) {
		mgmt_rx_reo_err("Mgmt Rx REO params is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	host_ss = &rx_reo_pdev_ctx->host_snapshot;
	host_ss->valid = true;
	host_ss->global_timestamp = reo_params->global_timestamp;
	host_ss->mgmt_pkt_ctr = reo_params->mgmt_pkt_ctr;

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_MGMT_RX_REO_DEBUG_SUPPORT
/**
 * mgmt_rx_reo_log_ingress_frame() - Log the information about a frame entering
 * the reorder algorithm.
 * @reo_ctx: management rx reorder context
 * @desc: Pointer to frame descriptor
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_log_ingress_frame(struct mgmt_rx_reo_context *reo_ctx,
			      struct mgmt_rx_reo_frame_descriptor *desc)
{
	struct reo_ingress_debug_info *ingress_frame_debug_info;
	struct reo_ingress_debug_frame_info *cur_frame_debug_info;

	if (!reo_ctx || !desc)
		return QDF_STATUS_E_NULL_VALUE;

	ingress_frame_debug_info = &reo_ctx->ingress_frame_debug_info;

	cur_frame_debug_info = &ingress_frame_debug_info->debug_info
			[ingress_frame_debug_info->next_index];

	cur_frame_debug_info->link_id =
				mgmt_rx_reo_get_link_id(desc->rx_params);
	cur_frame_debug_info->mgmt_pkt_ctr =
				mgmt_rx_reo_get_pkt_counter(desc->rx_params);
	cur_frame_debug_info->global_timestamp =
				mgmt_rx_reo_get_global_ts(desc->rx_params);
	cur_frame_debug_info->type = desc->type;
	cur_frame_debug_info->wait_count = desc->wait_count;
	cur_frame_debug_info->ingress_timestamp = desc->ingress_timestamp;

	ingress_frame_debug_info->next_index++;
	ingress_frame_debug_info->next_index %=
				MGMT_RX_REO_INGRESS_FRAME_DEBUG_ENTRIES_MAX;

	return QDF_STATUS_SUCCESS;
}
#else
/**
 * mgmt_rx_reo_log_ingress_frame() - Log the information about a frame entering
 * the reorder algorithm.
 * @reo_ctx: management rx reorder context
 * @desc: Pointer to frame descriptor
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_log_ingress_frame(struct mgmt_rx_reo_context *reo_ctx,
			      struct mgmt_rx_reo_frame_descriptor *desc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MGMT_RX_REO_DEBUG_SUPPORT */

QDF_STATUS
wlan_mgmt_rx_reo_algo_entry(struct wlan_objmgr_pdev *pdev,
			    struct mgmt_rx_reo_frame_descriptor *desc,
			    bool *is_queued)
{
	struct mgmt_rx_reo_context *reo_ctx;
	QDF_STATUS status;
	int8_t num_mlo_links;

	if (!is_queued)
		return QDF_STATUS_E_NULL_VALUE;

	*is_queued = false;

	if (!desc || !desc->rx_params) {
		mgmt_rx_reo_err("MGMT Rx REO descriptor or rx params are null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	reo_ctx = mgmt_rx_reo_get_context();
	if (!reo_ctx) {
		mgmt_rx_reo_err("REO context is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	num_mlo_links = mgmt_rx_reo_get_num_mlo_links(reo_ctx);
	qdf_assert_always(num_mlo_links > 0);
	qdf_assert_always(num_mlo_links <= MGMT_RX_REO_MAX_LINKS);

	/**
	 * Critical Section = Host snapshot update + Calculation of wait
	 * counts + Update reorder list. Following section describes the
	 * motivation for making this a critical section.
	 * Lets take an example of 2 links (Link A & B) and each has received
	 * a management frame A1 and B1 such that MLO global time stamp of A1 <
	 * MLO global time stamp of B1. Host is concurrently executing
	 * "wlan_mgmt_rx_reo_algo_entry" for A1 and B1 in 2 different CPUs.
	 *
	 * A lock less version of this API("wlan_mgmt_rx_reo_algo_entry_v1") is
	 * as follows.
	 *
	 * wlan_mgmt_rx_reo_algo_entry()
	 * {
	 *     Host snapshot update
	 *     Calculation of wait counts
	 *     Update reorder list
	 *     Release to upper layer
	 * }
	 *
	 * We may run into race conditions under the following sequence of
	 * operations.
	 *
	 * 1. Host snapshot update for link A in context of frame A1
	 * 2. Host snapshot update for link B in context of frame B1
	 * 3. Calculation of wait count for frame B1
	 *        link A wait count =  0
	 *        link B wait count =  0
	 * 4. Update reorder list with frame B1
	 * 5. Release B1 to upper layer
	 * 6. Calculation of wait count for frame A1
	 *        link A wait count =  0
	 *        link B wait count =  0
	 * 7. Update reorder list with frame A1
	 * 8. Release A1 to upper layer
	 *
	 * This leads to incorrect behaviour as B1 goes to upper layer before
	 * A1.
	 *
	 * To prevent this lets make Host snapshot update + Calculate wait count
	 * a critical section by adding locks. The updated version of the API
	 * ("wlan_mgmt_rx_reo_algo_entry_v2") is as follows.
	 *
	 * wlan_mgmt_rx_reo_algo_entry()
	 * {
	 *     LOCK
	 *         Host snapshot update
	 *         Calculation of wait counts
	 *     UNLOCK
	 *     Update reorder list
	 *     Release to upper layer
	 * }
	 *
	 * With this API also We may run into race conditions under the
	 * following sequence of operations.
	 *
	 * 1. Host snapshot update for link A in context of frame A1 +
	 *    Calculation of wait count for frame A1
	 *        link A wait count =  0
	 *        link B wait count =  0
	 * 2. Host snapshot update for link B in context of frame B1 +
	 *    Calculation of wait count for frame B1
	 *        link A wait count =  0
	 *        link B wait count =  0
	 * 4. Update reorder list with frame B1
	 * 5. Release B1 to upper layer
	 * 7. Update reorder list with frame A1
	 * 8. Release A1 to upper layer
	 *
	 * This also leads to incorrect behaviour as B1 goes to upper layer
	 * before A1.
	 *
	 * To prevent this, let's make Host snapshot update + Calculate wait
	 * count + Update reorder list a critical section by adding locks.
	 * The updated version of the API ("wlan_mgmt_rx_reo_algo_entry_final")
	 * is as follows.
	 *
	 * wlan_mgmt_rx_reo_algo_entry()
	 * {
	 *     LOCK
	 *         Host snapshot update
	 *         Calculation of wait counts
	 *         Update reorder list
	 *     UNLOCK
	 *     Release to upper layer
	 * }
	 */
	qdf_spin_lock(&reo_ctx->reo_algo_entry_lock);

	/* Update the Host snapshot */
	status = wlan_mgmt_rx_reo_update_host_snapshot(
						pdev,
						desc->rx_params->reo_params);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock(&reo_ctx->reo_algo_entry_lock);
		return QDF_STATUS_E_FAILURE;
	}

	/* Compute wait count for this frame/event */
	status = wlan_mgmt_rx_reo_algo_calculate_wait_count(
						pdev,
						desc->rx_params->reo_params,
						num_mlo_links,
						&desc->wait_count);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock(&reo_ctx->reo_algo_entry_lock);
		return QDF_STATUS_E_FAILURE;
	}

	/* Update the REO list */
	status = mgmt_rx_reo_update_list(&reo_ctx->reo_list, num_mlo_links,
					 desc, is_queued);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock(&reo_ctx->reo_algo_entry_lock);
		return QDF_STATUS_E_FAILURE;
	}

	status = mgmt_rx_reo_log_ingress_frame(reo_ctx, desc);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock(&reo_ctx->reo_algo_entry_lock);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_unlock(&reo_ctx->reo_algo_entry_lock);

	/* Finally, release the entries for which pending frame is received */
	return mgmt_rx_reo_list_release_entries(reo_ctx);
}

#ifndef WLAN_MGMT_RX_REO_SIM_SUPPORT
/**
 * mgmt_rx_reo_sim_init() - Initialize management rx reorder simulation
 * context.
 * @reo_context: Pointer to reo context
 *
 * Return: QDF_STATUS of operation
 */
static inline QDF_STATUS
mgmt_rx_reo_sim_init(struct mgmt_rx_reo_context *reo_context)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_deinit() - De initialize management rx reorder simulation
 * context.
 * @reo_context: Pointer to reo context
 *
 * Return: QDF_STATUS of operation
 */
static inline QDF_STATUS
mgmt_rx_reo_sim_deinit(struct mgmt_rx_reo_context *reo_context)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_sim_pdev_object_create_notification(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_sim_pdev_object_destroy_notification(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#else
/**
 * mgmt_rx_reo_sim_remove_frame_from_pending_list() - Removes frame from the
 * pending management frame list
 * @pending_frame_list: pointer to pending management frame list
 * @frame: pointer to management frame parameters
 *
 * This API removes frames from the pending management frame list. This API is
 * used in case of FW consumed management frames or management frames which
 * are dropped at host due to any error.
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_sim_remove_frame_from_pending_list(
		struct mgmt_rx_reo_pending_frame_list *pending_frame_list,
		const struct mgmt_rx_frame_params *frame)
{
	struct mgmt_rx_reo_pending_frame_list_entry *cur_entry;
	struct mgmt_rx_reo_pending_frame_list_entry *matching_entry = NULL;
	QDF_STATUS status;

	if (!pending_frame_list) {
		mgmt_rx_reo_err("Mgmt pending frame list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame) {
		mgmt_rx_reo_err("Pointer to mgmt frame params is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock(&pending_frame_list->lock);

	qdf_list_for_each(&pending_frame_list->list, cur_entry, node) {
		if (cur_entry->params.link_id == frame->link_id &&
		    cur_entry->params.mgmt_pkt_ctr == frame->mgmt_pkt_ctr &&
		    cur_entry->params.global_timestamp ==
		    frame->global_timestamp) {
			matching_entry = cur_entry;
			break;
		}
	}

	if (!matching_entry) {
		qdf_spin_unlock(&pending_frame_list->lock);
		mgmt_rx_reo_err("No matching frame in the list to remove");
		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_list_remove_node(&pending_frame_list->list,
				      &matching_entry->node);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock(&pending_frame_list->lock);
		mgmt_rx_reo_err("Failed to remove the matching entry");
		return status;
	}

	qdf_spin_unlock(&pending_frame_list->lock);

	qdf_mem_free(matching_entry);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_add_frame_to_pending_list() - Inserts frame to the
 * pending management frame list
 * @pending_frame_list: pointer to pending management frame list
 * @frame: pointer to management frame parameters
 *
 * This API inserts frames to the pending management frame list. This API is
 * used to insert frames generated by the MAC HW to the pending frame list.
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_sim_add_frame_to_pending_list(
		struct mgmt_rx_reo_pending_frame_list *pending_frame_list,
		const struct mgmt_rx_frame_params *frame)
{
	struct mgmt_rx_reo_pending_frame_list_entry *new_entry;
	QDF_STATUS status;

	if (!pending_frame_list) {
		mgmt_rx_reo_err("Mgmt pending frame list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame) {
		mgmt_rx_reo_err("Pointer mgmt frame params is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	new_entry = qdf_mem_malloc(sizeof(*new_entry));
	if (!new_entry) {
		mgmt_rx_reo_err("Failed to allocate new entry to frame list");
		return QDF_STATUS_E_NOMEM;
	}

	new_entry->params = *frame;

	qdf_spin_lock(&pending_frame_list->lock);

	status = qdf_list_insert_back(&pending_frame_list->list,
				      &new_entry->node);

	qdf_spin_unlock(&pending_frame_list->lock);

	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to add frame to pending list");
		qdf_mem_free(new_entry);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * struct mgmt_rx_reo_pending_frame_debug_info - This structure holds the
 * necessary information about an entry in the pending frame list. This can be
 * used for debug purposes.
 * @link_id: link id
 * @mgmt_pkt_ctr: management packet counter
 * @global_timestamp: global time stamp
 */
struct mgmt_rx_reo_pending_frame_debug_info {
	uint8_t link_id;
	uint16_t mgmt_pkt_ctr;
	uint32_t global_timestamp;
};

/**
 * mgmt_rx_reo_sim_print_pending_frame_list() - Print the contents of management
 * rx-reorder pending frame list used for simulation.
 * @pending_frame_list: Pointer to pending frame list
 *
 * This API prints the contents of management pending frame list used for
 * simulation.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_print_pending_frame_list(
		struct mgmt_rx_reo_pending_frame_list *pending_frame_list)
{
	struct mgmt_rx_reo_pending_frame_list_entry *cur_entry;
	uint32_t index;
	uint32_t pending_frame_list_size;
	struct mgmt_rx_reo_pending_frame_debug_info *debug_info;

	if (!pending_frame_list) {
		mgmt_rx_reo_err("Pointer to pending frame list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock(&pending_frame_list->lock);

	pending_frame_list_size = qdf_list_size(&pending_frame_list->list);

	qdf_spin_unlock(&pending_frame_list->lock);

	if (pending_frame_list_size == 0) {
		mgmt_rx_reo_debug("Num entries in the pending frame list = %u",
				  pending_frame_list_size);
		return QDF_STATUS_SUCCESS;
	}

	debug_info =
		qdf_mem_malloc(pending_frame_list_size * sizeof(*debug_info));
	if (!debug_info) {
		mgmt_rx_reo_err("Memory allocation failed");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock(&pending_frame_list->lock);

	index = 0;
	qdf_list_for_each(&pending_frame_list->list, cur_entry, node) {
		struct mgmt_rx_frame_params *params = &cur_entry->params;

		debug_info[index].link_id = params->link_id;
		debug_info[index].mgmt_pkt_ctr = params->mgmt_pkt_ctr;
		debug_info[index].global_timestamp = params->global_timestamp;

		++index;
	}

	qdf_spin_unlock(&pending_frame_list->lock);

	mgmt_rx_reo_debug("Pending management frames list");
	mgmt_rx_reo_debug("##################################################");
	for (index = 0; index < pending_frame_list_size; index++)
		mgmt_rx_reo_debug("index %u: link_id = %u, ctr = %u, ts = %u",
				  index, debug_info[index].link_id,
				  debug_info[index].mgmt_pkt_ctr,
				  debug_info[index].global_timestamp);
	mgmt_rx_reo_debug("##################################################");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_sim_process_rx_frame(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t buf,
				 struct mgmt_rx_event_params *mgmt_rx_params)
{
	struct mgmt_rx_reo_context *reo_context;
	struct mgmt_rx_reo_sim_context *sim_context;
	QDF_STATUS status;
	struct mgmt_rx_reo_params *reo_params;
	int8_t num_mlo_links;

	if (!mgmt_rx_params) {
		mgmt_rx_reo_err("Mgmt rx params null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	reo_params = mgmt_rx_params->reo_params;

	reo_context = mgmt_rx_reo_get_context();
	if (!reo_context) {
		mgmt_rx_reo_err("Mgmt reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	sim_context = &reo_context->sim_context;

	num_mlo_links = mgmt_rx_reo_sim_get_num_mlo_links(sim_context);

	if (num_mlo_links < 0 || num_mlo_links > MGMT_RX_REO_MAX_LINKS) {
		mgmt_rx_reo_err("Invalid number of MLO links %d",
				num_mlo_links);
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock(&sim_context->pending_frame_list.lock);

	if (qdf_list_empty(&sim_context->pending_frame_list.list)) {
		qdf_spin_unlock(&sim_context->pending_frame_list.lock);
		mgmt_rx_reo_err("reo sim failure: pending frame list is empty");
		qdf_assert_always(0);
	} else {
		struct mgmt_rx_frame_params *cur_entry_params;
		struct mgmt_rx_reo_pending_frame_list_entry *cur_entry;
		struct mgmt_rx_reo_pending_frame_list_entry *matching_entry = NULL;

		/**
		 * Make sure the frames delivered to upper layer are in the
		 * increasing order of global time stamp. For that the frame
		 * which is being delivered should be present at the head of the
		 * pending frame list. There could be multiple frames with the
		 * same global time stamp in the pending frame list. Search
		 * among all the frames at the head of the list which has the
		 * same global time stamp as the frame which is being delivered.
		 * To find matching frame, check whether packet counter,
		 * global time stamp and link id are same.
		 */
		qdf_list_for_each(&sim_context->pending_frame_list.list,
				  cur_entry, node) {
			cur_entry_params = &cur_entry->params;

			if (cur_entry_params->global_timestamp !=
			    reo_params->global_timestamp)
				break;

			if (cur_entry_params->link_id == reo_params->link_id &&
			    cur_entry_params->mgmt_pkt_ctr ==
			    reo_params->mgmt_pkt_ctr) {
				matching_entry = cur_entry;
				break;
			}
		}

		if (!matching_entry) {
			qdf_spin_unlock(&sim_context->pending_frame_list.lock);
			mgmt_rx_reo_err("reo sim failure: mismatch");
			qdf_assert_always(0);
		}

		status = qdf_list_remove_node(
					&sim_context->pending_frame_list.list,
					&matching_entry->node);
		qdf_mem_free(matching_entry);
		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_spin_unlock(&sim_context->pending_frame_list.lock);
			mgmt_rx_reo_err("Failed to remove matching entry");
			return status;
		}
	}

	qdf_spin_unlock(&sim_context->pending_frame_list.lock);

	mgmt_rx_reo_debug("Successfully processed mgmt frame");
	mgmt_rx_reo_debug("link_id = %u, ctr = %u, ts = %u",
			  reo_params->link_id, reo_params->mgmt_pkt_ctr,
			  reo_params->global_timestamp);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_get_random_bool() - Generate true/false randomly
 * @percentage_true: probability (in percentage) of true
 *
 * API to generate true with probability @percentage_true % and false with
 * probability (100 - @percentage_true) %.
 *
 * Return: true with probability @percentage_true % and false with probability
 * (100 - @percentage_true) %
 */
static bool
mgmt_rx_reo_sim_get_random_bool(uint8_t percentage_true)
{
	uint32_t rand;

	if (percentage_true > 100) {
		mgmt_rx_reo_err("Invalid probability value for true, %u",
				percentage_true);
		return -EINVAL;
	}

	get_random_bytes(&rand, sizeof(rand));

	return ((rand % 100) < percentage_true);
}

/**
 * mgmt_rx_reo_sim_get_random_unsigned_int() - Generate random unsigned integer
 * value in the range [0, max)
 * @max: upper limit for the output
 *
 * API to generate random unsigned integer value in the range [0, max).
 *
 * Return: unsigned integer value in the range [0, max)
 */
static uint32_t
mgmt_rx_reo_sim_get_random_unsigned_int(uint32_t max)
{
	uint32_t rand;

	get_random_bytes(&rand, sizeof(rand));

	return (rand % max);
}

/**
 * mgmt_rx_reo_sim_sleep() - Wrapper API to sleep for given micro seconds
 * @sleeptime_us: Sleep time in micro seconds
 *
 * This API uses msleep() internally. So the granularity is limited to
 * milliseconds.
 *
 * Return: none
 */
static void
mgmt_rx_reo_sim_sleep(uint32_t sleeptime_us)
{
	msleep(sleeptime_us / USEC_PER_MSEC);
}

/**
 * mgmt_rx_reo_sim_frame_handler_host() - Management frame handler at the host
 * layer
 * @arg: Argument
 *
 * This API handles the management frame at the host layer. This is applicable
 * for simulation alone.
 *
 * Return: none
 */
static void
mgmt_rx_reo_sim_frame_handler_host(void *arg)
{
	struct mgmt_rx_frame_fw *frame_fw = (struct mgmt_rx_frame_fw *)arg;
	uint32_t fw_to_host_delay_us;
	bool is_error_frame = false;
	struct mgmt_rx_reo_frame_descriptor *frame_descriptor;
	int8_t link_id = -1;
	struct mgmt_rx_event_params *rx_params;
	enum mgmt_rx_reo_frame_descriptor_type frame_type;
	QDF_STATUS status;
	struct mgmt_rx_reo_sim_context *sim_context;
	bool is_queued = false;
	struct wlan_objmgr_pdev *pdev;

	if (!frame_fw) {
		mgmt_rx_reo_err("HOST-%d : Pointer to FW frame struct is null",
				link_id);
		goto error_print;
	}

	link_id = frame_fw->params.link_id;

	sim_context = frame_fw->sim_context;
	if (!sim_context) {
		mgmt_rx_reo_err("HOST-%d : Mgmt rx reo simulation context null",
				link_id);
		goto error_free_fw_frame;
	}

	fw_to_host_delay_us = MGMT_RX_REO_SIM_DELAY_FW_TO_HOST_MIN +
			      mgmt_rx_reo_sim_get_random_unsigned_int(
			      MGMT_RX_REO_SIM_DELAY_FW_TO_HOST_MIN_MAX_DELTA);

	mgmt_rx_reo_sim_sleep(fw_to_host_delay_us);

	if (!frame_fw->is_consumed_by_fw) {
		is_error_frame = mgmt_rx_reo_sim_get_random_bool(
				 MGMT_RX_REO_SIM_PERCENTAGE_ERROR_FRAMES);

		if (is_error_frame) {
			status = mgmt_rx_reo_sim_remove_frame_from_pending_list(
					&sim_context->pending_frame_list,
					&frame_fw->params);

			if (QDF_IS_STATUS_ERROR(status)) {
				mgmt_rx_reo_err("HOST-%d : Failed to remove error frame",
						link_id);
				goto error_free_fw_frame;
			}
		}
	}

	mgmt_rx_reo_debug("HOST-%d : Received frame with ts = %u, ctr = %u, consume = %u, error = %u",
			  link_id, frame_fw->params.global_timestamp,
			  frame_fw->params.mgmt_pkt_ctr,
			  frame_fw->is_consumed_by_fw, is_error_frame);

	frame_descriptor = qdf_mem_malloc(sizeof(*frame_descriptor));
	if (!frame_descriptor) {
		mgmt_rx_reo_err("HOST-%d : Failed to allocate descriptor",
				link_id);
		goto error_free_fw_frame;
	}

	rx_params = alloc_mgmt_rx_event_params();
	if (!rx_params) {
		mgmt_rx_reo_err("HOST-%d : Failed to allocate event params",
				link_id);
		goto error_free_frame_descriptor;
	}

	rx_params->reo_params->link_id = frame_fw->params.link_id;
	rx_params->reo_params->global_timestamp =
					frame_fw->params.global_timestamp;
	rx_params->reo_params->mgmt_pkt_ctr = frame_fw->params.mgmt_pkt_ctr;
	rx_params->reo_params->valid = true;

	frame_descriptor->nbuf = NULL;
	frame_descriptor->rx_params = rx_params;
	frame_descriptor->ingress_timestamp = qdf_get_log_timestamp();

	if (is_error_frame)
		frame_type = MGMT_RX_REO_FRAME_DESC_ERROR_FRAME;
	else
		if (frame_fw->is_consumed_by_fw)
			frame_type = MGMT_RX_REO_FRAME_DESC_FW_CONSUMED_FRAME;
		else
			frame_type = MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME;
	frame_descriptor->type = frame_type;

	pdev = wlan_get_pdev_from_mlo_link_id(link_id);
	if (!pdev) {
		mgmt_rx_reo_err("No pdev corresponding to link_id %d", link_id);
		goto error_free_mgmt_rx_event_params;
	}

	status = wlan_mgmt_rx_reo_algo_entry(pdev, frame_descriptor,
					     &is_queued);

	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to execute reo algorithm");
		goto error_free_mgmt_rx_event_params;
	}

	if (!is_queued)
		free_mgmt_rx_event_params(frame_descriptor->rx_params);
	qdf_mem_free(frame_descriptor);
	qdf_mem_free(frame_fw);

	return;

error_free_mgmt_rx_event_params:
	if (!is_queued)
		free_mgmt_rx_event_params(frame_descriptor->rx_params);
error_free_frame_descriptor:
	qdf_mem_free(frame_descriptor);
error_free_fw_frame:
	qdf_mem_free(frame_fw);
error_print:
	mgmt_rx_reo_err("HOST-%d : Exiting host frame handler due to error",
			link_id);
}

/**
 * mgmt_rx_reo_sim_write_snapshot() - API to write snapshots used for management
 * frame reordering
 * @link_id: link id
 * @id: snapshot id
 * @value: snapshot value
 *
 * This API writes the snapshots used for management frame reordering. MAC HW
 * and FW can use this API to update the MAC HW/FW consumed/FW forwarded
 * snapshots.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_write_snapshot(uint8_t link_id,
			       enum mgmt_rx_reo_shared_snapshot_id id,
			       struct mgmt_rx_reo_snapshot value)
{
	struct wlan_objmgr_pdev *pdev;
	struct mgmt_rx_reo_snapshot *snapshot_address;
	QDF_STATUS status;

	pdev = wlan_get_pdev_from_mlo_link_id(link_id);

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = mgmt_rx_reo_sim_get_snapshot_address(pdev, id,
						      &snapshot_address);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to get snapshot address %d of pdev %pK",
				id, pdev);
		return QDF_STATUS_E_FAILURE;
	}

	snapshot_address->mgmt_rx_reo_snapshot_low =
						value.mgmt_rx_reo_snapshot_low;
	snapshot_address->mgmt_rx_reo_snapshot_high =
						value.mgmt_rx_reo_snapshot_high;

	return QDF_STATUS_SUCCESS;
}

#define MGMT_RX_REO_SNAPSHOT_LOW_VALID_POS                       (0)
#define MGMT_RX_REO_SNAPSHOT_LOW_VALID_SIZE                      (1)
#define MGMT_RX_REO_SNAPSHOT_LOW_MGMT_PKT_CTR_POS                (1)
#define MGMT_RX_REO_SNAPSHOT_LOW_MGMT_PKT_CTR_SIZE               (16)
#define MGMT_RX_REO_SNAPSHOT_LOW_GLOBAL_TIMESTAMP_POS            (17)
#define MGMT_RX_REO_SNAPSHOT_LOW_GLOBAL_TIMESTAMP_SIZE           (15)

#define MGMT_RX_REO_SNAPSHOT_HIGH_GLOBAL_TIMESTAMP_POS           (0)
#define MGMT_RX_REO_SNAPSHOT_HIGH_GLOBAL_TIMESTAMP_SIZE          (17)
#define MGMT_RX_REO_SNAPSHOT_HIGH_MGMT_PKT_CTR_REDUNDANT_POS     (17)
#define MGMT_RX_REO_SNAPSHOT_HIGH_MGMT_PKT_CTR_REDUNDANT_SIZE    (15)

/**
 * mgmt_rx_reo_sim_get_snapshot_value() - API to get snapshot value for a given
 * management frame
 * @global_timestamp: global time stamp
 * @mgmt_pkt_ctr: management packet counter
 *
 * This API gets the snapshot value for a frame with time stamp
 * @global_timestamp and sequence number @mgmt_pkt_ctr.
 *
 * Return: snapshot value (struct mgmt_rx_reo_snapshot)
 */
static struct mgmt_rx_reo_snapshot
mgmt_rx_reo_sim_get_snapshot_value(uint32_t global_timestamp,
				   uint16_t mgmt_pkt_ctr)
{
	struct mgmt_rx_reo_snapshot snapshot = {0};

	QDF_SET_BITS(snapshot.mgmt_rx_reo_snapshot_low,
		     MGMT_RX_REO_SNAPSHOT_LOW_VALID_POS,
		     MGMT_RX_REO_SNAPSHOT_LOW_VALID_SIZE, 1);
	QDF_SET_BITS(snapshot.mgmt_rx_reo_snapshot_low,
		     MGMT_RX_REO_SNAPSHOT_LOW_MGMT_PKT_CTR_POS,
		     MGMT_RX_REO_SNAPSHOT_LOW_MGMT_PKT_CTR_SIZE, mgmt_pkt_ctr);
	QDF_SET_BITS(snapshot.mgmt_rx_reo_snapshot_low,
		     MGMT_RX_REO_SNAPSHOT_LOW_GLOBAL_TIMESTAMP_POS,
		     MGMT_RX_REO_SNAPSHOT_LOW_GLOBAL_TIMESTAMP_SIZE,
		     global_timestamp);

	QDF_SET_BITS(snapshot.mgmt_rx_reo_snapshot_high,
		     MGMT_RX_REO_SNAPSHOT_HIGH_GLOBAL_TIMESTAMP_POS,
		     MGMT_RX_REO_SNAPSHOT_HIGH_GLOBAL_TIMESTAMP_SIZE,
		     global_timestamp >> 15);
	QDF_SET_BITS(snapshot.mgmt_rx_reo_snapshot_high,
		     MGMT_RX_REO_SNAPSHOT_HIGH_MGMT_PKT_CTR_REDUNDANT_POS,
		     MGMT_RX_REO_SNAPSHOT_HIGH_MGMT_PKT_CTR_REDUNDANT_SIZE,
		     mgmt_pkt_ctr);

	return snapshot;
}

/**
 * mgmt_rx_reo_sim_frame_handler_fw() - Management frame handler at the fw layer
 * @arg: Argument
 *
 * This API handles the management frame at the fw layer. This is applicable
 * for simulation alone.
 *
 * Return: none
 */
static void
mgmt_rx_reo_sim_frame_handler_fw(void *arg)
{
	struct mgmt_rx_frame_mac_hw *frame_hw =
					(struct mgmt_rx_frame_mac_hw *)arg;
	uint32_t mac_hw_to_fw_delay_us;
	bool is_consumed_by_fw;
	struct  mgmt_rx_frame_fw *frame_fw;
	int8_t link_id = -1;
	QDF_STATUS status;
	struct mgmt_rx_reo_sim_context *sim_context;
	enum mgmt_rx_reo_shared_snapshot_id snapshot_id;
	struct mgmt_rx_reo_snapshot snapshot_value;
	bool ret;

	if (!frame_hw) {
		mgmt_rx_reo_err("FW-%d : Pointer to HW frame struct is null",
				link_id);
		qdf_assert_always(0);
	}

	link_id = frame_hw->params.link_id;

	sim_context = frame_hw->sim_context;
	if (!sim_context) {
		mgmt_rx_reo_err("FW-%d : Mgmt rx reo simulation context null",
				link_id);
		goto error_free_mac_hw_frame;
	}

	mac_hw_to_fw_delay_us = MGMT_RX_REO_SIM_DELAY_MAC_HW_TO_FW_MIN +
			mgmt_rx_reo_sim_get_random_unsigned_int(
			MGMT_RX_REO_SIM_DELAY_MAC_HW_TO_FW_MIN_MAX_DELTA);
	mgmt_rx_reo_sim_sleep(mac_hw_to_fw_delay_us);

	is_consumed_by_fw = mgmt_rx_reo_sim_get_random_bool(
			    MGMT_RX_REO_SIM_PERCENTAGE_FW_CONSUMED_FRAMES);

	if (is_consumed_by_fw) {
		/**
		 * "pending_frame_list" keeps track of all the management frames
		 * which are supposed to be consumed by host. This frame is
		 * consumed by FW and hence we have to remove it from the
		 * "pending_frame_list".
		 */
		status = mgmt_rx_reo_sim_remove_frame_from_pending_list(
					&sim_context->pending_frame_list,
					&frame_hw->params);

		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("FW-%d : Failed to remove consumed frame",
					link_id);
			goto error_free_mac_hw_frame;
		}
	}

	mgmt_rx_reo_debug("FW-%d : Processing frame with ts = %u, ctr = %u, consume = %u",
			  link_id, frame_hw->params.global_timestamp,
			  frame_hw->params.mgmt_pkt_ctr, is_consumed_by_fw);

	frame_fw = qdf_mem_malloc(sizeof(*frame_fw));
	if (!frame_fw) {
		mgmt_rx_reo_err("FW-%d : Failed to allocate FW mgmt frame",
				link_id);
		goto error_free_mac_hw_frame;
	}

	frame_fw->params = frame_hw->params;
	frame_fw->is_consumed_by_fw = is_consumed_by_fw;
	frame_fw->sim_context = frame_hw->sim_context;

	snapshot_id = is_consumed_by_fw ?
		      MGMT_RX_REO_SHARED_SNAPSHOT_FW_CONSUMED :
		      MGMT_RX_REO_SHARED_SNAPSHOT_FW_FORWADED;

	snapshot_value = mgmt_rx_reo_sim_get_snapshot_value(
					frame_hw->params.global_timestamp,
					frame_hw->params.mgmt_pkt_ctr);

	status = mgmt_rx_reo_sim_write_snapshot(link_id, snapshot_id,
						snapshot_value);

	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("FW-%d : Failed to write snapshot %d",
				link_id, snapshot_id);
		goto error_free_fw_frame;
	}

	status = qdf_create_work(NULL, &frame_fw->frame_handler_host,
				 mgmt_rx_reo_sim_frame_handler_host, frame_fw);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("FW-%d : Failed to create work", link_id);
		goto error_free_fw_frame;
	}

	ret = qdf_queue_work(
			NULL, sim_context->host_mgmt_frame_handler[link_id],
			&frame_fw->frame_handler_host);
	if (!ret) {
		mgmt_rx_reo_err("FW-%d : Work is already present on the queue",
				link_id);
		goto error_free_fw_frame;
	}

	qdf_mem_free(frame_hw);

	return;

error_free_fw_frame:
	qdf_mem_free(frame_fw);
error_free_mac_hw_frame:
	qdf_mem_free(frame_hw);

	mgmt_rx_reo_err("FW-%d : Exiting fw frame handler due to error",
			link_id);
}

/**
 * mgmt_rx_reo_sim_receive_from_air() - Simulate management frame reception from
 * the air
 * @mac_hw: pointer to structure representing MAC HW
 * @num_mlo_links: number of MLO HW links
 * @frame: pointer to management frame parameters
 *
 * This API simulates the management frame reception from air.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_receive_from_air(struct mgmt_rx_reo_sim_mac_hw *mac_hw,
				 uint8_t num_mlo_links,
				 struct mgmt_rx_frame_params *frame)
{
	uint8_t link_id;

	if (!mac_hw) {
		mgmt_rx_reo_err("pointer to MAC HW struct is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (num_mlo_links == 0 || num_mlo_links > MGMT_RX_REO_MAX_LINKS) {
		mgmt_rx_reo_err("Invalid number of MLO links %u",
				num_mlo_links);
		return QDF_STATUS_E_INVAL;
	}

	if (!frame) {
		mgmt_rx_reo_err("pointer to frame parameters is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	link_id = mgmt_rx_reo_sim_get_random_unsigned_int(num_mlo_links);

	frame->global_timestamp = div_u64(ktime_get_ns(), NSEC_PER_USEC);
	frame->mgmt_pkt_ctr = ++mac_hw->mgmt_pkt_ctr[link_id];
	frame->link_id = link_id;

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_undo_receive_from_air() - API to restore the state of MAC
 * HW in case of any Rx error.
 * @mac_hw: pointer to structure representing MAC HW
 * @frame: pointer to management frame parameters
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_undo_receive_from_air(struct mgmt_rx_reo_sim_mac_hw *mac_hw,
				      struct mgmt_rx_frame_params *frame)
{
	if (!mac_hw) {
		mgmt_rx_reo_err("pointer to MAC HW struct is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!frame) {
		mgmt_rx_reo_err("pointer to frame parameters is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (frame->link_id >= MGMT_RX_REO_MAX_LINKS) {
		mgmt_rx_reo_err("Invalid link id %u", frame->link_id);
		return QDF_STATUS_E_INVAL;
	}

	--mac_hw->mgmt_pkt_ctr[frame->link_id];

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_mac_hw_thread() - kthread to simulate MAC HW
 * @data: pointer to data input
 *
 * kthread handler to simulate MAC HW.
 *
 * Return: 0 for success, else failure
 */
static int
mgmt_rx_reo_sim_mac_hw_thread(void *data)
{
	struct mgmt_rx_reo_sim_context *sim_context = data;
	struct mgmt_rx_reo_sim_mac_hw *mac_hw;

	if (!sim_context) {
		mgmt_rx_reo_err("HW: Mgmt rx reo simulation context is null");
		return -EINVAL;
	}

	mac_hw = &sim_context->mac_hw_sim.mac_hw_info;

	while (!qdf_thread_should_stop()) {
		uint32_t inter_frame_delay_us;
		struct mgmt_rx_frame_params frame;
		struct mgmt_rx_frame_mac_hw *frame_mac_hw;
		int8_t link_id = -1;
		QDF_STATUS status;
		enum mgmt_rx_reo_shared_snapshot_id snapshot_id;
		struct mgmt_rx_reo_snapshot snapshot_value;
		int8_t num_mlo_links;
		bool ret;

		num_mlo_links = mgmt_rx_reo_sim_get_num_mlo_links(sim_context);
		if (num_mlo_links < 0 ||
		    num_mlo_links > MGMT_RX_REO_MAX_LINKS) {
			mgmt_rx_reo_err("Invalid number of MLO links %d",
					num_mlo_links);
			qdf_assert_always(0);
		}

		status = mgmt_rx_reo_sim_receive_from_air(mac_hw, num_mlo_links,
							  &frame);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("Receive from the air failed");
			/**
			 * Frame reception failed and we are not sure about the
			 * link id. Without link id there is no way to restore
			 * the mac hw state. Hence assert unconditionally.
			 */
			qdf_assert_always(0);
		}
		link_id = frame.link_id;

		mgmt_rx_reo_debug("HW-%d: received frame with ts = %u, ctr = %u",
				  link_id, frame.global_timestamp,
				  frame.mgmt_pkt_ctr);

		frame_mac_hw = qdf_mem_malloc(sizeof(*frame_mac_hw));
		if (!frame_mac_hw) {
			mgmt_rx_reo_err("HW-%d: Failed to alloc mac hw frame",
					link_id);

			/* Cleanup */
			status = mgmt_rx_reo_sim_undo_receive_from_air(
								mac_hw, &frame);
			qdf_assert_always(QDF_IS_STATUS_SUCCESS(status));

			continue;
		}

		frame_mac_hw->params = frame;
		frame_mac_hw->sim_context = sim_context;

		status = mgmt_rx_reo_sim_add_frame_to_pending_list(
				&sim_context->pending_frame_list, &frame);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("HW-%d: Failed to add frame to list",
					link_id);

			/* Cleanup */
			status = mgmt_rx_reo_sim_undo_receive_from_air(
								mac_hw, &frame);
			qdf_assert_always(QDF_IS_STATUS_SUCCESS(status));

			qdf_mem_free(frame_mac_hw);

			continue;
		}

		snapshot_id = MGMT_RX_REO_SHARED_SNAPSHOT_MAC_HW;
		snapshot_value = mgmt_rx_reo_sim_get_snapshot_value(
						frame.global_timestamp,
						frame.mgmt_pkt_ctr);

		status = mgmt_rx_reo_sim_write_snapshot(link_id, snapshot_id,
							snapshot_value);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("HW-%d : Failed to write snapshot %d",
					link_id, snapshot_id);

			/* Cleanup */
			status = mgmt_rx_reo_sim_remove_frame_from_pending_list(
				&sim_context->pending_frame_list, &frame);
			qdf_assert_always(QDF_IS_STATUS_SUCCESS(status));

			status = mgmt_rx_reo_sim_undo_receive_from_air(
								mac_hw, &frame);
			qdf_assert_always(QDF_IS_STATUS_SUCCESS(status));

			qdf_mem_free(frame_mac_hw);

			continue;
		}

		status = qdf_create_work(NULL, &frame_mac_hw->frame_handler_fw,
					 mgmt_rx_reo_sim_frame_handler_fw,
					 frame_mac_hw);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("HW-%d : Failed to create work",
					link_id);
			qdf_assert_always(0);
		}

		ret = qdf_queue_work(
			NULL, sim_context->fw_mgmt_frame_handler[link_id],
			&frame_mac_hw->frame_handler_fw);
		if (!ret) {
			mgmt_rx_reo_err("HW-%d : Work is already present in Q",
					link_id);
			qdf_assert_always(0);
		}

		inter_frame_delay_us = MGMT_RX_REO_SIM_INTER_FRAME_DELAY_MIN +
			mgmt_rx_reo_sim_get_random_unsigned_int(
			MGMT_RX_REO_SIM_INTER_FRAME_DELAY_MIN_MAX_DELTA);

		mgmt_rx_reo_sim_sleep(inter_frame_delay_us);
	}

	return 0;
}

/**
 * mgmt_rx_reo_sim_init_pending_frame_list() - Initializes the pending
 * management frame list
 * @pending_frame_list: Pointer to pending frame list
 *
 * This API initializes the pending management frame list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_init_pending_frame_list(
		struct mgmt_rx_reo_pending_frame_list *pending_frame_list)
{
	qdf_spinlock_create(&pending_frame_list->lock);
	qdf_list_create(&pending_frame_list->list,
			MGMT_RX_REO_SIM_PENDING_FRAME_LIST_MAX_SIZE);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_init_stale_frame_list() - Initializes the stale
 * management frame list
 * @stale_frame_list: Pointer to stale frame list
 *
 * This API initializes the stale management frame list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_init_stale_frame_list(
		struct mgmt_rx_reo_stale_frame_list *stale_frame_list)
{
	qdf_spinlock_create(&stale_frame_list->lock);
	qdf_list_create(&stale_frame_list->list,
			MGMT_RX_REO_SIM_STALE_FRAME_LIST_MAX_SIZE);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_flush_pending_frame_list() - Flush all the entries in the
 * pending management frame list.
 * @pending_frame_list: Pointer to pending frame list
 *
 * This API flushes all the entries in the pending management frame list used
 * for simulation.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_flush_pending_frame_list(
		struct mgmt_rx_reo_pending_frame_list *pending_frame_list)
{
	struct mgmt_rx_reo_pending_frame_list_entry *cur_entry;
	struct mgmt_rx_reo_pending_frame_list_entry *temp;
	QDF_STATUS status;

	qdf_spin_lock(&pending_frame_list->lock);

	qdf_list_for_each_del(&pending_frame_list->list, cur_entry, temp,
			      node) {
		status = qdf_list_remove_node(&pending_frame_list->list,
					      &cur_entry->node);
		if (QDF_IS_STATUS_ERROR(status))
			break;

		qdf_mem_free(cur_entry);
	}

	qdf_spin_unlock(&pending_frame_list->lock);

	return status;
}

/**
 * mgmt_rx_reo_sim_deinit_pending_frame_list() - De initializes the pending
 * management frame list
 * @pending_frame_list: Pointer to pending frame list
 *
 * This API de initializes the pending management frame list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_deinit_pending_frame_list(
		struct mgmt_rx_reo_pending_frame_list *pending_frame_list)
{
	qdf_spin_lock(&pending_frame_list->lock);
	qdf_list_destroy(&pending_frame_list->list);
	qdf_spin_unlock(&pending_frame_list->lock);

	qdf_spinlock_destroy(&pending_frame_list->lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_deinit_stale_frame_list() - De initializes the stale
 * management frame list
 * @stale_frame_list: Pointer to stale frame list
 *
 * This API de initializes the stale management frame list
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_deinit_stale_frame_list(
		struct mgmt_rx_reo_stale_frame_list *stale_frame_list)
{
	qdf_spin_lock(&stale_frame_list->lock);
	qdf_list_destroy(&stale_frame_list->list);
	qdf_spin_unlock(&stale_frame_list->lock);

	qdf_spinlock_destroy(&stale_frame_list->lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_insert_into_link_id_to_pdev_map() - Builds the MLO HW link id
 * to pdev map
 * @link_id_to_pdev_map: pointer to link id to pdev map
 * @pdev: pointer to pdev object
 *
 * This API incrementally builds the MLO HW link id to pdev map. This API is
 * used only for simulation.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_insert_into_link_id_to_pdev_map(
		struct mgmt_rx_reo_sim_link_id_to_pdev_map *link_id_to_pdev_map,
		struct wlan_objmgr_pdev *pdev)
{
	if (!link_id_to_pdev_map) {
		mgmt_rx_reo_err("Link id to pdev map is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock(&link_id_to_pdev_map->lock);

	link_id_to_pdev_map->map[link_id_to_pdev_map->num_mlo_links] = pdev;
	link_id_to_pdev_map->num_mlo_links++;

	qdf_spin_unlock(&link_id_to_pdev_map->lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_remove_from_link_id_to_pdev_map() - Destroys the MLO HW link
 * id to pdev map
 * @link_id_to_pdev_map: pointer to link id to pdev map
 * @pdev: pointer to pdev object
 *
 * This API incrementally destroys the MLO HW link id to pdev map. This API is
 * used only for simulation.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_sim_remove_from_link_id_to_pdev_map(
		struct mgmt_rx_reo_sim_link_id_to_pdev_map *link_id_to_pdev_map,
		struct wlan_objmgr_pdev *pdev)
{
	uint8_t link_id;

	if (!link_id_to_pdev_map) {
		mgmt_rx_reo_err("Link id to pdev map is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock(&link_id_to_pdev_map->lock);

	for (link_id = 0; link_id < link_id_to_pdev_map->num_mlo_links;
	     link_id++) {
		if (link_id_to_pdev_map->map[link_id] == pdev) {
			link_id_to_pdev_map->map[link_id] = NULL;
			qdf_spin_unlock(&link_id_to_pdev_map->lock);

			return QDF_STATUS_SUCCESS;
		}
	}

	qdf_spin_unlock(&link_id_to_pdev_map->lock);

	mgmt_rx_reo_err("Pdev %pK is not found in map", pdev);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
mgmt_rx_reo_sim_pdev_object_create_notification(struct wlan_objmgr_pdev *pdev)
{
	struct mgmt_rx_reo_sim_context *sim_context;
	QDF_STATUS status;

	sim_context = mgmt_rx_reo_sim_get_context();
	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt simulation context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = mgmt_rx_reo_sim_insert_into_link_id_to_pdev_map(
				&sim_context->link_id_to_pdev_map, pdev);

	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to add pdev to the map %pK", pdev);
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_sim_pdev_object_destroy_notification(struct wlan_objmgr_pdev *pdev)
{
	struct mgmt_rx_reo_sim_context *sim_context;
	QDF_STATUS status;

	sim_context = mgmt_rx_reo_sim_get_context();
	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt simulation context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = mgmt_rx_reo_sim_remove_from_link_id_to_pdev_map(
				&sim_context->link_id_to_pdev_map, pdev);

	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to remove pdev from the map");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_sim_start(void)
{
	struct mgmt_rx_reo_sim_context *sim_context;
	qdf_thread_t *mac_hw_thread;
	uint8_t link_id;
	uint8_t id;
	int8_t num_mlo_links;
	QDF_STATUS status;

	sim_context = mgmt_rx_reo_sim_get_context();
	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt reo simulation context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	num_mlo_links = mgmt_rx_reo_sim_get_num_mlo_links(sim_context);
	if (num_mlo_links <= 0) {
		mgmt_rx_reo_err("Invalid number of MLO links %d",
				num_mlo_links);
		return QDF_STATUS_E_INVAL;
	}

	for (link_id = 0; link_id < num_mlo_links; link_id++) {
		struct workqueue_struct *wq;

		wq = alloc_ordered_workqueue("mgmt_rx_reo_sim_host-%u", 0,
					     link_id);
		if (!wq) {
			mgmt_rx_reo_err("Host workqueue creation failed");
			status = QDF_STATUS_E_FAILURE;
			goto error_destroy_fw_and_host_work_queues_till_last_link;
		}
		sim_context->host_mgmt_frame_handler[link_id] = wq;

		wq = alloc_ordered_workqueue("mgmt_rx_reo_sim_fw-%u", 0,
					     link_id);
		if (!wq) {
			mgmt_rx_reo_err("FW workqueue creation failed");
			status = QDF_STATUS_E_FAILURE;
			goto error_destroy_host_work_queue_of_last_link;
		}
		sim_context->fw_mgmt_frame_handler[link_id] = wq;
	}

	mac_hw_thread = qdf_create_thread(mgmt_rx_reo_sim_mac_hw_thread,
					  sim_context, "MAC_HW_thread");
	if (!mac_hw_thread) {
		mgmt_rx_reo_err("MAC HW thread creation failed");
		status = QDF_STATUS_E_FAILURE;
		goto error_destroy_fw_and_host_work_queues_of_last_link;
	}

	sim_context->mac_hw_sim.mac_hw_thread = mac_hw_thread;

	qdf_wake_up_process(sim_context->mac_hw_sim.mac_hw_thread);

	return QDF_STATUS_SUCCESS;

error_destroy_fw_and_host_work_queues_of_last_link:
	drain_workqueue(sim_context->fw_mgmt_frame_handler[link_id]);
	destroy_workqueue(sim_context->fw_mgmt_frame_handler[link_id]);

error_destroy_host_work_queue_of_last_link:
	drain_workqueue(sim_context->host_mgmt_frame_handler[link_id]);
	destroy_workqueue(sim_context->host_mgmt_frame_handler[link_id]);

error_destroy_fw_and_host_work_queues_till_last_link:
	for (id = 0; id < link_id; id++) {
		drain_workqueue(sim_context->fw_mgmt_frame_handler[id]);
		destroy_workqueue(sim_context->fw_mgmt_frame_handler[id]);

		drain_workqueue(sim_context->host_mgmt_frame_handler[id]);
		destroy_workqueue(sim_context->host_mgmt_frame_handler[id]);
	}

	return status;
}

QDF_STATUS
mgmt_rx_reo_sim_stop(void)
{
	struct mgmt_rx_reo_context *reo_context;
	struct mgmt_rx_reo_sim_context *sim_context;
	struct mgmt_rx_reo_pending_frame_list *pending_frame_list;
	struct mgmt_rx_reo_stale_frame_list *stale_frame_list;
	uint8_t link_id;
	QDF_STATUS status;
	int8_t num_mlo_links;

	reo_context = mgmt_rx_reo_get_context();
	if (!reo_context) {
		mgmt_rx_reo_err("reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	sim_context = &reo_context->sim_context;

	num_mlo_links = mgmt_rx_reo_sim_get_num_mlo_links(sim_context);
	if (num_mlo_links <= 0) {
		mgmt_rx_reo_err("Invalid number of MLO links %d",
				num_mlo_links);
		return QDF_STATUS_E_INVAL;
	}

	status = qdf_thread_join(sim_context->mac_hw_sim.mac_hw_thread);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to stop the thread");
		return status;
	}

	sim_context->mac_hw_sim.mac_hw_thread = NULL;

	for (link_id = 0; link_id < num_mlo_links; link_id++) {
		/* Wait for all the pending frames to be processed by FW */
		drain_workqueue(sim_context->fw_mgmt_frame_handler[link_id]);
		destroy_workqueue(sim_context->fw_mgmt_frame_handler[link_id]);

		/* Wait for all the pending frames to be processed by host */
		drain_workqueue(sim_context->host_mgmt_frame_handler[link_id]);
		destroy_workqueue(
				sim_context->host_mgmt_frame_handler[link_id]);
	}

	pending_frame_list = &sim_context->pending_frame_list;
	stale_frame_list = &sim_context->stale_frame_list;
	if (!qdf_list_empty(&pending_frame_list->list) ||
	    !qdf_list_empty(&stale_frame_list->list)) {
		mgmt_rx_reo_err("reo sim failure: pending/stale frame list non empty");

		status = mgmt_rx_reo_list_display(
				&reo_context->reo_list, num_mlo_links);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("Failed to print reorder list");
			return status;
		}

		status = mgmt_rx_reo_sim_print_pending_frame_list(
							pending_frame_list);
		if (QDF_IS_STATUS_ERROR(status)) {
			mgmt_rx_reo_err("Failed to print mgmt frame list");
			return status;
		}

		qdf_assert_always(0);
	} else {
		mgmt_rx_reo_err("reo sim passed");
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_init() - Initialize management rx reorder simulation
 * context.
 * @reo_context: Pointer to reo context
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_sim_init(struct mgmt_rx_reo_context *reo_context)
{
	QDF_STATUS status;
	struct mgmt_rx_reo_sim_context *sim_context;

	if (!reo_context) {
		mgmt_rx_reo_err("reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	sim_context = &reo_context->sim_context;

	qdf_mem_zero(sim_context, sizeof(*sim_context));

	status = mgmt_rx_reo_sim_init_pending_frame_list(
					&sim_context->pending_frame_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to create mgmt frame list");
		return status;
	}

	status = mgmt_rx_reo_sim_init_stale_frame_list(
					&sim_context->stale_frame_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to create stale frame list");
		return status;
	}

	qdf_spinlock_create(&sim_context->link_id_to_pdev_map.lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_sim_deinit() - De initialize management rx reorder simulation
 * context.
 * @reo_context: Pointer to reo context
 *
 * Return: QDF_STATUS of operation
 */
static QDF_STATUS
mgmt_rx_reo_sim_deinit(struct mgmt_rx_reo_context *reo_context)
{
	QDF_STATUS status;
	struct mgmt_rx_reo_sim_context *sim_context;

	if (!reo_context) {
		mgmt_rx_reo_err("reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	sim_context = &reo_context->sim_context;

	qdf_spinlock_destroy(&sim_context->link_id_to_pdev_map.lock);

	status = mgmt_rx_reo_sim_deinit_stale_frame_list(
					&sim_context->stale_frame_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to destroy stale frame list");
		return status;
	}

	status = mgmt_rx_reo_sim_deinit_pending_frame_list(
					&sim_context->pending_frame_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to destroy mgmt frame list");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_sim_get_snapshot_address(
			struct wlan_objmgr_pdev *pdev,
			enum mgmt_rx_reo_shared_snapshot_id id,
			struct mgmt_rx_reo_snapshot **address)
{
	int8_t link_id;
	struct mgmt_rx_reo_sim_context *sim_context;

	sim_context = mgmt_rx_reo_sim_get_context();
	if (!sim_context) {
		mgmt_rx_reo_err("Mgmt reo simulation context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!pdev) {
		mgmt_rx_reo_err("pdev is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (id < 0 || id >= MGMT_RX_REO_SHARED_SNAPSHOT_MAX) {
		mgmt_rx_reo_err("Invalid snapshot ID %d", id);
		return QDF_STATUS_E_INVAL;
	}

	if (!address) {
		mgmt_rx_reo_err("Pointer to snapshot address is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	link_id = wlan_get_mlo_link_id_from_pdev(pdev);
	if (link_id < 0 || link_id >= MGMT_RX_REO_MAX_LINKS) {
		mgmt_rx_reo_err("Invalid link id %d for the pdev %pK", link_id,
				pdev);
		return QDF_STATUS_E_INVAL;
	}

	*address = &sim_context->snapshot[link_id][id];

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MGMT_RX_REO_SIM_SUPPORT */

/**
 * mgmt_rx_reo_flush_reorder_list() - Flush all entries in the reorder list
 * @reo_list: Pointer to reorder list
 *
 * API to flush all the entries of the reorder list. This API would acquire
 * the lock protecting the list.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_flush_reorder_list(struct mgmt_rx_reo_list *reo_list)
{
	struct mgmt_rx_reo_list_entry *cur_entry;
	struct mgmt_rx_reo_list_entry *temp;

	if (!reo_list) {
		mgmt_rx_reo_err("reorder list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&reo_list->list_lock);

	qdf_list_for_each_del(&reo_list->list, cur_entry, temp, node) {
		uint8_t link_id;
		struct wlan_objmgr_pdev *pdev = NULL;

		free_mgmt_rx_event_params(cur_entry->rx_params);

		link_id = mgmt_rx_reo_get_link_id(cur_entry->rx_params);

		pdev = wlan_get_pdev_from_mlo_link_id(link_id);
		if (!pdev) {
			qdf_spin_unlock_bh(&reo_list->list_lock);
			mgmt_rx_reo_err("Pdev for link_id %u is null", link_id);
			return QDF_STATUS_E_NULL_VALUE;
		}

		/**
		 * Release the reference taken when the entry is inserted into
		 * the reorder list.
		 */
		wlan_objmgr_pdev_release_ref(pdev, WLAN_MGMT_RX_REO_ID);

		qdf_mem_free(cur_entry);
	}

	qdf_spin_unlock_bh(&reo_list->list_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_list_deinit() - De initialize the management rx-reorder list
 * @reo_list: Pointer to reorder list
 *
 * API to de initialize the management rx-reorder list.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_list_deinit(struct mgmt_rx_reo_list *reo_list)
{
	QDF_STATUS status;

	qdf_timer_free(&reo_list->ageout_timer);

	status = mgmt_rx_reo_flush_reorder_list(reo_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to flush the reorder list");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_spinlock_destroy(&reo_list->list_lock);
	qdf_list_destroy(&reo_list->list);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_deinit_context(void)
{
	QDF_STATUS status;
	struct mgmt_rx_reo_context *reo_context;

	reo_context = mgmt_rx_reo_get_context();
	if (!reo_context) {
		mgmt_rx_reo_err("reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spinlock_destroy(&reo_context->reo_algo_entry_lock);

	status = mgmt_rx_reo_sim_deinit(reo_context);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to de initialize reo sim context");
		return QDF_STATUS_E_FAILURE;
	}

	status = mgmt_rx_reo_list_deinit(&reo_context->reo_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to de-initialize mgmt Rx reo list");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
mgmt_rx_reo_init_context(void)
{
	QDF_STATUS status;
	QDF_STATUS temp;
	struct mgmt_rx_reo_context *reo_context;

	reo_context = mgmt_rx_reo_get_context();
	if (!reo_context) {
		mgmt_rx_reo_err("reo context is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	qdf_mem_zero(reo_context, sizeof(*reo_context));

	status = mgmt_rx_reo_list_init(&reo_context->reo_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to initialize mgmt Rx reo list");
		return status;
	}

	status = mgmt_rx_reo_sim_init(reo_context);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to initialize reo simulation context");
		goto error_reo_list_deinit;
	}

	qdf_spinlock_create(&reo_context->reo_algo_entry_lock);

	return QDF_STATUS_SUCCESS;

error_reo_list_deinit:
	temp = mgmt_rx_reo_list_deinit(&reo_context->reo_list);
	if (QDF_IS_STATUS_ERROR(temp)) {
		mgmt_rx_reo_err("Failed to de-initialize mgmt Rx reo list");
		return temp;
	}

	return status;
}

/**
 * wlan_mgmt_rx_reo_initialize_snapshot_params() - Initialize a given snapshot
 * params object
 * @snapshot_params: Pointer to snapshot params object
 *
 * Return: void
 */
static void
wlan_mgmt_rx_reo_initialize_snapshot_params(
			struct mgmt_rx_reo_snapshot_params *snapshot_params)
{
	snapshot_params->valid = false;
	snapshot_params->mgmt_pkt_ctr = 0;
	snapshot_params->global_timestamp = 0;
}

QDF_STATUS
mgmt_rx_reo_pdev_obj_create_notification(
	struct wlan_objmgr_pdev *pdev,
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	QDF_STATUS status;
	QDF_STATUS temp_status;
	struct mgmt_rx_reo_pdev_info *mgmt_rx_reo_pdev_ctx = NULL;
	enum mgmt_rx_reo_shared_snapshot_id snapshot_id;

	if (!pdev) {
		mgmt_rx_reo_err("pdev is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto failure;
	}

	if (!wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(pdev)) {
		status = QDF_STATUS_SUCCESS;
		goto failure;
	}

	status = mgmt_rx_reo_sim_pdev_object_create_notification(pdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to handle pdev create for reo sim");
		goto failure;
	}

	mgmt_rx_reo_pdev_ctx = qdf_mem_malloc(sizeof(*mgmt_rx_reo_pdev_ctx));
	if (!mgmt_rx_reo_pdev_ctx) {
		mgmt_rx_reo_err("Allocation failure for REO pdev context");
		status = QDF_STATUS_E_NOMEM;
		goto failure;
	}

	snapshot_id = 0;
	while (snapshot_id < MGMT_RX_REO_SHARED_SNAPSHOT_MAX) {
		struct mgmt_rx_reo_snapshot **snapshot_address;

		snapshot_address = &mgmt_rx_reo_pdev_ctx->
				host_target_shared_snapshot[snapshot_id];
		temp_status = wlan_mgmt_rx_reo_get_snapshot_address(
				pdev, snapshot_id, snapshot_address);
		if (QDF_IS_STATUS_ERROR(temp_status)) {
			mgmt_rx_reo_err("Get snapshot address failed, id = %u",
					snapshot_id);
			status = temp_status;
			goto failure;
		}

		wlan_mgmt_rx_reo_initialize_snapshot_params(
				&mgmt_rx_reo_pdev_ctx->
				last_valid_shared_snapshot[snapshot_id]);
		snapshot_id++;
	}

	/* Initialize Host snapshot params */
	wlan_mgmt_rx_reo_initialize_snapshot_params(&mgmt_rx_reo_pdev_ctx->
						    host_snapshot);

	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = mgmt_rx_reo_pdev_ctx;

	return QDF_STATUS_SUCCESS;

failure:
	if (mgmt_rx_reo_pdev_ctx)
		qdf_mem_free(mgmt_rx_reo_pdev_ctx);

	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = NULL;

	return status;
}

QDF_STATUS
mgmt_rx_reo_pdev_obj_destroy_notification(
	struct wlan_objmgr_pdev *pdev,
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx)
{
	QDF_STATUS status;

	if (!wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(pdev))
		return QDF_STATUS_SUCCESS;

	qdf_mem_free(mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx);
	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = NULL;

	status = mgmt_rx_reo_sim_pdev_object_destroy_notification(pdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to handle pdev create for reo sim");
		return status;
	}

	return QDF_STATUS_SUCCESS;
}
