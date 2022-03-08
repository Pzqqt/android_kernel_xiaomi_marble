/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "qdf_module.h"
#include "dp_types.h"
#include "hal_rx_flow.h"

/**
 * hal_rx_flow_get_cmem_fse() - Get FSE from CMEM
 * @hal_soc_hdl: HAL SOC handle
 * @fse_offset: CMEM FSE offset
 * @fse: referece where FSE will be copied
 * @len: length of FSE
 *
 * Return: If read is succesfull or not
 */
static void
hal_rx_flow_get_cmem_fse(hal_soc_handle_t hal_soc_hdl, uint32_t fse_offset,
			 uint32_t *fse, qdf_size_t len)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_flow_get_cmem_fse) {
		return hal_soc->ops->hal_rx_flow_get_cmem_fse(
						hal_soc, fse_offset, fse, len);
	}
}

#if defined(WLAN_SUPPORT_RX_FISA)
static inline void hal_rx_dump_fse(struct rx_flow_search_entry *fse, int index)
{
		dp_info("index %d:"
		" src_ip_127_96 0x%x"
		" src_ip_95_640 0x%x"
		" src_ip_63_32 0x%x"
		" src_ip_31_0 0x%x"
		" dest_ip_127_96 0x%x"
		" dest_ip_95_64 0x%x"
		" dest_ip_63_32 0x%x"
		" dest_ip_31_0 0x%x"
		" src_port 0x%x"
		" dest_port 0x%x"
		" l4_protocol 0x%x"
		" valid 0x%x"
		" reo_destination_indication 0x%x"
		" msdu_drop 0x%x"
		" reo_destination_handler 0x%x"
		" metadata 0x%x"
		" aggregation_count0x%x"
		" lro_eligible 0x%x"
		" msdu_count 0x%x"
		" msdu_byte_count 0x%x"
		" timestamp 0x%x"
		" cumulative_l4_checksum 0x%x"
		" cumulative_ip_length 0x%x"
		" tcp_sequence_number 0x%x",
		index,
		fse->src_ip_127_96,
		fse->src_ip_95_64,
		fse->src_ip_63_32,
		fse->src_ip_31_0,
		fse->dest_ip_127_96,
		fse->dest_ip_95_64,
		fse->dest_ip_63_32,
		fse->dest_ip_31_0,
		fse->src_port,
		fse->dest_port,
		fse->l4_protocol,
		fse->valid,
		fse->reo_destination_indication,
		fse->msdu_drop,
		fse->reo_destination_handler,
		fse->metadata,
		fse->aggregation_count,
		fse->lro_eligible,
		fse->msdu_count,
		fse->msdu_byte_count,
		fse->timestamp,
#ifdef QCA_WIFI_KIWI_V2
		fse->cumulative_ip_length_pmac1,
#else
		fse->cumulative_l4_checksum,
#endif
		fse->cumulative_ip_length,
		fse->tcp_sequence_number);
}

void hal_rx_dump_fse_table(struct hal_rx_fst *fst)
{
	int i = 0;
	struct rx_flow_search_entry *fse =
		(struct rx_flow_search_entry *)fst->base_vaddr;

	dp_info("Number flow table entries %d", fst->add_flow_count);
	for (i = 0; i < fst->max_entries; i++) {
		if (fse[i].valid)
			hal_rx_dump_fse(&fse[i], i);
	}
}

void hal_rx_dump_cmem_fse(hal_soc_handle_t hal_soc_hdl, uint32_t fse_offset,
			  int index)
{
	struct rx_flow_search_entry fse = {0};

	if (!fse_offset)
		return;

	hal_rx_flow_get_cmem_fse(hal_soc_hdl, fse_offset, (uint32_t *)&fse,
				 sizeof(struct rx_flow_search_entry));
	if (fse.valid)
		hal_rx_dump_fse(&fse, index);
}
#else
void hal_rx_dump_fse_table(struct hal_rx_fst *fst)
{
}

void hal_rx_dump_cmem_fse(hal_soc_handle_t hal_soc_hdl, uint32_t fse_offset,
			  int index)
{
}
#endif

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
		      struct hal_rx_flow *flow)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_flow_setup_fse) {
		return hal_soc->ops->hal_rx_flow_setup_fse((uint8_t *)fst,
							   table_offset,
							   (uint8_t *)flow);
	}

	return NULL;
}
qdf_export_symbol(hal_rx_flow_setup_fse);

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
			   uint32_t table_offset, struct hal_rx_flow *flow)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_flow_setup_cmem_fse) {
		return hal_soc->ops->hal_rx_flow_setup_cmem_fse(
						hal_soc, cmem_ba,
						table_offset, (uint8_t *)flow);
	}

	return 0;
}
qdf_export_symbol(hal_rx_flow_setup_cmem_fse);

/**
 * hal_rx_flow_get_cmem_fse_timestamp() - Get timestamp field from CMEM FSE
 * @hal_soc_hdl: HAL SOC handle
 * @fse_offset: CMEM FSE offset
 *
 * Return: Timestamp
 */
uint32_t hal_rx_flow_get_cmem_fse_timestamp(hal_soc_handle_t hal_soc_hdl,
					    uint32_t fse_offset)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (hal_soc->ops->hal_rx_flow_get_cmem_fse_ts) {
		return hal_soc->ops->hal_rx_flow_get_cmem_fse_ts(hal_soc,
								 fse_offset);
	}

	return 0;
}
qdf_export_symbol(hal_rx_flow_get_cmem_fse_timestamp);

/**
 * hal_rx_flow_delete_entry() - Delete a flow from the Rx Flow Search Table
 * @fst: Pointer to the Rx Flow Search Table
 * @hal_rx_fse: Pointer to the Rx Flow that is to be deleted from the FST
 *
 * Return: Success/Failure
 */
QDF_STATUS
hal_rx_flow_delete_entry(struct hal_rx_fst *fst, void *hal_rx_fse)
{
	uint8_t *fse = (uint8_t *)hal_rx_fse;

	if (!HAL_GET_FLD(fse, HAL_RX_FLOW_SEARCH_ENTRY, VALID))
		return QDF_STATUS_E_NOENT;

	HAL_CLR_FLD(fse, HAL_RX_FLOW_SEARCH_ENTRY, VALID);

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(hal_rx_flow_delete_entry);

#ifndef WLAN_SUPPORT_RX_FISA
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
#else
static void hal_rx_fst_key_configure(struct hal_rx_fst *fst)
{
}
#endif

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

	if (!HAL_GET_FLD(hal_fse, HAL_RX_FLOW_SEARCH_ENTRY, VALID))
		return QDF_STATUS_E_NOENT;

	tuple_info->src_ip_127_96 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      SRC_IP_127_96));
	tuple_info->src_ip_95_64 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      SRC_IP_95_64));
	tuple_info->src_ip_63_32 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      SRC_IP_63_32));
	tuple_info->src_ip_31_0 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      SRC_IP_31_0));
	tuple_info->dest_ip_127_96 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      DEST_IP_127_96));
	tuple_info->dest_ip_95_64 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      DEST_IP_95_64));
	tuple_info->dest_ip_63_32 =
				qdf_ntohl(HAL_GET_FLD(hal_fse,
						      HAL_RX_FLOW_SEARCH_ENTRY,
						      DEST_IP_63_32));
	tuple_info->dest_ip_31_0 =
			qdf_ntohl(HAL_GET_FLD(hal_fse,
					      HAL_RX_FLOW_SEARCH_ENTRY,
					      DEST_IP_31_0));
	tuple_info->dest_port = HAL_GET_FLD(hal_fse,
					    HAL_RX_FLOW_SEARCH_ENTRY,
					    DEST_PORT);
	tuple_info->src_port = HAL_GET_FLD(hal_fse,
					   HAL_RX_FLOW_SEARCH_ENTRY,
					   SRC_PORT);
	tuple_info->l4_protocol = HAL_GET_FLD(hal_fse,
					      HAL_RX_FLOW_SEARCH_ENTRY,
					      L4_PROTOCOL);

	return QDF_STATUS_SUCCESS;
}

#ifndef WLAN_SUPPORT_RX_FISA
/**
 * hal_flow_toeplitz_create_cache() - Calculate hashes for each possible
 * byte value with the key taken as is
 *
 * @fst: FST Handle
 * @key: Hash Key
 *
 * Return: Success/Failure
 */
static void hal_flow_toeplitz_create_cache(struct hal_rx_fst *fst)
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
#else
static void hal_flow_toeplitz_create_cache(struct hal_rx_fst *fst)
{
}
#endif

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
		  uint16_t max_search, uint8_t *hash_key)
{
	struct hal_rx_fst *fst = qdf_mem_malloc(sizeof(struct hal_rx_fst));

	if (!fst) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("hal fst allocation failed,"));
		return NULL;
	}

	qdf_mem_set(fst, sizeof(struct hal_rx_fst), 0);

	fst->key = hash_key;
	fst->max_skid_length = max_search;
	fst->max_entries = max_entries;
	fst->hash_mask = max_entries - 1;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "HAL FST allocation %pK %d * %d\n", fst,
		  fst->max_entries, HAL_RX_FST_ENTRY_SIZE);

	fst->base_vaddr = (uint8_t *)qdf_mem_alloc_consistent(qdf_dev,
				qdf_dev->dev,
				(fst->max_entries * HAL_RX_FST_ENTRY_SIZE),
				&fst->base_paddr);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
		  "hal_rx_fst base address 0x%pK", (void *)fst->base_paddr);
	if (!fst->base_vaddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  FL("hal fst->base_vaddr allocation failed"));
		qdf_mem_free(fst);
		return NULL;
	}
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_ANY, QDF_TRACE_LEVEL_DEBUG,
			   (void *)fst->key, HAL_FST_HASH_KEY_SIZE_BYTES);

	qdf_mem_set((uint8_t *)fst->base_vaddr,
		    (fst->max_entries * HAL_RX_FST_ENTRY_SIZE), 0);

	hal_rx_fst_key_configure(fst);
	hal_flow_toeplitz_create_cache(fst);
	*hal_fst_base_paddr = (uint64_t)fst->base_paddr;
	return fst;
}
qdf_export_symbol(hal_rx_fst_attach);

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
qdf_export_symbol(hal_rx_fst_detach);

#ifndef WLAN_SUPPORT_RX_FISA
/**
 * hal_flow_toeplitz_hash() - Calculate Toeplitz hash by using the cached key
 *
 * @hal_fst: FST Handle
 * @flow: Flow Parameters
 *
 * Return: Success/Failure
 */
uint32_t
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
#else
uint32_t
hal_flow_toeplitz_hash(void *hal_fst, struct hal_rx_flow *flow)
{
	return 0;
}
#endif
qdf_export_symbol(hal_flow_toeplitz_hash);

/**
 * hal_rx_get_hal_hash() - Retrieve hash index of a flow in the FST table
 *
 * @hal_fst: HAL Rx FST Handle
 * @flow_hash: Flow hash computed from flow tuple
 *
 * Return: hash index truncated to the size of the hash table
 */
uint32_t hal_rx_get_hal_hash(struct hal_rx_fst *hal_fst, uint32_t flow_hash)
{
	uint32_t trunc_hash = flow_hash;

	/* Take care of hash wrap around scenario */
	if (flow_hash >= hal_fst->max_entries)
		trunc_hash &= hal_fst->hash_mask;
	return trunc_hash;
}
qdf_export_symbol(hal_rx_get_hal_hash);

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
			 void *flow_tuple_info, uint32_t *flow_idx)
{
	int i;
	void *hal_fse = NULL;
	uint32_t hal_hash = 0;
	struct hal_flow_tuple_info hal_tuple_info = { 0 };
	QDF_STATUS status;

	for (i = 0; i < fst->max_skid_length; i++) {
		hal_hash = hal_rx_get_hal_hash(fst, (flow_hash + i));
		hal_fse = (uint8_t *)fst->base_vaddr +
			(hal_hash * HAL_RX_FST_ENTRY_SIZE);
		status = hal_rx_flow_get_tuple_info(hal_fse, &hal_tuple_info);
		if (status == QDF_STATUS_E_NOENT)
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
qdf_export_symbol(hal_rx_insert_flow_entry);

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
	void *hal_fse = NULL;
	uint32_t hal_hash = 0;
	struct hal_flow_tuple_info hal_tuple_info = { 0 };
	QDF_STATUS status;

	for (i = 0; i < fst->max_skid_length; i++) {
		hal_hash = hal_rx_get_hal_hash(fst, (flow_hash + i));
		hal_fse = (uint8_t *)fst->base_vaddr +
			(hal_hash * HAL_RX_FST_ENTRY_SIZE);
		status = hal_rx_flow_get_tuple_info(hal_fse, &hal_tuple_info);
		if (status != QDF_STATUS_SUCCESS)
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
qdf_export_symbol(hal_rx_find_flow_from_tuple);
