/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "cdp_txrx_mon_struct.h"
#include "qdf_trace.h"
#include "hal_rx.h"
#include "hal_tx.h"
#include "dp_types.h"
#include "hal_api_mon.h"

/**
 * hal_tx_desc_set_dscp_tid_table_id_8074() - Sets DSCP to TID conversion
 *						table ID
 * @desc: Handle to Tx Descriptor
 * @id: DSCP to tid conversion table to be used for this frame
 *
 * Return: void
 */

static void hal_tx_desc_set_dscp_tid_table_id_8074(void *desc, uint8_t id)
{
	HAL_SET_FLD(desc, TCL_DATA_CMD_3,
		    DSCP_TO_TID_PRIORITY_TABLE_ID) |=
	HAL_TX_SM(TCL_DATA_CMD_3,
		  DSCP_TO_TID_PRIORITY_TABLE_ID, id);
}

/**
 * hal_tx_set_dscp_tid_map_8074() - Configure default DSCP to TID map table
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
static void hal_tx_set_dscp_tid_map_8074(struct hal_soc *soc, uint8_t *map,
					 uint8_t id)
{
	int i;
	uint32_t addr;
	uint32_t value;

	if (id == HAL_TX_DSCP_TID_MAP_TABLE_DEFAULT) {
		addr = HWIO_TCL_R0_DSCP_TID1_MAP_0_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);
	} else {
		addr = HWIO_TCL_R0_DSCP_TID2_MAP_0_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);
	}

	for (i = 0; i < 64; i += 10) {
		value =
		  (map[i] |
		  (map[i + 1] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_1_SHFT) |
		  (map[i + 2] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_2_SHFT) |
		  (map[i + 3] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_3_SHFT) |
		  (map[i + 4] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_4_SHFT) |
		  (map[i + 5] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_5_SHFT) |
		  (map[i + 6] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_6_SHFT) |
		  (map[i + 7] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_7_SHFT) |
		  (map[i + 8] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_8_SHFT) |
		  (map[i + 9] << HWIO_TCL_R0_DSCP_TID1_MAP_0_DSCP_9_SHFT));

		HAL_REG_WRITE(soc, addr,
			      (value & HWIO_TCL_R0_DSCP_TID1_MAP_1_RMSK));

		addr += 4;
	}
}

/**
 * hal_tx_update_dscp_tid_8074() - Update the dscp tid map table as
					updated by user
 * @soc: HAL SoC context
 * @map: DSCP-TID mapping table
 * @id : MAP ID
 * @dscp: DSCP_TID map index
 *
 * Return: void
 */
static
void hal_tx_update_dscp_tid_8074(struct hal_soc *soc, uint8_t tid,
				 uint8_t id, uint8_t dscp)
{
	int index;
	uint32_t addr;
	uint32_t value;
	uint32_t regval;

	if (id == HAL_TX_DSCP_TID_MAP_TABLE_DEFAULT)
		addr = HWIO_TCL_R0_DSCP_TID1_MAP_0_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);
	else
		addr = HWIO_TCL_R0_DSCP_TID2_MAP_0_ADDR(
				SEQ_WCSS_UMAC_MAC_TCL_REG_OFFSET);

	index = dscp % HAL_TX_NUM_DSCP_PER_REGISTER;
	addr += 4 * (dscp / HAL_TX_NUM_DSCP_PER_REGISTER);
	value = tid << (HAL_TX_BITS_PER_TID * index);

	/* Read back previous DSCP TID config and update
	 * with new config.
	 */
	regval = HAL_REG_READ(soc, addr);
	regval &= ~(HAL_TX_TID_BITS_MASK << (HAL_TX_BITS_PER_TID * index));
	regval |= value;

	HAL_REG_WRITE(soc, addr,
		      (regval & HWIO_TCL_R0_DSCP_TID1_MAP_1_RMSK));
}

/**
 * hal_tx_desc_set_lmac_id - Set the lmac_id value
 * @desc: Handle to Tx Descriptor
 * @lmac_id: mac Id to ast matching
 *		     b00 – mac 0
 *		     b01 – mac 1
 *		     b10 – mac 2
 *		     b11 – all macs (legacy HK way)
 *
 * Return: void
 */
static void hal_tx_desc_set_lmac_id_8074(void *desc, uint8_t lmac_id)
{
}
