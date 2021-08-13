/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/**
 *  DOC: target_if_mgmt_txrx_rx_reo.h
 *  This file contains declarations of management rx re-ordering related APIs.
 */

#ifndef _TARGET_IF_MGMT_TXRX_RX_REO_H_
#define _TARGET_IF_MGMT_TXRX_RX_REO_H_

#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>

#ifdef WLAN_MGMT_RX_REO_SUPPORT
/**
 * target_if_mgmt_rx_reo_register_event_handlers() - Register management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mgmt_rx_reo_register_event_handlers(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_mgmt_rx_reo_unregister_event_handlers() - Unregister management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_mgmt_rx_reo_unregister_event_handlers(struct wlan_objmgr_psoc *psoc);
#else
/**
 * target_if_mgmt_rx_reo_register_event_handlers() - Register management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_register_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_mgmt_rx_reo_unregister_event_handlers() - Unregister management
 * rx-reordering related event handlers.
 * @psoc: psoc object
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
target_if_mgmt_rx_reo_unregister_event_handlers(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _TARGET_IF_MGMT_TXRX_RX_REO_H_ */
