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
 *  DOC: mlo_global_h_shmem_arena_api.c
 *  This file contains definition of functions that MLO global
 *  shared memory arena exposes.
 */
#include <mlo_global_h_shmem_arena.h>

/**
 * mgmt_rx_reo_snapshot_is_valid() - Check if an MGMT Rx REO snapshot is valid
 * @snapshot_low: lower 32-bits of the snapshot
 *
 * Return: true if snapshot is valid, else false
 */
static bool mgmt_rx_reo_snapshot_is_valid(uint32_t snapshot_low)
{
	return MLO_SHMEM_MGMT_RX_REO_SNAPSHOT_PARAM_VALID_GET(snapshot_low);
}

/**
 * mgmt_rx_reo_snapshot_get_mgmt_pkt_ctr() - Get the management packet counter
 * from an MGMT Rx REO snapshot
 * @snapshot_low: lower 32-bits of the snapshot
 *
 * Return: Management packet counter of the snapshot
 */
static uint16_t mgmt_rx_reo_snapshot_get_mgmt_pkt_ctr(uint32_t snapshot_low)
{
	return MLO_SHMEM_MGMT_RX_REO_SNAPSHOT_PARAM_MGMT_PKT_CTR_GET(
			snapshot_low);
}

/**
 * mgmt_rx_reo_snapshot_get_mgmt_pkt_ctr() - Get the redundant management packet
 * counter from MGMT Rx REO snapshot
 * @snapshot_high: higher 32-bits of the snapshot
 *
 * Return: Redundant management packet counter of the snapshot
 */
static uint16_t mgmt_rx_reo_snapshot_get_redundant_mgmt_pkt_ctr(
	uint32_t snapshot_high)
{
	return MLO_SHMEM_MGMT_RX_REO_SNAPSHOT_PARAM_MGMT_PKT_CTR_REDUNDANT_GET(
			snapshot_high);
}

/**
 * mgmt_rx_reo_snapshot_is_consistent() - Check if an MGMT Rx REO snapshot is
 * consistent
 * @mgmt_pkt_ctr: Management packet counter of the snapshot
 * @redundant_mgmt_pkt_ctr: Redundant management packet counter of the snapshot
 *
 * Return: true if the snapshot is consistent, else false
 */
static bool mgmt_rx_reo_snapshot_is_consistent(uint16_t mgmt_pkt_ctr,
					       uint16_t redundant_mgmt_pkt_ctr)
{
	return MLO_SHMEM_MGMT_RX_REO_SNAPSHOT_IS_CONSISTENT(
			mgmt_pkt_ctr, redundant_mgmt_pkt_ctr);
}

/**
 * mgmt_rx_reo_snapshot_get_global_timestamp() - Get the global timestamp from
 * MGMT Rx REO snapshot
 * @snapshot_low: lower 32-bits of the snapshot
 * @snapshot_high: higher 32-bits of the snapshot
 *
 * Return: Global timestamp of the snapshot
 */
static uint32_t mgmt_rx_reo_snapshot_get_global_timestamp(
	uint32_t snapshot_low, uint32_t snapshot_high)
{
	return MLO_SHMEM_MGMT_RX_REO_SNAPSHOT_PARAM_GLOBAL_TIMESTAMP_GET_FROM_DWORDS(
		snapshot_low, snapshot_high);
}

QDF_STATUS mgmt_rx_reo_register_wifi3_0_ops(
	struct wlan_lmac_if_mgmt_rx_reo_low_level_ops *reo_low_level_ops)
{
	if (!reo_low_level_ops) {
		target_if_err("Low level ops of MGMT Rx REO is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	reo_low_level_ops->init_shmem_arena_ctx =
		mlo_glb_h_shmem_arena_ctx_init;
	reo_low_level_ops->deinit_shmem_arena_ctx =
		mlo_glb_h_shmem_arena_ctx_deinit;
	reo_low_level_ops->get_num_links = mgmt_rx_reo_get_num_links;
	reo_low_level_ops->get_snapshot_address =
		mgmt_rx_reo_get_snapshot_address;
	reo_low_level_ops->snapshot_is_valid =
		mgmt_rx_reo_snapshot_is_valid;
	reo_low_level_ops->snapshot_get_mgmt_pkt_ctr =
		mgmt_rx_reo_snapshot_get_mgmt_pkt_ctr;
	reo_low_level_ops->snapshot_get_redundant_mgmt_pkt_ctr =
		mgmt_rx_reo_snapshot_get_redundant_mgmt_pkt_ctr;
	reo_low_level_ops->snapshot_is_consistent =
		mgmt_rx_reo_snapshot_is_consistent;
	reo_low_level_ops->snapshot_get_global_timestamp =
		mgmt_rx_reo_snapshot_get_global_timestamp;

	reo_low_level_ops->implemented = true;

	return QDF_STATUS_SUCCESS;
}
