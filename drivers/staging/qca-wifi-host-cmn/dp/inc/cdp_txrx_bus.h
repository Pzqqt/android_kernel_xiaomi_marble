/*
 * Copyright (c) 2016-2017, 2019-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_bus.h
 * @brief Define the host data path bus related functions
 */
#ifndef _CDP_TXRX_BUS_H_
#define _CDP_TXRX_BUS_H_

/**
 * cdp_bus_suspend() - suspend bus
 * @soc: data path soc handle
 * @pdev_id: id of dp pdev handle
 *
 * suspend bus
 *
 * return QDF_STATUS_SUCCESS suspend is not implemented or suspend done
 */
static inline QDF_STATUS cdp_bus_suspend(ol_txrx_soc_handle soc,
					 uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->bus_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->bus_ops->bus_suspend)
		return soc->ops->bus_ops->bus_suspend(soc, pdev_id);
	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_bus_resume() - resume bus
 * @soc: data path soc handle
 * @pdev_id: id of dp pdev handle
 *
 * resume bus
 *
 * return QDF_STATUS_SUCCESS resume is not implemented or suspend done
 */
static inline QDF_STATUS cdp_bus_resume(ol_txrx_soc_handle soc,
					uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->bus_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->bus_ops->bus_resume)
		return soc->ops->bus_ops->bus_resume(soc, pdev_id);
	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_process_wow_ack() - Process wow ack response
 * @soc: data path soc handle
 * @pdev_id: id of dp pdev handle
 *
 * Do any required data path operations for target wow ack
 * suspend response.
 *
 * Return: None
 */
static inline void cdp_process_wow_ack_rsp(ol_txrx_soc_handle soc,
					   uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->bus_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->bus_ops->process_wow_ack_rsp)
		return soc->ops->bus_ops->process_wow_ack_rsp(soc, pdev_id);
}

/**
 * cdp_process_target_suspend_req() - Process target suspend request
 * @soc: data path soc handle
 * @pdev_id: id of dp pdev handle
 *
 * Complete the datapath specific work before target suspend
 *
 * Return: None
 */
static inline void cdp_process_target_suspend_req(ol_txrx_soc_handle soc,
						  uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->bus_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->bus_ops->process_target_suspend_req)
		return soc->ops->bus_ops->process_target_suspend_req(soc,
								     pdev_id);
}
#endif /* _CDP_TXRX_BUS_H_ */
