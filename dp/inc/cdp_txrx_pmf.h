/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#ifndef _CDP_TXRX_PMF_H_
#define _CDP_TXRX_PMF_H_

/**
 * cdp_get_pn_info() - Returns pn info from peer
 * @soc - data path soc handle
 * @peer: handle to peer
 * @last_pn_valid: return last_rmf_pn_valid value from peer.
 * @last_pn: return last_rmf_pn value from peer.
 * @rmf_pn_replays: return rmf_pn_replays value from peer.
 *
 * Return: NONE
 */
static inline void
cdp_get_pn_info(ol_txrx_soc_handle soc, void *peer, uint8_t **last_pn_valid,
		    uint64_t **last_pn, uint32_t **rmf_pn_replays)
{
	if (!soc || !soc->ops || !soc->ops->pmf_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->pmf_ops->get_pn_info)
		return soc->ops->pmf_ops->get_pn_info(
			peer, last_pn_valid, last_pn, rmf_pn_replays);

	return;
}
#endif /* _CDP_TXRX_PMF_H_ */
