/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
static void hal_tx_set_dscp_tid_map_8074(void *hal_soc, uint8_t *map,
					 uint8_t id)
{
	int i;
	uint32_t addr;
	uint32_t value;

	struct hal_soc *soc = (struct hal_soc *)hal_soc;

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
void hal_tx_update_dscp_tid_8074(void *hal_soc, uint8_t tid,
				 uint8_t id, uint8_t dscp)
{
	int index;
	uint32_t addr;
	uint32_t value;
	uint32_t regval;

	struct hal_soc *soc = (struct hal_soc *)hal_soc;

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
