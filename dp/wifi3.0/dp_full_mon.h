/**
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifndef _DP_FULL_MON_H_
#define _DP_FULL_MON_H_

#define DP_RX_MON_PPDU_ID_WRAP 32535
/**
 * struct dp_mon_mpdu () - DP Monitor mpdu object
 *
 * @head: Head msdu
 * @tail: Tail msdu
 * @mpdu_list_elem: mpdu list element
 * @rs_flags: Rx status flags
 * @ant_signal_db: RSSI in dBm
 * @is_stbc: is stbc is enabled
 * @sgi: SGI
 * @beamformed: if beamformed
 */
struct dp_mon_mpdu {
	qdf_nbuf_t head;
	qdf_nbuf_t tail;
	TAILQ_ENTRY(dp_mon_mpdu) mpdu_list_elem;

	uint8_t  rs_flags;
	uint8_t  ant_signal_db;
	uint8_t  is_stbc;
	uint8_t  sgi;
	uint8_t  beamformed;
};

static inline QDF_STATUS
dp_rx_mon_is_rxdma_error(struct hal_rx_mon_desc_info *desc_info)
{
	enum hal_rxdma_error_code rxdma_err = desc_info->rxdma_error_code;

	if (qdf_unlikely(desc_info->rxdma_push_reason ==
				HAL_RX_WBM_RXDMA_PSH_RSN_ERROR)) {
		if (qdf_unlikely((rxdma_err == HAL_RXDMA_ERR_FLUSH_REQUEST) ||
				 (rxdma_err == HAL_RXDMA_ERR_MPDU_LENGTH) ||
				 (rxdma_err == HAL_RXDMA_ERR_OVERFLOW))) {
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_E_FAILURE;
}
#endif /* _DP_FULL_MON_H_ */
