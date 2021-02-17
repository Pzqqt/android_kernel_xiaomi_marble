/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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

#include "qdf_types.h"
#include "qdf_util.h"
#include "qdf_types.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "hal_api.h"
#include "target_type.h"
#include "wcss_version.h"
#include "qdf_module.h"
#include "hal_flow.h"
#include "rx_flow_search_entry.h"
#include "hal_rx_flow_info.h"

#include "hal_be_api.h"

#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_OFFSET \
	RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_OFFSET
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_MASK \
	RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_MASK
#define UNIFIED_RXPCU_PPDU_END_INFO_8_RX_PPDU_DURATION_LSB \
	RXPCU_PPDU_END_INFO_RX_PPDU_DURATION_LSB
#define UNIFIED_PHYRX_HT_SIG_0_HT_SIG_INFO_PHYRX_HT_SIG_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_L_SIG_B_0_L_SIG_B_INFO_PHYRX_L_SIG_B_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_B_PHYRX_L_SIG_B_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_L_SIG_A_0_L_SIG_A_INFO_PHYRX_L_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_L_SIG_A_PHYRX_L_SIG_A_INFO_DETAILS_RATE_OFFSET
#define UNIFIED_PHYRX_VHT_SIG_A_0_VHT_SIG_A_INFO_PHYRX_VHT_SIG_A_INFO_DETAILS_OFFSET \
	PHYRX_VHT_SIG_A_PHYRX_VHT_SIG_A_INFO_DETAILS_BANDWIDTH_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_SU_0_HE_SIG_A_SU_INFO_PHYRX_HE_SIG_A_SU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_SU_PHYRX_HE_SIG_A_SU_INFO_DETAILS_FORMAT_INDICATION_OFFSET
#define UNIFIED_PHYRX_HE_SIG_A_MU_DL_0_HE_SIG_A_MU_DL_INFO_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_A_MU_DL_PHYRX_HE_SIG_A_MU_DL_INFO_DETAILS_DL_UL_FLAG_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B1_MU_0_HE_SIG_B1_MU_INFO_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B1_MU_PHYRX_HE_SIG_B1_MU_INFO_DETAILS_RU_ALLOCATION_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_MU_0_HE_SIG_B2_MU_INFO_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_MU_PHYRX_HE_SIG_B2_MU_INFO_DETAILS_STA_ID_OFFSET
#define UNIFIED_PHYRX_HE_SIG_B2_OFDMA_0_HE_SIG_B2_OFDMA_INFO_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_OFFSET \
	PHYRX_HE_SIG_B2_OFDMA_PHYRX_HE_SIG_B2_OFDMA_INFO_DETAILS_STA_ID_OFFSET

#define UNIFIED_PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_3_RECEIVE_RSSI_INFO_PRE_RSSI_INFO_DETAILS_OFFSET
#define UNIFIED_PHYRX_RSSI_LEGACY_19_RECEIVE_RSSI_INFO_PREAMBLE_RSSI_INFO_DETAILS_OFFSET \
	PHYRX_RSSI_LEGACY_PREAMBLE_RSSI_INFO_DETAILS_RSSI_PRI20_CHAIN0_OFFSET
#define UNIFIED_RX_MPDU_START_0_RX_MPDU_INFO_RX_MPDU_INFO_DETAILS_OFFSET \
	RX_MPDU_START_0_RX_MPDU_INFO_DETAILS_RXPT_CLASSIFY_INFO_DETAILS_REO_DESTINATION_INDICATION_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	RX_MPDU_DETAILS_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET \
	REO_DESTINATION_RING_2_RX_MPDU_DESC_INFO_RX_MPDU_DESC_INFO_DETAILS_OFFSET
#define UNIFORM_REO_STATUS_HEADER_STATUS_HEADER_GENERIC \
	UNIFORM_REO_STATUS_HEADER_STATUS_HEADER
#define UNIFIED_RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET \
	RX_MSDU_DETAILS_2_RX_MSDU_DESC_INFO_RX_MSDU_DESC_INFO_DETAILS_OFFSET
#define UNIFIED_RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET \
	RX_MSDU_LINK_8_RX_MSDU_DETAILS_MSDU_0_OFFSET
#define UNIFIED_TCL_DATA_CMD_0_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_31_0_OFFSET
#define UNIFIED_TCL_DATA_CMD_1_BUFFER_ADDR_INFO_BUF_ADDR_INFO_OFFSET \
	TCL_DATA_CMD_BUF_ADDR_INFO_BUFFER_ADDR_39_32_OFFSET
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_OFFSET \
	TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_OFFSET
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_LSB \
	BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_LSB
#define UNIFIED_BUFFER_ADDR_INFO_0_BUFFER_ADDR_31_0_MASK \
	BUFFER_ADDR_INFO_BUFFER_ADDR_31_0_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_LSB \
	BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_BUFFER_ADDR_39_32_MASK \
	BUFFER_ADDR_INFO_BUFFER_ADDR_39_32_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_LSB \
	BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_RETURN_BUFFER_MANAGER_MASK \
	BUFFER_ADDR_INFO_RETURN_BUFFER_MANAGER_MASK
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_LSB \
	BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_LSB
#define UNIFIED_BUFFER_ADDR_INFO_1_SW_BUFFER_COOKIE_MASK \
	BUFFER_ADDR_INFO_SW_BUFFER_COOKIE_MASK
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_LSB \
	TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_LSB
#define UNIFIED_TCL_DATA_CMD_2_BUF_OR_EXT_DESC_TYPE_MASK \
	TCL_DATA_CMD_BUF_OR_EXT_DESC_TYPE_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_MASK \
	WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_MASK
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_OFFSET \
	WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_OFFSET
#define UNIFIED_WBM_RELEASE_RING_6_TX_RATE_STATS_INFO_TX_RATE_STATS_LSB \
	WBM2SW_COMPLETION_RING_TX_TX_RATE_STATS_PPDU_TRANSMISSION_TSF_LSB

#include "hal_7850_tx.h"
#include "hal_7850_rx.h"

#include "hal_7850_rx_tlv.h"
#include "hal_be_rx_tlv.h"

#include <hal_generic_api.h>
#include <hal_be_generic_api.h>

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)

static uint32_t hal_get_link_desc_size_7850(void)
{
	return LINK_DESC_SIZE;
}

/**
 * hal_reo_status_get_header_7850 - Process reo desc info
 * @d - Pointer to reo descriptior
 * @b - tlv type info
 * @h1 - Pointer to hal_reo_status_header where info to be stored
 *
 * Return - none.
 *
 */
static void hal_reo_status_get_header_7850(hal_ring_desc_t ring_desc, int b,
					   void *h1)
{
	uint64_t *d = (uint64_t *)ring_desc;
	uint64_t val1 = 0;
	struct hal_reo_status_header *h =
			(struct hal_reo_status_header *)h1;

	/* Offsets of descriptor fields defined in HW headers start
	 * from the field after TLV header
	 */
	d += HAL_GET_NUM_QWORDS(sizeof(struct tlv_32_hdr));

	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_GET_QUEUE_STATS_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_QUEUE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_CACHE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UNBLOCK_CACHE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_TIMEOUT_LIST_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_QW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS,
		  STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UPDATE_RX_REO_QUEUE_STATUS,
			STATUS_HEADER_REO_STATUS_NUMBER)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->cmd_num =
		HAL_GET_FIELD(
			      UNIFORM_REO_STATUS_HEADER, REO_STATUS_NUMBER,
			      val1);
	h->exec_time =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER,
			      CMD_EXECUTION_TIME, val1);
	h->status =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER,
			      REO_CMD_EXECUTION_STATUS, val1);
	switch (b) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_GET_QUEUE_STATS_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_QUEUE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_CACHE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UNBLOCK_CACHE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_FLUSH_TIMEOUT_LIST_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		val1 =
		  d[HAL_OFFSET_QW(REO_DESCRIPTOR_THRESHOLD_REACHED_STATUS,
		  STATUS_HEADER_TIMESTAMP)];
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		val1 = d[HAL_OFFSET_QW(REO_UPDATE_RX_REO_QUEUE_STATUS,
			STATUS_HEADER_TIMESTAMP)];
		break;
	default:
		qdf_nofl_err("ERROR: Unknown tlv\n");
		break;
	}
	h->tstamp =
		HAL_GET_FIELD(UNIFORM_REO_STATUS_HEADER, TIMESTAMP, val1);
}

static
void *hal_rx_msdu0_buffer_addr_lsb_7850(void *link_desc_va)
{
	return (void *)HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va);
}

static
void *hal_rx_msdu_desc_info_ptr_get_7850(void *msdu0)
{
	return (void *)HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0);
}

static
void *hal_ent_mpdu_desc_info_7850(void *ent_ring_desc)
{
	return (void *)HAL_ENT_MPDU_DESC_INFO(ent_ring_desc);
}

static
void *hal_dst_mpdu_desc_info_7850(void *dst_ring_desc)
{
	return (void *)HAL_DST_MPDU_DESC_INFO(dst_ring_desc);
}

/**
 * hal_reo_config_7850(): Set reo config parameters
 * @soc: hal soc handle
 * @reg_val: value to be set
 * @reo_params: reo parameters
 *
 * Return: void
 */
static
void hal_reo_config_7850(struct hal_soc *soc,
			 uint32_t reg_val,
			 struct hal_reo_params *reo_params)
{
	HAL_REO_R0_CONFIG(soc, reg_val, reo_params);
}

/**
 * hal_rx_msdu_desc_info_get_ptr_7850() - Get msdu desc info ptr
 * @msdu_details_ptr - Pointer to msdu_details_ptr
 *
 * Return - Pointer to rx_msdu_desc_info structure.
 *
 */
static void *hal_rx_msdu_desc_info_get_ptr_7850(void *msdu_details_ptr)
{
	return HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr);
}

/**
 * hal_rx_link_desc_msdu0_ptr_7850 - Get pointer to rx_msdu details
 * @link_desc - Pointer to link desc
 *
 * Return - Pointer to rx_msdu_details structure
 *
 */
static void *hal_rx_link_desc_msdu0_ptr_7850(void *link_desc)
{
	return HAL_RX_LINK_DESC_MSDU0_PTR(link_desc);
}

/**
 * hal_get_window_address_7850(): Function to get hp/tp address
 * @hal_soc: Pointer to hal_soc
 * @addr: address offset of register
 *
 * Return: modified address offset of register
 */
static inline qdf_iomem_t hal_get_window_address_7850(struct hal_soc *hal_soc,
						      qdf_iomem_t addr)
{
	return addr;
}

/**
 * hal_reo_set_err_dst_remap_7850(): Function to set REO error destination
 *				     ring remap register
 * @hal_soc: Pointer to hal_soc
 *
 * Return: none.
 */
static void
hal_reo_set_err_dst_remap_7850(void *hal_soc)
{
	/*
	 * Set REO error 2k jump (error code 5) / OOR (error code 7)
	 * frame routed to REO2TCL ring.
	 */
	uint32_t dst_remap_ix0 =
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_RELEASE, 0) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_RELEASE, 1) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_RELEASE, 2) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_RELEASE, 3) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_RELEASE, 4) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 5) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 6) |
		HAL_REO_ERR_REMAP_IX0(REO_REMAP_TCL, 7);

	uint32_t dst_remap_ix1 =
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_RELEASE, 14) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_RELEASE, 13) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_RELEASE, 12) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_RELEASE, 11) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_RELEASE, 10) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_RELEASE, 9) |
		HAL_REO_ERR_REMAP_IX1(REO_REMAP_TCL, 8);

		HAL_REG_WRITE(hal_soc,
			      HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ADDR(
			      REO_REG_REG_BASE),
			      dst_remap_ix0);

		hal_info("HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0 0x%x",
			 HAL_REG_READ(
			 hal_soc,
			 HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_0_ADDR(
			 REO_REG_REG_BASE)));

		HAL_REG_WRITE(hal_soc,
			      HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ADDR(
			      REO_REG_REG_BASE),
			      dst_remap_ix1);

		hal_info("HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1 0x%x",
			 HAL_REG_READ(
			 hal_soc,
			 HWIO_REO_R0_ERROR_DESTINATION_MAPPING_IX_1_ADDR(
			 REO_REG_REG_BASE)));
}

/**
 * hal_rx_flow_setup_fse_7850() - Setup a flow search entry in HW FST
 * @fst: Pointer to the Rx Flow Search Table
 * @table_offset: offset into the table where the flow is to be setup
 * @flow: Flow Parameters
 *
 * Flow table entry fields are updated in host byte order, little endian order.
 *
 * Return: Success/Failure
 */
static void *
hal_rx_flow_setup_fse_7850(uint8_t *rx_fst, uint32_t table_offset,
			   uint8_t *rx_flow)
{
	struct hal_rx_fst *fst = (struct hal_rx_fst *)rx_fst;
	struct hal_rx_flow *flow = (struct hal_rx_flow *)rx_flow;
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

	fse_valid = HAL_GET_FLD(fse, RX_FLOW_SEARCH_ENTRY, VALID);

	if (fse_valid) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "HAL FSE %pK already valid", fse);
		return NULL;
	}

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_127_96,
			       (flow->tuple_info.src_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_95_64,
			       (flow->tuple_info.src_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_63_32,
			       (flow->tuple_info.src_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_IP_31_0,
			       (flow->tuple_info.src_ip_31_0));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_127_96) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_127_96,
			       (flow->tuple_info.dest_ip_127_96));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_95_64) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_95_64,
			       (flow->tuple_info.dest_ip_95_64));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_63_32) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_63_32,
			       (flow->tuple_info.dest_ip_63_32));

	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_IP_31_0) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_IP_31_0,
			       (flow->tuple_info.dest_ip_31_0));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_PORT);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, DEST_PORT) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, DEST_PORT,
			       (flow->tuple_info.dest_port));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_PORT);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, SRC_PORT) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, SRC_PORT,
			       (flow->tuple_info.src_port));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, L4_PROTOCOL);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, L4_PROTOCOL) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, L4_PROTOCOL,
			       flow->tuple_info.l4_protocol);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_HANDLER);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_HANDLER) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_HANDLER,
			       flow->reo_destination_handler);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, VALID);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, VALID) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, VALID, 1);

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, METADATA);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, METADATA) =
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY, METADATA,
			       (flow->fse_metadata));

	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_INDICATION);
	HAL_SET_FLD(fse, RX_FLOW_SEARCH_ENTRY, REO_DESTINATION_INDICATION) |=
		HAL_SET_FLD_SM(RX_FLOW_SEARCH_ENTRY,
			       REO_DESTINATION_INDICATION,
			       flow->reo_destination_indication);

	/* Reset all the other fields in FSE */
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, RESERVED_9);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, MSDU_DROP);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, MSDU_COUNT);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, MSDU_BYTE_COUNT);
	HAL_CLR_FLD(fse, RX_FLOW_SEARCH_ENTRY, TIMESTAMP);

	return fse;
}

static
void hal_compute_reo_remap_ix2_ix3_7850(uint32_t *ring, uint32_t num_rings,
					uint32_t *remap1, uint32_t *remap2)
{
	switch (num_rings) {
	case 3:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[1], 17) |
				HAL_REO_REMAP_IX2(ring[2], 18) |
				HAL_REO_REMAP_IX2(ring[0], 19) |
				HAL_REO_REMAP_IX2(ring[1], 20) |
				HAL_REO_REMAP_IX2(ring[2], 21) |
				HAL_REO_REMAP_IX2(ring[0], 22) |
				HAL_REO_REMAP_IX2(ring[1], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[2], 24) |
				HAL_REO_REMAP_IX3(ring[0], 25) |
				HAL_REO_REMAP_IX3(ring[1], 26) |
				HAL_REO_REMAP_IX3(ring[2], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[1], 29) |
				HAL_REO_REMAP_IX3(ring[2], 30) |
				HAL_REO_REMAP_IX3(ring[0], 31);
		break;
	case 4:
		*remap1 = HAL_REO_REMAP_IX2(ring[0], 16) |
				HAL_REO_REMAP_IX2(ring[1], 17) |
				HAL_REO_REMAP_IX2(ring[2], 18) |
				HAL_REO_REMAP_IX2(ring[3], 19) |
				HAL_REO_REMAP_IX2(ring[0], 20) |
				HAL_REO_REMAP_IX2(ring[1], 21) |
				HAL_REO_REMAP_IX2(ring[2], 22) |
				HAL_REO_REMAP_IX2(ring[3], 23);

		*remap2 = HAL_REO_REMAP_IX3(ring[0], 24) |
				HAL_REO_REMAP_IX3(ring[1], 25) |
				HAL_REO_REMAP_IX3(ring[2], 26) |
				HAL_REO_REMAP_IX3(ring[3], 27) |
				HAL_REO_REMAP_IX3(ring[0], 28) |
				HAL_REO_REMAP_IX3(ring[1], 29) |
				HAL_REO_REMAP_IX3(ring[2], 30) |
				HAL_REO_REMAP_IX3(ring[3], 31);
		break;
	}
}

/* NUM TCL Bank registers in WCN7850 */
#define HAL_NUM_TCL_BANKS_7850 8

/**
 * hal_tx_get_num_tcl_banks_7850() - Get number of banks in target
 *
 * Returns: number of bank
 */
static uint8_t hal_tx_get_num_tcl_banks_7850(void)
{
	return HAL_NUM_TCL_BANKS_7850;
}

static void hal_hw_txrx_ops_attach_wcn7850(struct hal_soc *hal_soc)
{
	/* init and setup */
	hal_soc->ops->hal_srng_dst_hw_init = hal_srng_dst_hw_init_generic;
	hal_soc->ops->hal_srng_src_hw_init = hal_srng_src_hw_init_generic;
	hal_soc->ops->hal_get_hw_hptp = hal_get_hw_hptp_generic;
	hal_soc->ops->hal_reo_setup = hal_reo_setup_generic_be;
	hal_soc->ops->hal_get_window_address = hal_get_window_address_7850;
	hal_soc->ops->hal_reo_set_err_dst_remap =
						hal_reo_set_err_dst_remap_7850;

	/* tx */
	hal_soc->ops->hal_tx_set_dscp_tid_map = hal_tx_set_dscp_tid_map_7850;
	hal_soc->ops->hal_tx_update_dscp_tid = hal_tx_update_dscp_tid_7850;
	hal_soc->ops->hal_tx_desc_set_lmac_id = hal_tx_desc_set_lmac_id_7850;
	hal_soc->ops->hal_tx_desc_set_buf_addr =
					hal_tx_desc_set_buf_addr_generic_be;
	hal_soc->ops->hal_tx_desc_set_search_index =
					hal_tx_desc_set_search_index_generic_be;
	hal_soc->ops->hal_tx_desc_set_cache_set_num =
				hal_tx_desc_set_cache_set_num_generic_be;
	hal_soc->ops->hal_tx_comp_get_status =
					hal_tx_comp_get_status_generic_be;
	hal_soc->ops->hal_tx_comp_get_release_reason =
				hal_tx_comp_get_release_reason_generic_be;
	hal_soc->ops->hal_get_wbm_internal_error =
					hal_get_wbm_internal_error_generic_be;
	hal_soc->ops->hal_tx_init_cmd_credit_ring =
					hal_tx_init_cmd_credit_ring_7850;

	/* rx */
	hal_soc->ops->hal_rx_msdu_start_nss_get = hal_rx_tlv_nss_get_be;
	hal_soc->ops->hal_rx_mon_hw_desc_get_mpdu_status =
		hal_rx_mon_hw_desc_get_mpdu_status_be;
	hal_soc->ops->hal_rx_get_tlv = hal_rx_get_tlv_7850;
	hal_soc->ops->hal_rx_pkt_hdr_get = hal_rx_pkt_hdr_get_be;
	hal_soc->ops->hal_rx_proc_phyrx_other_receive_info_tlv =
		hal_rx_proc_phyrx_other_receive_info_tlv_7850;
	hal_soc->ops->hal_get_link_desc_size = hal_get_link_desc_size_7850;
	hal_soc->ops->hal_rx_mpdu_start_tid_get = hal_rx_tlv_tid_get_be;
	hal_soc->ops->hal_rx_msdu_start_reception_type_get =
		hal_rx_tlv_reception_type_get_be;
	hal_soc->ops->hal_rx_msdu_end_da_idx_get =
					hal_rx_msdu_end_da_idx_get_be;
	hal_soc->ops->hal_rx_msdu_desc_info_get_ptr =
					hal_rx_msdu_desc_info_get_ptr_7850;
	hal_soc->ops->hal_rx_link_desc_msdu0_ptr =
					hal_rx_link_desc_msdu0_ptr_7850;
	hal_soc->ops->hal_reo_status_get_header =
					hal_reo_status_get_header_7850;
	hal_soc->ops->hal_rx_status_get_tlv_info =
					hal_rx_status_get_tlv_info_generic_be;
	hal_soc->ops->hal_rx_wbm_err_info_get =
					hal_rx_wbm_err_info_get_generic_be;
	hal_soc->ops->hal_rx_priv_info_set_in_tlv =
					hal_rx_priv_info_set_in_tlv_be;
	hal_soc->ops->hal_rx_priv_info_get_from_tlv =
					hal_rx_priv_info_get_from_tlv_be;

	hal_soc->ops->hal_tx_set_pcp_tid_map =
					hal_tx_set_pcp_tid_map_generic_be;
	hal_soc->ops->hal_tx_update_pcp_tid_map =
					hal_tx_update_pcp_tid_generic_be;
	hal_soc->ops->hal_tx_set_tidmap_prty =
					hal_tx_update_tidmap_prty_generic_be;
	hal_soc->ops->hal_rx_get_rx_fragment_number =
					hal_rx_get_rx_fragment_number_be;
	hal_soc->ops->hal_rx_msdu_end_da_is_mcbc_get =
					hal_rx_tlv_da_is_mcbc_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_is_valid_get =
		hal_rx_tlv_sa_is_valid_get_be;
	hal_soc->ops->hal_rx_msdu_end_sa_idx_get = hal_rx_tlv_sa_idx_get_be,
	hal_soc->ops->hal_rx_desc_is_first_msdu =
					hal_rx_desc_is_first_msdu_be;
	hal_soc->ops->hal_rx_msdu_end_l3_hdr_padding_get =
		hal_rx_tlv_l3_hdr_padding_get_be;
	hal_soc->ops->hal_rx_encryption_info_valid =
					hal_rx_encryption_info_valid_be;
	hal_soc->ops->hal_rx_print_pn = hal_rx_print_pn_be;
	hal_soc->ops->hal_rx_msdu_end_first_msdu_get =
					hal_rx_tlv_first_msdu_get_be;
	hal_soc->ops->hal_rx_msdu_end_da_is_valid_get =
		hal_rx_tlv_da_is_valid_get_be;
	hal_soc->ops->hal_rx_msdu_end_last_msdu_get =
					hal_rx_tlv_last_msdu_get_be;
	hal_soc->ops->hal_rx_get_mpdu_mac_ad4_valid =
					hal_rx_get_mpdu_mac_ad4_valid_be;
	hal_soc->ops->hal_rx_mpdu_start_sw_peer_id_get =
		hal_rx_mpdu_start_sw_peer_id_get_be;
	hal_soc->ops->hal_rx_mpdu_get_to_ds = hal_rx_mpdu_get_to_ds_be;
	hal_soc->ops->hal_rx_mpdu_get_fr_ds = hal_rx_mpdu_get_fr_ds_be;
	hal_soc->ops->hal_rx_get_mpdu_frame_control_valid =
		hal_rx_get_mpdu_frame_control_valid_be;
	hal_soc->ops->hal_rx_mpdu_get_addr1 = hal_rx_mpdu_get_addr1_be;
	hal_soc->ops->hal_rx_mpdu_get_addr2 = hal_rx_mpdu_get_addr2_be;
	hal_soc->ops->hal_rx_mpdu_get_addr3 = hal_rx_mpdu_get_addr3_be;
	hal_soc->ops->hal_rx_mpdu_get_addr4 = hal_rx_mpdu_get_addr4_be;
	hal_soc->ops->hal_rx_get_mpdu_sequence_control_valid =
		hal_rx_get_mpdu_sequence_control_valid_be;
	hal_soc->ops->hal_rx_is_unicast = hal_rx_is_unicast_be;
	hal_soc->ops->hal_rx_tid_get = hal_rx_tid_get_be;
	hal_soc->ops->hal_rx_hw_desc_get_ppduid_get =
					hal_rx_hw_desc_get_ppduid_get_be;
	hal_soc->ops->hal_rx_msdu0_buffer_addr_lsb =
					hal_rx_msdu0_buffer_addr_lsb_7850;
	hal_soc->ops->hal_rx_msdu_desc_info_ptr_get =
					hal_rx_msdu_desc_info_ptr_get_7850;
	hal_soc->ops->hal_ent_mpdu_desc_info = hal_ent_mpdu_desc_info_7850;
	hal_soc->ops->hal_dst_mpdu_desc_info = hal_dst_mpdu_desc_info_7850;
	hal_soc->ops->hal_rx_get_fc_valid = hal_rx_get_fc_valid_be;
	hal_soc->ops->hal_rx_get_to_ds_flag = hal_rx_get_to_ds_flag_be;
	hal_soc->ops->hal_rx_get_mac_addr2_valid =
					hal_rx_get_mac_addr2_valid_be;
	hal_soc->ops->hal_rx_get_filter_category =
					hal_rx_get_filter_category_be;
	hal_soc->ops->hal_rx_get_ppdu_id = hal_rx_get_ppdu_id_be;
	hal_soc->ops->hal_reo_config = hal_reo_config_7850;
	hal_soc->ops->hal_rx_msdu_flow_idx_get = hal_rx_msdu_flow_idx_get_be;
	hal_soc->ops->hal_rx_msdu_flow_idx_invalid =
					hal_rx_msdu_flow_idx_invalid_be;
	hal_soc->ops->hal_rx_msdu_flow_idx_timeout =
					hal_rx_msdu_flow_idx_timeout_be;
	hal_soc->ops->hal_rx_msdu_fse_metadata_get =
					hal_rx_msdu_fse_metadata_get_be;
	hal_soc->ops->hal_rx_msdu_cce_metadata_get =
					hal_rx_msdu_cce_metadata_get_be;
	hal_soc->ops->hal_rx_msdu_get_flow_params =
					hal_rx_msdu_get_flow_params_be;
	hal_soc->ops->hal_rx_tlv_get_tcp_chksum =
					hal_rx_tlv_get_tcp_chksum_be;
	hal_soc->ops->hal_rx_get_rx_sequence = hal_rx_get_rx_sequence_be;
#if defined(QCA_WIFI_WCN7850) && defined(WLAN_CFR_ENABLE) && \
	defined(WLAN_ENH_CFR_ENABLE)
	hal_soc->ops->hal_rx_get_bb_info = hal_rx_get_bb_info_7850;
	hal_soc->ops->hal_rx_get_rtt_info = hal_rx_get_rtt_info_7850;
#else
	hal_soc->ops->hal_rx_get_bb_info = NULL;
	hal_soc->ops->hal_rx_get_rtt_info = NULL;
#endif
	/* rx - msdu end fast path info fields */
	hal_soc->ops->hal_rx_msdu_packet_metadata_get =
		hal_rx_msdu_packet_metadata_get_generic_be;
	hal_soc->ops->hal_rx_get_fisa_cumulative_l4_checksum =
		hal_rx_get_fisa_cumulative_l4_checksum_be;
	hal_soc->ops->hal_rx_get_fisa_cumulative_ip_length =
		hal_rx_get_fisa_cumulative_ip_length_be;
	hal_soc->ops->hal_rx_get_udp_proto = hal_rx_get_udp_proto_be;
	hal_soc->ops->hal_rx_get_fisa_flow_agg_continuation =
		hal_rx_get_flow_agg_continuation_be;
	hal_soc->ops->hal_rx_get_fisa_flow_agg_count =
					hal_rx_get_flow_agg_count_be;
	hal_soc->ops->hal_rx_get_fisa_timeout = hal_rx_get_fisa_timeout_be;
	hal_soc->ops->hal_rx_mpdu_start_tlv_tag_valid =
		hal_rx_mpdu_start_tlv_tag_valid_be;

	/* rx - TLV struct offsets */
	hal_soc->ops->hal_rx_msdu_end_offset_get =
					hal_rx_msdu_end_offset_get_generic;
//	hal_soc->ops->hal_rx_attn_offset_get = hal_rx_attn_offset_get_generic;
//	hal_soc->ops->hal_rx_msdu_start_offset_get =
//					hal_rx_msdu_start_offset_get_generic;
//	hal_soc->ops->hal_rx_mpdu_end_offset_get =
//					hal_rx_mpdu_end_offset_get_generic;
	hal_soc->ops->hal_rx_mpdu_start_offset_get =
					hal_rx_mpdu_start_offset_get_generic;
	hal_soc->ops->hal_rx_pkt_tlv_offset_get =
					hal_rx_pkt_tlv_offset_get_generic;
	hal_soc->ops->hal_rx_flow_setup_fse = hal_rx_flow_setup_fse_7850;
	hal_soc->ops->hal_compute_reo_remap_ix2_ix3 =
					hal_compute_reo_remap_ix2_ix3_7850;
	hal_soc->ops->hal_rx_flow_setup_cmem_fse = NULL;
	hal_soc->ops->hal_rx_flow_get_cmem_fse_ts = NULL;
	hal_soc->ops->hal_rx_flow_get_cmem_fse = NULL;
	hal_soc->ops->hal_rx_msdu_get_reo_destination_indication =
		hal_rx_msdu_get_reo_destination_indication_be;
	hal_soc->ops->hal_tx_get_num_tcl_banks = hal_tx_get_num_tcl_banks_7850;
	hal_soc->ops->hal_rx_get_tlv_size = hal_rx_get_tlv_size_generic_be;
	hal_soc->ops->hal_rx_msdu_is_wlan_mcast =
					hal_rx_msdu_is_wlan_mcast_generic_be;
	hal_soc->ops->hal_rx_tlv_bw_get =
					hal_rx_tlv_bw_get_be;
	hal_soc->ops->hal_rx_tlv_get_is_decrypted =
						hal_rx_tlv_get_is_decrypted_be;
	hal_soc->ops->hal_rx_tlv_mic_err_get = hal_rx_tlv_mic_err_get_be;
	hal_soc->ops->hal_rx_tlv_get_pkt_type = hal_rx_tlv_get_pkt_type_be;
	hal_soc->ops->hal_rx_tlv_get_freq = hal_rx_tlv_get_freq_be;
	hal_soc->ops->hal_rx_tlv_get_freq = hal_rx_tlv_get_freq_be;
	hal_soc->ops->hal_rx_tlv_mpdu_len_err_get =
					hal_rx_tlv_mpdu_len_err_get_be;
	hal_soc->ops->hal_rx_tlv_mpdu_fcs_err_get =
					hal_rx_tlv_mpdu_fcs_err_get_be;

	hal_soc->ops->hal_rx_tlv_first_mpdu_get = hal_rx_tlv_first_mpdu_get_be;
	hal_soc->ops->hal_rx_tlv_decrypt_err_get =
					hal_rx_tlv_decrypt_err_get_be;
	hal_soc->ops->hal_rx_tlv_rate_mcs_get = hal_rx_tlv_rate_mcs_get_be;
	hal_soc->ops->hal_rx_tlv_sgi_get = hal_rx_tlv_sgi_get_be;
	hal_soc->ops->hal_rx_tlv_decap_format_get =
					hal_rx_tlv_decap_format_get_be;
	hal_soc->ops->hal_rx_tlv_get_offload_info =
					hal_rx_tlv_get_offload_info_be;
	hal_soc->ops->hal_rx_tlv_phy_ppdu_id_get =
					hal_rx_attn_phy_ppdu_id_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_done_get = hal_rx_tlv_msdu_done_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_len_get =
					hal_rx_msdu_start_msdu_len_get_be;
	hal_soc->ops->hal_rx_get_frame_ctrl_field =
					hal_rx_get_frame_ctrl_field_be;
	hal_soc->ops->hal_rx_get_proto_params = hal_rx_get_proto_params_be;
	hal_soc->ops->hal_rx_get_l3_l4_offsets = hal_rx_get_l3_l4_offsets_be;
	hal_soc->ops->hal_rx_tlv_csum_err_get = hal_rx_tlv_csum_err_get_be;
	hal_soc->ops->hal_rx_mpdu_info_ampdu_flag_get =
					hal_rx_mpdu_info_ampdu_flag_get_be;
	hal_soc->ops->hal_rx_tlv_msdu_len_set =
					hal_rx_msdu_start_msdu_len_set_be;
};

struct hal_hw_srng_config hw_srng_table_7850[] = {
	/* TODO: max_rings can populated by querying HW capabilities */
	{ /* REO_DST */
		.start_ring_id = HAL_SRNG_REO2SW1,
		.max_rings = 4,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO2SW1_RING_HP_ADDR(
				REO_REG_REG_BASE)
		},
		.reg_size = {
			HWIO_REO_R0_REO2SW2_RING_BASE_LSB_ADDR(0) -
				HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(0),
			HWIO_REO_R2_REO2SW2_RING_HP_ADDR(0) -
				HWIO_REO_R2_REO2SW1_RING_HP_ADDR(0),
		},
		.max_size =
			HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_EXCEPTION */
		/* Designating REO2SW0 ring as exception ring. */
		.start_ring_id = HAL_SRNG_REO2SW0,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW0_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO2SW0_RING_HP_ADDR(
				REO_REG_REG_BASE)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2SW0_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_REINJECT */
		.start_ring_id = HAL_SRNG_SW2REO,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_SW2REO_RING_HP_ADDR(
				REO_REG_REG_BASE)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_BMSK >>
				HWIO_REO_R0_SW2REO_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_CMD */
		.start_ring_id = HAL_SRNG_REO_CMD,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct reo_get_queue_stats)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_REO_CMD_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO_CMD_RING_HP_ADDR(
				REO_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_STATUS */
		.start_ring_id = HAL_SRNG_REO_STATUS,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct reo_get_queue_stats_status)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO_STATUS_RING_BASE_LSB_ADDR(
				REO_REG_REG_BASE),
			HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(
				REO_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_DATA */
		.start_ring_id = HAL_SRNG_SW2TCL1,
		.max_rings = 3,
		.entry_size = sizeof(struct tcl_data_cmd) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		.reg_size = {
			HWIO_TCL_R0_SW2TCL2_RING_BASE_LSB_ADDR(0) -
				HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(0),
			HWIO_TCL_R2_SW2TCL2_RING_HP_ADDR(0) -
				HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(0),
		},
		.max_size =
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_CMD */
		.start_ring_id = HAL_SRNG_SW2TCL_CMD,
		.max_rings = 1,
		.entry_size = sizeof(struct tcl_gse_cmd) >> 2,
		.lmac_ring =  FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_SW2TCL_CREDIT_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		      HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_BMSK >>
		      HWIO_TCL_R0_SW2TCL_CREDIT_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_STATUS */
		.start_ring_id = HAL_SRNG_TCL_STATUS,
		.max_rings = 1,
		/* confirm that TLV header is needed */
		.entry_size = sizeof(struct tcl_status_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_LSB_ADDR(
				MAC_TCL_REG_REG_BASE),
			HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(
				MAC_TCL_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_SRC */
		.start_ring_id = HAL_SRNG_CE_0_SRC,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_src_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_LSB_ADDR,
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R2_SRC_RING_HP_ADDR,
		},
		.reg_size = {
		SOC_CE_1_SRC_WFSS_CE_1_CHANNEL_SRC_REG_REG_BASE -
		SOC_CE_0_SRC_WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE,
		SOC_CE_1_SRC_WFSS_CE_1_CHANNEL_SRC_REG_REG_BASE -
		SOC_CE_0_SRC_WFSS_CE_0_CHANNEL_SRC_REG_REG_BASE,
		},
		.max_size =
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_SOC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST */
		.start_ring_id = HAL_SRNG_CE_0_DST,
		.max_rings = 12,
		.entry_size = 8 >> 2,
		/*TODO: entry_size above should actually be
		 * sizeof(struct ce_dst_desc) >> 2, but couldn't find definition
		 * of struct ce_dst_desc in HW header files
		 */
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR,
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR,
		},
		.reg_size = {
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		},
		.max_size =
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST_STATUS */
		.start_ring_id = HAL_SRNG_CE_0_DST_STATUS,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_stat_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_LSB_ADDR,
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R2_STATUS_RING_HP_ADDR,
		},
		.reg_size = {
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		SOC_CE_1_DST_WFSS_CE_1_CHANNEL_DST_REG_REG_BASE -
		SOC_CE_0_DST_WFSS_CE_0_CHANNEL_DST_REG_REG_BASE,
		},
		.max_size =
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_SOC_CE_0_DST_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM_IDLE_LINK */
		.start_ring_id = HAL_SRNG_WBM_IDLE_LINK,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_link_descriptor_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* SW2WBM_RELEASE */
		.start_ring_id = HAL_SRNG_WBM_SW_RELEASE,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM2SW_RELEASE */
		.start_ring_id = HAL_SRNG_WBM2SW0_RELEASE,
		.max_rings = 4,
		.entry_size = sizeof(struct wbm_release_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		.reg_size = {
		HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE) -
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(WBM_REG_REG_BASE),
		HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE) -
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(WBM_REG_REG_BASE),
		},
		.max_size =
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* RXDMA_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA0_BUF0,
#ifdef IPA_OFFLOAD
		.max_rings = 3,
#else
		.max_rings = 2,
#endif
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_DST */
		.start_ring_id = HAL_SRNG_WMAC1_RXDMA2SW0,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring =  TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA2_BUF,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_STATUS */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_STATBUF,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_DST */
		.start_ring_id = HAL_SRNG_WMAC1_RXDMA2SW1,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_DST_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* RXDMA_MONITOR_DESC */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA1_DESC,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
	{ /* DIR_BUF_RX_DMA_SRC */
		.start_ring_id = HAL_SRNG_DIR_BUF_RX_SRC_DMA_RING,
		/*
		 * one ring is for spectral scan
		 * the other is for cfr
		 */
		.max_rings = 2,
		.entry_size = 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
#ifdef WLAN_FEATURE_CIF_CFR
	{ /* WIFI_POS_SRC */
		.start_ring_id = HAL_SRNG_WIFI_POS_SRC_DMA_RING,
		.max_rings = 1,
		.entry_size = sizeof(wmi_oem_dma_buf_release_entry)  >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
		.max_size = HAL_RXDMA_MAX_RING_SIZE,
	},
#endif
};

int32_t hal_hw_reg_offset_wcn7850[] = {
	/* dst */
	REG_OFFSET(DST, HP),
	REG_OFFSET(DST, TP),
	REG_OFFSET(DST, ID),
	REG_OFFSET(DST, MISC),
	REG_OFFSET(DST, HP_ADDR_LSB),
	REG_OFFSET(DST, HP_ADDR_MSB),
	REG_OFFSET(DST, MSI1_BASE_LSB),
	REG_OFFSET(DST, MSI1_BASE_MSB),
	REG_OFFSET(DST, MSI1_DATA),
	REG_OFFSET(DST, BASE_LSB),
	REG_OFFSET(DST, BASE_MSB),
	REG_OFFSET(DST, PRODUCER_INT_SETUP),
	/* src */
	REG_OFFSET(SRC, HP),
	REG_OFFSET(SRC, TP),
	REG_OFFSET(SRC, ID),
	REG_OFFSET(SRC, MISC),
	REG_OFFSET(SRC, TP_ADDR_LSB),
	REG_OFFSET(SRC, TP_ADDR_MSB),
	REG_OFFSET(SRC, MSI1_BASE_LSB),
	REG_OFFSET(SRC, MSI1_BASE_MSB),
	REG_OFFSET(SRC, MSI1_DATA),
	REG_OFFSET(SRC, BASE_LSB),
	REG_OFFSET(SRC, BASE_MSB),
	REG_OFFSET(SRC, CONSUMER_INT_SETUP_IX0),
	REG_OFFSET(SRC, CONSUMER_INT_SETUP_IX1),
};

/**
 * hal_wcn7850_attach() - Attach 7850 target specific hal_soc ops,
 *			  offset and srng table
 */
void hal_wcn7850_attach(struct hal_soc *hal_soc)
{
	hal_soc->hw_srng_table = hw_srng_table_7850;
	hal_soc->hal_hw_reg_offset = hal_hw_reg_offset_wcn7850;
	hal_hw_txrx_default_ops_attach_be(hal_soc);
	hal_hw_txrx_ops_attach_wcn7850(hal_soc);
}
