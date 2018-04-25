/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_mon.h
 * @brief Define the monitor mode API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_MON_H_
#define _CDP_TXRX_MON_H_
#include "cdp_txrx_handle.h"
static inline void cdp_monitor_set_filter_ucast_data
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev, u_int8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_monitor_set_filter_ucast_data)
		return;

	soc->ops->mon_ops->txrx_monitor_set_filter_ucast_data
			(pdev, val);
}

static inline void cdp_monitor_set_filter_mcast_data
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev, u_int8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_monitor_set_filter_mcast_data)
		return;

	soc->ops->mon_ops->txrx_monitor_set_filter_mcast_data
			(pdev, val);
}

static inline void cdp_monitor_set_filter_non_data
	(ol_txrx_soc_handle soc, struct cdp_pdev *pdev, u_int8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_monitor_set_filter_non_data)
		return;

	soc->ops->mon_ops->txrx_monitor_set_filter_non_data
			(pdev, val);
}

static inline bool cdp_monitor_get_filter_ucast_data
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev_txrx_handle)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_monitor_get_filter_ucast_data)
		return 0;

	return soc->ops->mon_ops->txrx_monitor_get_filter_ucast_data
			(vdev_txrx_handle);
}

static inline bool cdp_monitor_get_filter_mcast_data
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev_txrx_handle)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_monitor_get_filter_mcast_data)
		return 0;

	return soc->ops->mon_ops->txrx_monitor_get_filter_mcast_data
			(vdev_txrx_handle);
}

static inline bool cdp_monitor_get_filter_non_data
(ol_txrx_soc_handle soc, struct cdp_vdev *vdev_txrx_handle)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_monitor_get_filter_non_data)
		return 0;

	return soc->ops->mon_ops->txrx_monitor_get_filter_non_data
			(vdev_txrx_handle);
}

static inline int cdp_reset_monitor_mode
(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->mon_ops ||
	    !soc->ops->mon_ops->txrx_reset_monitor_mode)
		return 0;

	return soc->ops->mon_ops->txrx_reset_monitor_mode(pdev);
}
#endif
