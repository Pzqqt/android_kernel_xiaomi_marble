/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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

/**
 * @file cdp_txrx_mscs.h
 * @brief Define the host data path MSCS API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_MSCS_H_
#define _CDP_TXRX_MSCS_H_
#include "cdp_txrx_handle.h"
#ifdef WLAN_SUPPORT_MSCS
/**
 * @brief find MSCS enabled peer for this mac address and validate priority
 * @details
 *  This function checks if there is a peer for this mac adress with MSCS
 *  enabled flag set and nbuf priority is valid from user priority bitmap.
 *
 * @param src_mac - source mac address of peer
 * @param dst_mac - destination mac address of peer
 * @param nbuf - nbuf pointer
 * @return - 0 for non error case, 1 for failure
 */
static inline int
cdp_mscs_peer_lookup_n_get_priority(ol_txrx_soc_handle soc,
	uint8_t *src_mac, uint8_t *dst_mac,
	qdf_nbuf_t nbuf)
{
	if (!soc || !soc->ops || !soc->ops->mscs_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
				"%s invalid instance", __func__);
		return 1;
	}

	if (soc->ops->mscs_ops->mscs_peer_lookup_n_get_priority)
		return soc->ops->mscs_ops->mscs_peer_lookup_n_get_priority(soc,
						src_mac, dst_mac, nbuf);
	return 0;
}
#endif
#endif
