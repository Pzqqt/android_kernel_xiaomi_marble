/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#include "hal_api.h"
#include "qdf_module.h"

/* TODO: See if the following definition is available in HW headers */
#define HAL_REO_OWNED 4
#define HAL_REO_QUEUE_DESC 8
#define HAL_REO_QUEUE_EXT_DESC 9

/* TODO: Using associated link desc counter 1 for Rx. Check with FW on
 * how these counters are assigned
 */
#define HAL_RX_LINK_DESC_CNTR 1
/* TODO: Following definition should be from HW headers */
#define HAL_DESC_REO_OWNED 4

/* TODO: Move this to common header file */
static inline void hal_uniform_desc_hdr_setup(uint32_t *desc, uint32_t owner,
	uint32_t buffer_type)
{
	HAL_DESC_SET_FIELD(desc, UNIFORM_DESCRIPTOR_HEADER_0, OWNER,
		owner);
	HAL_DESC_SET_FIELD(desc, UNIFORM_DESCRIPTOR_HEADER_0, BUFFER_TYPE,
		buffer_type);
}

#ifndef TID_TO_WME_AC
#define WME_AC_BE 0 /* best effort */
#define WME_AC_BK 1 /* background */
#define WME_AC_VI 2 /* video */
#define WME_AC_VO 3 /* voice */

#define TID_TO_WME_AC(_tid) ( \
	(((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE : \
	(((_tid) == 1) || ((_tid) == 2)) ? WME_AC_BK : \
	(((_tid) == 4) || ((_tid) == 5)) ? WME_AC_VI : \
	WME_AC_VO)
#endif
#define HAL_NON_QOS_TID 16

/**
 * hal_reo_qdesc_setup - Setup HW REO queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 * @hw_qdesc_vaddr: Virtual address of REO queue descriptor memory
 * @hw_qdesc_paddr: Physical address of REO queue descriptor memory
 * @tid: TID
 *
 */
void hal_reo_qdesc_setup(void *hal_soc, int tid, uint32_t ba_window_size,
	uint32_t start_seq, void *hw_qdesc_vaddr, qdf_dma_addr_t hw_qdesc_paddr,
	int pn_type)
{
	uint32_t *reo_queue_desc = (uint32_t *)hw_qdesc_vaddr;
	uint32_t *reo_queue_ext_desc;
	uint32_t reg_val;
	uint32_t pn_enable;
	uint32_t pn_size = 0;

	qdf_mem_zero(hw_qdesc_vaddr, sizeof(struct rx_reo_queue));

	hal_uniform_desc_hdr_setup(reo_queue_desc, HAL_DESC_REO_OWNED,
		HAL_REO_QUEUE_DESC);
	/* Fixed pattern in reserved bits for debugging */
	HAL_DESC_SET_FIELD(reo_queue_desc, UNIFORM_DESCRIPTOR_HEADER_0,
		RESERVED_0A, 0xDDBEEF);

	/* This a just a SW meta data and will be copied to REO destination
	 * descriptors indicated by hardware.
	 * TODO: Setting TID in this field. See if we should set something else.
	 */
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_1,
		RECEIVE_QUEUE_NUMBER, tid);
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
		VLD, 1);
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
		ASSOCIATED_LINK_DESCRIPTOR_COUNTER, HAL_RX_LINK_DESC_CNTR);

	/*
	 * Fields DISABLE_DUPLICATE_DETECTION and SOFT_REORDER_ENABLE will be 0
	 */

	reg_val = TID_TO_WME_AC(tid);
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, AC, reg_val);

	if (ba_window_size < 1)
		ba_window_size = 1;

	/* Set RTY bit for non-BA case. Duplicate detection is currently not
	 * done by HW in non-BA case if RTY bit is not set.
	 * TODO: This is a temporary War and should be removed once HW fix is
	 * made to check and discard duplicates even if RTY bit is not set.
	 */
	if (ba_window_size == 1)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, RTY, 1);

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, BA_WINDOW_SIZE,
		ba_window_size - 1);

	switch (pn_type) {
	case HAL_PN_WPA:
		pn_enable = 1;
		pn_size = PN_SIZE_48;
	case HAL_PN_WAPI_EVEN:
	case HAL_PN_WAPI_UNEVEN:
		pn_enable = 1;
		pn_size = PN_SIZE_128;
	default:
		pn_enable = 0;
	}

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, PN_CHECK_NEEDED,
		pn_enable);

	if (pn_type == HAL_PN_WAPI_EVEN)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
			PN_SHALL_BE_EVEN, 1);
	else if (pn_type == HAL_PN_WAPI_UNEVEN)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
			PN_SHALL_BE_UNEVEN, 1);

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, PN_HANDLING_ENABLE,
		pn_enable);

	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2, PN_SIZE,
		pn_size);

	/* TODO: Check if RX_REO_QUEUE_2_IGNORE_AMPDU_FLAG need to be set
	 * based on BA window size and/or AMPDU capabilities
	 */
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_2,
		IGNORE_AMPDU_FLAG, 1);

	if (start_seq <= 0xfff)
		HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_3, SSN,
			start_seq);

	/* TODO: SVLD should be set to 1 if a valid SSN is received in ADDBA,
	 * but REO is not delivering packets if we set it to 1. Need to enable
	 * this once the issue is resolved */
	HAL_DESC_SET_FIELD(reo_queue_desc, RX_REO_QUEUE_3, SVLD, 0);

	/* TODO: Check if we should set start PN for WAPI */

#ifdef notyet
	/* Setup first queue extension if BA window size is more than 1 */
	if (ba_window_size > 1) {
		reo_queue_ext_desc =
			(uint32_t *)(((struct rx_reo_queue *)reo_queue_desc) +
			1);
		qdf_mem_zero(reo_queue_ext_desc,
			sizeof(struct rx_reo_queue_ext));
		hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
			HAL_DESC_REO_OWNED, HAL_REO_QUEUE_EXT_DESC);
	}
	/* Setup second queue extension if BA window size is more than 105 */
	if (ba_window_size > 105) {
		reo_queue_ext_desc = (uint32_t *)
			(((struct rx_reo_queue_ext *)reo_queue_ext_desc) + 1);
		qdf_mem_zero(reo_queue_ext_desc,
			sizeof(struct rx_reo_queue_ext));
		hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
			HAL_DESC_REO_OWNED, HAL_REO_QUEUE_EXT_DESC);
	}
	/* Setup third queue extension if BA window size is more than 210 */
	if (ba_window_size > 210) {
		reo_queue_ext_desc = (uint32_t *)
			(((struct rx_reo_queue_ext *)reo_queue_ext_desc) + 1);
		qdf_mem_zero(reo_queue_ext_desc,
			sizeof(struct rx_reo_queue_ext));
		hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
			HAL_DESC_REO_OWNED, HAL_REO_QUEUE_EXT_DESC);
	}
#else
	/* TODO: HW queue descriptors are currently allocated for max BA
	 * window size for all QOS TIDs so that same descriptor can be used
	 * later when ADDBA request is recevied. This should be changed to
	 * allocate HW queue descriptors based on BA window size being
	 * negotiated (0 for non BA cases), and reallocate when BA window
	 * size changes and also send WMI message to FW to change the REO
	 * queue descriptor in Rx peer entry as part of dp_rx_tid_update.
	 */
	if (tid != HAL_NON_QOS_TID) {
		reo_queue_ext_desc = (uint32_t *)
			(((struct rx_reo_queue *)reo_queue_desc) + 1);
		qdf_mem_zero(reo_queue_ext_desc, 3 *
			sizeof(struct rx_reo_queue_ext));
		/* Initialize first reo queue extension descriptor */
		hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
			HAL_DESC_REO_OWNED, HAL_REO_QUEUE_EXT_DESC);
		/* Fixed pattern in reserved bits for debugging */
		HAL_DESC_SET_FIELD(reo_queue_ext_desc,
			UNIFORM_DESCRIPTOR_HEADER_0, RESERVED_0A, 0xADBEEF);
		/* Initialize second reo queue extension descriptor */
		reo_queue_ext_desc = (uint32_t *)
			(((struct rx_reo_queue_ext *)reo_queue_ext_desc) + 1);
		hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
			HAL_DESC_REO_OWNED, HAL_REO_QUEUE_EXT_DESC);
		/* Fixed pattern in reserved bits for debugging */
		HAL_DESC_SET_FIELD(reo_queue_ext_desc,
			UNIFORM_DESCRIPTOR_HEADER_0, RESERVED_0A, 0xBDBEEF);
		/* Initialize third reo queue extension descriptor */
		reo_queue_ext_desc = (uint32_t *)
			(((struct rx_reo_queue_ext *)reo_queue_ext_desc) + 1);
		hal_uniform_desc_hdr_setup(reo_queue_ext_desc,
			HAL_DESC_REO_OWNED, HAL_REO_QUEUE_EXT_DESC);
		/* Fixed pattern in reserved bits for debugging */
		HAL_DESC_SET_FIELD(reo_queue_ext_desc,
			UNIFORM_DESCRIPTOR_HEADER_0, RESERVED_0A, 0xCDBEEF);
	}
#endif
}
qdf_export_symbol(hal_reo_qdesc_setup);


/**
 * hal_reo_setup - Initialize HW REO block
 *
 * @hal_soc: Opaque HAL SOC handle
 * @reo_params: parameters needed by HAL for REO config
 */
void hal_reo_setup(void *hal_soc,
	 struct hal_reo_params *reo_params)
{
	struct hal_soc *soc = (struct hal_soc *)hal_soc;

	HAL_REG_WRITE(soc, HWIO_REO_R0_GENERAL_ENABLE_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		HAL_SM(HWIO_REO_R0_GENERAL_ENABLE,
		FRAGMENT_DEST_RING, reo_params->frag_dst_ring) |
		HAL_SM(HWIO_REO_R0_GENERAL_ENABLE, AGING_LIST_ENABLE, 1) |
		HAL_SM(HWIO_REO_R0_GENERAL_ENABLE, AGING_FLUSH_ENABLE, 1));
	/* Other ring enable bits and REO_ENABLE will be set by FW */

	/* TODO: Setup destination ring mapping if enabled */

	/* TODO: Error destination ring setting is left to default.
	 * Default setting is to send all errors to release ring.
	 */

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_0_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		HAL_DEFAULT_REO_TIMEOUT_MS * 1000);

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_1_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		(HAL_DEFAULT_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_2_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		(HAL_DEFAULT_REO_TIMEOUT_MS * 1000));

	HAL_REG_WRITE(soc,
		HWIO_REO_R0_AGING_THRESHOLD_IX_3_ADDR(
		SEQ_WCSS_UMAC_REO_REG_OFFSET),
		(HAL_DEFAULT_REO_TIMEOUT_MS * 1000));

	/*
	 * When hash based routing is enabled, routing of the rx packet
	 * is done based on the following value: 1 _ _ _ _ The last 4
	 * bits are based on hash[3:0]. This means the possible values
	 * are 0x10 to 0x1f. This value is used to look-up the
	 * ring ID configured in Destination_Ring_Ctrl_IX_* register.
	 * The Destination_Ring_Ctrl_IX_2 and Destination_Ring_Ctrl_IX_3
	 * registers need to be configured to set-up the 16 entries to
	 * map the hash values to a ring number. There are 3 bits per
	 * hash entry  which are mapped as follows:
	 * 0: TCL, 1:SW1, 2:SW2, * 3:SW3, 4:SW4, 5:Release, 6:FW(WIFI),
	 * 7: NOT_USED.
	*/
	if (reo_params->rx_hash_enabled) {
		HAL_REG_WRITE(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET),
			reo_params->remap1);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR 0x%x\n"),
			HAL_REG_READ(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET)));

		HAL_REG_WRITE(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET),
			reo_params->remap2);

		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR 0x%x\n"),
			HAL_REG_READ(soc,
			HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
			SEQ_WCSS_UMAC_REO_REG_OFFSET)));
	}


	/* TODO: Check if the following registers shoould be setup by host:
	 * AGING_CONTROL
	 * HIGH_MEMORY_THRESHOLD
	 * GLOBAL_LINK_DESC_COUNT_THRESH_IX_0[1,2]
	 * GLOBAL_LINK_DESC_COUNT_CTRL
	 */
}
qdf_export_symbol(hal_reo_setup);

