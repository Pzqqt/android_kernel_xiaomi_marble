/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
#include "hal_hw_headers.h"
#include "hal_reo.h"
#include "qdf_module.h"

void hal_reo_init_cmd_ring(hal_soc_handle_t hal_soc_hdl,
			   hal_ring_handle_t hal_ring_hdl)
{
	int cmd_num;
	uint32_t *desc_addr;
	struct hal_srng_params srng_params;
	uint32_t desc_size;
	uint32_t num_desc;
	struct hal_soc *soc = (struct hal_soc *)hal_soc_hdl;
	uint8_t tlv_hdr_size;

	hal_get_srng_params(hal_soc_hdl, hal_ring_hdl, &srng_params);

	desc_addr = (uint32_t *)(srng_params.ring_base_vaddr);
	tlv_hdr_size = hal_get_tlv_hdr_size(hal_soc_hdl);
	desc_addr += HAL_GET_NUM_DWORDS(tlv_hdr_size);
	desc_size = HAL_GET_NUM_DWORDS(hal_srng_get_entrysize(soc, REO_CMD));
	num_desc = srng_params.num_entries;
	cmd_num = 1;
	while (num_desc) {
		/* Offsets of descriptor fields defined in HW headers start
		 * from the field after TLV header */
		HAL_DESC_SET_FIELD(desc_addr, HAL_UNIFORM_REO_CMD_HEADER,
				   REO_CMD_NUMBER, cmd_num);
		desc_addr += desc_size;
		num_desc--; cmd_num++;
	}

	soc->reo_res_bitmap = 0;
}
qdf_export_symbol(hal_reo_init_cmd_ring);
