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

#ifndef _WLAN_MGMT_TXRX_RX_REO_UTILS_API_H_
#define _WLAN_MGMT_TXRX_RX_REO_UTILS_API_H_

/**
 * DOC:  wlan_mgmt_txrx_rx_reo_utils_api.h
 *
 * management rx-reorder public APIs and structures
 * for umac converged components.
 *
 */

#include <wlan_mgmt_txrx_utils_api.h>

#ifdef WLAN_MGMT_RX_REO_SUPPORT
#define mgmt_rx_reo_alert mgmt_txrx_alert
#define mgmt_rx_reo_err mgmt_txrx_err
#define mgmt_rx_reo_warn mgmt_txrx_warn
#define mgmt_rx_reo_notice mgmt_txrx_notice
#define mgmt_rx_reo_info mgmt_txrx_info
#define mgmt_rx_reo_debug mgmt_txrx_debug
#endif /* WLAN_MGMT_RX_REO_SUPPORT*/
#endif /* _WLAN_MGMT_TXRX_RX_REO_UTILS_API_H_ */
