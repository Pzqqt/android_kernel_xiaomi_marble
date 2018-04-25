/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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

#ifndef __CE_REG_H__
#define __CE_REG_H__

#define COPY_ENGINE_ID(COPY_ENGINE_BASE_ADDRESS) ((COPY_ENGINE_BASE_ADDRESS \
		- CE0_BASE_ADDRESS)/(CE1_BASE_ADDRESS - CE0_BASE_ADDRESS))

#define DST_WR_INDEX_ADDRESS    (scn->target_ce_def->d_DST_WR_INDEX_ADDRESS)
#define SRC_WATERMARK_ADDRESS   (scn->target_ce_def->d_SRC_WATERMARK_ADDRESS)
#define SRC_WATERMARK_LOW_MASK  (scn->target_ce_def->d_SRC_WATERMARK_LOW_MASK)
#define SRC_WATERMARK_HIGH_MASK (scn->target_ce_def->d_SRC_WATERMARK_HIGH_MASK)
#define DST_WATERMARK_LOW_MASK  (scn->target_ce_def->d_DST_WATERMARK_LOW_MASK)
#define DST_WATERMARK_HIGH_MASK (scn->target_ce_def->d_DST_WATERMARK_HIGH_MASK)
#define CURRENT_SRRI_ADDRESS    (scn->target_ce_def->d_CURRENT_SRRI_ADDRESS)
#define CURRENT_DRRI_ADDRESS    (scn->target_ce_def->d_CURRENT_DRRI_ADDRESS)

#define SHADOW_VALUE0    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_0)
#define SHADOW_VALUE1    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_1)
#define SHADOW_VALUE2    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_2)
#define SHADOW_VALUE3    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_3)
#define SHADOW_VALUE4    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_4)
#define SHADOW_VALUE5    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_5)
#define SHADOW_VALUE6    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_6)
#define SHADOW_VALUE7    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_7)
#define SHADOW_VALUE8    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_8)
#define SHADOW_VALUE9    (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_9)
#define SHADOW_VALUE10   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_10)
#define SHADOW_VALUE11   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_11)
#define SHADOW_VALUE12   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_12)
#define SHADOW_VALUE13   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_13)
#define SHADOW_VALUE14   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_14)
#define SHADOW_VALUE15   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_15)
#define SHADOW_VALUE16   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_16)
#define SHADOW_VALUE17   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_17)
#define SHADOW_VALUE18   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_18)
#define SHADOW_VALUE19   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_19)
#define SHADOW_VALUE20   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_20)
#define SHADOW_VALUE21   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_21)
#define SHADOW_VALUE22   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_22)
#define SHADOW_VALUE23   (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_VALUE_23)
#define SHADOW_ADDRESS0  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_0)
#define SHADOW_ADDRESS1  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_1)
#define SHADOW_ADDRESS2  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_2)
#define SHADOW_ADDRESS3  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_3)
#define SHADOW_ADDRESS4  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_4)
#define SHADOW_ADDRESS5  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_5)
#define SHADOW_ADDRESS6  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_6)
#define SHADOW_ADDRESS7  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_7)
#define SHADOW_ADDRESS8  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_8)
#define SHADOW_ADDRESS9  (scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_9)
#define SHADOW_ADDRESS10 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_10)
#define SHADOW_ADDRESS11 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_11)
#define SHADOW_ADDRESS12 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_12)
#define SHADOW_ADDRESS13 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_13)
#define SHADOW_ADDRESS14 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_14)
#define SHADOW_ADDRESS15 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_15)
#define SHADOW_ADDRESS16 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_16)
#define SHADOW_ADDRESS17 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_17)
#define SHADOW_ADDRESS18 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_18)
#define SHADOW_ADDRESS19 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_19)
#define SHADOW_ADDRESS20 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_20)
#define SHADOW_ADDRESS21 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_21)
#define SHADOW_ADDRESS22 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_22)
#define SHADOW_ADDRESS23 \
			(scn->host_shadow_regs->d_A_LOCAL_SHADOW_REG_ADDRESS_23)

#define SHADOW_ADDRESS(i) \
			(SHADOW_ADDRESS0 + i*(SHADOW_ADDRESS1-SHADOW_ADDRESS0))

#define HOST_IS_SRC_RING_HIGH_WATERMARK_MASK \
	(scn->target_ce_def->d_HOST_IS_SRC_RING_HIGH_WATERMARK_MASK)
#define HOST_IS_SRC_RING_LOW_WATERMARK_MASK \
	(scn->target_ce_def->d_HOST_IS_SRC_RING_LOW_WATERMARK_MASK)
#define HOST_IS_DST_RING_HIGH_WATERMARK_MASK \
	(scn->target_ce_def->d_HOST_IS_DST_RING_HIGH_WATERMARK_MASK)
#define HOST_IS_DST_RING_LOW_WATERMARK_MASK \
	(scn->target_ce_def->d_HOST_IS_DST_RING_LOW_WATERMARK_MASK)
#define MISC_IS_ADDRESS         (scn->target_ce_def->d_MISC_IS_ADDRESS)
#define HOST_IS_COPY_COMPLETE_MASK \
	(scn->target_ce_def->d_HOST_IS_COPY_COMPLETE_MASK)
#define CE_WRAPPER_BASE_ADDRESS (scn->target_ce_def->d_CE_WRAPPER_BASE_ADDRESS)
#define CE_WRAPPER_INTERRUPT_SUMMARY_ADDRESS \
	(scn->target_ce_def->d_CE_WRAPPER_INTERRUPT_SUMMARY_ADDRESS)
#define CE_DDR_ADDRESS_FOR_RRI_LOW \
	(scn->target_ce_def->d_CE_DDR_ADDRESS_FOR_RRI_LOW)
#define CE_DDR_ADDRESS_FOR_RRI_HIGH \
	(scn->target_ce_def->d_CE_DDR_ADDRESS_FOR_RRI_HIGH)
#define HOST_IE_COPY_COMPLETE_MASK \
	(scn->target_ce_def->d_HOST_IE_COPY_COMPLETE_MASK)
#define SR_BA_ADDRESS             (scn->target_ce_def->d_SR_BA_ADDRESS)
#define SR_BA_ADDRESS_HIGH        (scn->target_ce_def->d_SR_BA_ADDRESS_HIGH)
#define SR_SIZE_ADDRESS           (scn->target_ce_def->d_SR_SIZE_ADDRESS)
#define CE_CTRL1_ADDRESS          (scn->target_ce_def->d_CE_CTRL1_ADDRESS)
#define CE_CTRL1_DMAX_LENGTH_MASK \
	(scn->target_ce_def->d_CE_CTRL1_DMAX_LENGTH_MASK)
#define DR_BA_ADDRESS             (scn->target_ce_def->d_DR_BA_ADDRESS)
#define DR_BA_ADDRESS_HIGH        (scn->target_ce_def->d_DR_BA_ADDRESS_HIGH)
#define DR_SIZE_ADDRESS           (scn->target_ce_def->d_DR_SIZE_ADDRESS)
#define CE_CMD_REGISTER           (scn->target_ce_def->d_CE_CMD_REGISTER)
#define CE_MSI_ADDRESS            (scn->target_ce_def->d_CE_MSI_ADDRESS)
#define CE_MSI_ADDRESS_HIGH       (scn->target_ce_def->d_CE_MSI_ADDRESS_HIGH)
#define CE_MSI_DATA               (scn->target_ce_def->d_CE_MSI_DATA)
#define CE_MSI_ENABLE_BIT         (scn->target_ce_def->d_CE_MSI_ENABLE_BIT)
#define MISC_IE_ADDRESS           (scn->target_ce_def->d_MISC_IE_ADDRESS)
#define MISC_IS_AXI_ERR_MASK      (scn->target_ce_def->d_MISC_IS_AXI_ERR_MASK)
#define MISC_IS_DST_ADDR_ERR_MASK \
	(scn->target_ce_def->d_MISC_IS_DST_ADDR_ERR_MASK)
#define MISC_IS_SRC_LEN_ERR_MASK \
	(scn->target_ce_def->d_MISC_IS_SRC_LEN_ERR_MASK)
#define MISC_IS_DST_MAX_LEN_VIO_MASK \
	(scn->target_ce_def->d_MISC_IS_DST_MAX_LEN_VIO_MASK)
#define MISC_IS_DST_RING_OVERFLOW_MASK \
	(scn->target_ce_def->d_MISC_IS_DST_RING_OVERFLOW_MASK)
#define MISC_IS_SRC_RING_OVERFLOW_MASK \
	(scn->target_ce_def->d_MISC_IS_SRC_RING_OVERFLOW_MASK)
#define SRC_WATERMARK_LOW_LSB     (scn->target_ce_def->d_SRC_WATERMARK_LOW_LSB)
#define SRC_WATERMARK_HIGH_LSB    (scn->target_ce_def->d_SRC_WATERMARK_HIGH_LSB)
#define DST_WATERMARK_LOW_LSB     (scn->target_ce_def->d_DST_WATERMARK_LOW_LSB)
#define DST_WATERMARK_HIGH_LSB    (scn->target_ce_def->d_DST_WATERMARK_HIGH_LSB)
#define CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_MASK \
	(scn->target_ce_def->d_CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_MASK)
#define CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_LSB  \
	(scn->target_ce_def->d_CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_LSB)
#define CE_CTRL1_DMAX_LENGTH_LSB \
				(scn->target_ce_def->d_CE_CTRL1_DMAX_LENGTH_LSB)
#define CE_CTRL1_IDX_UPD_EN  (scn->target_ce_def->d_CE_CTRL1_IDX_UPD_EN_MASK)
#define CE_CTRL1_SRC_RING_BYTE_SWAP_EN_MASK \
	(scn->target_ce_def->d_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_MASK)
#define CE_CTRL1_DST_RING_BYTE_SWAP_EN_MASK \
	(scn->target_ce_def->d_CE_CTRL1_DST_RING_BYTE_SWAP_EN_MASK)
#define CE_CTRL1_SRC_RING_BYTE_SWAP_EN_LSB \
	(scn->target_ce_def->d_CE_CTRL1_SRC_RING_BYTE_SWAP_EN_LSB)
#define CE_CTRL1_DST_RING_BYTE_SWAP_EN_LSB \
	(scn->target_ce_def->d_CE_CTRL1_DST_RING_BYTE_SWAP_EN_LSB)
#define WLAN_DEBUG_INPUT_SEL_OFFSET \
	(scn->targetdef->d_WLAN_DEBUG_INPUT_SEL_OFFSET)
#define WLAN_DEBUG_INPUT_SEL_SRC_MSB \
	(scn->targetdef->d_WLAN_DEBUG_INPUT_SEL_SRC_MSB)
#define WLAN_DEBUG_INPUT_SEL_SRC_LSB \
	(scn->targetdef->d_WLAN_DEBUG_INPUT_SEL_SRC_LSB)
#define WLAN_DEBUG_INPUT_SEL_SRC_MASK \
	(scn->targetdef->d_WLAN_DEBUG_INPUT_SEL_SRC_MASK)
#define WLAN_DEBUG_CONTROL_OFFSET  (scn->targetdef->d_WLAN_DEBUG_CONTROL_OFFSET)
#define WLAN_DEBUG_CONTROL_ENABLE_MSB \
	(scn->targetdef->d_WLAN_DEBUG_CONTROL_ENABLE_MSB)
#define WLAN_DEBUG_CONTROL_ENABLE_LSB \
	(scn->targetdef->d_WLAN_DEBUG_CONTROL_ENABLE_LSB)
#define WLAN_DEBUG_CONTROL_ENABLE_MASK \
	(scn->targetdef->d_WLAN_DEBUG_CONTROL_ENABLE_MASK)
#define WLAN_DEBUG_OUT_OFFSET    (scn->targetdef->d_WLAN_DEBUG_OUT_OFFSET)
#define WLAN_DEBUG_OUT_DATA_MSB  (scn->targetdef->d_WLAN_DEBUG_OUT_DATA_MSB)
#define WLAN_DEBUG_OUT_DATA_LSB  (scn->targetdef->d_WLAN_DEBUG_OUT_DATA_LSB)
#define WLAN_DEBUG_OUT_DATA_MASK (scn->targetdef->d_WLAN_DEBUG_OUT_DATA_MASK)
#define AMBA_DEBUG_BUS_OFFSET    (scn->targetdef->d_AMBA_DEBUG_BUS_OFFSET)
#define AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MSB \
	(scn->targetdef->d_AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MSB)
#define AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_LSB \
	(scn->targetdef->d_AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_LSB)
#define AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MASK \
	(scn->targetdef->d_AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MASK)
#define AMBA_DEBUG_BUS_SEL_MSB    (scn->targetdef->d_AMBA_DEBUG_BUS_SEL_MSB)
#define AMBA_DEBUG_BUS_SEL_LSB    (scn->targetdef->d_AMBA_DEBUG_BUS_SEL_LSB)
#define AMBA_DEBUG_BUS_SEL_MASK   (scn->targetdef->d_AMBA_DEBUG_BUS_SEL_MASK)
#define CE_WRAPPER_DEBUG_OFFSET   \
				(scn->target_ce_def->d_CE_WRAPPER_DEBUG_OFFSET)
#define CE_WRAPPER_DEBUG_SEL_MSB  \
				(scn->target_ce_def->d_CE_WRAPPER_DEBUG_SEL_MSB)
#define CE_WRAPPER_DEBUG_SEL_LSB  \
				(scn->target_ce_def->d_CE_WRAPPER_DEBUG_SEL_LSB)
#define CE_WRAPPER_DEBUG_SEL_MASK \
			(scn->target_ce_def->d_CE_WRAPPER_DEBUG_SEL_MASK)
#define CE_DEBUG_OFFSET           (scn->target_ce_def->d_CE_DEBUG_OFFSET)
#define CE_DEBUG_SEL_MSB          (scn->target_ce_def->d_CE_DEBUG_SEL_MSB)
#define CE_DEBUG_SEL_LSB          (scn->target_ce_def->d_CE_DEBUG_SEL_LSB)
#define CE_DEBUG_SEL_MASK         (scn->target_ce_def->d_CE_DEBUG_SEL_MASK)
#define HOST_IE_ADDRESS           (scn->target_ce_def->d_HOST_IE_ADDRESS)
#define HOST_IE_REG1_CE_LSB       (scn->target_ce_def->d_HOST_IE_REG1_CE_LSB)
#define HOST_IE_ADDRESS_2         (scn->target_ce_def->d_HOST_IE_ADDRESS_2)
#define HOST_IE_REG2_CE_LSB       (scn->target_ce_def->d_HOST_IE_REG2_CE_LSB)
#define HOST_IE_ADDRESS_3         (scn->target_ce_def->d_HOST_IE_ADDRESS_3)
#define HOST_IE_REG3_CE_LSB       (scn->target_ce_def->d_HOST_IE_REG3_CE_LSB)
#define HOST_IS_ADDRESS           (scn->target_ce_def->d_HOST_IS_ADDRESS)

#define SRC_WATERMARK_LOW_SET(x) \
	(((x) << SRC_WATERMARK_LOW_LSB) & SRC_WATERMARK_LOW_MASK)
#define SRC_WATERMARK_HIGH_SET(x) \
	(((x) << SRC_WATERMARK_HIGH_LSB) & SRC_WATERMARK_HIGH_MASK)
#define DST_WATERMARK_LOW_SET(x) \
	(((x) << DST_WATERMARK_LOW_LSB) & DST_WATERMARK_LOW_MASK)
#define DST_WATERMARK_HIGH_SET(x) \
	(((x) << DST_WATERMARK_HIGH_LSB) & DST_WATERMARK_HIGH_MASK)
#define CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_GET(x) \
	(((x) & CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_MASK) >> \
		CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_LSB)
#define CE_CTRL1_DMAX_LENGTH_SET(x) \
	(((x) << CE_CTRL1_DMAX_LENGTH_LSB) & CE_CTRL1_DMAX_LENGTH_MASK)
#define CE_CTRL1_SRC_RING_BYTE_SWAP_EN_SET(x) \
	(((x) << CE_CTRL1_SRC_RING_BYTE_SWAP_EN_LSB) & \
		CE_CTRL1_SRC_RING_BYTE_SWAP_EN_MASK)
#define CE_CTRL1_DST_RING_BYTE_SWAP_EN_SET(x) \
	(((x) << CE_CTRL1_DST_RING_BYTE_SWAP_EN_LSB) & \
		CE_CTRL1_DST_RING_BYTE_SWAP_EN_MASK)
#define WLAN_DEBUG_INPUT_SEL_SRC_GET(x) \
	(((x) & WLAN_DEBUG_INPUT_SEL_SRC_MASK) >> \
		WLAN_DEBUG_INPUT_SEL_SRC_LSB)
#define WLAN_DEBUG_INPUT_SEL_SRC_SET(x) \
	(((x) << WLAN_DEBUG_INPUT_SEL_SRC_LSB) & \
		WLAN_DEBUG_INPUT_SEL_SRC_MASK)
#define WLAN_DEBUG_CONTROL_ENABLE_GET(x) \
	(((x) & WLAN_DEBUG_CONTROL_ENABLE_MASK) >> \
		WLAN_DEBUG_CONTROL_ENABLE_LSB)
#define WLAN_DEBUG_CONTROL_ENABLE_SET(x) \
	(((x) << WLAN_DEBUG_CONTROL_ENABLE_LSB) & \
		WLAN_DEBUG_CONTROL_ENABLE_MASK)
#define WLAN_DEBUG_OUT_DATA_GET(x) \
	(((x) & WLAN_DEBUG_OUT_DATA_MASK) >> WLAN_DEBUG_OUT_DATA_LSB)
#define WLAN_DEBUG_OUT_DATA_SET(x) \
	(((x) << WLAN_DEBUG_OUT_DATA_LSB) & WLAN_DEBUG_OUT_DATA_MASK)
#define AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_GET(x) \
	(((x) & AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MASK) >> \
		AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_LSB)
#define AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_SET(x) \
	(((x) << AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_LSB) & \
		AMBA_DEBUG_BUS_PCIE_DEBUG_SEL_MASK)
#define AMBA_DEBUG_BUS_SEL_GET(x) \
	(((x) & AMBA_DEBUG_BUS_SEL_MASK) >> AMBA_DEBUG_BUS_SEL_LSB)
#define AMBA_DEBUG_BUS_SEL_SET(x) \
	(((x) << AMBA_DEBUG_BUS_SEL_LSB) & AMBA_DEBUG_BUS_SEL_MASK)
#define CE_WRAPPER_DEBUG_SEL_GET(x) \
	(((x) & CE_WRAPPER_DEBUG_SEL_MASK) >> CE_WRAPPER_DEBUG_SEL_LSB)
#define CE_WRAPPER_DEBUG_SEL_SET(x) \
	(((x) << CE_WRAPPER_DEBUG_SEL_LSB) & CE_WRAPPER_DEBUG_SEL_MASK)
#define CE_DEBUG_SEL_GET(x) (((x) & CE_DEBUG_SEL_MASK) >> CE_DEBUG_SEL_LSB)
#define CE_DEBUG_SEL_SET(x) (((x) << CE_DEBUG_SEL_LSB) & CE_DEBUG_SEL_MASK)
#define HOST_IE_REG1_CE_BIT(_ce_id) (1 << (_ce_id + HOST_IE_REG1_CE_LSB))
#define HOST_IE_REG2_CE_BIT(_ce_id) (1 << (_ce_id + HOST_IE_REG2_CE_LSB))
#define HOST_IE_REG3_CE_BIT(_ce_id) (1 << (_ce_id + HOST_IE_REG3_CE_LSB))

uint32_t DEBUG_CE_SRC_RING_READ_IDX_GET(struct hif_softc *scn,
		uint32_t CE_ctrl_addr);
uint32_t DEBUG_CE_DEST_RING_READ_IDX_GET(struct hif_softc *scn,
		uint32_t CE_ctrl_addr);

#define BITS0_TO_31(val) ((uint32_t)((uint64_t)(paddr_rri_on_ddr)\
				     & (uint64_t)(0xFFFFFFFF)))
#define BITS32_TO_35(val) ((uint32_t)(((uint64_t)(paddr_rri_on_ddr)\
				     & (uint64_t)(0xF00000000))>>32))

#define VADDR_FOR_CE(scn, CE_ctrl_addr)\
	((scn->vaddr_rri_on_ddr) + COPY_ENGINE_ID(CE_ctrl_addr))

#define SRRI_FROM_DDR_ADDR(addr) ((*(addr)) & 0xFFFF)
#define DRRI_FROM_DDR_ADDR(addr) (((*(addr))>>16) & 0xFFFF)

#define CE_SRC_RING_READ_IDX_GET_FROM_REGISTER(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + CURRENT_SRRI_ADDRESS)
#define CE_DEST_RING_READ_IDX_GET_FROM_REGISTER(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + CURRENT_DRRI_ADDRESS)

#ifdef ADRASTEA_RRI_ON_DDR
#ifdef SHADOW_REG_DEBUG
#define CE_SRC_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr)\
	DEBUG_CE_SRC_RING_READ_IDX_GET(scn, CE_ctrl_addr)
#define CE_DEST_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr)\
	DEBUG_CE_DEST_RING_READ_IDX_GET(scn, CE_ctrl_addr)
#else
#define CE_SRC_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr)\
	SRRI_FROM_DDR_ADDR(VADDR_FOR_CE(scn, CE_ctrl_addr))
#define CE_DEST_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr)\
	DRRI_FROM_DDR_ADDR(VADDR_FOR_CE(scn, CE_ctrl_addr))
#endif

unsigned int hif_get_src_ring_read_index(struct hif_softc *scn,
		uint32_t CE_ctrl_addr);
unsigned int hif_get_dst_ring_read_index(struct hif_softc *scn,
		uint32_t CE_ctrl_addr);

#define CE_SRC_RING_READ_IDX_GET(scn, CE_ctrl_addr)\
	hif_get_src_ring_read_index(scn, CE_ctrl_addr)
#define CE_DEST_RING_READ_IDX_GET(scn, CE_ctrl_addr)\
	hif_get_dst_ring_read_index(scn, CE_ctrl_addr)
#else
#define CE_SRC_RING_READ_IDX_GET(scn, CE_ctrl_addr) \
	CE_SRC_RING_READ_IDX_GET_FROM_REGISTER(scn, CE_ctrl_addr)
#define CE_DEST_RING_READ_IDX_GET(scn, CE_ctrl_addr)\
	CE_DEST_RING_READ_IDX_GET_FROM_REGISTER(scn, CE_ctrl_addr)

/**
 * if RRI on DDR is not enabled, get idx from ddr defaults to
 * using the register value & force wake must be used for
 * non interrupt processing.
 */
#define CE_SRC_RING_READ_IDX_GET_FROM_DDR(scn, CE_ctrl_addr)\
	A_TARGET_READ(scn, (CE_ctrl_addr) + CURRENT_SRRI_ADDRESS)
#endif

#define CE_SRC_RING_BASE_ADDR_SET(scn, CE_ctrl_addr, addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + SR_BA_ADDRESS, (addr))

#define CE_SRC_RING_BASE_ADDR_HIGH_SET(scn, CE_ctrl_addr, addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + SR_BA_ADDRESS_HIGH, (addr))

#define CE_SRC_RING_BASE_ADDR_HIGH_GET(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + SR_BA_ADDRESS_HIGH)

#define CE_SRC_RING_SZ_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + SR_SIZE_ADDRESS, (n))

#define CE_SRC_RING_DMAX_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_CTRL1_ADDRESS, \
	   (A_TARGET_READ(scn, (CE_ctrl_addr) + \
	   CE_CTRL1_ADDRESS) & ~CE_CTRL1_DMAX_LENGTH_MASK) | \
	   CE_CTRL1_DMAX_LENGTH_SET(n))

#define CE_IDX_UPD_EN_SET(scn, CE_ctrl_addr)  \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_CTRL1_ADDRESS, \
	(A_TARGET_READ(scn, (CE_ctrl_addr) + CE_CTRL1_ADDRESS) \
	| CE_CTRL1_IDX_UPD_EN))

#define CE_CMD_REGISTER_GET(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + CE_CMD_REGISTER)

#define CE_CMD_REGISTER_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_CMD_REGISTER, n)

#define CE_MSI_ADDR_LOW_SET(scn, CE_ctrl_addr, addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_MSI_ADDRESS, (addr))

#define CE_MSI_ADDR_HIGH_SET(scn, CE_ctrl_addr, addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_MSI_ADDRESS_HIGH, (addr))

#define CE_MSI_DATA_SET(scn, CE_ctrl_addr, data) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_MSI_DATA, (data))

#define CE_CTRL_REGISTER1_SET(scn, CE_ctrl_addr, val) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_CTRL1_ADDRESS, val)

#define CE_CTRL_REGISTER1_GET(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + CE_CTRL1_ADDRESS)

#define CE_SRC_RING_BYTE_SWAP_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + CE_CTRL1_ADDRESS, \
		       (A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + CE_CTRL1_ADDRESS) \
		       & ~CE_CTRL1_SRC_RING_BYTE_SWAP_EN_MASK) | \
		       CE_CTRL1_SRC_RING_BYTE_SWAP_EN_SET(n))

#define CE_DEST_RING_BYTE_SWAP_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr)+CE_CTRL1_ADDRESS, \
		       (A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + CE_CTRL1_ADDRESS) \
		       & ~CE_CTRL1_DST_RING_BYTE_SWAP_EN_MASK) | \
		       CE_CTRL1_DST_RING_BYTE_SWAP_EN_SET(n))


#define CE_DEST_RING_BASE_ADDR_SET(scn, CE_ctrl_addr, addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + DR_BA_ADDRESS, (addr))

#define CE_DEST_RING_BASE_ADDR_HIGH_SET(scn, CE_ctrl_addr, addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + DR_BA_ADDRESS_HIGH, (addr))

#define CE_DEST_RING_BASE_ADDR_HIGH_GET(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + DR_BA_ADDRESS_HIGH)

#define CE_DEST_RING_SZ_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + DR_SIZE_ADDRESS, (n))

#define CE_SRC_RING_HIGHMARK_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + SRC_WATERMARK_ADDRESS, \
		       (A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + SRC_WATERMARK_ADDRESS) \
		       & ~SRC_WATERMARK_HIGH_MASK) | \
		       SRC_WATERMARK_HIGH_SET(n))

#define CE_SRC_RING_LOWMARK_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + SRC_WATERMARK_ADDRESS, \
		       (A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + SRC_WATERMARK_ADDRESS) \
		       & ~SRC_WATERMARK_LOW_MASK) | \
		       SRC_WATERMARK_LOW_SET(n))

#define CE_DEST_RING_HIGHMARK_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + DST_WATERMARK_ADDRESS, \
		       (A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + DST_WATERMARK_ADDRESS) \
		       & ~DST_WATERMARK_HIGH_MASK) | \
		       DST_WATERMARK_HIGH_SET(n))

#define CE_DEST_RING_LOWMARK_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + DST_WATERMARK_ADDRESS, \
		       (A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + DST_WATERMARK_ADDRESS) \
		       & ~DST_WATERMARK_LOW_MASK) | \
		       DST_WATERMARK_LOW_SET(n))

#define CE_COPY_COMPLETE_INTR_ENABLE(scn, CE_ctrl_addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + HOST_IE_ADDRESS, \
		       A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + HOST_IE_ADDRESS) | \
		       HOST_IE_COPY_COMPLETE_MASK)

#define CE_COPY_COMPLETE_INTR_DISABLE(scn, CE_ctrl_addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + HOST_IE_ADDRESS, \
		       A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + HOST_IE_ADDRESS) \
		       & ~HOST_IE_COPY_COMPLETE_MASK)

#define CE_BASE_ADDRESS(CE_id) \
	CE0_BASE_ADDRESS + ((CE1_BASE_ADDRESS - \
	CE0_BASE_ADDRESS)*(CE_id))

#define CE_WATERMARK_INTR_ENABLE(scn, CE_ctrl_addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + HOST_IE_ADDRESS, \
		       A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + HOST_IE_ADDRESS) | \
		       CE_WATERMARK_MASK)

#define CE_WATERMARK_INTR_DISABLE(scn, CE_ctrl_addr)	\
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + HOST_IE_ADDRESS, \
		       A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + HOST_IE_ADDRESS) \
		       & ~CE_WATERMARK_MASK)

#define CE_ERROR_INTR_ENABLE(scn, CE_ctrl_addr) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + MISC_IE_ADDRESS, \
		       A_TARGET_READ(scn, \
		       (CE_ctrl_addr) + MISC_IE_ADDRESS) | CE_ERROR_MASK)

#define CE_MISC_INT_STATUS_GET(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + MISC_IS_ADDRESS)

#define CE_ENGINE_INT_STATUS_GET(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + HOST_IS_ADDRESS)

#define CE_ENGINE_INT_STATUS_CLEAR(scn, CE_ctrl_addr, mask) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + HOST_IS_ADDRESS, (mask))

#define CE_WATERMARK_MASK (HOST_IS_SRC_RING_LOW_WATERMARK_MASK  | \
			   HOST_IS_SRC_RING_HIGH_WATERMARK_MASK | \
			   HOST_IS_DST_RING_LOW_WATERMARK_MASK  | \
			   HOST_IS_DST_RING_HIGH_WATERMARK_MASK)

#define CE_ERROR_MASK     (MISC_IS_AXI_ERR_MASK           | \
			   MISC_IS_DST_ADDR_ERR_MASK      | \
			   MISC_IS_SRC_LEN_ERR_MASK       | \
			   MISC_IS_DST_MAX_LEN_VIO_MASK   | \
			   MISC_IS_DST_RING_OVERFLOW_MASK | \
			   MISC_IS_SRC_RING_OVERFLOW_MASK)

#define CE_SRC_RING_TO_DESC(baddr, idx) \
	(&(((struct CE_src_desc *)baddr)[idx]))
#define CE_DEST_RING_TO_DESC(baddr, idx) \
	(&(((struct CE_dest_desc *)baddr)[idx]))

/* Ring arithmetic (modulus number of entries in ring, which is a pwr of 2). */
#define CE_RING_DELTA(nentries_mask, fromidx, toidx) \
	(((int)(toidx)-(int)(fromidx)) & (nentries_mask))

#define CE_RING_IDX_INCR(nentries_mask, idx) \
	(((idx) + 1) & (nentries_mask))

#define CE_RING_IDX_ADD(nentries_mask, idx, num) \
	(((idx) + (num)) & (nentries_mask))

#define CE_INTERRUPT_SUMMARY(scn) \
	CE_WRAPPER_INTERRUPT_SUMMARY_HOST_MSI_GET( \
		A_TARGET_READ(scn, CE_WRAPPER_BASE_ADDRESS + \
		CE_WRAPPER_INTERRUPT_SUMMARY_ADDRESS))

#define READ_CE_DDR_ADDRESS_FOR_RRI_LOW(scn) \
	(A_TARGET_READ(scn, \
		       CE_WRAPPER_BASE_ADDRESS + CE_DDR_ADDRESS_FOR_RRI_LOW))

#define READ_CE_DDR_ADDRESS_FOR_RRI_HIGH(scn) \
	(A_TARGET_READ(scn, \
		       CE_WRAPPER_BASE_ADDRESS + CE_DDR_ADDRESS_FOR_RRI_HIGH))

#define WRITE_CE_DDR_ADDRESS_FOR_RRI_LOW(scn, val) \
	(A_TARGET_WRITE(scn, \
			CE_WRAPPER_BASE_ADDRESS + CE_DDR_ADDRESS_FOR_RRI_LOW, \
			val))

#define WRITE_CE_DDR_ADDRESS_FOR_RRI_HIGH(scn, val) \
	(A_TARGET_WRITE(scn, \
			CE_WRAPPER_BASE_ADDRESS + CE_DDR_ADDRESS_FOR_RRI_HIGH, \
			val))

/*Macro to increment CE packet errors*/
#define OL_ATH_CE_PKT_ERROR_COUNT_INCR(_scn, _ce_ecode) \
	do { if (_ce_ecode == CE_RING_DELTA_FAIL) \
			(_scn->pkt_stats.ce_ring_delta_fail_count) \
			+= 1; } while (0)

/* Given a Copy Engine's ID, determine the interrupt number for that
 * copy engine's interrupts.
 */
#define CE_ID_TO_INUM(id) (A_INUM_CE0_COPY_COMP_BASE + (id))
#define CE_INUM_TO_ID(inum) ((inum) - A_INUM_CE0_COPY_COMP_BASE)
#define CE0_BASE_ADDRESS         (scn->target_ce_def->d_CE0_BASE_ADDRESS)
#define CE1_BASE_ADDRESS         (scn->target_ce_def->d_CE1_BASE_ADDRESS)


#ifdef ADRASTEA_SHADOW_REGISTERS
#define NUM_SHADOW_REGISTERS 24
u32 shadow_sr_wr_ind_addr(struct hif_softc *scn, u32 ctrl_addr);
u32 shadow_dst_wr_ind_addr(struct hif_softc *scn, u32 ctrl_addr);
#endif


#ifdef ADRASTEA_SHADOW_REGISTERS
#define CE_SRC_RING_WRITE_IDX_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, shadow_sr_wr_ind_addr(scn, CE_ctrl_addr), n)
#define CE_DEST_RING_WRITE_IDX_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, shadow_dst_wr_ind_addr(scn, CE_ctrl_addr), n)

#else

#define CE_SRC_RING_WRITE_IDX_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + SR_WR_INDEX_ADDRESS, (n))
#define CE_DEST_RING_WRITE_IDX_SET(scn, CE_ctrl_addr, n) \
	A_TARGET_WRITE(scn, (CE_ctrl_addr) + DST_WR_INDEX_ADDRESS, (n))
#endif

/* The write index read is only needed durring initialization because
 * we keep track of the index that was last written.  Thus the register
 * is the only hardware supported location to read the initial value from.
 */
#define CE_SRC_RING_WRITE_IDX_GET_FROM_REGISTER(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + SR_WR_INDEX_ADDRESS)
#define CE_DEST_RING_WRITE_IDX_GET_FROM_REGISTER(scn, CE_ctrl_addr) \
	A_TARGET_READ(scn, (CE_ctrl_addr) + DST_WR_INDEX_ADDRESS)

#endif /* __CE_REG_H__ */
