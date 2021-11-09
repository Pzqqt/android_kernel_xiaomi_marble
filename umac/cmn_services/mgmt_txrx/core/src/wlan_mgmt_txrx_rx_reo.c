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

	if (mgmt_rx_reo_list_max_size_exceeded(reo_list))
		release_reason |=
		   MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_LIST_MAX_SIZE_EXCEEDED;

	if (!MGMT_RX_REO_LIST_ENTRY_IS_WAITING_FOR_FRAME_ON_OTHER_LINK(entry))
		release_reason |=
			MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_ZERO_WAIT_COUNT;

	if (MGMT_RX_REO_LIST_ENTRY_IS_AGED_OUT(entry))
		release_reason |=
				MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_AGED_OUT;

	if (reo_list->ts_latest_aged_out_frame.valid &&
	    MGMT_RX_REO_LIST_ENTRY_IS_OLDER_THAN_LATEST_AGED_OUT_FRAME(
				&reo_list->ts_latest_aged_out_frame, entry))
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
 * acquiring the lock which protects the reorder list.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
mgmt_rx_reo_list_entry_send_up(struct mgmt_rx_reo_list *reo_list,
			       struct mgmt_rx_reo_list_entry *entry)
{
	uint8_t release_reason;
	QDF_STATUS status;
	uint8_t link_id;
	struct wlan_objmgr_pdev *pdev;
	uint32_t entry_global_ts;
	struct mgmt_rx_reo_global_ts_info *ts_last_delivered_frame;

	qdf_assert_always(reo_list);
	qdf_assert_always(entry);

	entry_global_ts = mgmt_rx_reo_get_global_ts(entry->rx_params);
	ts_last_delivered_frame = &reo_list->ts_last_delivered_frame;

	release_reason = mgmt_rx_reo_list_entry_get_release_reason(
					reo_list, entry);

	qdf_assert_always(release_reason != 0);

	status = qdf_list_remove_node(&reo_list->list, &entry->node);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to remove entry %pK from list", entry);
		qdf_assert_always(0);
	}

	link_id = mgmt_rx_reo_get_link_id(entry->rx_params);

	/**
	 * Last delivered frame global time stamp is invalid means that
	 * current frame is the first frame to be delivered to the upper layer
	 * from the reorder list. Blindly update the last delivered frame global
	 * time stamp to the current frame's global time stamp and set the valid
	 * to true.
	 * If the last delivered frame global time stamp is valid and
	 * current frame's global time stamp is >= last delivered frame global
	 * time stamp, deliver the current frame to upper layer and update the
	 * last delivered frame global time stamp.
	 */
	if (!ts_last_delivered_frame->valid ||
	    mgmt_rx_reo_compare_global_timestamps_gte(
		    entry_global_ts, ts_last_delivered_frame->global_ts)) {
		/* TODO Process current management frame here */

		ts_last_delivered_frame->global_ts = entry_global_ts;
		ts_last_delivered_frame->valid = true;
	} else {
		/**
		 * We need to replicate all the cleanup activities which the
		 * upper layer would have done.
		 */
		qdf_nbuf_free(entry->nbuf);
	}

	free_mgmt_rx_event_params(entry->rx_params);

	pdev = wlan_get_pdev_from_mlo_link_id(link_id);
	if (!pdev) {
		mgmt_rx_reo_err("Unable to get pdev corresponding to entry %pK",
				entry);
		return QDF_STATUS_E_FAILURE;
	}

	/**
	 * Release the reference taken when the entry is inserted into
	 * the reorder list
	 */
	wlan_objmgr_pdev_release_ref(pdev, WLAN_MGMT_RX_REO_ID);

	return QDF_STATUS_SUCCESS;
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
	       (reo_list->ts_latest_aged_out_frame.valid &&
		MGMT_RX_REO_LIST_ENTRY_IS_OLDER_THAN_LATEST_AGED_OUT_FRAME(
				&reo_list->ts_latest_aged_out_frame, entry));
}

/**
 * mgmt_rx_reo_list_release_entries() - Release entries from the reorder list
 * @reo_list: Pointer to reorder list
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
mgmt_rx_reo_list_release_entries(struct mgmt_rx_reo_list *reo_list)
{
	struct mgmt_rx_reo_list_entry *cur_entry;
	struct mgmt_rx_reo_list_entry *temp;
	/* TODO yield if release_count > THRESHOLD */
	uint16_t release_count = 0;
	QDF_STATUS status;

	if (!reo_list) {
		mgmt_rx_reo_err("reo list is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&reo_list->list_lock);

	qdf_list_for_each_del(&reo_list->list, cur_entry, temp, node) {
		if (mgmt_rx_reo_list_is_ready_to_send_up_entry(reo_list,
							       cur_entry)) {
			mgmt_rx_reo_debug("Freeing up entry %pK", cur_entry);
			status = mgmt_rx_reo_list_entry_send_up(reo_list,
								cur_entry);
			if (QDF_IS_STATUS_ERROR(status))
				goto error;

			release_count++;
		} else {
			break;
		}

		qdf_mem_free(cur_entry);
	}

	status = QDF_STATUS_SUCCESS;
error:
	qdf_spin_unlock_bh(&reo_list->list_lock);
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

	if (!reo_list)
		return;

	qdf_spin_lock_bh(&reo_list->list_lock);

	cur_ts = qdf_get_log_timestamp();

	qdf_list_for_each(&reo_list->list, cur_entry, node) {
		if (cur_ts - cur_entry->insertion_ts >=
		    reo_list->list_entry_timeout_us) {
			uint32_t cur_entry_global_ts;
			struct mgmt_rx_reo_global_ts_info *ts_ageout;

			ts_ageout = &reo_list->ts_latest_aged_out_frame;
			cur_entry_global_ts = mgmt_rx_reo_get_global_ts(
					      cur_entry->rx_params);

			if (!ts_ageout->valid ||
			    mgmt_rx_reo_compare_global_timestamps_gte(
			    cur_entry_global_ts, ts_ageout->global_ts)) {
				ts_ageout->global_ts = cur_entry_global_ts;
				ts_ageout->valid = true;
			}

			cur_entry->status |= MGMT_RX_REO_STATUS_AGED_OUT;
		}
	}

	qdf_spin_unlock_bh(&reo_list->list_lock);

	status = mgmt_rx_reo_list_release_entries(reo_list);
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

	if (frame_desc->type == MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME) {
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
	reo_list->ts_latest_aged_out_frame.valid = false;

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

/**
 * mgmt_rx_reo_get_num_mlo_links() - Get number of MLO HW links from the reo
 * context object
 * @reo_context: Pointer to reo context object

 * Return: On success returns number of MLO HW links. On failure returns -1.
 */
static int8_t
mgmt_rx_reo_get_num_mlo_links(struct mgmt_rx_reo_context *reo_context)
{
	if (!reo_context) {
		mgmt_rx_reo_err("Mgmt reo context is null");
		return MGMT_RX_REO_INVALID_NUM_LINKS;
	}

	return reo_context->num_mlo_links;
}

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

	qdf_spin_unlock(&reo_ctx->reo_algo_entry_lock);

	/* Finally, release the entries for which pending frame is received */
	return mgmt_rx_reo_list_release_entries(&reo_ctx->reo_list);
}

QDF_STATUS
mgmt_rx_reo_init_context(void)
{
	QDF_STATUS status;
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

	qdf_spinlock_create(&reo_context->reo_algo_entry_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * mgmt_rx_reo_flush_reorder_list() - flush all entries in the reorder list
 * @reo_list: pointer to reorder list
 *
 * api to flush all the entries of the reorder list. this api would acquire
 * the lock protecting the list.
 *
 * return: qdf_status
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
			mgmt_rx_reo_err("pdev for link_id %u is null", link_id);
			return QDF_STATUS_E_NULL_VALUE;
		}

		/**
		 * Release the reference taken when the entry is inserted into
		 * the reorder list
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

	status = mgmt_rx_reo_list_deinit(&reo_context->reo_list);
	if (QDF_IS_STATUS_ERROR(status)) {
		mgmt_rx_reo_err("Failed to de-initialize mgmt Rx reo list");
		return status;
	}

	return QDF_STATUS_SUCCESS;
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
		temp_status = tgt_mgmt_rx_reo_get_snapshot_address(
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
	if (!wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(pdev))
		return QDF_STATUS_SUCCESS;

	qdf_mem_free(mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx);
	mgmt_txrx_pdev_ctx->mgmt_rx_reo_pdev_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}
