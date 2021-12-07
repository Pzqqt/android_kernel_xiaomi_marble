/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _DP_BE_RX_H_
#define _DP_BE_RX_H_

#include <dp_types.h>
#include "dp_be.h"

#ifndef QCA_HOST_MODE_WIFI_DISABLED

/*
 * dp_rx_intrabss_fwd_be() - API for intrabss fwd. For EAPOL
 *  pkt with DA not equal to vdev mac addr, fwd is not allowed.
 * @soc: core txrx main context
 * @ta_peer: source peer entry
 * @rx_tlv_hdr: start address of rx tlvs
 * @nbuf: nbuf that has to be intrabss forwarded
 * @msdu_metadata: msdu metadata
 *
 * Return: true if it is forwarded else false
 */

bool dp_rx_intrabss_fwd_be(struct dp_soc *soc,
			   struct dp_peer *ta_peer,
			   uint8_t *rx_tlv_hdr,
			   qdf_nbuf_t nbuf,
			   struct hal_rx_msdu_metadata msdu_metadata);
#endif

uint32_t dp_rx_process_be(struct dp_intr *int_ctx,
			  hal_ring_handle_t hal_ring_hdl, uint8_t reo_ring_num,
			  uint32_t quota);

/**
 * dp_rx_desc_pool_init_be() - Initialize Rx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @rx_desc_pool: Rx descriptor pool handler
 * @pool_id: Rx descriptor pool ID
 *
 * Return: QDF_STATUS_SUCCESS - succeeded, others - failed
 */
QDF_STATUS dp_rx_desc_pool_init_be(struct dp_soc *soc,
				   struct rx_desc_pool *rx_desc_pool,
				   uint32_t pool_id);

/**
 * dp_rx_desc_pool_deinit_be() - De-initialize Rx Descriptor pool(s)
 * @soc: Handle to DP Soc structure
 * @rx_desc_pool: Rx descriptor pool handler
 * @pool_id: Rx descriptor pool ID
 *
 * Return: None
 */
void dp_rx_desc_pool_deinit_be(struct dp_soc *soc,
			       struct rx_desc_pool *rx_desc_pool,
			       uint32_t pool_id);

/**
 * dp_wbm_get_rx_desc_from_hal_desc_be() - Get corresponding Rx Desc
 *					address from WBM ring Desc
 * @soc: Handle to DP Soc structure
 * @ring_desc: ring descriptor structure pointer
 * @r_rx_desc: pointer to a pointer of Rx Desc
 *
 * Return: QDF_STATUS_SUCCESS - succeeded, others - failed
 */
QDF_STATUS dp_wbm_get_rx_desc_from_hal_desc_be(struct dp_soc *soc,
					       void *ring_desc,
					       struct dp_rx_desc **r_rx_desc);

/**
 * dp_rx_desc_cookie_2_va_be() - Convert RX Desc cookie ID to VA
 * @soc:Handle to DP Soc structure
 * @cookie: cookie used to lookup virtual address
 *
 * Return: Rx descriptor virtual address
 */
struct dp_rx_desc *dp_rx_desc_cookie_2_va_be(struct dp_soc *soc,
					     uint32_t cookie);

#if !defined(DP_FEATURE_HW_COOKIE_CONVERSION) || \
		defined(DP_HW_COOKIE_CONVERT_EXCEPTION)
/**
 * dp_rx_desc_sw_cc_check() - check if RX desc VA is got correctly,
			      if not, do SW cookie conversion.
 * @soc:Handle to DP Soc structure
 * @rx_buf_cookie: RX desc cookie ID
 * @r_rx_desc: double pointer for RX desc
 *
 * Return: None
 */
static inline void
dp_rx_desc_sw_cc_check(struct dp_soc *soc,
		       uint32_t rx_buf_cookie,
		       struct dp_rx_desc **r_rx_desc)
{
	if (qdf_unlikely(!(*r_rx_desc))) {
		*r_rx_desc = (struct dp_rx_desc *)
				dp_cc_desc_find(soc,
						rx_buf_cookie);
	}
}
#else
static inline void
dp_rx_desc_sw_cc_check(struct dp_soc *soc,
		       uint32_t rx_buf_cookie,
		       struct dp_rx_desc **r_rx_desc)
{
}
#endif /* DP_FEATURE_HW_COOKIE_CONVERSION && DP_HW_COOKIE_CONVERT_EXCEPTION */

#define DP_PEER_METADATA_OFFLOAD_GET_BE(_peer_metadata)		(0)

#ifdef DP_USE_REDUCED_PEER_ID_FIELD_WIDTH
static inline uint16_t
dp_rx_peer_metadata_peer_id_get_be(struct dp_soc *soc, uint32_t peer_metadata)
{
	struct htt_rx_peer_metadata_v1 *metadata =
			(struct htt_rx_peer_metadata_v1 *)&peer_metadata;
	uint16_t peer_id;

	peer_id = metadata->peer_id |
		  (metadata->ml_peer_valid << soc->peer_id_shift);

	return peer_id;
}
#else
/* Combine ml_peer_valid and peer_id field */
#define DP_BE_PEER_METADATA_PEER_ID_MASK	0x00003fff
#define DP_BE_PEER_METADATA_PEER_ID_SHIFT	0

static inline uint16_t
dp_rx_peer_metadata_peer_id_get_be(struct dp_soc *soc, uint32_t peer_metadata)
{
	return ((peer_metadata & DP_BE_PEER_METADATA_PEER_ID_MASK) >>
		DP_BE_PEER_METADATA_PEER_ID_SHIFT);
}
#endif

static inline uint16_t
dp_rx_peer_metadata_vdev_id_get_be(struct dp_soc *soc, uint32_t peer_metadata)
{
	struct htt_rx_peer_metadata_v1 *metadata =
			(struct htt_rx_peer_metadata_v1 *)&peer_metadata;

	return metadata->vdev_id;
}

#ifdef WLAN_FEATURE_NEAR_FULL_IRQ
/**
 * dp_rx_nf_process() - Near Full state handler for RX rings.
 * @int_ctx: interrupt context
 * @hal_ring_hdl: Rx ring handle
 * @reo_ring_num: RX ring number
 * @quota: Quota of work to be done
 *
 * Return: work done in the handler
 */
uint32_t dp_rx_nf_process(struct dp_intr *int_ctx,
			  hal_ring_handle_t hal_ring_hdl,
			  uint8_t reo_ring_num,
			  uint32_t quota);
#else
static inline
uint32_t dp_rx_nf_process(struct dp_intr *int_ctx,
			  hal_ring_handle_t hal_ring_hdl,
			  uint8_t reo_ring_num,
			  uint32_t quota)
{
	return 0;
}
#endif /*WLAN_FEATURE_NEAR_FULL_IRQ */

#if defined(WLAN_FEATURE_11BE_MLO) && defined(WLAN_MLO_MULTI_CHIP)
struct dp_soc *
dp_rx_replensih_soc_get(struct dp_soc *soc, uint8_t reo_ring_num);
#else
static inline struct dp_soc *
dp_rx_replensih_soc_get(struct dp_soc *soc, uint8_t reo_ring_num)
{
	return soc;
}
#endif
#endif
