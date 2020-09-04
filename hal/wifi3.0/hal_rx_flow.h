/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef __HAL_RX_FLOW_H
#define __HAL_RX_FLOW_H

#include "hal_flow.h"
#include "wlan_cfg.h"
#include "hal_api.h"
#include "qdf_mem.h"
#include "rx_flow_search_entry.h"
#include "hal_rx_flow_info.h"

#define HAL_FST_HASH_KEY_SIZE_BITS 315
#define HAL_FST_HASH_KEY_SIZE_WORDS 10
#define HAL_FST_HASH_DATA_SIZE 37
#define HAL_FST_HASH_MASK 0x7ffff
#define HAL_RX_FST_ENTRY_SIZE (NUM_OF_DWORDS_RX_FLOW_SEARCH_ENTRY * 4)

/**
 * Four possible options for IP SA/DA prefix, currently use 0x0 which
 * maps to type 2 in HW spec
 */
#define HAL_FST_IP_DA_SA_PFX_TYPE_IPV4_COMPATIBLE_IPV6 2

#define HAL_IP_DA_SA_PREFIX_IPV4_COMPATIBLE_IPV6 0x0

/**
 * REO destination indication is a lower 4-bits of hash value
 * This should match the REO destination used in Rx hash based routing.
 */
#define HAL_REO_DEST_IND_HASH_MASK	0xF

/**
 * REO destinations are valid from 16-31 for Hawkeye
 * and 0-15 are not setup for SW
 */
#define HAL_REO_DEST_IND_START_OFFSET 0x10

/**
 * enum hal_rx_fse_reo_destination_handler
 * @HAL_RX_FSE_REO_DEST_FT: Use this entry's destination indication
 * @HAL_RX_FSE_REO_DEST_ASPT: Use Address Search + Peer Table's entry
 * @HAL_RX_FSE_REO_DEST_FT2: Use FT2's destination indication
 * @HAL_RX_FSE_REO_DEST_CCE: Use CCE's destination indication for this entry
 */
enum hal_rx_fse_reo_destination_handler {
	HAL_RX_FSE_REO_DEST_FT = 0,
	HAL_RX_FSE_REO_DEST_ASPT = 1,
	HAL_RX_FSE_REO_DEST_FT2 = 2,
	HAL_RX_FSE_REO_DEST_CCE = 3,
};

/**
 * hal_rx_flow_setup_fse() - Setup a flow search entry in HW FST
 * @fst: Pointer to the Rx Flow Search Table
 * @table_offset: offset into the table where the flow is to be setup
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
void *
hal_rx_flow_setup_fse(hal_soc_handle_t hal_soc_hdl,
		      struct hal_rx_fst *fst, uint32_t table_offset,
		      struct hal_rx_flow *flow);

/**
 * hal_rx_flow_setup_cmem_fse() - Setup a flow search entry in HW CMEM FST
 * @hal_soc_hdl: HAL SOC handle
 * @cmem_ba: CMEM base address
 * @table_offset: offset into the table where the flow is to be setup
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
uint32_t
hal_rx_flow_setup_cmem_fse(hal_soc_handle_t hal_soc_hdl, uint32_t cmem_ba,
			   uint32_t table_offset, struct hal_rx_flow *flow);

/**
 * hal_rx_flow_get_cmem_fse_timestamp() - Get timestamp field from CMEM FSE
 * @hal_soc_hdl: HAL SOC handle
 * @fse_offset: CMEM FSE offset
 *
 * Return: Timestamp
 */
uint32_t hal_rx_flow_get_cmem_fse_timestamp(hal_soc_handle_t hal_soc_hdl,
					    uint32_t fse_offset);

/**
 * hal_rx_flow_delete_entry() - Delete a flow from the Rx Flow Search Table
 * @fst: Pointer to the Rx Flow Search Table
 * @hal_rx_fse: Pointer to the Rx Flow that is to be deleted from the FST
 *
 * Return: Success/Failure
 */
QDF_STATUS
hal_rx_flow_delete_entry(struct hal_rx_fst *fst, void *hal_rx_fse);

/**
 * hal_rx_flow_get_tuple_info() - Retrieve the 5-tuple flow info for an entry
 * @hal_fse: Pointer to the Flow in Rx FST
 * @tuple_info: 5-tuple info of the flow returned to the caller
 *
 * Return: Success/Failure
 */
QDF_STATUS hal_rx_flow_get_tuple_info(void *hal_fse,
				      struct hal_flow_tuple_info *tuple_info);

/**
 * hal_rx_fst_attach() - Initialize Rx flow search table in HW FST
 *
 * @qdf_dev: QDF device handle
 * @hal_fst_base_paddr: Pointer to the physical base address of the Rx FST
 * @max_entries: Max number of flows allowed in the FST
 * @max_search: Number of collisions allowed in the hash-based FST
 * @hash_key: Toeplitz key used for the hash FST
 *
 * Return:
 */
struct hal_rx_fst *
hal_rx_fst_attach(qdf_device_t qdf_dev,
		  uint64_t *hal_fst_base_paddr, uint16_t max_entries,
		  uint16_t max_search, uint8_t *hash_key);

/**
 * hal_rx_fst_detach() - De-init the Rx flow search table from HW
 *
 * @rx_fst: Pointer to the Rx FST
 * @qdf_dev: QDF device handle
 *
 * Return:
 */
void hal_rx_fst_detach(struct hal_rx_fst *rx_fst, qdf_device_t qdf_dev);

/**
 * hal_rx_insert_flow_entry() - Add a flow into the FST table
 *
 * @hal_fst: HAL Rx FST Handle
 * @flow_hash: Flow hash computed from flow tuple
 * @flow_tuple_info: Flow tuple used to compute the hash
 * @flow_index: Hash index of the flow in the table when inserted successfully
 *
 * Return: Success if flow is inserted into the table, error otherwise
 */
QDF_STATUS
hal_rx_insert_flow_entry(struct hal_rx_fst *fst, uint32_t flow_hash,
			 void *flow_tuple_info, uint32_t *flow_idx);

/**
 * hal_rx_find_flow_from_tuple() - Find a flow in the FST table
 *
 * @fst: HAL Rx FST Handle
 * @flow_hash: Flow hash computed from flow tuple
 * @flow_tuple_info: Flow tuple used to compute the hash
 * @flow_index: Hash index of the flow in the table when found
 *
 * Return: Success if matching flow is found in the table, error otherwise
 */
QDF_STATUS
hal_rx_find_flow_from_tuple(struct hal_rx_fst *fst, uint32_t flow_hash,
			    void *flow_tuple_info, uint32_t *flow_idx);

/**
 * hal_rx_get_hal_hash() - Retrieve hash index of a flow in the FST table
 *
 * @hal_fst: HAL Rx FST Handle
 * @flow_hash: Flow hash computed from flow tuple
 *
 * Return: hash index truncated to the size of the hash table
 */
uint32_t hal_rx_get_hal_hash(struct hal_rx_fst *hal_fst, uint32_t flow_hash);

/**
 * hal_flow_toeplitz_hash() - Calculate Toeplitz hash by using the cached key
 *
 * @hal_fst: FST Handle
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
uint32_t
hal_flow_toeplitz_hash(void *hal_fst, struct hal_rx_flow *flow);

void hal_rx_dump_fse_table(struct hal_rx_fst *fst);

/**
 * hal_rx_dump_cmem_fse() - Dump flow search table entry which is in CMEM
 * @hal_soc_hdl: HAL SOC handle
 * @fse_offset: Offset in to the CMEM where FSE is located
 * @index: FSE index
 *
 * Return: None
 */
void hal_rx_dump_cmem_fse(hal_soc_handle_t hal_soc_hdl, uint32_t fse_offset,
			  int index);
#endif /* HAL_RX_FLOW_H */
