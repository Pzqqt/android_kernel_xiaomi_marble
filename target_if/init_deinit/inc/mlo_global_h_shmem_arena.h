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
 *  DOC: mlo_global_h_shmem_arena.h
 *  This file contains APIs and data structures that are used to parse the MLO
 *  global shared memory arena.
 */

#ifndef _MLO_GLOBAL_H_SHMEM_ARENA_H_
#define _MLO_GLOBAL_H_SHMEM_ARENA_H_

#include <qdf_types.h>
#include <target_if.h>
#include "wmi.h"
#include <osdep.h>

/**
 * wlan_host_mlo_glb_h_shmem_params - MLO global shared memory parameters
 * @major_version: Major version
 * @minor_version: Minor version
 */
struct wlan_host_mlo_glb_h_shmem_params {
	uint16_t major_version;
	uint16_t minor_version;
};

/**
 * wlan_host_mlo_glb_rx_reo_per_link_info - MGMT Rx REO information of a link in
 * MLO global shared memory
 * @link_id: Hardware link ID
 * @fw_consumed: Address of FW consumed snapshot
 * @fw_forwarded: Address of FW forwarded snapshot
 * @hw_forwarded: Address of HW forwarded snapshot
 */
struct wlan_host_mlo_glb_rx_reo_per_link_info {
	uint8_t link_id;
	void *fw_consumed;
	void *fw_forwarded;
	void *hw_forwarded;
};

/**
 * wlan_host_mlo_glb_rx_reo_snapshot_info - MGMT Rx REO information in MLO
 * global shared memory
 * @num_links: Number of valid links
 * @valid_link_bmap: Valid link bitmap
 * @link_info: pointer to an array of Rx REO per-link information
 */
struct wlan_host_mlo_glb_rx_reo_snapshot_info {
	uint8_t num_links;
	uint16_t valid_link_bmap;
	struct wlan_host_mlo_glb_rx_reo_per_link_info *link_info;
};

/**
 * wlan_host_mlo_glb_h_shmem_arena_ctx - MLO Global shared memory arena context
 * @shmem_params: shared memory parameters
 * @rx_reo_snapshot_info: MGMT Rx REO snapshot information
 * @init_count: Number of init calls
 */
struct wlan_host_mlo_glb_h_shmem_arena_ctx {
	struct wlan_host_mlo_glb_h_shmem_params shmem_params;
	struct wlan_host_mlo_glb_rx_reo_snapshot_info rx_reo_snapshot_info;
	qdf_atomic_t init_count;
};

/**
 * mlo_glb_h_shmem_arena_ctx_init() - Initialize MLO Global shared memory arena
 * context on Host
 * @arena_vaddr: Virtual address of the MLO Global shared memory arena
 * @arena_len: Length (in bytes) of the MLO Global shared memory arena
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS mlo_glb_h_shmem_arena_ctx_init(void *arena_vaddr,
					  size_t arena_len);

/**
 * mlo_glb_h_shmem_arena_ctx_deinit() - De-initialize MLO Global shared memory
 * arena context on Host
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS mlo_glb_h_shmem_arena_ctx_deinit(void);

/**
 * mgmt_rx_reo_get_num_links() - Get number of links to be used by MGMT Rx REO
 *
 * Return: number of links in case of success, else -1
 */
int mgmt_rx_reo_get_num_links(void);

/**
 * mgmt_rx_reo_get_snapshot_address() - Get the address of MGMT Rx REO snapshot
 * @link_id: Link ID of the radio to which this snapshot belongs
 * @snapshot_id: ID of the snapshot
 *
 * Return: virtual address of the snapshot on success, else NULL
 */
void *mgmt_rx_reo_get_snapshot_address(
	uint8_t link_id, enum mgmt_rx_reo_shared_snapshot_id snapshot_id);
#endif
