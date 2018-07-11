/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "hal_6290_tx.h"
#include "hal_6290_rx.h"

struct hal_hw_txrx_ops qca6290_hal_hw_txrx_ops = {
	/* tx */
	hal_tx_desc_set_dscp_tid_table_id_6290,
	hal_tx_set_dscp_tid_map_6290,
	hal_tx_update_dscp_tid_6290,
	hal_tx_desc_set_lmac_id_6290,

	/* rx */
	hal_rx_msdu_start_nss_get_6290,
	hal_rx_mon_hw_desc_get_mpdu_status_6290,
	hal_rx_get_tlv_6290,
	hal_rx_proc_phyrx_other_receive_info_tlv_6290,
	hal_rx_dump_msdu_start_tlv_6290,
	hal_get_link_desc_size_6290,
	hal_rx_mpdu_start_tid_get_6290,
	hal_rx_msdu_start_reception_type_get_6290,
};

struct hal_hw_srng_config hw_srng_table_6290[] = {
	/* TODO: max_rings can populated by querying HW capabilities */
	{ /* REO_DST */
		.start_ring_id = HAL_SRNG_REO2SW1,
		.max_rings = 4,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO2SW1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
		},
		.reg_size = {
			HWIO_REO_R0_REO2SW2_RING_BASE_LSB_ADDR(0) -
				HWIO_REO_R0_REO2SW1_RING_BASE_LSB_ADDR(0),
			HWIO_REO_R2_REO2SW2_RING_HP_ADDR(0) -
				HWIO_REO_R2_REO2SW1_RING_HP_ADDR(0),
		},
		.max_size = HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2SW1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_EXCEPTION */
		/* Designating REO2TCL ring as exception ring. This ring is
		 * similar to other REO2SW rings though it is named as REO2TCL.
		 * Any of theREO2SW rings can be used as exception ring.
		 */
		.start_ring_id = HAL_SRNG_REO2TCL,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_destination_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_REO_R0_REO2TCL_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO2TCL_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_REO2TCL_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_REO_R0_REO2TCL_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* REO_REINJECT */
		.start_ring_id = HAL_SRNG_SW2REO,
		.max_rings = 1,
		.entry_size = sizeof(struct reo_entrance_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_REO_R0_SW2REO_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_SW2REO_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET)
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
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO_CMD_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size = HWIO_REO_R0_REO_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
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
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
			HWIO_REO_R2_REO_STATUS_RING_HP_ADDR(
				SEQ_WCSS_UMAC_REO_REG_OFFSET),
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
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_data_cmd)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
		},
		.reg_size = {
			HWIO_TCL_R0_SW2TCL2_RING_BASE_LSB_ADDR(0) -
				HWIO_TCL_R0_SW2TCL1_RING_BASE_LSB_ADDR(0),
			HWIO_TCL_R2_SW2TCL2_RING_HP_ADDR(0) -
				HWIO_TCL_R2_SW2TCL1_RING_HP_ADDR(0),
		},
		.max_size = HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL1_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_CMD */
		.start_ring_id = HAL_SRNG_SW2TCL_CMD,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_gse_cmd)) >> 2,
		.lmac_ring =  FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
			HWIO_TCL_R0_SW2TCL_CMD_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_SW2TCL_CMD_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
		},
		/* Single ring - provide ring size if multiple rings of this
		 * type are supported
		 */
		.reg_size = {},
		.max_size =
			HWIO_TCL_R0_SW2TCL_CMD_RING_BASE_MSB_RING_SIZE_BMSK >>
			HWIO_TCL_R0_SW2TCL_CMD_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* TCL_STATUS */
		.start_ring_id = HAL_SRNG_TCL_STATUS,
		.max_rings = 1,
		.entry_size = (sizeof(struct tlv_32_hdr) +
			sizeof(struct tcl_status_ring)) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
			HWIO_TCL_R0_TCL_STATUS1_RING_BASE_LSB_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
			HWIO_TCL_R2_TCL_STATUS1_RING_HP_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET),
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
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET),
		},
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_SRC_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_SRC_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_SRC_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
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
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_DEST_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		},
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* CE_DST_STATUS */
		.start_ring_id = HAL_SRNG_CE_0_DST_STATUS,
		.max_rings = 12,
		.entry_size = sizeof(struct ce_stat_desc) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_DST_RING,
		.reg_start = {
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_LSB_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		HWIO_WFSS_CE_CHANNEL_DST_R2_STATUS_RING_HP_ADDR(
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET),
		},
			/* TODO: check destination status ring registers */
		.reg_size = {
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_1_CHANNEL_DST_REG_OFFSET -
		SEQ_WCSS_UMAC_WFSS_CE_0_REG_WFSS_CE_0_CHANNEL_DST_REG_OFFSET,
		},
		.max_size =
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_BMSK >>
		HWIO_WFSS_CE_CHANNEL_DST_R0_STATUS_RING_BASE_MSB_RING_SIZE_SHFT,
	},
	{ /* WBM_IDLE_LINK */
		.start_ring_id = HAL_SRNG_WBM_IDLE_LINK,
		.max_rings = 1,
		.entry_size = sizeof(struct wbm_link_descriptor_ring) >> 2,
		.lmac_ring = FALSE,
		.ring_dir = HAL_SRNG_SRC_RING,
		.reg_start = {
		HWIO_WBM_R0_WBM_IDLE_LINK_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM_IDLE_LINK_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
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
		HWIO_WBM_R0_SW_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_SW_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
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
		HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
		},
		.reg_size = {
			HWIO_WBM_R0_WBM2SW1_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET) -
				HWIO_WBM_R0_WBM2SW0_RELEASE_RING_BASE_LSB_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
			HWIO_WBM_R2_WBM2SW1_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET) -
				HWIO_WBM_R2_WBM2SW0_RELEASE_RING_HP_ADDR(SEQ_WCSS_UMAC_WBM_REG_OFFSET),
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
		.max_rings = 1,
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

int32_t hal_hw_reg_offset_qca6290[] = {
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

void hal_qca6290_attach(struct hal_soc *hal_soc)
{
	hal_soc->hw_srng_table = hw_srng_table_6290;
	hal_soc->hal_hw_reg_offset = hal_hw_reg_offset_qca6290;
	hal_soc->ops = &qca6290_hal_hw_txrx_ops;
}
