/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_flow_ctrl_v2.h
 * @brief Define the host data path flow control version 2 API
 * functions
 */
#ifndef _CDP_TXRX_FC_V2_H_
#define _CDP_TXRX_FC_V2_H_
#include <cdp_txrx_ops.h>

/**
 * cdp_register_pause_cb() - Register flow control callback function pointer
 * @soc - data path soc handle
 * @pause_cb - Pause callback intend to register
 *
 * Register flow control callback function pointer and client context pointer
 *
 * return QDF_STATUS_SUCCESS success
 */
static inline QDF_STATUS
cdp_register_pause_cb(ol_txrx_soc_handle soc,
		tx_pause_callback pause_cb)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->register_pause_cb)
		return QDF_STATUS_SUCCESS;

	return soc->ops->flowctl_ops->register_pause_cb(soc, pause_cb);

}

/**
 * cdp_set_desc_global_pool_size() - set global device pool size
 * @soc - data path soc handle
 * @num_msdu_desc - descriptor pool size
 *
 * set global device pool size
 *
 * return none
 */
static inline void
cdp_set_desc_global_pool_size(ol_txrx_soc_handle soc,
		uint32_t num_msdu_desc)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->set_desc_global_pool_size)
		return;

	soc->ops->flowctl_ops->set_desc_global_pool_size(
			num_msdu_desc);
}

/**
 * cdp_dump_flow_pool_info() - dump flow pool information
 * @soc - data path soc handle
 *
 * dump flow pool information
 *
 * return none
 */
static inline void
cdp_dump_flow_pool_info(struct cdp_soc_t *soc)
{
	void *dp_soc = (void *)soc;

	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->flowctl_ops ||
	    !soc->ops->flowctl_ops->dump_flow_pool_info)
		return;

	soc->ops->flowctl_ops->dump_flow_pool_info(dp_soc);
}
#endif /* _CDP_TXRX_FC_V2_H_ */
