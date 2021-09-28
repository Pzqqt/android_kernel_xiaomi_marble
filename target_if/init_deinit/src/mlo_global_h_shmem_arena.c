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
 *  DOC: mlo_global_h_shmem_arena.c
 *  This file contains definition of functions that parse the MLO global
 *  shared memory arena.
 */

#include<mlo_global_h_shmem_arena.h>

static struct wlan_host_mlo_glb_h_shmem_arena_ctx g_shmem_arena_ctx;

#define get_shmem_arena_ctx() (&g_shmem_arena_ctx)

/**
 * get_field_value_in_tlv() - Get the value of a given field in a given TLV
 * @ptlv: Pointer to start of the TLV
 * @field_name: name of the field in the TLV structure
 * @tlv_len: Length of the TLV
 *
 * Return: Value of the given field if the offset of the field with in the TLV
 * structure is less than the TLV length, else 0.
 */
#define get_field_value_in_tlv(ptlv, field_name, tlv_len) \
	(qdf_offsetof(typeof(*(ptlv)), field_name) < (tlv_len) ? \
		(ptlv)->field_name : 0)

/**
 * get_field_pointer_in_tlv() - Get the address of a given field in a given TLV
 * @ptlv: Pointer to start of the TLV
 * @field_name: name of the field in the TLV structure
 * @tlv_len: Length of the TLV
 *
 * Return: Address of the given field if the offset of the field with in the
 * TLV structure is less than the TLV length, else NULL.
 */
#define get_field_pointer_in_tlv(ptlv, field_name, tlv_len) \
	(qdf_offsetof(typeof(*(ptlv)), field_name) < (tlv_len) ? \
		&(ptlv)->field_name : NULL)

/**
 * process_tlv_header() - Process a given TLV header
 * @data: Pointer to start of the TLV
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @expected_tag: Expected TLV tag
 * @tlv_len: Address of TLV length variable to be populated. This API populates
 * the entire length(payload + header) of the TLV into @tlv_len
 * @tlv_tag: Address of TLV Tag variable to be populated.
 *
 * Return: 0 on success, -1 on failure
 */
static int
process_tlv_header(const uint8_t *data, size_t remaining_len,
		   uint32_t expected_tag, uint32_t *tlv_len,
		   uint32_t *tlv_tag)
{
	if (remaining_len < MLO_SHMEM_TLV_HDR_SIZE) {
		target_if_err("Not enough space(%zu) to read TLV header(%u)",
			      remaining_len, (uint32_t)MLO_SHMEM_TLV_HDR_SIZE);
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	*tlv_len = MLO_SHMEMTLV_GET_TLVLEN(MLO_SHMEMTLV_GET_HDR(data));
	*tlv_len += MLO_SHMEM_TLV_HDR_SIZE;
	if (remaining_len < *tlv_len) {
		target_if_err("Not enough space(%zu) to read TLV payload(%u)",
			      remaining_len, *tlv_len);
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	*tlv_tag = MLO_SHMEMTLV_GET_TLVTAG(MLO_SHMEMTLV_GET_HDR(data));
	if (*tlv_tag != expected_tag) {
		target_if_err("Unexpected TLV tag: %u is seen. Expected: %u",
			      *tlv_tag,
			      expected_tag);
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	return 0;
}

#define validate_parsed_bytes_advance_data_pointer(parsed_bytes, data, \
						   remaining_len) \
do { \
	if ((parsed_bytes) < 0) { \
		target_if_err("TLV extraction failed"); \
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE); \
	} \
	(data) += (parsed_bytes); \
	(remaining_len) -= (parsed_bytes); \
} while (0)

/**
 * extract_mgmt_rx_reo_snapshot_tlv() - extract MGMT_RX_REO_SNAPSHOT TLV
 * @data: Pointer to start of the TLV
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @address_ptr: Pointer to the snapshot address. This API will populate the
 * snapshot address into the variable pointed by @address_ptr
 *
 * Return: On success, the number of bytes parsed. On failure, -1.
 */
static int
extract_mgmt_rx_reo_snapshot_tlv(uint8_t *data, size_t remaining_len,
				 void **address_ptr)
{
	mgmt_rx_reo_snapshot *ptlv;
	uint32_t tlv_len, tlv_tag;

	qdf_assert_always(data);
	qdf_assert_always(address_ptr);

	/* process MLO_SHMEM_TLV_STRUCT_MGMT_RX_REO_SNAPSHOT TLV */
	if (process_tlv_header(data, remaining_len,
			       MLO_SHMEM_TLV_STRUCT_MGMT_RX_REO_SNAPSHOT,
			       &tlv_len, &tlv_tag) != 0) {
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ptlv = (mgmt_rx_reo_snapshot *)data;
	*address_ptr = get_field_pointer_in_tlv(ptlv, mgmt_rx_reo_snapshot_low,
						tlv_len);

	return tlv_len;
}

/**
 * extract_mlo_glb_rx_reo_per_link_info_tlv() - extract
 * RX_REO_PER_LINK_SNAPSHOT_INFO TLV
 * @data: Pointer to start of the TLV
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @link_info: Pointer to MGMT Rx REO per link info. Extracted information
 * will be populated in this data structure.
 *
 * Return: On success, the number of bytes parsed. On failure, -1.
 */
static int
extract_mlo_glb_rx_reo_per_link_info_tlv(
	uint8_t *data, size_t remaining_len, uint8_t link_id,
	struct wlan_host_mlo_glb_rx_reo_per_link_info *link_info)
{
	mlo_glb_rx_reo_per_link_snapshot_info *ptlv;
	uint32_t tlv_len, tlv_tag;
	int len;
	uint8_t *fw_consumed;

	qdf_assert_always(data);
	qdf_assert_always(link_info);

	/* process MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_PER_LINK_SNAPSHOT_INFO TLV */
	if (process_tlv_header(data, remaining_len,
			       MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_PER_LINK_SNAPSHOT_INFO,
			       &tlv_len, &tlv_tag) != 0) {
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ptlv = (mlo_glb_rx_reo_per_link_snapshot_info *)data;

	link_info->link_id = link_id;

	/**
	 * Get the pointer to the fw_consumed snapshot with in the TLV.
	 * Note that snapshots are nested TLVs within link_sanpshot_info TLV.
	 */
	fw_consumed = (uint8_t *)get_field_pointer_in_tlv(ptlv, fw_consumed,
							  tlv_len);
	remaining_len -= qdf_offsetof(mlo_glb_rx_reo_per_link_snapshot_info,
				      fw_consumed);

	/* extract fw_consumed snapshot */
	len = extract_mgmt_rx_reo_snapshot_tlv(fw_consumed, remaining_len,
					       &link_info->fw_consumed);
	validate_parsed_bytes_advance_data_pointer(len, data, remaining_len);

	/* extract fw_forwarded snapshot */
	len = extract_mgmt_rx_reo_snapshot_tlv(fw_consumed, remaining_len,
					       &link_info->fw_forwarded);
	validate_parsed_bytes_advance_data_pointer(len, data, remaining_len);

	/* extract hw_forwarded snapshot */
	len = extract_mgmt_rx_reo_snapshot_tlv(fw_consumed, remaining_len,
					       &link_info->hw_forwarded);
	validate_parsed_bytes_advance_data_pointer(len, data, remaining_len);

	/**
	 * Return the length of link_sanpshot_info TLV itself as the snapshots
	 * are nested inside link_sanpshot_info TLV and hence no need to add
	 * their lengths separately.
	 */
	return tlv_len;
}

/**
 * get_num_links_from_valid_link_bitmap() - Get the number of valid links
 * @valid_link_bmap: Link bit map where the valid links are set to 1
 *
 * Return: Number of valid links
 */
static uint8_t
get_num_links_from_valid_link_bitmap(uint16_t valid_link_bmap)
{
	uint8_t num_links = 0;

	/* Find the number of set bits */
	while (valid_link_bmap) {
		num_links++;
		valid_link_bmap &= (valid_link_bmap - 1);
	}

	return num_links;
}

/**
 * extract_mlo_glb_rx_reo_snapshot_info_tlv() - extract
 * MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_SNAPSHOT_INFO TLV
 * @data: Pointer to start of the TLV
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @snapshot_info: Pointer to MGMT Rx REO snapshot info. Extracted information
 * will be populated in this data structure.
 *
 * Return: On success, the number of bytes parsed. On failure, -1.
 */
static int
extract_mlo_glb_rx_reo_snapshot_info_tlv(
	uint8_t *data, size_t remaining_len,
	struct wlan_host_mlo_glb_rx_reo_snapshot_info *snapshot_info)
{
	mlo_glb_rx_reo_snapshot_info *ptlv;
	uint32_t tlv_len, tlv_tag;
	uint32_t link_info;
	uint16_t valid_link_bmap;

	qdf_assert_always(data);
	qdf_assert_always(snapshot_info);

	/* process MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_SNAPSHOT_INFO TLV */
	if (process_tlv_header(data, remaining_len,
			       MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_SNAPSHOT_INFO,
			       &tlv_len, &tlv_tag) != 0) {
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ptlv = (mlo_glb_rx_reo_snapshot_info *)data;
	link_info = get_field_value_in_tlv(ptlv, link_info, tlv_len);
	valid_link_bmap =
		MLO_SHMEM_GLB_LINK_INFO_PARAM_VALID_LINK_BMAP_GET(link_info);

	snapshot_info->valid_link_bmap = valid_link_bmap;
	snapshot_info->num_links =
			get_num_links_from_valid_link_bitmap(valid_link_bmap);
	snapshot_info->link_info = qdf_mem_malloc(
					sizeof(*snapshot_info->link_info) *
					snapshot_info->num_links);
	if (!snapshot_info->link_info) {
		target_if_err("Couldn't allocate memory for rx_reo_per_link_info");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	return tlv_len;
}

/**
 * free_mlo_glb_rx_reo_per_link_info() - Free Rx REO per-link info
 * @snapshot_info: Pointer to MGMT Rx REO snapshot info
 *
 * Return: None
 */
static void free_mlo_glb_rx_reo_per_link_info(
	struct wlan_host_mlo_glb_rx_reo_snapshot_info *snapshot_info)
{
	if (snapshot_info && snapshot_info->link_info) {
		qdf_mem_free(snapshot_info->link_info);
		snapshot_info->link_info =  NULL;
	}
}

/**
 * get_next_valid_link_id() - Get next valid link ID
 * @valid_link_bmap: Link bit map where the valid links are set to 1
 * @prev_link_id: Previous link ID
 *
 * Return: Next valid link ID if there are valid links after @prev_link_id,
 * else -1
 */
static int
get_next_valid_link_id(uint16_t valid_link_bmap, int prev_link_id)
{
	int cur_link_id;
	uint16_t mask;
	uint8_t maxbits = sizeof(valid_link_bmap) * QDF_CHAR_BIT;

	cur_link_id = prev_link_id + 1;
	mask = 1 << cur_link_id;

	while (!(valid_link_bmap & mask)) {
		cur_link_id++;
		if (cur_link_id == maxbits)
			return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
		mask = mask << 1;
	}

	return cur_link_id;
}

/**
 * extract_mlo_glb_rx_reo_snapshot_info() - extract MGMT Rx REO snapshot info
 * @data: Pointer to start of MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_SNAPSHOT_INFO
 * TLV
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @snapshot_info: Pointer to MGMT Rx REO snapshot info. Extracted information
 * will be populated in this data structure.
 *
 * Return: On success, the number of bytes parsed. On failure, -1.
 */
static int
extract_mlo_glb_rx_reo_snapshot_info(
	uint8_t *data, size_t remaining_len,
	struct wlan_host_mlo_glb_rx_reo_snapshot_info *snapshot_info)
{
	int parsed_bytes, len;
	uint8_t link;
	int cur_link_id, prev_link_id = -1;
	uint16_t valid_link_bmap;

	qdf_assert_always(data);
	qdf_assert_always(snapshot_info);

	/* Extract MLO_SHMEM_TLV_STRUCT_MLO_GLB_RX_REO_SNAPSHOT_INFO TLV */
	len = extract_mlo_glb_rx_reo_snapshot_info_tlv(data, remaining_len,
						       snapshot_info);
	validate_parsed_bytes_advance_data_pointer(len, data, remaining_len);
	parsed_bytes = len;

	valid_link_bmap = snapshot_info->valid_link_bmap;
	/* Foreach valid link */
	for (link = 0; link < snapshot_info->num_links; ++link) {
		cur_link_id = get_next_valid_link_id(valid_link_bmap,
						     prev_link_id);

		qdf_assert_always(cur_link_id >= 0);

		/* Extract per_link_info */
		len  = extract_mlo_glb_rx_reo_per_link_info_tlv(
					data, remaining_len, cur_link_id,
					&snapshot_info->link_info[link]);
		validate_parsed_bytes_advance_data_pointer(len, data,
							   remaining_len);
		parsed_bytes += len;
		prev_link_id = cur_link_id;
	}

	return parsed_bytes;
}

/**
 * extract_mlo_glb_h_shmem_tlv() - extract MLO_SHMEM_TLV_STRUCT_MLO_GLB_H_SHMEM
 * TLV
 * @data: Pointer to start of the TLV
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @shmem_params: Pointer to MLO Global shared memory parameters. Extracted
 * information will be populated in this data structure.
 *
 * Return: On success, the number of bytes parsed. On failure, -1.
 */
static int
extract_mlo_glb_h_shmem_tlv(
		uint8_t *data, size_t remaining_len,
		struct wlan_host_mlo_glb_h_shmem_params *shmem_params)
{
	mlo_glb_h_shmem *ptlv;
	uint32_t tlv_len, tlv_tag;
	uint32_t major_minor_version;

	qdf_assert_always(data);
	qdf_assert_always(shmem_params);
	if (process_tlv_header(data, remaining_len,
			       MLO_SHMEM_TLV_STRUCT_MLO_GLB_H_SHMEM,
			       &tlv_len, &tlv_tag) != 0) {
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	ptlv = (mlo_glb_h_shmem *)data;
	major_minor_version = get_field_value_in_tlv(ptlv, major_minor_version,
						     tlv_len);
	shmem_params->major_version =
			MLO_SHMEM_GLB_H_SHMEM_PARAM_MAJOR_VERSION_GET(
				major_minor_version);
	shmem_params->minor_version =
			MLO_SHMEM_GLB_H_SHMEM_PARAM_MINOR_VERSION_GET(
				major_minor_version);

	return tlv_len;
}

/**
 * parse_mlo_glb_h_shmem_arena() - Parse MLO Global shared memory arena
 * @data: Pointer to the first TLV in the arena
 * @remaining_len: Length (in bytes) remaining in the arena from @data pointer
 * @shmem_arena_ctx: Pointer to MLO Global shared memory arena context.
 * Extracted information will be populated in this data structure.
 *
 * Return: On success, the number of bytes parsed. On failure, -1.
 */
static int parse_mlo_glb_h_shmem_arena(
	uint8_t *data, size_t remaining_len,
	struct wlan_host_mlo_glb_h_shmem_arena_ctx *shmem_arena_ctx)
{
	int parsed_bytes, len;

	qdf_assert_always(data);
	qdf_assert_always(shmem_arena_ctx);

	len = extract_mlo_glb_h_shmem_tlv(data, remaining_len,
					  &shmem_arena_ctx->shmem_params);
	validate_parsed_bytes_advance_data_pointer(len, data, remaining_len);
	parsed_bytes += len;

	len = extract_mlo_glb_rx_reo_snapshot_info(
		data, remaining_len, &shmem_arena_ctx->rx_reo_snapshot_info);
	validate_parsed_bytes_advance_data_pointer(len, data, remaining_len);
	parsed_bytes += len;

	return parsed_bytes;
}

QDF_STATUS mlo_glb_h_shmem_arena_ctx_init(void *arena_vaddr,
					  size_t arena_len)
{
	struct wlan_host_mlo_glb_h_shmem_arena_ctx *shmem_arena_ctx;

	shmem_arena_ctx = get_shmem_arena_ctx();
	if (!shmem_arena_ctx) {
		target_if_err("mlo_glb_h_shmem_arena context is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* We need to initialize only for the first invocation */
	if (qdf_atomic_read(&shmem_arena_ctx->init_count))
		goto success;

	if (parse_mlo_glb_h_shmem_arena(arena_vaddr, arena_len,
					shmem_arena_ctx) < 0) {
		free_mlo_glb_rx_reo_per_link_info(
			&shmem_arena_ctx->rx_reo_snapshot_info);
		return QDF_STATUS_E_FAILURE;
	}

success:
	qdf_atomic_inc(&shmem_arena_ctx->init_count);
	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(mlo_glb_h_shmem_arena_ctx_init);

QDF_STATUS mlo_glb_h_shmem_arena_ctx_deinit(void)
{
	struct wlan_host_mlo_glb_h_shmem_arena_ctx *shmem_arena_ctx;

	shmem_arena_ctx = get_shmem_arena_ctx();
	if (!shmem_arena_ctx) {
		target_if_err("mlo_glb_h_shmem_arena context is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!qdf_atomic_read(&shmem_arena_ctx->init_count)) {
		target_if_fatal("shmem_arena_ctx ref cnt is 0");
		return QDF_STATUS_E_FAILURE;
	}

	/* We need to de-initialize only for the last invocation */
	if (qdf_atomic_dec_and_test(&shmem_arena_ctx->init_count))
		goto success;

	free_mlo_glb_rx_reo_per_link_info(
		&shmem_arena_ctx->rx_reo_snapshot_info);

success:
	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(mlo_glb_h_shmem_arena_ctx_deinit);

int mgmt_rx_reo_get_num_links(void)
{
	struct wlan_host_mlo_glb_h_shmem_arena_ctx *shmem_arena_ctx;

	shmem_arena_ctx = get_shmem_arena_ctx();
	if (!shmem_arena_ctx) {
		target_if_err("mlo_glb_h_shmem_arena context is NULL");
		return qdf_status_to_os_return(QDF_STATUS_E_FAILURE);
	}

	return shmem_arena_ctx->rx_reo_snapshot_info.num_links;
}

void *mgmt_rx_reo_get_snapshot_address(
	uint8_t link_id, enum mgmt_rx_reo_shared_snapshot_id snapshot_id)
{
	struct wlan_host_mlo_glb_h_shmem_arena_ctx *shmem_arena_ctx;
	struct wlan_host_mlo_glb_rx_reo_snapshot_info *snapshot_info;
	struct wlan_host_mlo_glb_rx_reo_per_link_info *snapshot_link_info;
	uint8_t link;

	if (snapshot_id >= MGMT_RX_REO_SHARED_SNAPSHOT_MAX) {
		target_if_err("Invalid snapshot ID: %d", snapshot_id);
		return NULL;
	}

	shmem_arena_ctx = get_shmem_arena_ctx();
	if (!shmem_arena_ctx) {
		target_if_err("mlo_glb_h_shmem_arena context is NULL");
		return NULL;
	}

	snapshot_info = &shmem_arena_ctx->rx_reo_snapshot_info;

	for (link = 0; link < snapshot_info->num_links; ++link) {
		snapshot_link_info = &snapshot_info->link_info[link];

		if (link_id == snapshot_link_info->link_id)
			break;
	}

	if (link == snapshot_info->num_links) {
		target_if_err("Couldn't find the snapshot link info"
			      "corresponding to the link %d", link_id);
		return NULL;
	}

	switch (snapshot_id) {
	case MGMT_RX_REO_SHARED_SNAPSHOT_MAC_HW:
		return snapshot_link_info->hw_forwarded;

	case MGMT_RX_REO_SHARED_SNAPSHOT_FW_CONSUMED:
		return snapshot_link_info->fw_consumed;

	case MGMT_RX_REO_SHARED_SNAPSHOT_FW_FORWADED:
		return snapshot_link_info->fw_forwarded;

	default:
		qdf_assert_always(0);
	}

	return NULL;
}
