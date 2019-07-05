/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#define HAL_FST_HASH_KEY_SIZE_BITS 315
#define HAL_FST_HASH_KEY_SIZE_BYTES 40
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
 * struct hal_rx_flow - Rx Flow parameters to be sent to HW
 * @tuple_info: Rx Flow 5-tuple (src & dest IP, src & dest ports, L4 protocol)
 * @reo_destination_handler: REO destination for this flow
 * @reo_destination_indication: REO indication for this flow
 * @fse_metadata: Flow metadata or tag passed to HW for marking packets
 */
struct hal_rx_flow {
	struct hal_flow_tuple_info tuple_info;
	uint8_t reo_destination_handler;
	uint8_t reo_destination_indication;
	uint32_t fse_metadata;
};

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
 * struct hal_rx_fst - HAL RX Flow search table context
 * @base_vaddr: Virtual Base address of HW FST
 * @base_paddr: Physical Base address of HW FST
 * @key: Pointer to 320-bit Key read from cfg
 * @shifted_key: Pointer to left-shifted 320-bit Key used for Toeplitz Hash
 * @max_entries : Max number of entries in flow searchh  table
 * @max_skid_length : Max search length if there is hash collision
 * @hash_mask: Hash mask to apply to index into FST
 * @key_cache: Toepliz Key Cache configured key
 */
struct hal_rx_fst {
	uint8_t *base_vaddr;
	qdf_dma_addr_t base_paddr;
	uint8_t *key;
	uint8_t  shifted_key[HAL_FST_HASH_KEY_SIZE_BYTES];
	uint16_t max_entries;
	uint16_t max_skid_length;
	uint16_t hash_mask;
	uint32_t key_cache[HAL_FST_HASH_KEY_SIZE_BYTES][1 << 8];
};

/**
 * hal_rx_flow_setup_fse() - Setup a flow search entry in HW FST
 * @fst: Pointer to the Rx Flow Search Table
 * @table_offset: offset into the table where the flow is to be setup
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
static void *
hal_rx_flow_setup_fse(struct hal_rx_fst *fst, uint32_t table_offset,
		      struct hal_rx_flow *flow)
{
	uint8_t *fse;
	bool fse_valid;

	if (table_offset >= fst->max_entries) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "HAL FSE table offset %u exceeds max entries %u",
			  table_offset, fst->max_entries);
		return NULL;
	}

	fse = (uint8_t *)fst->base_vaddr +
			(table_offset * HAL_RX_FST_ENTRY_SIZE);

	fse_valid = HAL_GET_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, VALID);

	if (fse_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "HAL FSE %pK already valid", fse);
		return NULL;
	}

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_0, SRC_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_0, SRC_IP_127_96,
			       qdf_htonl(flow->tuple_info.src_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_1, SRC_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_1, SRC_IP_95_64,
			       qdf_htonl(flow->tuple_info.src_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_2, SRC_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_2, SRC_IP_63_32,
			       qdf_htonl(flow->tuple_info.src_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_3, SRC_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_3, SRC_IP_31_0,
			       qdf_htonl(flow->tuple_info.src_ip_31_0));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_4, DEST_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_4, DEST_IP_127_96,
			       qdf_htonl(flow->tuple_info.dest_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_5, DEST_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_5, DEST_IP_95_64,
			       qdf_htonl(flow->tuple_info.dest_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_6, DEST_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_6, DEST_IP_63_32,
			       qdf_htonl(flow->tuple_info.dest_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_7, DEST_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_7, DEST_IP_31_0,
			       qdf_htonl(flow->tuple_info.dest_ip_31_0));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_8, DEST_PORT);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_8, DEST_PORT) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_8, DEST_PORT,
			       (flow->tuple_info.dest_port));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_8, SRC_PORT);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_8, SRC_PORT) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_8, SRC_PORT,
			       (flow->tuple_info.src_port));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, L4_PROTOCOL);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, L4_PROTOCOL) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_9, L4_PROTOCOL,
			       flow->tuple_info.l4_protocol);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, REO_DESTINATION_HANDLER);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, REO_DESTINATION_HANDLER) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_9, REO_DESTINATION_HANDLER,
			       flow->reo_destination_handler);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, VALID);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, VALID) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_9, VALID, 1);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_10, METADATA);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_10, METADATA) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_10, METADATA,
			       flow->fse_metadata);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_11, REO_DESTINATION_INDICATION);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY_11, REO_DESTINATION_INDICATION) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY_11,
			       REO_DESTINATION_INDICATION,
			       flow->reo_destination_indication);

	/* Reset all the other fields in FSE */
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, RESERVED_9);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_11, MSDU_DROP);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_11, RESERVED_11);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_11, MSDU_COUNT);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_12, MSDU_BYTE_COUNT);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_13, TIMESTAMP);

	return fse;
}

/**
 * hal_rx_flow_delete_entry() - Delete a flow from the Rx Flow Search Table
 * @fst: Pointer to the Rx Flow Search Table
 * @hal_rx_fse: Pointer to the Rx Flow that is to be deleted from the FST
 *
 * Return: Success/Failure
 */
static inline QDF_STATUS
hal_rx_flow_delete_entry(struct hal_rx_fst *fst, void *hal_rx_fse)
{
	uint8_t *fse = (uint8_t *)hal_rx_fse;

	if (!HAL_GET_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, VALID))
		return QDF_STATUS_E_NOENT;

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY_9, VALID);

	return QDF_STATUS_SUCCESS;
}

/**
 * hal_rx_fst_key_configure() - Configure the Toeplitz key in the FST
 * @fst: Pointer to the Rx Flow Search Table
 *
 * Return: Success/Failure
 */
static void hal_rx_fst_key_configure(struct hal_rx_fst *fst)
{
	uint8_t key_bytes[HAL_FST_HASH_KEY_SIZE_BYTES];

	qdf_mem_copy(key_bytes, fst->key, HAL_FST_HASH_KEY_SIZE_BYTES);

	/**
	 * The Toeplitz algorithm as per the Microsoft spec works in a
	 * “big-endian” manner, using the MSBs of the key to hash the
	 * initial bytes of the input going on to use up the lower order bits
	 * of the key to hash further bytes of the input until the LSBs of the
	 * key are used finally.
	 *
	 * So first, rightshift 320-bit input key 5 times to get 315 MS bits
	 */
	key_bitwise_shift_left(key_bytes, HAL_FST_HASH_KEY_SIZE_BYTES, 5);
	key_reverse(fst->shifted_key, key_bytes, HAL_FST_HASH_KEY_SIZE_BYTES);
}

/**
 * hal_rx_fst_get_base() - Retrieve the virtual base address of the Rx FST
 * @fst: Pointer to the Rx Flow Search Table
 *
 * Return: Success/Failure
 */
static inline void *hal_rx_fst_get_base(struct hal_rx_fst *fst)
{
	return fst->base_vaddr;
}

/**
 * hal_rx_fst_get_fse_size() - Retrieve the size of each entry(flow) in Rx FST
 *
 * Return: size of each entry/flow in Rx FST
 */
static inline uint32_t hal_rx_fst_get_fse_size(void)
{
	return HAL_RX_FST_ENTRY_SIZE;
}

/**
 * hal_rx_flow_get_tuple_info() - Retrieve the 5-tuple flow info for an entry
 * @hal_fse: Pointer to the Flow in Rx FST
 * @tuple_info: 5-tuple info of the flow returned to the caller
 *
 * Return: Success/Failure
 */
QDF_STATUS hal_rx_flow_get_tuple_info(void *hal_fse,
				      struct hal_flow_tuple_info *tuple_info)
{
	if (!hal_fse || !tuple_info)
		return QDF_STATUS_E_INVAL;

	if (!HAL_GET_FLD(hal_fse, RX_FLOW_SEARCH_ENTRY_9, VALID))
		return QDF_STATUS_E_NOENT;

	tuple_info->src_ip_127_96 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_0, SRC_IP_127_96));
	tuple_info->src_ip_95_64 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_1, SRC_IP_95_64));
	tuple_info->src_ip_63_32 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_2, SRC_IP_63_32));
	tuple_info->src_ip_31_0 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_3, SRC_IP_31_0));
	tuple_info->dest_ip_127_96 =
		 qdf_ntohl(HAL_GET_FLD(hal_fse,
				       RX_FLOW_SEARCH_ENTRY_4, DEST_IP_127_96));
	tuple_info->dest_ip_95_64 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_5, DEST_IP_95_64));
	tuple_info->dest_ip_63_32 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_6, DEST_IP_63_32));
	tuple_info->dest_ip_31_0 = qdf_ntohl(HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_7, DEST_IP_31_0));
	tuple_info->dest_port = (HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_8, DEST_PORT));
	tuple_info->src_port = (HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_8, SRC_PORT));
	tuple_info->l4_protocol = HAL_GET_FLD(hal_fse,
					RX_FLOW_SEARCH_ENTRY_9, L4_PROTOCOL);

	return QDF_STATUS_SUCCESS;
}

/**
 * hal_flow_toeplitz_create_cache() - Calculate hashes for each possible
 * byte value with the key taken as is
 *
 * @fst: FST Handle
 * @key: Hash Key
 *
 * Return: Success/Failure
 */
void hal_flow_toeplitz_create_cache(struct hal_rx_fst *fst)
{
	int bit;
	int val;
	int i;
	uint8_t *key = fst->shifted_key;

	/*
	 * Initialise to first 32 bits of the key; shift in further key material
	 * through the loop
	 */
	uint32_t cur_key = (key[0] << 24) | (key[1] << 16) | (key[2] << 8) |
		key[3];

	for (i = 0; i < HAL_FST_HASH_KEY_SIZE_BYTES; i++) {
		uint8_t new_key_byte;
		uint32_t shifted_key[8];

		if (i + 4 < HAL_FST_HASH_KEY_SIZE_BYTES)
			new_key_byte = key[i + 4];
		else
			new_key_byte = 0;

		shifted_key[0] = cur_key;

		for (bit = 1; bit < 8; bit++) {
			/*
			 * For each iteration, shift out one more bit of the
			 * current key and shift in one more bit of the new key
			 * material
			 */
			shifted_key[bit] = cur_key << bit |
				new_key_byte >> (8 - bit);
		}

		for (val = 0; val < (1 << 8); val++) {
			uint32_t hash = 0;
			int mask;

			/*
			 * For each bit set in the input, XOR in
			 * the appropriately shifted key
			 */
			for (bit = 0, mask = 1 << 7; bit < 8; bit++, mask >>= 1)
				if ((val & mask))
					hash ^= shifted_key[bit];

			fst->key_cache[i][val] = hash;
		}

		cur_key = cur_key << 8 | new_key_byte;
	}
}

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
static struct hal_rx_fst *
hal_rx_fst_attach(qdf_device_t qdf_dev,
		  uint64_t *hal_fst_base_paddr, uint16_t max_entries,
		  uint16_t max_search, uint8_t *hash_key)
{
	struct hal_rx_fst *fst = qdf_mem_malloc(sizeof(struct hal_rx_fst));

	if (!fst) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("hal fst allocation failed,"));
		return NULL;
	}

	qdf_mem_set(fst, 0, sizeof(struct hal_rx_fst));

	fst->key = hash_key;
	fst->max_skid_length = max_search;
	fst->max_entries = max_entries;
	fst->hash_mask = max_entries - 1;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "HAL FST allocation %x %d * %d\n", fst,
		  fst->max_entries, HAL_RX_FST_ENTRY_SIZE);

	fst->base_vaddr = (uint8_t *)qdf_mem_alloc_consistent(qdf_dev,
				qdf_dev->dev,
				(fst->max_entries * HAL_RX_FST_ENTRY_SIZE),
				&fst->base_paddr);

	if (!fst->base_vaddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("hal fst->base_vaddr allocation failed"));
		qdf_mem_free(fst);
		return NULL;
	}
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_ANY, QDF_TRACE_LEVEL_DEBUG,
			   (void *)fst->key, HAL_FST_HASH_KEY_SIZE_BYTES);

	qdf_mem_set((uint8_t *)fst->base_vaddr, 0,
		    (fst->max_entries * HAL_RX_FST_ENTRY_SIZE));

	hal_rx_fst_key_configure(fst);
	hal_flow_toeplitz_create_cache(fst);
	*hal_fst_base_paddr = (uint64_t)fst->base_paddr;
	return fst;
}

/**
 * hal_rx_fst_detach() - De-init the Rx flow search table from HW
 *
 * @rx_fst: Pointer to the Rx FST
 * @qdf_dev: QDF device handle
 *
 * Return:
 */
void hal_rx_fst_detach(struct hal_rx_fst *rx_fst,
		       qdf_device_t qdf_dev)
{
	if (!rx_fst || !qdf_dev)
		return;

	qdf_mem_free_consistent(qdf_dev, qdf_dev->dev,
				rx_fst->max_entries * HAL_RX_FST_ENTRY_SIZE,
				rx_fst->base_vaddr, rx_fst->base_paddr, 0);

	qdf_mem_free(rx_fst);
}

/**
 * hal_flow_toeplitz_hash() - Calculate Toeplitz hash by using the cached key
 *
 * @hal_fst: FST Handle
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
static inline uint32_t
hal_flow_toeplitz_hash(void *hal_fst, struct hal_rx_flow *flow)
{
	int i, j;
	uint32_t hash = 0;
	struct hal_rx_fst *fst = (struct hal_rx_fst *)hal_fst;
	uint32_t input[HAL_FST_HASH_KEY_SIZE_WORDS];
	uint8_t *tuple;

	qdf_mem_zero(input, HAL_FST_HASH_KEY_SIZE_BYTES);
	*(uint32_t *)&input[0] = qdf_htonl(flow->tuple_info.src_ip_127_96);
	*(uint32_t *)&input[1] = qdf_htonl(flow->tuple_info.src_ip_95_64);
	*(uint32_t *)&input[2] = qdf_htonl(flow->tuple_info.src_ip_63_32);
	*(uint32_t *)&input[3] = qdf_htonl(flow->tuple_info.src_ip_31_0);
	*(uint32_t *)&input[4] = qdf_htonl(flow->tuple_info.dest_ip_127_96);
	*(uint32_t *)&input[5] = qdf_htonl(flow->tuple_info.dest_ip_95_64);
	*(uint32_t *)&input[6] = qdf_htonl(flow->tuple_info.dest_ip_63_32);
	*(uint32_t *)&input[7] = qdf_htonl(flow->tuple_info.dest_ip_31_0);
	*(uint32_t *)&input[8] = (flow->tuple_info.dest_port << 16) |
				 (flow->tuple_info.src_port);
	*(uint32_t *)&input[9] = flow->tuple_info.l4_protocol;

	tuple = (uint8_t *)input;
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			   tuple, sizeof(input));
	for (i = 0, j = HAL_FST_HASH_DATA_SIZE - 1;
	     i < HAL_FST_HASH_KEY_SIZE_BYTES && j >= 0; i++, j--) {
		hash ^= fst->key_cache[i][tuple[j]];
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_LOW,
		  "Hash value %u %u truncated hash %u\n", hash,
		  (hash >> 12), (hash >> 12) % (fst->max_entries));

	hash >>= 12;
	hash &= (fst->max_entries - 1);

	return hash;
}

/**
 * hal_rx_get_hal_hash() - Retrieve hash index of a flow in the FST table
 *
 * @hal_fst: HAL Rx FST Handle
 * @flow_hash: Flow hash computed from flow tuple
 *
 * Return: hash index truncated to the size of the hash table
 */
inline
uint32_t hal_rx_get_hal_hash(struct hal_rx_fst *hal_fst, uint32_t flow_hash)
{
	uint32_t trunc_hash = flow_hash;

	/* Take care of hash wrap around scenario */
	if (flow_hash >= hal_fst->max_entries)
		trunc_hash &= hal_fst->hash_mask;
	return trunc_hash;
}

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
			 void *flow_tuple_info, uint32_t *flow_idx) {
	int i;
	void *hal_fse;
	uint32_t hal_hash;
	struct hal_flow_tuple_info hal_tuple_info = { 0 };
	QDF_STATUS status;

	for (i = 0; i < fst->max_skid_length; i++) {
		hal_hash = hal_rx_get_hal_hash(fst, (flow_hash + i));
		hal_fse = (uint8_t *)fst->base_vaddr +
			(hal_hash * HAL_RX_FST_ENTRY_SIZE);
		status = hal_rx_flow_get_tuple_info(hal_fse, &hal_tuple_info);
		if (QDF_STATUS_E_NOENT == status)
			break;

		/* Find the matching flow entry in HW FST */
		if (!qdf_mem_cmp(&hal_tuple_info,
				 flow_tuple_info,
				 sizeof(struct hal_flow_tuple_info))) {
			dp_err("Duplicate flow entry in FST %u at skid %u ",
			       hal_hash, i);
			return QDF_STATUS_E_EXISTS;
		}
	}
	if (i == fst->max_skid_length) {
		dp_err("Max skid length reached for hash %u", flow_hash);
		return QDF_STATUS_E_RANGE;
	}
	*flow_idx = hal_hash;
	dp_info("flow_hash = %u, skid_entry = %d, flow_addr = %pK flow_idx = %d",
		flow_hash, i, hal_fse, *flow_idx);

	return QDF_STATUS_SUCCESS;
}

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
			    void *flow_tuple_info, uint32_t *flow_idx)
{
	int i;
	void *hal_fse;
	uint32_t hal_hash;
	struct hal_flow_tuple_info hal_tuple_info = { 0 };
	QDF_STATUS status;

	for (i = 0; i < fst->max_skid_length; i++) {
		hal_hash = hal_rx_get_hal_hash(fst, (flow_hash + i));
		hal_fse = (uint8_t *)fst->base_vaddr +
			(hal_hash * HAL_RX_FST_ENTRY_SIZE);
		status = hal_rx_flow_get_tuple_info(hal_fse, &hal_tuple_info);
		if (QDF_STATUS_SUCCESS != status)
			continue;

		/* Find the matching flow entry in HW FST */
		if (!qdf_mem_cmp(&hal_tuple_info,
				 flow_tuple_info,
				 sizeof(struct hal_flow_tuple_info))) {
			break;
		}
	}

	if (i == fst->max_skid_length) {
		dp_err("Max skid length reached for hash %u", flow_hash);
		return QDF_STATUS_E_RANGE;
	}

	*flow_idx = hal_hash;
	dp_info("flow_hash = %u, skid_entry = %d, flow_addr = %pK flow_idx = %d",
		flow_hash, i, hal_fse, *flow_idx);

	return QDF_STATUS_SUCCESS;
}

#endif /* HAL_RX_FLOW_H */
