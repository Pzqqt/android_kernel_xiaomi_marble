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
#ifndef __DP_BE_TX_H
#define __DP_BE_TX_H
/**
 *  DOC: dp_be_tx.h
 *
 * BE specific TX Datapath header file. Need not be exposed to common DP code.
 *
 */

#include <dp_types.h>
#include "dp_be.h"

struct __attribute__((__packed__)) dp_tx_comp_peer_id {
	uint16_t peer_id:13,
		 ml_peer_valid:1,
		 reserved:2;
};

/* Invalid TX Bank ID value */
#define DP_BE_INVALID_BANK_ID -1

/**
 * dp_tx_hw_enqueue_be() - Enqueue to TCL HW for transmit for BE target
 * @soc: DP Soc Handle
 * @vdev: DP vdev handle
 * @tx_desc: Tx Descriptor Handle
 * @fw_metadata: Metadata to send to Target Firmware along with frame
 * @tx_exc_metadata: Handle that holds exception path meta data
 * @msdu_info: msdu_info containing information about TX buffer
 *
 *  Gets the next free TCL HW DMA descriptor and sets up required parameters
 *  from software Tx descriptor
 *
 * Return: QDF_STATUS_SUCCESS: success
 *         QDF_STATUS_E_RESOURCES: Error return
 */
QDF_STATUS dp_tx_hw_enqueue_be(struct dp_soc *soc, struct dp_vdev *vdev,
			       struct dp_tx_desc_s *tx_desc,
				uint16_t fw_metadata,
				struct cdp_tx_exception_metadata *metadata,
				struct dp_tx_msdu_info_s *msdu_info);

/**
 * dp_tx_comp_get_params_from_hal_desc_be() - Get TX desc from HAL comp desc
 * @soc: DP soc handle
 * @tx_comp_hal_desc: HAL TX Comp Descriptor
 * @r_tx_desc: SW Tx Descriptor retrieved from HAL desc.
 *
 * Return: None
 */
void dp_tx_comp_get_params_from_hal_desc_be(struct dp_soc *soc,
					    void *tx_comp_hal_desc,
					    struct dp_tx_desc_s **r_tx_desc);

/**
 * dp_tx_init_bank_profiles() - Init TX bank profiles
 * @soc: DP soc handle
 *
 * Return: QDF_STATUS_SUCCESS or QDF error code.
 */
QDF_STATUS dp_tx_init_bank_profiles(struct dp_soc_be *soc);

/**
 * dp_tx_deinit_bank_profiles() - De-Init TX bank profiles
 * @soc: DP soc handle
 *
 * Return: None
 */
void dp_tx_deinit_bank_profiles(struct dp_soc_be *soc);

/**
 * dp_tx_get_bank_profile() - get TX bank profile for vdev
 * @soc: DP soc handle
 * @be_vdev: BE vdev pointer
 *
 * Return: bank profile allocated to vdev or DP_BE_INVALID_BANK_ID
 */
int dp_tx_get_bank_profile(struct dp_soc_be *soc,
			   struct dp_vdev_be *be_vdev);

/**
 * dp_tx_put_bank_profile() - release TX bank profile for vdev
 * @soc: DP soc handle
 *
 * Return: None
 */
void dp_tx_put_bank_profile(struct dp_soc_be *soc, struct dp_vdev_be *be_vdev);

/**
 * dp_tx_update_bank_profile() - release existing and allocate new bank profile
 * @soc: DP soc handle
 * @be_vdev: pointer to be_vdev structure
 *
 * The function releases the existing bank profile allocated to the vdev and
 * looks for a new bank profile based on updated dp_vdev TX params.
 *
 * Return: None
 */
void dp_tx_update_bank_profile(struct dp_soc_be *be_soc,
			       struct dp_vdev_be *be_vdev);

/**
 * dp_tx_desc_pool_init_be() - Initialize Tx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @num_elem: number of descriptor in pool
 * @pool_id: pool ID to allocate
 *
 * Return: QDF_STATUS_SUCCESS - success, others - failure
 */
QDF_STATUS dp_tx_desc_pool_init_be(struct dp_soc *soc,
				   uint16_t num_elem,
				   uint8_t pool_id);
/**
 * dp_tx_desc_pool_deinit_be() - De-initialize Tx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @tx_desc_pool: Tx descriptor pool handler
 * @pool_id: pool ID to deinit
 *
 * Return: None
 */
void dp_tx_desc_pool_deinit_be(struct dp_soc *soc,
			       struct dp_tx_desc_pool_s *tx_desc_pool,
			       uint8_t pool_id);

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * dp_tx_comp_nf_handler() - Tx completion ring Near full scenario handler
 * @int_ctx: Interrupt context
 * @soc: Datapath SoC handle
 * @hal_ring_hdl: TX completion ring handle
 * @ring_id: TX completion ring number
 * @quota: Quota of the work to be done
 *
 * Return: work done
 */
uint32_t dp_tx_comp_nf_handler(struct dp_intr *int_ctx, struct dp_soc *soc,
			       hal_ring_handle_t hal_ring_hdl, uint8_t ring_id,
			       uint32_t quota);
#else
static inline
uint32_t dp_tx_comp_nf_handler(struct dp_intr *int_ctx, struct dp_soc *soc,
			       hal_ring_handle_t hal_ring_hdl, uint8_t ring_id,
			       uint32_t quota)
{
	return 0;
}
#endif /* WLAN_FEATURE_NEAR_FULL_IRQ */
#endif
