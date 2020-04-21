/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_HW_INTERNAL_H_
#define _HAL_HW_INTERNAL_H_
#include "qdf_types.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "rx_msdu_link.h"
#include "rx_reo_queue.h"
#include "rx_reo_queue_ext.h"
#include "wcss_seq_hwiobase.h"
#include "tlv_hdr.h"
#include "tlv_tag_def.h"
#include "reo_destination_ring.h"
#include "reo_reg_seq_hwioreg.h"
#include "reo_entrance_ring.h"
#include "reo_get_queue_stats.h"
#include "reo_get_queue_stats_status.h"
#include "tcl_data_cmd.h"
#include "tcl_gse_cmd.h"
#include "tcl_status_ring.h"
#include "mac_tcl_reg_seq_hwioreg.h"
#include "ce_src_desc.h"
#include "ce_stat_desc.h"
#ifdef QCA_WIFI_QCA6490
#include "wfss_ce_channel_dst_reg_seq_hwioreg.h"
#include "wfss_ce_channel_src_reg_seq_hwioreg.h"
#else
#include "wfss_ce_reg_seq_hwioreg.h"
#endif /* QCA_WIFI_QCA6490 */
#include "wbm_link_descriptor_ring.h"
#include "wbm_reg_seq_hwioreg.h"
#include "wbm_buffer_ring.h"
#include "wbm_release_ring.h"
#include "rx_msdu_desc_info.h"
#include "rx_mpdu_start.h"
#include "rx_mpdu_end.h"
#include "rx_msdu_start.h"
#include "rx_msdu_end.h"
#include "rx_attention.h"
#include "rx_ppdu_start.h"
#include "rx_ppdu_start_user_info.h"
#include "rx_ppdu_end_user_stats.h"
#include "rx_ppdu_end_user_stats_ext.h"
#include "rx_mpdu_desc_info.h"
#include "rxpcu_ppdu_end_info.h"
#include "phyrx_he_sig_a_su.h"
#include "phyrx_he_sig_a_mu_dl.h"
#if defined(QCA_WIFI_QCA6290_11AX_MU_UL) && defined(QCA_WIFI_QCA6290_11AX)
#include "phyrx_he_sig_a_mu_ul.h"
#endif
#include "phyrx_he_sig_b1_mu.h"
#include "phyrx_he_sig_b2_mu.h"
#include "phyrx_he_sig_b2_ofdma.h"
#include "phyrx_l_sig_a.h"
#include "phyrx_l_sig_b.h"
#include "phyrx_vht_sig_a.h"
#include "phyrx_ht_sig.h"
#include "tx_msdu_extension.h"
#include "receive_rssi_info.h"
#include "phyrx_pkt_end.h"
#include "phyrx_rssi_legacy.h"
#include "wcss_version.h"
#include "rx_msdu_link.h"
#include "hal_internal.h"

#define HAL_SRNG_REO_EXCEPTION HAL_SRNG_REO2SW1
#define HAL_SRNG_REO_ALTERNATE_SELECT 0x7
#define HAL_NON_QOS_TID 16

/* TODO: Check if the following can be provided directly by HW headers */
#define SRNG_LOOP_CNT_MASK REO_DESTINATION_RING_15_LOOPING_COUNT_MASK
#define SRNG_LOOP_CNT_LSB REO_DESTINATION_RING_15_LOOPING_COUNT_LSB

/* HAL Macro to get the buffer info size */
#define HAL_RX_BUFFINFO_NUM_DWORDS NUM_OF_DWORDS_BUFFER_ADDR_INFO

#define HAL_DEFAULT_BE_BK_VI_REO_TIMEOUT_MS 100 /* milliseconds */
#define HAL_DEFAULT_VO_REO_TIMEOUT_MS 40 /* milliseconds */

#define HAL_DESC_SET_FIELD(_desc, _word, _fld, _value) do { \
	((uint32_t *)(_desc))[(_word ## _ ## _fld ## _OFFSET) >> 2] &= \
		~(_word ## _ ## _fld ## _MASK); \
	((uint32_t *)(_desc))[(_word ## _ ## _fld ## _OFFSET) >> 2] |= \
		((_value) << _word ## _ ## _fld ## _LSB); \
} while (0)

#define HAL_SM(_reg, _fld, _val) \
	(((_val) << (_reg ## _ ## _fld ## _SHFT)) & \
		(_reg ## _ ## _fld ## _BMSK))

#define HAL_MS(_reg, _fld, _val) \
	(((_val) & (_reg ## _ ## _fld ## _BMSK)) >> \
		(_reg ## _ ## _fld ## _SHFT))

#define HAL_REG_WRITE(_soc, _reg, _value) \
	hal_write32_mb(_soc, (_reg), (_value))

/* Check register writing result */
#define HAL_REG_WRITE_CONFIRM(_soc, _reg, _value) \
	hal_write32_mb_confirm(_soc, (_reg), (_value))

#define HAL_REG_READ(_soc, _offset) \
	hal_read32_mb(_soc, (_offset))

#define WBM_IDLE_DESC_LIST 1

/**
 * Common SRNG register access macros:
 * The SRNG registers are distributed across various UMAC and LMAC HW blocks,
 * but the register group and format is exactly same for all rings, with some
 * difference between producer rings (these are 'producer rings' with respect
 * to HW and referred as 'destination rings' in SW) and consumer rings (these
 * are 'consumer rings' with respect to HW and
 * referred as 'source rings' in SW).
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
	((_srng)->hal_soc->hal_hw_reg_offset[_dir ## _ ##_reg]))

#define CALCULATE_REG_OFFSET(_dir, _reg, _reg_group) \
		(SRNG_ ## _dir ## _REG_OFFSET(_reg, _reg_group) - \
		SRNG_ ## _dir ## _START_OFFSET(_reg_group))

#define REG_OFFSET(_dir, _reg) \
		CALCULATE_REG_OFFSET(_dir, _reg, _reg ## _GROUP)

#define SRNG_DST_ADDR(_srng, _reg) \
	SRNG_REG_ADDR(_srng, _reg, _reg ## _GROUP, DST)

#define SRNG_SRC_ADDR(_srng, _reg) \
	SRNG_REG_ADDR(_srng, _reg, _reg ## _GROUP, SRC)

#define SRNG_REG_WRITE(_srng, _reg, _value, _dir) \
	hal_write_address_32_mb(_srng->hal_soc,\
		SRNG_ ## _dir ## _ADDR(_srng, _reg), (_value), false)

#define SRNG_REG_WRITE_CONFIRM(_srng, _reg, _value, _dir) \
	hal_write_address_32_mb(_srng->hal_soc,\
		SRNG_ ## _dir ## _ADDR(_srng, _reg), (_value), true)

#define SRNG_REG_READ(_srng, _reg, _dir) \
	hal_read_address_32_mb(_srng->hal_soc, \
		SRNG_ ## _dir ## _ADDR(_srng, _reg))

#define SRNG_SRC_REG_WRITE(_srng, _reg, _value) \
	SRNG_REG_WRITE(_srng, _reg, _value, SRC)

#define SRNG_DST_REG_WRITE(_srng, _reg, _value) \
	SRNG_REG_WRITE(_srng, _reg, _value, DST)

#define SRNG_DST_REG_WRITE_CONFIRM(_srng, _reg, _value) \
	SRNG_REG_WRITE_CONFIRM(_srng, _reg, _value, DST)

#define SRNG_SRC_REG_READ(_srng, _reg) \
	SRNG_REG_READ(_srng, _reg, SRC)

#define SRNG_DST_REG_READ(_srng, _reg) \
	SRNG_REG_READ(_srng, _reg, DST)

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
#define HAL_SRNG_CONFIG(_hal_soc, _ring_type) \
			(&_hal_soc->hw_srng_table[_ring_type])

enum SRNG_REGISTERS {
DST_HP = 0,
DST_TP,
DST_ID,
DST_MISC,
DST_HP_ADDR_LSB,
DST_HP_ADDR_MSB,
DST_MSI1_BASE_LSB,
DST_MSI1_BASE_MSB,
DST_MSI1_DATA,
DST_BASE_LSB,
DST_BASE_MSB,
DST_PRODUCER_INT_SETUP,

SRC_HP,
SRC_TP,
SRC_ID,
SRC_MISC,
SRC_TP_ADDR_LSB,
SRC_TP_ADDR_MSB,
SRC_MSI1_BASE_LSB,
SRC_MSI1_BASE_MSB,
SRC_MSI1_DATA,
SRC_BASE_LSB,
SRC_BASE_MSB,
SRC_CONSUMER_INT_SETUP_IX0,
SRC_CONSUMER_INT_SETUP_IX1,
};

/**
 * hal_set_link_desc_addr - Setup link descriptor in a buffer_addr_info
 * HW structure
 *
 * @desc: Descriptor entry (from WBM_IDLE_LINK ring)
 * @cookie: SW cookie for the buffer/descriptor
 * @link_desc_paddr: Physical address of link descriptor entry
 *
 */
static inline void hal_set_link_desc_addr(void *desc, uint32_t cookie,
	qdf_dma_addr_t link_desc_paddr)
{
	uint32_t *buf_addr = (uint32_t *)desc;

	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_0, BUFFER_ADDR_31_0,
			   link_desc_paddr & 0xffffffff);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, BUFFER_ADDR_39_32,
			   (uint64_t)link_desc_paddr >> 32);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, RETURN_BUFFER_MANAGER,
			   WBM_IDLE_DESC_LIST);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, SW_BUFFER_COOKIE,
			   cookie);
}

/**
 * hal_get_reo_qdesc_size - Get size of reo queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 * @tid: TID number
 *
 */
static inline
uint32_t hal_get_reo_qdesc_size(hal_soc_handle_t hal_soc_hdl,
				uint32_t ba_window_size, int tid)
{
	/* Return descriptor size corresponding to window size of 2 since
	 * we set ba_window_size to 2 while setting up REO descriptors as
	 * a WAR to get 2k jump exception aggregates are received without
	 * a BA session.
	 */
	if (ba_window_size <= 1) {
		if (tid != HAL_NON_QOS_TID)
			return sizeof(struct rx_reo_queue) +
				sizeof(struct rx_reo_queue_ext);
		else
			return sizeof(struct rx_reo_queue);
	}

	if (ba_window_size <= 105)
		return sizeof(struct rx_reo_queue) +
			sizeof(struct rx_reo_queue_ext);

	if (ba_window_size <= 210)
		return sizeof(struct rx_reo_queue) +
			(2 * sizeof(struct rx_reo_queue_ext));

	return sizeof(struct rx_reo_queue) +
		(3 * sizeof(struct rx_reo_queue_ext));
}

#endif /* _HAL_HW_INTERNAL_H_ */
