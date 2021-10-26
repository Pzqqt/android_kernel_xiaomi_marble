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

#ifndef _DP_LI_RX_H_
#define _DP_LI_RX_H_

#include <dp_types.h>
#include <dp_rx.h>
#include "dp_li.h"

uint32_t dp_rx_process_li(struct dp_intr *int_ctx,
			  hal_ring_handle_t hal_ring_hdl, uint8_t reo_ring_num,
			  uint32_t quota);

/**
 * dp_rx_desc_pool_init_li() - Initialize Rx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @rx_desc_pool: Rx descriptor pool handler
 * @pool_id: Rx descriptor pool ID
 *
 * Return: None
 */
QDF_STATUS dp_rx_desc_pool_init_li(struct dp_soc *soc,
				   struct rx_desc_pool *rx_desc_pool,
				   uint32_t pool_id);

/**
 * dp_rx_desc_pool_deinit_li() - De-initialize Rx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @rx_desc_pool: Rx descriptor pool handler
 * @pool_id: Rx descriptor pool ID
 *
 * Return: None
 */
void dp_rx_desc_pool_deinit_li(struct dp_soc *soc,
			       struct rx_desc_pool *rx_desc_pool,
			       uint32_t pool_id);

/**
 * dp_wbm_get_rx_desc_from_hal_desc_li() - Get corresponding Rx Desc
 *					address from WBM ring Desc
 * @soc: Handle to DP Soc structure
 * @ring_desc: ring descriptor structure pointer
 * @r_rx_desc: pointer to a pointer of Rx Desc
 *
 * Return: QDF_STATUS_SUCCESS - succeeded, others - failed
 */
QDF_STATUS dp_wbm_get_rx_desc_from_hal_desc_li(
					struct dp_soc *soc,
					void *ring_desc,
					struct dp_rx_desc **r_rx_desc);

/**
 * dp_rx_desc_cookie_2_va_li() - Convert RX Desc cookie ID to VA
 * @soc:Handle to DP Soc structure
 * @cookie: cookie used to lookup virtual address
 *
 * Return: Rx descriptor virtual address
 */
static inline
struct dp_rx_desc *dp_rx_desc_cookie_2_va_li(struct dp_soc *soc,
					     uint32_t cookie)
{
	return dp_rx_cookie_2_va_rxdma_buf(soc, cookie);
}

#define DP_PEER_METADATA_VDEV_ID_MASK	0x003f0000
#define DP_PEER_METADATA_VDEV_ID_SHIFT	16
#define DP_PEER_METADATA_OFFLOAD_MASK	0x01000000
#define DP_PEER_METADATA_OFFLOAD_SHIFT	24

#define DP_PEER_METADATA_VDEV_ID_GET_LI(_peer_metadata)		\
	(((_peer_metadata) & DP_PEER_METADATA_VDEV_ID_MASK)	\
			>> DP_PEER_METADATA_VDEV_ID_SHIFT)

#define DP_PEER_METADATA_OFFLOAD_GET_LI(_peer_metadata)		\
	(((_peer_metadata) & DP_PEER_METADATA_OFFLOAD_MASK)	\
			>> DP_PEER_METADATA_OFFLOAD_SHIFT)

static inline uint16_t
dp_rx_peer_metadata_peer_id_get_li(struct dp_soc *soc, uint32_t peer_metadata)
{
	struct htt_rx_peer_metadata_v0 *metadata =
			(struct htt_rx_peer_metadata_v0 *)&peer_metadata;

	return metadata->peer_id;
}
#endif
