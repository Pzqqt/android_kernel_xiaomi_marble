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
#ifndef __DP_LI_TX_H
#define __DP_LI_TX_H

#include <dp_types.h>

/**
 * dp_tx_hw_enqueue_li() - Enqueue to TCL HW for transmit
 * @soc: DP Soc Handle
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 * @tid: TID from HLOS for overriding default DSCP-TID mapping
 * @fw_metadata: Metadata to send to Target Firmware along with frame
 * @ring_id: Ring ID of H/W ring to which we enqueue the packet
 * @tx_exc_metadata: Handle that holds exception path meta data
 *
 *  Gets the next free TCL HW DMA descriptor and sets up required parameters
 *  from software Tx descriptor
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS
dp_tx_hw_enqueue_li(struct dp_soc *soc, struct dp_vdev *vdev,
		    struct dp_tx_desc_s *tx_desc, uint16_t fw_metadata,
		    struct cdp_tx_exception_metadata *tx_exc_metadata,
		    struct dp_tx_msdu_info_s *msdu_info);
/**
 * dp_tx_comp_get_params_from_hal_desc_li() - Get TX desc from HAL comp desc
 * @soc: DP soc handle
 * @tx_comp_hal_desc: HAL TX Comp Descriptor
 * @r_tx_desc: SW Tx Descriptor retrieved from HAL desc.
 *
 * Return: None
 */
void dp_tx_comp_get_params_from_hal_desc_li(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc);

/**
 * dp_tx_desc_pool_init_li() - Initialize Tx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @num_elem: pool descriptor number
 * @pool_id: pool to allocate
 *
 * Return: QDF_STATUS_SUCCESS - success, others - failure
 */
QDF_STATUS dp_tx_desc_pool_init_li(struct dp_soc *soc,
				   uint16_t num_elem,
				   uint8_t pool_id);

/**
 * dp_tx_desc_pool_deinit_li() - De-initialize Tx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @tx_desc_pool: Tx descriptor pool handler
 * @pool_id: pool to deinit
 *
 * Return: None.
 */
void dp_tx_desc_pool_deinit_li(struct dp_soc *soc,
			       struct dp_tx_desc_pool_s *tx_desc_pool,
			       uint8_t pool_id);
#endif
