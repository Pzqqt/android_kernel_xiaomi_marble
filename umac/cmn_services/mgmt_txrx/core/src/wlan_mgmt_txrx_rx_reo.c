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

struct mgmt_rx_reo_context rx_reo_global;

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
 * TODO: Dummy function to get the MLO link ID from the pdev.
 * This is added only as a place holder for the time being.
 * Remove this once this API is implemented in MLO manager.
 */
static inline uint8_t
wlan_get_mlo_link_id_from_pdev(struct wlan_objmgr_pdev *pdev)
{
	return 0;
}

/**
 * TODO: Dummy function to get pdev handle from MLO link ID.
 * This is added only as a place holder for the time being.
 * Remove this once this API is implemented in MLO manager.
 */
static inline struct wlan_objmgr_pdev *
wlan_get_pdev_from_mlo_link_id(uint8_t mlo_lin_id)
{
	return NULL;
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
	uint8_t link, in_frame_link;
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

	qdf_assert(num_mlo_links <= MAX_MLO_LINKS);

	/* Get the MLO link ID of incoming frame */
	in_frame_link = wlan_get_mlo_link_id_from_pdev(in_frame_pdev);

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
			mgmt_rx_reo_err("Mgmt Rx REO context empty for pdev %pK",
					pdev);
			return QDF_STATUS_E_FAILURE;
		}

		host_ss = &rx_reo_pdev_ctx->host_snapshot;

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
		if (mgmt_rx_reo_compare_global_timestamps_gte(
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

			status = tgt_mgmt_rx_reo_read_snapshot(
						pdev, address, snapshot_id,
						&snapshot_params[snapshot_id]);

			/* Read operation shouldn't fail */
			if (QDF_IS_STATUS_ERROR(status)) {
				mgmt_rx_reo_err("snapshot(%d) read failed on"
						"link (%d)", snapshot_id, link);
				return status;
			}

			/* If snpashot is valid, save it in the pdev context */
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
			wait_count->total_count = UINT_MAX;
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
			wait_count->per_link_count[link] = frames_pending;
			wait_count->total_count += frames_pending;
	}

	return QDF_STATUS_SUCCESS;
}
