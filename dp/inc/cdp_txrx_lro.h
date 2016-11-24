/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */
 /**
 * @file cdp_txrx_lro.h
 * @brief Define the host data path Large Receive Offload API
 * functions
 */
#ifndef _CDP_TXRX_LRO_H_
#define _CDP_TXRX_LRO_H_
/**
 * cdp_register_lro_flush_cb() - register lro flsu cb function pointer
 * @soc - data path soc handle
 * @pdev - device instance pointer
 *
 * register lro flush callback function pointer
 *
 * return none
 */
static inline void cdp_register_lro_flush_cb(ol_txrx_soc_handle soc,
		void (lro_flush_cb)(void *), void *(lro_init_cb)(void))
{
	if (!soc || !soc->ops || !soc->ops->lro_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->lro_ops->register_lro_flush_cb)
		return soc->ops->lro_ops->register_lro_flush_cb(lro_flush_cb,
			lro_init_cb);

	return;
}
/**
 * cdp_deregister_lro_flush_cb() - deregister lro flsu cb function pointer
 * @soc - data path soc handle
 *
 * deregister lro flush callback function pointer
 *
 * return none
 */
static inline void cdp_deregister_lro_flush_cb(ol_txrx_soc_handle soc,
		void (lro_deinit_cb)(void *))
{
	if (!soc || !soc->ops || !soc->ops->lro_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->lro_ops->deregister_lro_flush_cb)
		return soc->ops->lro_ops->deregister_lro_flush_cb(
			lro_deinit_cb);

	return;
}

#endif /* _CDP_TXRX_LRO_H_ */
