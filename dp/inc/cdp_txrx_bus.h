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
 * @file cdp_txrx_bus.h
 * @brief Define the host data path bus related functions
 */
#ifndef _CDP_TXRX_BUS_H_
#define _CDP_TXRX_BUS_H_

/**
 * cdp_bus_suspend() - suspend bus
 * @soc - data path soc handle
 *
 * suspend bus
 *
 * return QDF_STATUS_SUCCESS suspend is not implemented or suspend done
 */
static inline QDF_STATUS cdp_bus_suspend(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->bus_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->bus_ops->bus_suspend)
		return soc->ops->bus_ops->bus_suspend();
	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * cdp_bus_resume() - resume bus
 * @soc - data path soc handle
 *
 * resume bus
 *
 * return QDF_STATUS_SUCCESS resume is not implemented or suspend done
 */
static inline QDF_STATUS cdp_bus_resume(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->bus_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->bus_ops->bus_resume)
		return soc->ops->bus_ops->bus_resume();
	return QDF_STATUS_E_NOSUPPORT;
}

#endif /* _CDP_TXRX_BUS_H_ */
