/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_me.h
 * @brief Define the host data path mcast enhance API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_ME_H_
#define _CDP_TXRX_ME_H_

#include <cdp_txrx_ops.h>
/* TODO: adf need to be replaced with qdf */
#include "cdp_txrx_handle.h"

static inline u_int16_t
cdp_tx_desc_alloc_and_mark_for_mcast_clone(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev, u_int16_t buf_count)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->tx_desc_alloc_and_mark_for_mcast_clone)
		return 0;

	return soc->ops->me_ops->
			tx_desc_alloc_and_mark_for_mcast_clone
			(pdev, buf_count);
}

static inline u_int16_t
cdp_tx_desc_free_and_unmark_for_mcast_clone(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev, u_int16_t buf_count)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->tx_desc_free_and_unmark_for_mcast_clone)
		return 0;

	return soc->ops->me_ops->
			tx_desc_free_and_unmark_for_mcast_clone
			(pdev, buf_count);
}

static inline u_int16_t
cdp_tx_get_mcast_buf_allocated_marked(ol_txrx_soc_handle soc,
	struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->tx_get_mcast_buf_allocated_marked)
		return 0;

	return soc->ops->me_ops->tx_get_mcast_buf_allocated_marked
			(pdev);
}

static inline void
cdp_tx_me_alloc_descriptor(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->tx_me_alloc_descriptor)
		return;

	soc->ops->me_ops->tx_me_alloc_descriptor(pdev);
}

static inline void
cdp_tx_me_free_descriptor(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->tx_me_free_descriptor)
		return;

	soc->ops->me_ops->tx_me_free_descriptor(pdev);
}

static inline uint16_t cdp_tx_me_convert_ucast(
	ol_txrx_soc_handle soc, uint8_t vdev_id,
	qdf_nbuf_t wbuf, u_int8_t newmac[][6], uint8_t newmaccnt)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->tx_me_convert_ucast)
		return 0;

	return soc->ops->me_ops->tx_me_convert_ucast
			(soc, vdev_id, wbuf, newmac, newmaccnt);
}

/* Should be a function pointer in ol_txrx_osif_ops{} */
/**
 * @brief notify mcast frame indication from FW.
 * @details
 *      This notification will be used to convert
 *      multicast frame to unicast.
 *
 * @param pdev - handle to the ctrl SW's physical device object
 * @param vdev_id - ID of the virtual device received the special data
 * @param msdu - the multicast msdu returned by FW for host inspect
 */

static inline int cdp_mcast_notify(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, u_int8_t vdev_id, qdf_nbuf_t msdu)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->me_ops ||
	    !soc->ops->me_ops->mcast_notify)
		return 0;

	return soc->ops->me_ops->mcast_notify(pdev, vdev_id, msdu);
}
#endif
