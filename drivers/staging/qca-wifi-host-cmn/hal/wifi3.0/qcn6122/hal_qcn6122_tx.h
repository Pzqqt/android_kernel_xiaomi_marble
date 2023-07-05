/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "cdp_txrx_mon_struct.h"
#include "qdf_trace.h"
#include "hal_rx.h"
#include "hal_tx.h"
#include "dp_types.h"
#include "hal_api_mon.h"

/**
 * hal_tx_desc_set_dscp_tid_table_id_6122() - Sets DSCP to TID conversion
 *						table ID
 * @desc: Handle to Tx Descriptor
 * @id: DSCP to tid conversion table to be used for this frame
 *
 * Return: void
 */
static void hal_tx_desc_set_dscp_tid_table_id_6122(void *desc, uint8_t id)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_5,
		    DSCP_TID_TABLE_NUM) |=
		    HAL_TX_SM(TCL_DATA_CMD_5, DSCP_TID_TABLE_NUM, id);
}

#define DSCP_TID_TABLE_SIZE 24
#define NUM_WORDS_PER_DSCP_TID_TABLE (DSCP_TID_TABLE_SIZE / 4)
#define HAL_TX_NUM_DSCP_REGISTER_SIZE 32

/**
 * hal_tx_set_dscp_tid_map_6122() - Configure default DSCP to TID map table
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id: mapping table ID - 0,1
 *
 * DSCP are mapped to 8 TID values using TID values programmed
 * in two set of mapping registers DSCP_TID1_MAP_<0 to 6> (id = 0)
 * and DSCP_TID2_MAP_<0 to 6> (id = 1)
 * Each mapping register has TID mapping for 10 DSCP values
 *
 * Return: none
 */
static void hal_tx_set_dscp_tid_map_6122(struct hal_soc *soc,
					 uint8_t *map, uint8_t id)
{
	int i;
	uint32_t addr, cmn_reg_addr;
	uint32_t value = 0, regval;
	uint8_t val[DSCP_TID_TABLE_SIZE], cnt = 0;

	if (id >= HAL_MAX_HW_DSCP_TID_MAPS_11AX)
		return;

	cmn_reg_addr = HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	addr = HWIO_TCL_R0_DSCP_TID_MAP_n_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET,
				id * NUM_WORDS_PER_DSCP_TID_TABLE);

	/* Enable read/write access */
	regval = HAL_REG_READ(soc, cmn_reg_addr);
	regval |=
	(1 << HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_SHFT);

	HAL_REG_WRITE(soc, cmn_reg_addr, regval);

	/* Write 8 (24 bits) DSCP-TID mappings in each interation */
	for (i = 0; i < 64; i += 8) {
		value = (map[i] |
			(map[i + 1] << 0x3) |
			(map[i + 2] << 0x6) |
			(map[i + 3] << 0x9) |
			(map[i + 4] << 0xc) |
			(map[i + 5] << 0xf) |
			(map[i + 6] << 0x12) |
			(map[i + 7] << 0x15));

		qdf_mem_copy(&val[cnt], &value, 3);
		cnt += 3;
	}

	for (i = 0; i < DSCP_TID_TABLE_SIZE; i += 4) {
		regval = *(uint32_t *)(val + i);
		HAL_REG_WRITE(soc, addr,
			      (regval & HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK));
		addr += 4;
	}

	/* Diasble read/write access */
	regval = HAL_REG_READ(soc, cmn_reg_addr);
	regval &=
	~(HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_BMSK);

	HAL_REG_WRITE(soc, cmn_reg_addr, regval);
}

/**
 * hal_tx_update_dscp_tid_6122() - Update the dscp tid map table as
					updated by user
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id : MAP ID
 * @dscp: DSCP_TID map index
 *
 * Return: void
 */
static void hal_tx_update_dscp_tid_6122(struct hal_soc *soc, uint8_t tid,
					uint8_t id, uint8_t dscp)
{
	uint32_t addr, addr1, cmn_reg_addr;
	uint32_t start_value = 0, end_value = 0;
	uint32_t regval;
	uint8_t end_bits = 0;
	uint8_t start_bits = 0;
	uint32_t start_index, end_index;

	cmn_reg_addr = HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_ADDR(
					SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	addr = HWIO_TCL_R0_DSCP_TID_MAP_n_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET,
				id * NUM_WORDS_PER_DSCP_TID_TABLE);

	start_index = dscp * HAL_TX_BITS_PER_TID;
	end_index = (start_index + (HAL_TX_BITS_PER_TID - 1))
		    % HAL_TX_NUM_DSCP_REGISTER_SIZE;
	start_index = start_index % HAL_TX_NUM_DSCP_REGISTER_SIZE;
	addr += (4 * ((dscp * HAL_TX_BITS_PER_TID) /
			HAL_TX_NUM_DSCP_REGISTER_SIZE));

	if (end_index < start_index) {
		end_bits = end_index + 1;
		start_bits = HAL_TX_BITS_PER_TID - end_bits;
		start_value = tid << start_index;
		end_value = tid >> start_bits;
		addr1 = addr + 4;
	} else {
		start_bits = HAL_TX_BITS_PER_TID - end_bits;
		start_value = tid << start_index;
		addr1 = 0;
	}

	/* Enable read/write access */
	regval = HAL_REG_READ(soc, cmn_reg_addr);
	regval |=
	(1 << HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_SHFT);

	HAL_REG_WRITE(soc, cmn_reg_addr, regval);

	regval = HAL_REG_READ(soc, addr);

	if (end_index < start_index)
		regval &= (~0) >> start_bits;
	else
		regval &= ~(7 << start_index);

	regval |= start_value;

	HAL_REG_WRITE(soc, addr, (regval & HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK));

	if (addr1) {
		regval = HAL_REG_READ(soc, addr1);
		regval &= (~0) << end_bits;
		regval |= end_value;

		HAL_REG_WRITE(soc, addr1, (regval &
			     HWIO_TCL_R0_DSCP_TID_MAP_n_RMSK));
	}

	/* Diasble read/write access */
	regval = HAL_REG_READ(soc, cmn_reg_addr);
	regval &=
	~(HWIO_TCL_R0_CONS_RING_CMN_CTRL_REG_DSCP_TID_MAP_PROGRAM_EN_BMSK);
	HAL_REG_WRITE(soc, cmn_reg_addr, regval);
}

/**
 * hal_tx_desc_set_lmac_id_6122 - Set the lmac_id value
 * @desc: Handle to Tx Descriptor
 * @lmac_id: mac Id to ast matching
 *		     b00 – mac 0
 *		     b01 – mac 1
 *		     b10 – mac 2
 *		     b11 – all macs (legacy HK way)
 *
 * Return: void
 */
static void hal_tx_desc_set_lmac_id_6122(void *desc, uint8_t lmac_id)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_4, LMAC_ID) |=
		HAL_TX_SM(TCL_DATA_CMD_4, LMAC_ID, lmac_id);
}

/**
 * hal_tx_init_cmd_credit_ring_6122() - Initialize TCL command/credit SRNG
 * @hal_soc_hdl: Handle to HAL SoC structure
 * @hal_srng: Handle to HAL SRNG structure
 *
 * Return: none
 */
static inline void
hal_tx_init_cmd_credit_ring_6122(hal_soc_handle_t hal_soc_hdl,
				 hal_ring_handle_t hal_ring_hdl)
{
	uint8_t *desc_addr;
	struct hal_srng_params srng_params;
	uint32_t desc_size;
	uint32_t num_desc;

	hal_get_srng_params(hal_soc_hdl, hal_ring_hdl, &srng_params);

	desc_addr = (uint8_t *)srng_params.ring_base_vaddr;
	desc_size = sizeof(struct tcl_data_cmd);
	num_desc = srng_params.num_entries;

	while (num_desc) {
		/* using CMD/CREDIT Ring to send DATA CMD tag */
		HAL_TX_DESC_SET_TLV_HDR(desc_addr, WIFITCL_DATA_CMD_E,
					desc_size);
		desc_addr += (desc_size + sizeof(struct tlv_32_hdr));
		num_desc--;
	}
}
