/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef _HAL_GENERIC_API_H_
#define _HAL_GENERIC_API_H_

#include <hal_rx.h>

/**
 * hal_get_radiotap_he_gi_ltf() - Convert HE ltf and GI value
 * from stats enum to radiotap enum
 * @he_gi: HE GI value used in stats
 * @he_ltf: HE LTF value used in stats
 *
 * Return: void
 */
static inline void hal_get_radiotap_he_gi_ltf(uint16_t *he_gi, uint16_t *he_ltf)
{
	switch (*he_gi) {
	case HE_GI_0_8:
		*he_gi = HE_GI_RADIOTAP_0_8;
		break;
	case HE_GI_1_6:
		*he_gi = HE_GI_RADIOTAP_1_6;
		break;
	case HE_GI_3_2:
		*he_gi = HE_GI_RADIOTAP_3_2;
		break;
	default:
		*he_gi = HE_GI_RADIOTAP_RESERVED;
	}

	switch (*he_ltf) {
	case HE_LTF_1_X:
		*he_ltf = HE_LTF_RADIOTAP_1_X;
		break;
	case HE_LTF_2_X:
		*he_ltf = HE_LTF_RADIOTAP_2_X;
		break;
	case HE_LTF_4_X:
		*he_ltf = HE_LTF_RADIOTAP_4_X;
		break;
	default:
		*he_ltf = HE_LTF_RADIOTAP_UNKNOWN;
	}
}

/* channel number to freq conversion */
#define CHANNEL_NUM_14 14
#define CHANNEL_NUM_15 15
#define CHANNEL_NUM_27 27
#define CHANNEL_NUM_35 35
#define CHANNEL_NUM_182 182
#define CHANNEL_NUM_197 197
#define CHANNEL_FREQ_2484 2484
#define CHANNEL_FREQ_2407 2407
#define CHANNEL_FREQ_2512 2512
#define CHANNEL_FREQ_5000 5000
#define CHANNEL_FREQ_5950 5950
#define CHANNEL_FREQ_4000 4000
#define CHANNEL_FREQ_5150 5150
#define CHANNEL_FREQ_5920 5920
#define CHANNEL_FREQ_5935 5935
#define FREQ_MULTIPLIER_CONST_5MHZ 5
#define FREQ_MULTIPLIER_CONST_20MHZ 20
/**
 * hal_rx_radiotap_num_to_freq() - Get frequency from chan number
 * @chan_num - Input channel number
 * @center_freq - Input Channel Center frequency
 *
 * Return - Channel frequency in Mhz
 */
static uint16_t
hal_rx_radiotap_num_to_freq(uint16_t chan_num, qdf_freq_t center_freq)
{
	if (center_freq > CHANNEL_FREQ_5920 && center_freq < CHANNEL_FREQ_5950)
		return CHANNEL_FREQ_5935;

	if (center_freq < CHANNEL_FREQ_5950) {
		if (chan_num == CHANNEL_NUM_14)
			return CHANNEL_FREQ_2484;
		if (chan_num < CHANNEL_NUM_14)
			return CHANNEL_FREQ_2407 +
				(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);

		if (chan_num < CHANNEL_NUM_27)
			return CHANNEL_FREQ_2512 +
				((chan_num - CHANNEL_NUM_15) *
					FREQ_MULTIPLIER_CONST_20MHZ);

		if (chan_num > CHANNEL_NUM_182 &&
		    chan_num < CHANNEL_NUM_197)
			return ((chan_num * FREQ_MULTIPLIER_CONST_5MHZ) +
				CHANNEL_FREQ_4000);

		return CHANNEL_FREQ_5000 +
			(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);
	} else {
		return CHANNEL_FREQ_5950 +
			(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);
	}
}

/**
 * hal_get_hw_hptp_generic()  - Get HW head and tail pointer value for any ring
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Source ring pointer
 * @headp: Head Pointer
 * @tailp: Tail Pointer
 * @ring: Ring type
 *
 * Return: Update tail pointer and head pointer in arguments.
 */
static inline
void hal_get_hw_hptp_generic(struct hal_soc *hal_soc,
			     hal_ring_handle_t hal_ring_hdl,
			     uint32_t *headp, uint32_t *tailp,
			     uint8_t ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	struct hal_hw_srng_config *ring_config;
	enum hal_ring_type ring_type = (enum hal_ring_type)ring;

	if (!hal_soc  || !srng) {
		QDF_TRACE(QDF_MODULE_ID_HAL, QDF_TRACE_LEVEL_ERROR,
			  "%s: Context is Null", __func__);
		return;
	}

	ring_config = HAL_SRNG_CONFIG(hal_soc, ring_type);
	if (!ring_config->lmac_ring) {
		if (srng->ring_dir == HAL_SRNG_SRC_RING) {
			*headp = SRNG_SRC_REG_READ(srng, HP);
			*tailp = SRNG_SRC_REG_READ(srng, TP);
		} else {
			*headp = SRNG_DST_REG_READ(srng, HP);
			*tailp = SRNG_DST_REG_READ(srng, TP);
		}
	}
}

#if defined(WBM_IDLE_LSB_WRITE_CONFIRM_WAR)
/**
 * hal_wbm_idle_lsb_write_confirm() - Check and update WBM_IDLE_LINK ring LSB
 * @srng: srng handle
 *
 * Return: None
 */
static void hal_wbm_idle_lsb_write_confirm(struct hal_srng *srng)
{
	if (srng->ring_id == HAL_SRNG_WBM_IDLE_LINK) {
		while (SRNG_SRC_REG_READ(srng, BASE_LSB) !=
		       ((unsigned int)srng->ring_base_paddr & 0xffffffff))
				SRNG_SRC_REG_WRITE(srng, BASE_LSB,
						   srng->ring_base_paddr &
						   0xffffffff);
	}
}
#else
static void hal_wbm_idle_lsb_write_confirm(struct hal_srng *srng)
{
}
#endif

/**
 * hal_srng_src_hw_init - Private function to initialize SRNG
 * source ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline
void hal_srng_src_hw_init_generic(struct hal_soc *hal,
				  struct hal_srng *srng)
{
	uint32_t reg_val = 0;
	uint64_t tp_addr = 0;

	hal_debug("hw_init srng %d", srng->ring_id);

	if (srng->flags & HAL_SRNG_MSI_INTR) {
		SRNG_SRC_REG_WRITE(srng, MSI1_BASE_LSB,
			srng->msi_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_SRC_FLD(MSI1_BASE_MSB, ADDR),
			(uint64_t)(srng->msi_addr) >> 32) |
			SRNG_SM(SRNG_SRC_FLD(MSI1_BASE_MSB,
			MSI1_ENABLE), 1);
		SRNG_SRC_REG_WRITE(srng, MSI1_BASE_MSB, reg_val);
		SRNG_SRC_REG_WRITE(srng, MSI1_DATA,
				   qdf_cpu_to_le32(srng->msi_data));
	}

	SRNG_SRC_REG_WRITE(srng, BASE_LSB, srng->ring_base_paddr & 0xffffffff);
	hal_wbm_idle_lsb_write_confirm(srng);

	reg_val = SRNG_SM(SRNG_SRC_FLD(BASE_MSB, RING_BASE_ADDR_MSB),
		((uint64_t)(srng->ring_base_paddr) >> 32)) |
		SRNG_SM(SRNG_SRC_FLD(BASE_MSB, RING_SIZE),
		srng->entry_size * srng->num_entries);
	SRNG_SRC_REG_WRITE(srng, BASE_MSB, reg_val);

	reg_val = SRNG_SM(SRNG_SRC_FLD(ID, ENTRY_SIZE), srng->entry_size);
	SRNG_SRC_REG_WRITE(srng, ID, reg_val);

	/**
	 * Interrupt setup:
	 * Default interrupt mode is 'pulse'. Need to setup SW_INTERRUPT_MODE
	 * if level mode is required
	 */
	reg_val = 0;

	/*
	 * WAR - Hawkeye v1 has a hardware bug which requires timer value to be
	 * programmed in terms of 1us resolution instead of 8us resolution as
	 * given in MLD.
	 */
	if (srng->intr_timer_thres_us) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX0,
			INTERRUPT_TIMER_THRESHOLD),
			srng->intr_timer_thres_us);
		/* For HK v2 this should be (srng->intr_timer_thres_us >> 3) */
	}

	if (srng->intr_batch_cntr_thres_entries) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX0,
			BATCH_COUNTER_THRESHOLD),
			srng->intr_batch_cntr_thres_entries *
			srng->entry_size);
	}
	SRNG_SRC_REG_WRITE(srng, CONSUMER_INT_SETUP_IX0, reg_val);

	reg_val = 0;
	if (srng->flags & HAL_SRNG_LOW_THRES_INTR_ENABLE) {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(CONSUMER_INT_SETUP_IX1,
			LOW_THRESHOLD), srng->u.src_ring.low_threshold);
	}

	SRNG_SRC_REG_WRITE(srng, CONSUMER_INT_SETUP_IX1, reg_val);

	/* As per HW team, TP_ADDR and HP_ADDR for Idle link ring should
	 * remain 0 to avoid some WBM stability issues. Remote head/tail
	 * pointers are not required since this ring is completely managed
	 * by WBM HW
	 */
	reg_val = 0;
	if (srng->ring_id != HAL_SRNG_WBM_IDLE_LINK) {
		tp_addr = (uint64_t)(hal->shadow_rdptr_mem_paddr +
			((unsigned long)(srng->u.src_ring.tp_addr) -
			(unsigned long)(hal->shadow_rdptr_mem_vaddr)));
		SRNG_SRC_REG_WRITE(srng, TP_ADDR_LSB, tp_addr & 0xffffffff);
		SRNG_SRC_REG_WRITE(srng, TP_ADDR_MSB, tp_addr >> 32);
	} else {
		reg_val |= SRNG_SM(SRNG_SRC_FLD(MISC, RING_ID_DISABLE), 1);
	}

	/* Initilaize head and tail pointers to indicate ring is empty */
	SRNG_SRC_REG_WRITE(srng, HP, 0);
	SRNG_SRC_REG_WRITE(srng, TP, 0);
	*(srng->u.src_ring.tp_addr) = 0;

	reg_val |= ((srng->flags & HAL_SRNG_DATA_TLV_SWAP) ?
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

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * hal_srng_dst_msi2_setup() - Configure MSI2 register for a SRNG
 * @srng: SRNG handle
 *
 * Return: None
 */
static inline void hal_srng_dst_msi2_setup(struct hal_srng *srng)
{
	uint32_t reg_val = 0;

	if (srng->u.dst_ring.nf_irq_support) {
		SRNG_DST_REG_WRITE(srng, MSI2_BASE_LSB,
				   srng->msi2_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_DST_FLD(MSI2_BASE_MSB, ADDR),
				  (uint64_t)(srng->msi2_addr) >> 32) |
				  SRNG_SM(SRNG_DST_FLD(MSI2_BASE_MSB,
					  MSI2_ENABLE), 1);
		SRNG_DST_REG_WRITE(srng, MSI2_BASE_MSB, reg_val);
		SRNG_DST_REG_WRITE(srng, MSI2_DATA,
				   qdf_cpu_to_le32(srng->msi2_data));
	}
}

/**
 * hal_srng_dst_near_full_int_setup() - Configure near-full params for SRNG
 * @srng: SRNG handle
 *
 * Return: None
 */
static inline void hal_srng_dst_near_full_int_setup(struct hal_srng *srng)
{
	uint32_t reg_val = 0;

	if (srng->u.dst_ring.nf_irq_support) {
		if (srng->intr_timer_thres_us) {
			reg_val |= SRNG_SM(SRNG_DST_FLD(PRODUCER_INT2_SETUP,
					   INTERRUPT2_TIMER_THRESHOLD),
					   srng->intr_timer_thres_us >> 3);
		}

		reg_val |= SRNG_SM(SRNG_DST_FLD(PRODUCER_INT2_SETUP,
				   HIGH_THRESHOLD),
				   srng->u.dst_ring.high_thresh *
				   srng->entry_size);
	}

	SRNG_DST_REG_WRITE(srng, PRODUCER_INT2_SETUP, reg_val);
}
#else
static inline void hal_srng_dst_msi2_setup(struct hal_srng *srng)
{
}

static inline void hal_srng_dst_near_full_int_setup(struct hal_srng *srng)
{
}
#endif

/**
 * hal_srng_dst_hw_init - Private function to initialize SRNG
 * destination ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline
void hal_srng_dst_hw_init_generic(struct hal_soc *hal,
				  struct hal_srng *srng)
{
	uint32_t reg_val = 0;
	uint64_t hp_addr = 0;

	hal_debug("hw_init srng %d", srng->ring_id);

	if (srng->flags & HAL_SRNG_MSI_INTR) {
		SRNG_DST_REG_WRITE(srng, MSI1_BASE_LSB,
			srng->msi_addr & 0xffffffff);
		reg_val = SRNG_SM(SRNG_DST_FLD(MSI1_BASE_MSB, ADDR),
			(uint64_t)(srng->msi_addr) >> 32) |
			SRNG_SM(SRNG_DST_FLD(MSI1_BASE_MSB,
			MSI1_ENABLE), 1);
		SRNG_DST_REG_WRITE(srng, MSI1_BASE_MSB, reg_val);
		SRNG_DST_REG_WRITE(srng, MSI1_DATA,
				   qdf_cpu_to_le32(srng->msi_data));

		hal_srng_dst_msi2_setup(srng);
	}

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

	/**
	 * Near-Full Interrupt setup:
	 * Default interrupt mode is 'pulse'. Need to setup SW_INTERRUPT_MODE
	 * if level mode is required
	 */
	hal_srng_dst_near_full_int_setup(srng);

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
 * hal_srng_hw_reg_offset_init_generic() - Initialize the HW srng reg offset
 * @hal_soc: HAL Soc handle
 *
 * Return: None
 */
static inline void hal_srng_hw_reg_offset_init_generic(struct hal_soc *hal_soc)
{
	int32_t *hw_reg_offset = hal_soc->hal_hw_reg_offset;

	/* dst */
	hw_reg_offset[DST_HP] = REG_OFFSET(DST, HP);
	hw_reg_offset[DST_TP] = REG_OFFSET(DST, TP);
	hw_reg_offset[DST_ID] = REG_OFFSET(DST, ID);
	hw_reg_offset[DST_MISC] = REG_OFFSET(DST, MISC);
	hw_reg_offset[DST_HP_ADDR_LSB] = REG_OFFSET(DST, HP_ADDR_LSB);
	hw_reg_offset[DST_HP_ADDR_MSB] = REG_OFFSET(DST, HP_ADDR_MSB);
	hw_reg_offset[DST_MSI1_BASE_LSB] = REG_OFFSET(DST, MSI1_BASE_LSB);
	hw_reg_offset[DST_MSI1_BASE_MSB] = REG_OFFSET(DST, MSI1_BASE_MSB);
	hw_reg_offset[DST_MSI1_DATA] = REG_OFFSET(DST, MSI1_DATA);
	hw_reg_offset[DST_BASE_LSB] = REG_OFFSET(DST, BASE_LSB);
	hw_reg_offset[DST_BASE_MSB] = REG_OFFSET(DST, BASE_MSB);
	hw_reg_offset[DST_PRODUCER_INT_SETUP] =
					REG_OFFSET(DST, PRODUCER_INT_SETUP);

	/* src */
	hw_reg_offset[SRC_HP] = REG_OFFSET(SRC, HP);
	hw_reg_offset[SRC_TP] = REG_OFFSET(SRC, TP);
	hw_reg_offset[SRC_ID] = REG_OFFSET(SRC, ID);
	hw_reg_offset[SRC_MISC] = REG_OFFSET(SRC, MISC);
	hw_reg_offset[SRC_TP_ADDR_LSB] = REG_OFFSET(SRC, TP_ADDR_LSB);
	hw_reg_offset[SRC_TP_ADDR_MSB] = REG_OFFSET(SRC, TP_ADDR_MSB);
	hw_reg_offset[SRC_MSI1_BASE_LSB] = REG_OFFSET(SRC, MSI1_BASE_LSB);
	hw_reg_offset[SRC_MSI1_BASE_MSB] = REG_OFFSET(SRC, MSI1_BASE_MSB);
	hw_reg_offset[SRC_MSI1_DATA] = REG_OFFSET(SRC, MSI1_DATA);
	hw_reg_offset[SRC_BASE_LSB] = REG_OFFSET(SRC, BASE_LSB);
	hw_reg_offset[SRC_BASE_MSB] = REG_OFFSET(SRC, BASE_MSB);
	hw_reg_offset[SRC_CONSUMER_INT_SETUP_IX0] =
					REG_OFFSET(SRC, CONSUMER_INT_SETUP_IX0);
	hw_reg_offset[SRC_CONSUMER_INT_SETUP_IX1] =
					REG_OFFSET(SRC, CONSUMER_INT_SETUP_IX1);
}

#endif /* HAL_GENERIC_API_H_ */
