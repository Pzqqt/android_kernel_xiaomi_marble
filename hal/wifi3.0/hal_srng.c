/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#include "hal_api.h"
#include "wcss_version.h"

/**
 * Common SRNG register access macros:
 * The SRNG registers are distributed accross various UMAC and LMAC HW blocks,
 * but the register group and format is exactly same for all rings, with some
 * difference between producer rings (these are 'producer rings' with respect
 * to HW and refered as 'destination rings' in SW) and consumer rings (these
 * are 'consumer rings' with respect to HW and refered as 'source rings' in SW).
 * The following macros provide uniform access to all SRNG rings.
 */

/* SRNG registers are split among two groups R0 and R2 and following
 * definitions identify the group to which each register belongs to
 */
#define R0_INDEX 0
#define R2_INDEX 1

#define HWREG_INDEX(_reg_group) _reg_group ## _ ## INDEX

/* Registers in R0 group */
#define BASE_LSB_GROUP R0
#define BASE_MSB_GROUP R0
#define ID_GROUP R0
#define STATUS_GROUP R0
#define MISC_GROUP R0
#define HP_ADDR_LSB_GROUP R0
#define HP_ADDR_MSB_GROUP R0
#define PRODUCER_INT_SETUP_GROUP R0
#define PRODUCER_INT_STATUS_GROUP R0
#define PRODUCER_FULL_COUNTER_GROUP R0
#define MSI1_BASE_LSB_GROUP R0
#define MSI1_BASE_MSB_GROUP R0
#define MSI1_DATA_GROUP R0
#define HP_TP_SW_OFFSET_GROUP R0
#define TP_ADDR_LSB_GROUP R0
#define TP_ADDR_MSB_GROUP R0
#define CONSUMER_INT_SETUP_IX0_GROUP R0
#define CONSUMER_INT_SETUP_IX1_GROUP R0
#define CONSUMER_INT_STATUS_GROUP R0
#define CONSUMER_EMPTY_COUNTER_GROUP R0
#define CONSUMER_PREFETCH_TIMER_GROUP R0
#define CONSUMER_PREFETCH_STATUS_GROUP R0

/* Registers in R2 group */
#define HP_GROUP R2
#define TP_GROUP R2

/**
 * Register definitions for all SRNG based rings are same, except few
 * differences between source (HW consumer) and destination (HW producer)
 * registers. Following macros definitions provide generic access to all
 * SRNG based rings.
 * For source rings, we will use the register/field definitions of SW2TCL1
 * ring defined in the HW header file mac_tcl_reg_seq_hwioreg.h. To setup
 * individual fields, SRNG_SM macros should be used with fields specified
 * using SRNG_SRC_FLD(<register>, <field>), Register writes should be done
 * using SRNG_SRC_REG_WRITE(<hal_srng>, <register>, <value>).
 * Similarly for destination rings we will use definitions of REO2SW1 ring
 * defined in the register reo_destination_ring.h. To setup individual
 * fields SRNG_SM macros should be used with fields specified using
 * SRNG_DST_FLD(<register>, <field>). Register writes should be done using
 * SRNG_DST_REG_WRITE(<hal_srng>, <register>, <value>).
 */

#define SRNG_DST_REG_OFFSET(_reg, _reg_group) \
	HWIO_REO_ ## _reg_group ## _REO2SW1_RING_ ## _reg##_ADDR(0)

#define SRNG_SRC_REG_OFFSET(_reg, _reg_group) \
	HWIO_TCL_ ## _reg_group ## _SW2TCL1_RING_ ## _reg ## _ADDR(0)

#define _SRNG_DST_FLD(_reg_group, _reg_fld) \
	HWIO_REO_ ## _reg_group ## _REO2SW1_RING_ ## _reg_fld
#define _SRNG_SRC_FLD(_reg_group, _reg_fld) \
	HWIO_TCL_ ## _reg_group ## _SW2TCL1_RING_ ## _reg_fld

#define _SRNG_FLD(_reg_group, _reg_fld, _dir) \
	_SRNG_ ## _dir ## _FLD(_reg_group, _reg_fld)

#define SRNG_DST_FLD(_reg, _f) _SRNG_FLD(_reg ## _GROUP, _reg ## _ ## _f, DST)
#define SRNG_SRC_FLD(_reg, _f) _SRNG_FLD(_reg ## _GROUP, _reg ## _ ## _f, SRC)

#define SRNG_SRC_R0_START_OFFSET SRNG_SRC_REG_OFFSET(BASE_LSB, R0)
#define SRNG_DST_R0_START_OFFSET SRNG_DST_REG_OFFSET(BASE_LSB, R0)

#define SRNG_SRC_R2_START_OFFSET SRNG_SRC_REG_OFFSET(HP, R2)
#define SRNG_DST_R2_START_OFFSET SRNG_DST_REG_OFFSET(HP, R2)

#define SRNG_SRC_START_OFFSET(_reg_group) \
	SRNG_SRC_ ## _reg_group ## _START_OFFSET
#define SRNG_DST_START_OFFSET(_reg_group) \
	SRNG_DST_ ## _reg_group ## _START_OFFSET

#define SRNG_REG_ADDR(_srng, _reg, _reg_group, _dir) \
	((_srng)->hwreg_base[HWREG_INDEX(_reg_group)] + \
		SRNG_ ## _dir ## _REG_OFFSET(_reg, _reg_group) - \
		SRNG_ ## _dir ## _START_OFFSET(_reg_group))

#define SRNG_DST_ADDR(_srng, _reg) \
	SRNG_REG_ADDR(_srng, _reg, _reg ## _GROUP, DST)

#define SRNG_SRC_ADDR(_srng, _reg) \
	SRNG_REG_ADDR(_srng, _reg, _reg ## _GROUP, SRC)

#define SRNG_REG_WRITE(_srng, _reg, _value, _dir) \
	hal_write_address_32_mb(_srng->hal_soc, SRNG_ ## _dir ## _ADDR(_srng, _reg), (_value))

#define SRNG_REG_READ(_srng, _reg, _dir) \
	hal_read_address_32_mb(_srng->hal_soc, SRNG_ ## _dir ## _ADDR(_srng, _reg))

#define SRNG_SRC_REG_WRITE(_srng, _reg, _value) \
	SRNG_REG_WRITE(_srng, _reg, _value, SRC)

#define SRNG_DST_REG_WRITE(_srng, _reg, _value) \
	SRNG_REG_WRITE(_srng, _reg, _value, DST)

#define SRNG_SRC_REG_READ(_srng, _reg) \
	SRNG_REG_READ(_srng, _reg, SRC)

#define _SRNG_FM(_reg_fld) _reg_fld ## _BMSK
#define _SRNG_FS(_reg_fld) _reg_fld ## _SHFT

#define SRNG_SM(_reg_fld, _val) \
	(((_val) << _SRNG_FS(_reg_fld)) & _SRNG_FM(_reg_fld))

#define SRNG_MS(_reg_fld, _val) \
	(((_val) & _SRNG_FM(_reg_fld)) >> _SRNG_FS(_reg_fld))

#define SRNG_MAX_SIZE_DWORDS \
	(SRNG_MS(SRNG_SRC_FLD(BASE_MSB, RING_SIZE), 0xffffffff))

/**
 * HW ring configuration table to identify hardware ring attributes like
 * register addresses, number of rings, ring entry size etc., for each type
 * of SRNG ring.
 *
 * Currently there is just one HW ring table, but there could be multiple
 * configurations in future based on HW variants from the same wifi3.0 family
 * and hence need to be attached with hal_soc based on HW type
 */
#define HAL_SRNG_CONFIG(_hal_soc, _ring_type) (&hw_srng_table[_ring_type])
static struct hal_hw_srng_config hw_srng_table[] = {
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
		 * type are supported */
		.reg_size = {},
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
	},
	{ /* RXDMA_BUF */
		.start_ring_id = HAL_SRNG_WMAC1_SW2RXDMA0_BUF,
		.max_rings = 2,
		/* TODO: Check if the additional IPA buffer ring needs to be
		 * setup here (in which case max_rings should be set to 2),
		 * or it will be setup by IPA host driver
		 */
		.entry_size = sizeof(struct wbm_buffer_ring) >> 2,
		.lmac_ring = TRUE,
		.ring_dir = HAL_SRNG_SRC_RING,
		/* reg_start is not set because LMAC rings are not accessed
		 * from host
		 */
		.reg_start = {},
		.reg_size = {},
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
	},
#endif
};

/**
 * hal_get_srng_ring_id() - get the ring id of a descriped ring
 * @hal: hal_soc data structure
 * @ring_type: type enum describing the ring
 * @ring_num: which ring of the ring type
 * @mac_id: which mac does the ring belong to (or 0 for non-lmac rings)
 *
 * Return: the ring id or -EINVAL if the ring does not exist.
 */
static int hal_get_srng_ring_id(struct hal_soc *hal, int ring_type,
				int ring_num, int mac_id)
{
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	int ring_id;

	if (ring_num >= ring_config->max_rings) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: ring_num exceeded maximum no. of supported rings\n",
			__func__);
		return -EINVAL;
	}

	if (ring_config->lmac_ring) {
		ring_id = ring_config->start_ring_id + ring_num +
			(mac_id * HAL_MAX_RINGS_PER_LMAC);
	} else {
		ring_id = ring_config->start_ring_id + ring_num;
	}

	return ring_id;
}

static struct hal_srng *hal_get_srng(struct hal_soc *hal, int ring_id)
{
	/* TODO: Should we allocate srng structures dynamically? */
	return &(hal->srng_list[ring_id]);
}

#define HP_OFFSET_IN_REG_START 1
#define OFFSET_FROM_HP_TO_TP 4
static void hal_update_srng_hp_tp_address(void *hal_soc,
					  int shadow_config_index,
					  int ring_type,
					  int ring_num)
{
	struct hal_srng *srng;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	int ring_id;

	ring_id = hal_get_srng_ring_id(hal_soc, ring_type, ring_num, 0);
	if (ring_id < 0)
		return;

	srng = hal_get_srng(hal_soc, ring_id);

	if (srng->ring_dir == HAL_SRNG_DST_RING)
		srng->u.dst_ring.tp_addr = SHADOW_REGISTER(shadow_config_index)
			+ hal->dev_base_addr;
	else
		srng->u.src_ring.hp_addr = SHADOW_REGISTER(shadow_config_index)
			+ hal->dev_base_addr;
}

QDF_STATUS hal_set_one_shadow_config(void *hal_soc,
				      int ring_type,
				      int ring_num)
{
	uint32_t target_register;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *srng_config = &hw_srng_table[ring_type];
	int shadow_config_index = hal->num_shadow_registers_configured;

	if (shadow_config_index >= MAX_SHADOW_REGISTERS) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_RESOURCES;
	}

	hal->num_shadow_registers_configured++;

	target_register = srng_config->reg_start[HP_OFFSET_IN_REG_START];
	target_register += (srng_config->reg_size[HP_OFFSET_IN_REG_START]
			    *ring_num);

	/* if the ring is a dst ring, we need to shadow the tail pointer */
	if (srng_config->ring_dir == HAL_SRNG_DST_RING)
		target_register += OFFSET_FROM_HP_TO_TP;

	hal->shadow_config[shadow_config_index].addr = target_register;

	/* update hp/tp addr in the hal_soc structure*/
	hal_update_srng_hp_tp_address(hal_soc, shadow_config_index, ring_type,
				      ring_num);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
			"%s: target_reg %x, shadow_index %x, ring_type %d, ring num %d\n",
		       __func__, target_register, shadow_config_index,
		       ring_type, ring_num);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS hal_construct_shadow_config(void *hal_soc)
{
	int ring_type, ring_num;

	for (ring_type = 0; ring_type < MAX_RING_TYPES; ring_type++) {
		struct hal_hw_srng_config *srng_config =
			&hw_srng_table[ring_type];

		if (ring_type == CE_SRC ||
		    ring_type == CE_DST ||
		    ring_type == CE_DST_STATUS)
			continue;

		if (srng_config->lmac_ring)
			continue;

		for (ring_num = 0; ring_num < srng_config->max_rings;
		     ring_num++)
			hal_set_one_shadow_config(hal_soc, ring_type, ring_num);
	}

	return QDF_STATUS_SUCCESS;
}

void hal_get_shadow_config(void *hal_soc,
	struct pld_shadow_reg_v2_cfg **shadow_config,
	int *num_shadow_registers_configured)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	*shadow_config = hal->shadow_config;
	*num_shadow_registers_configured =
		hal->num_shadow_registers_configured;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s\n", __func__);
}


static void hal_validate_shadow_register(struct hal_soc *hal,
				  uint32_t *destination,
				  uint32_t *shadow_address)
{
	unsigned int index;
	uint32_t *shadow_0_offset = SHADOW_REGISTER(0) + hal->dev_base_addr;
	int destination_ba_offset =
		((char *)destination) - (char *)hal->dev_base_addr;

	index =	shadow_address - shadow_0_offset;

	if (index > MAX_SHADOW_REGISTERS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: index %x out of bounds\n", __func__, index);
		goto error;
	} else if (hal->shadow_config[index].addr != destination_ba_offset) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: sanity check failure, expected %x, found %x\n",
			__func__, destination_ba_offset,
			hal->shadow_config[index].addr);
		goto error;
	}
	return;
error:
	qdf_print("%s: baddr %p, desination %p, shadow_address %p s0offset %p index %x",
		  __func__, hal->dev_base_addr, destination, shadow_address,
		  shadow_0_offset, index);
	QDF_BUG(0);
	return;
}

static void hal_target_based_configure(struct hal_soc *hal)
{
	struct hif_target_info *tgt_info =
		hif_get_target_info_handle(hal->hif_handle);

	switch (tgt_info->target_type) {
	case TARGET_TYPE_QCA6290:
		hal->use_register_windowing = true;
	break;
	default:
	break;
	}
}

/**
 * hal_attach - Initalize HAL layer
 * @hif_handle: Opaque HIF handle
 * @qdf_dev: QDF device
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 *
 */
void *hal_attach(void *hif_handle, qdf_device_t qdf_dev)
{
	struct hal_soc *hal;
	int i;

	hal = qdf_mem_malloc(sizeof(*hal));

	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal_soc allocation failed\n", __func__);
		goto fail0;
	}
	hal->hif_handle = hif_handle;
	hal->dev_base_addr = hif_get_dev_ba(hif_handle);
	hal->qdf_dev = qdf_dev;
	hal->shadow_rdptr_mem_vaddr = (uint32_t *)qdf_mem_alloc_consistent(
		qdf_dev, qdf_dev->dev, sizeof(*(hal->shadow_rdptr_mem_vaddr)) *
		HAL_SRNG_ID_MAX, &(hal->shadow_rdptr_mem_paddr));
	if (!hal->shadow_rdptr_mem_paddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal->shadow_rdptr_mem_paddr allocation failed\n",
			__func__);
		goto fail1;
	}

	hal->shadow_wrptr_mem_vaddr =
		(uint32_t *)qdf_mem_alloc_consistent(qdf_dev, qdf_dev->dev,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS,
		&(hal->shadow_wrptr_mem_paddr));
	if (!hal->shadow_wrptr_mem_vaddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal->shadow_wrptr_mem_vaddr allocation failed\n",
			__func__);
		goto fail2;
	}

	for (i = 0; i < HAL_SRNG_ID_MAX; i++) {
		hal->srng_list[i].initialized = 0;
		hal->srng_list[i].ring_id = i;
	}

	qdf_spinlock_create(&hal->register_access_lock);
	hal->register_window = 0;

	hal_target_based_configure(hal);

	return (void *)hal;

fail2:
	qdf_mem_free_consistent(qdf_dev, qdf_dev->dev,
		sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX,
		hal->shadow_rdptr_mem_vaddr, hal->shadow_rdptr_mem_paddr, 0);
fail1:
	qdf_mem_free(hal);
fail0:
	return NULL;
}

/**
 * hal_mem_info - Retreive hal memory base address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @mem: pointer to structure to be updated with hal mem info
 */
void hal_get_meminfo(void *hal_soc, struct hal_mem_info *mem )
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	mem->dev_base_addr = (void *)hal->dev_base_addr;
        mem->shadow_rdptr_mem_vaddr = (void *)hal->shadow_rdptr_mem_vaddr;
	mem->shadow_wrptr_mem_vaddr = (void *)hal->shadow_wrptr_mem_vaddr;
        mem->shadow_rdptr_mem_paddr = (void *)hal->shadow_rdptr_mem_paddr;
	mem->shadow_wrptr_mem_paddr = (void *)hal->shadow_wrptr_mem_paddr;
	hif_read_phy_mem_base(hal->hif_handle, (qdf_dma_addr_t *)&mem->dev_base_paddr);
	return;
}

/**
 * hal_detach - Detach HAL layer
 * @hal_soc: HAL SOC handle
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 *
 */
extern void hal_detach(void *hal_soc)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	qdf_mem_free_consistent(hal->qdf_dev, hal->qdf_dev->dev,
		sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX,
		hal->shadow_rdptr_mem_vaddr, hal->shadow_rdptr_mem_paddr, 0);
	qdf_mem_free_consistent(hal->qdf_dev, hal->qdf_dev->dev,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS,
		hal->shadow_wrptr_mem_vaddr, hal->shadow_wrptr_mem_paddr, 0);
	qdf_mem_free(hal);

	return;
}



/**
 * hal_srng_src_hw_init - Private function to initialize SRNG
 * source ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_src_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	uint32_t reg_val = 0;
	uint64_t tp_addr = 0;

	HIF_INFO("%s: hw_init srng %d", __func__, srng->ring_id);

	if (srng->flags & HAL_SRNG_MSI_INTR) {
		SRNG_SRC_REG_WRITE(srng, MSI1_BASE_LSB,
			srng->msi_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_SRC_FLD(MSI1_BASE_MSB, ADDR),
			(uint64_t)(srng->msi_addr) >> 32) |
			SRNG_SM(SRNG_SRC_FLD(MSI1_BASE_MSB,
			MSI1_ENABLE), 1);
		SRNG_SRC_REG_WRITE(srng, MSI1_BASE_MSB, reg_val);
		SRNG_SRC_REG_WRITE(srng, MSI1_DATA, srng->msi_data);
	}

	HIF_INFO("%s: hw_init srng (msi_end) %d", __func__, srng->ring_id);


	SRNG_SRC_REG_WRITE(srng, BASE_LSB, srng->ring_base_paddr & 0xffffffff);
	reg_val = SRNG_SM(SRNG_SRC_FLD(BASE_MSB, RING_BASE_ADDR_MSB),
		((uint64_t)(srng->ring_base_paddr) >> 32)) |
		SRNG_SM(SRNG_SRC_FLD(BASE_MSB, RING_SIZE),
		srng->entry_size * srng->num_entries);
	SRNG_SRC_REG_WRITE(srng, BASE_MSB, reg_val);

#if defined(WCSS_VERSION) && \
	((defined(CONFIG_WIN) && (WCSS_VERSION > 81)) || \
	 (defined(CONFIG_MCL) && (WCSS_VERSION >= 72)))
	reg_val = SRNG_SM(SRNG_SRC_FLD(ID, ENTRY_SIZE), srng->entry_size);
#else
	reg_val = SRNG_SM(SRNG_SRC_FLD(ID, RING_ID), srng->ring_id) |
		SRNG_SM(SRNG_SRC_FLD(ID, ENTRY_SIZE), srng->entry_size);
#endif
	SRNG_SRC_REG_WRITE(srng, ID, reg_val);

	/**
	 * Interrupt setup:
	 * Default interrupt mode is 'pulse'. Need to setup SW_INTERRUPT_MODE
	 * if level mode is required
	 */
	reg_val = 0;
	if (srng->intr_timer_thres_us) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX0,
			INTERRUPT_TIMER_THRESHOLD),
			srng->intr_timer_thres_us >> 3);
	}

	if (srng->intr_batch_cntr_thres_entries) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX0,
			BATCH_COUNTER_THRESHOLD),
			srng->intr_batch_cntr_thres_entries *
			srng->entry_size);
	}
	SRNG_SRC_REG_WRITE(srng, CONSUMER_INT_SETUP_IX0, reg_val);

	if (srng->flags & HAL_SRNG_LOW_THRES_INTR_ENABLE) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX1,
			LOW_THRESHOLD), srng->u.src_ring.low_threshold);
	}

	SRNG_SRC_REG_WRITE(srng, CONSUMER_INT_SETUP_IX1, reg_val);

	/* As per HW team, TP_ADDR and HP_ADDR for Idle link ring should
	 * remain 0 to avoid some WBM stability issues. Remote head/tail
	 * pointers are not required since this ring is completly managed
	 * by WBM HW */
	if (srng->ring_id != HAL_SRNG_WBM_IDLE_LINK) {
		tp_addr = (uint64_t)(hal->shadow_rdptr_mem_paddr +
			((unsigned long)(srng->u.src_ring.tp_addr) -
			(unsigned long)(hal->shadow_rdptr_mem_vaddr)));
		SRNG_SRC_REG_WRITE(srng, TP_ADDR_LSB, tp_addr & 0xffffffff);
		SRNG_SRC_REG_WRITE(srng, TP_ADDR_MSB, tp_addr >> 32);
	}

	/* Initilaize head and tail pointers to indicate ring is empty */
	SRNG_SRC_REG_WRITE(srng, HP, 0);
	SRNG_SRC_REG_WRITE(srng, TP, 0);
	*(srng->u.src_ring.tp_addr) = 0;

	reg_val = ((srng->flags & HAL_SRNG_DATA_TLV_SWAP) ?
			SRNG_SM(SRNG_SRC_FLD(MISC, DATA_TLV_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_RING_PTR_SWAP) ?
			SRNG_SM(SRNG_SRC_FLD(MISC, HOST_FW_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_MSI_SWAP) ?
			SRNG_SM(SRNG_SRC_FLD(MISC, MSI_SWAP_BIT), 1) : 0);

	/* Loop count is not used for SRC rings */
	reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, LOOPCNT_DISABLE), 1);

	/*
	 * reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, SRNG_ENABLE), 1);
	 * todo: update fw_api and replace with above line
	 * (when SRNG_ENABLE field for the MISC register is available in fw_api)
	 * (WCSS_UMAC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_MISC)
	 */
	reg_val |= 0x40;

	SRNG_SRC_REG_WRITE(srng, MISC, reg_val);

}

/**
 * hal_ce_dst_setup - Initialize CE destination ring registers
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_ce_dst_setup(struct hal_soc *hal, struct hal_srng *srng,
				    int ring_num)
{
	uint32_t reg_val = 0;
	uint32_t reg_addr;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, CE_DST);

	/* set DEST_MAX_LENGTH according to ce assignment */
	reg_addr = HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_ADDR(
			ring_config->reg_start[R0_INDEX] +
			(ring_num * ring_config->reg_size[R0_INDEX]));

	reg_val = HAL_REG_READ(hal, reg_addr);
	reg_val &= ~HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_DEST_MAX_LENGTH_BMSK;
	reg_val |= srng->u.dst_ring.max_buffer_length &
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_DEST_MAX_LENGTH_BMSK;
	HAL_REG_WRITE(hal, reg_addr, reg_val);
}

/**
 * hal_srng_dst_hw_init - Private function to initialize SRNG
 * destination ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_dst_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	uint32_t reg_val = 0;
	uint64_t hp_addr = 0;

	HIF_INFO("%s: hw_init srng %d", __func__, srng->ring_id);

	if (srng->flags & HAL_SRNG_MSI_INTR) {
		SRNG_DST_REG_WRITE(srng, MSI1_BASE_LSB,
			srng->msi_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_DST_FLD(MSI1_BASE_MSB, ADDR),
			(uint64_t)(srng->msi_addr) >> 32) |
			SRNG_SM(SRNG_DST_FLD(MSI1_BASE_MSB,
			MSI1_ENABLE), 1);
		SRNG_DST_REG_WRITE(srng, MSI1_BASE_MSB, reg_val);
		SRNG_DST_REG_WRITE(srng, MSI1_DATA, srng->msi_data);
	}

	HIF_INFO("%s: hw_init srng msi end %d", __func__, srng->ring_id);

	SRNG_DST_REG_WRITE(srng, BASE_LSB, srng->ring_base_paddr & 0xffffffff);
	reg_val = SRNG_SM(SRNG_DST_FLD(BASE_MSB, RING_BASE_ADDR_MSB),
		((uint64_t)(srng->ring_base_paddr) >> 32)) |
		SRNG_SM(SRNG_DST_FLD(BASE_MSB, RING_SIZE),
		srng->entry_size * srng->num_entries);
	SRNG_DST_REG_WRITE(srng, BASE_MSB, reg_val);

	reg_val = SRNG_SM(SRNG_DST_FLD(ID, RING_ID), srng->ring_id) |
		SRNG_SM(SRNG_DST_FLD(ID, ENTRY_SIZE), srng->entry_size);
	SRNG_DST_REG_WRITE(srng, ID, reg_val);


	/**
	 * Interrupt setup:
	 * Default interrupt mode is 'pulse'. Need to setup SW_INTERRUPT_MODE
	 * if level mode is required
	 */
	reg_val = 0;
	if (srng->intr_timer_thres_us) {
		reg_val |= SRNG_SM(SRNG_DST_FLD(PRODUCER_INT_SETUP,
			INTERRUPT_TIMER_THRESHOLD),
			srng->intr_timer_thres_us >> 3);
	}

	if (srng->intr_batch_cntr_thres_entries) {
		reg_val |= SRNG_SM(SRNG_DST_FLD(PRODUCER_INT_SETUP,
			BATCH_COUNTER_THRESHOLD),
			srng->intr_batch_cntr_thres_entries *
			srng->entry_size);
	}

	SRNG_DST_REG_WRITE(srng, PRODUCER_INT_SETUP, reg_val);
	hp_addr = (uint64_t)(hal->shadow_rdptr_mem_paddr +
		((unsigned long)(srng->u.dst_ring.hp_addr) -
		(unsigned long)(hal->shadow_rdptr_mem_vaddr)));
	SRNG_DST_REG_WRITE(srng, HP_ADDR_LSB, hp_addr & 0xffffffff);
	SRNG_DST_REG_WRITE(srng, HP_ADDR_MSB, hp_addr >> 32);

	/* Initilaize head and tail pointers to indicate ring is empty */
	SRNG_DST_REG_WRITE(srng, HP, 0);
	SRNG_DST_REG_WRITE(srng, TP, 0);
	*(srng->u.dst_ring.hp_addr) = 0;

	reg_val = ((srng->flags & HAL_SRNG_DATA_TLV_SWAP) ?
			SRNG_SM(SRNG_DST_FLD(MISC, DATA_TLV_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_RING_PTR_SWAP) ?
			SRNG_SM(SRNG_DST_FLD(MISC, HOST_FW_SWAP_BIT), 1) : 0) |
			((srng->flags & HAL_SRNG_MSI_SWAP) ?
			SRNG_SM(SRNG_DST_FLD(MISC, MSI_SWAP_BIT), 1) : 0);

	/*
	 * reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, SRNG_ENABLE), 1);
	 * todo: update fw_api and replace with above line
	 * (when SRNG_ENABLE field for the MISC register is available in fw_api)
	 * (WCSS_UMAC_CE_0_SRC_WFSS_CE_CHANNEL_SRC_R0_SRC_RING_MISC)
	 */
	reg_val |= 0x40;

	SRNG_DST_REG_WRITE(srng, MISC, reg_val);

}

/**
 * hal_srng_hw_init - Private function to initialize SRNG HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	if (srng->ring_dir == HAL_SRNG_SRC_RING)
		hal_srng_src_hw_init(hal, srng);
	else
		hal_srng_dst_hw_init(hal, srng);
}

#ifdef CONFIG_SHADOW_V2
#define ignore_shadow false
#define CHECK_SHADOW_REGISTERS true
#else
#define ignore_shadow true
#define CHECK_SHADOW_REGISTERS false
#endif

/**
 * hal_srng_setup - Initalize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 * @ring_num: Ring number if there are multiple rings of same type (staring
 * from 0)
 * @mac_id: valid MAC Id should be passed if ring type is one of lmac rings
 * @ring_params: SRNG ring params in hal_srng_params structure.

 * Callers are expected to allocate contiguous ring memory of size
 * 'num_entries * entry_size' bytes and pass the physical and virtual base
 * addresses through 'ring_base_paddr' and 'ring_base_vaddr' in
 * hal_srng_params structure. Ring base address should be 8 byte aligned
 * and size of each ring entry should be queried using the API
 * hal_srng_get_entrysize
 *
 * Return: Opaque pointer to ring on success
 *		 NULL on failure (if given ring is not available)
 */
void *hal_srng_setup(void *hal_soc, int ring_type, int ring_num,
	int mac_id, struct hal_srng_params *ring_params)
{
	int ring_id;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_srng *srng;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	void *dev_base_addr;
	int i;

	ring_id = hal_get_srng_ring_id(hal_soc, ring_type, ring_num, mac_id);
	if (ring_id < 0)
		return NULL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			 "%s: mac_id %d ring_id %d\n",
			 __func__, mac_id, ring_id);

	srng = hal_get_srng(hal_soc, ring_id);

	if (srng->initialized) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring (ring_type, ring_num) already initialized\n",
			__func__);
		return NULL;
	}

	dev_base_addr = hal->dev_base_addr;
	srng->ring_id = ring_id;
	srng->ring_dir = ring_config->ring_dir;
	srng->ring_base_paddr = ring_params->ring_base_paddr;
	srng->ring_base_vaddr = ring_params->ring_base_vaddr;
	srng->entry_size = ring_config->entry_size;
	srng->num_entries = ring_params->num_entries;
	srng->ring_size = srng->num_entries * srng->entry_size;
	srng->ring_size_mask = srng->ring_size - 1;
	srng->msi_addr = ring_params->msi_addr;
	srng->msi_data = ring_params->msi_data;
	srng->intr_timer_thres_us = ring_params->intr_timer_thres_us;
	srng->intr_batch_cntr_thres_entries =
		ring_params->intr_batch_cntr_thres_entries;
	srng->hal_soc = hal_soc;

	for (i = 0 ; i < MAX_SRNG_REG_GROUPS; i++) {
		srng->hwreg_base[i] = dev_base_addr + ring_config->reg_start[i]
			+ (ring_num * ring_config->reg_size[i]);
	}

	/* Zero out the entire ring memory */
	qdf_mem_zero(srng->ring_base_vaddr, (srng->entry_size *
		srng->num_entries) << 2);

	srng->flags = ring_params->flags;
#ifdef BIG_ENDIAN_HOST
		/* TODO: See if we should we get these flags from caller */
	srng->flags |= HAL_SRNG_DATA_TLV_SWAP;
	srng->flags |= HAL_SRNG_MSI_SWAP;
	srng->flags |= HAL_SRNG_RING_PTR_SWAP;
#endif

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		srng->u.src_ring.hp = 0;
		srng->u.src_ring.reap_hp = srng->ring_size -
			srng->entry_size;
		srng->u.src_ring.tp_addr =
			&(hal->shadow_rdptr_mem_vaddr[ring_id]);
		srng->u.src_ring.low_threshold = ring_params->low_threshold;
		if (ring_config->lmac_ring) {
			/* For LMAC rings, head pointer updates will be done
			 * through FW by writing to a shared memory location
			 */
			srng->u.src_ring.hp_addr =
				&(hal->shadow_wrptr_mem_vaddr[ring_id -
					HAL_SRNG_LMAC1_ID_START]);
			srng->flags |= HAL_SRNG_LMAC_RING;
		} else if (ignore_shadow || (srng->u.src_ring.hp_addr == 0)) {
			srng->u.src_ring.hp_addr = SRNG_SRC_ADDR(srng, HP);

			if (CHECK_SHADOW_REGISTERS) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: Ring (%d, %d) missing shadow config\n",
					  __func__, ring_type, ring_num);
			}
		} else {
			hal_validate_shadow_register(hal,
						     SRNG_SRC_ADDR(srng, HP),
						     srng->u.src_ring.hp_addr);
		}
	} else {
		/* During initialization loop count in all the descriptors
		 * will be set to zero, and HW will set it to 1 on completing
		 * descriptor update in first loop, and increments it by 1 on
		 * subsequent loops (loop count wraps around after reaching
		 * 0xffff). The 'loop_cnt' in SW ring state is the expected
		 * loop count in descriptors updated by HW (to be processed
		 * by SW).
		 */
		srng->u.dst_ring.loop_cnt = 1;
		srng->u.dst_ring.tp = 0;
		srng->u.dst_ring.hp_addr =
			&(hal->shadow_rdptr_mem_vaddr[ring_id]);
		if (ring_config->lmac_ring) {
			/* For LMAC rings, tail pointer updates will be done
			 * through FW by writing to a shared memory location
			 */
			srng->u.dst_ring.tp_addr =
				&(hal->shadow_wrptr_mem_vaddr[ring_id -
				HAL_SRNG_LMAC1_ID_START]);
			srng->flags |= HAL_SRNG_LMAC_RING;
		} else if (ignore_shadow || srng->u.dst_ring.tp_addr == 0) {
			srng->u.dst_ring.tp_addr = SRNG_DST_ADDR(srng, TP);

			if (CHECK_SHADOW_REGISTERS) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					  QDF_TRACE_LEVEL_ERROR,
					  "%s: Ring (%d, %d) missing shadow config\n",
					  __func__, ring_type, ring_num);
			}
		} else {
			hal_validate_shadow_register(hal,
						     SRNG_DST_ADDR(srng, TP),
						     srng->u.dst_ring.tp_addr);
		}
	}

	if (!(ring_config->lmac_ring)) {
		hal_srng_hw_init(hal, srng);

		if (ring_type == CE_DST) {
			srng->u.dst_ring.max_buffer_length = ring_params->max_buffer_length;
			hal_ce_dst_setup(hal, srng, ring_num);
		}
	}

	SRNG_LOCK_INIT(&srng->lock);

	return (void *)srng;
}

/**
 * hal_srng_cleanup - Deinitialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @hal_srng: Opaque HAL SRNG pointer
 */
void hal_srng_cleanup(void *hal_soc, void *hal_srng)
{
	struct hal_srng *srng = (struct hal_srng *)hal_srng;
	SRNG_LOCK_DESTROY(&srng->lock);
	srng->initialized = 0;
}

/**
 * hal_srng_get_entrysize - Returns size of ring entry in bytes
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 */
uint32_t hal_srng_get_entrysize(void *hal_soc, int ring_type)
{
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	return ring_config->entry_size << 2;
}

/**
 * hal_srng_max_entries - Returns maximum possible number of ring entries
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 * Return: Maximum number of entries for the given ring_type
 */
uint32_t hal_srng_max_entries(void *hal_soc, int ring_type)
{
	struct hal_hw_srng_config *ring_config = HAL_SRNG_CONFIG(hal, ring_type);
	return SRNG_MAX_SIZE_DWORDS / ring_config->entry_size;
}

/**
 * hal_get_srng_params - Retreive SRNG parameters for a given ring from HAL
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 * @ring_params: SRNG parameters will be returned through this structure
 */
extern void hal_get_srng_params(void *hal_soc, void *hal_ring,
	struct hal_srng_params *ring_params)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	int i =0;
	ring_params->ring_id = srng->ring_id;
	ring_params->ring_dir = srng->ring_dir;
	ring_params->entry_size = srng->entry_size;

	ring_params->ring_base_paddr = srng->ring_base_paddr;
	ring_params->ring_base_vaddr = srng->ring_base_vaddr;
	ring_params->num_entries = srng->num_entries;
	ring_params->msi_addr = srng->msi_addr;
	ring_params->msi_data = srng->msi_data;
	ring_params->intr_timer_thres_us = srng->intr_timer_thres_us;
	ring_params->intr_batch_cntr_thres_entries =
		srng->intr_batch_cntr_thres_entries;
	ring_params->low_threshold = srng->u.src_ring.low_threshold;
	ring_params->flags = srng->flags;
	ring_params->ring_id = srng->ring_id;
	for (i = 0 ; i < MAX_SRNG_REG_GROUPS; i++)
		ring_params->hwreg_base[i] = srng->hwreg_base[i];
}
