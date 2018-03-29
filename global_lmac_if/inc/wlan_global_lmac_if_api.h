/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_GLOBAL_LMAC_IF_API_H_
#define _WLAN_GLOBAL_LMAC_IF_API_H_

#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_psoc_obj.h"

/**
 * wlan_global_lmac_if_open() - global lmac_if open
 * @psoc: psoc context
 *
 * Opens up lmac_if southbound layer. This function calls OL,DA and UMAC
 * modules to register respective tx and rx callbacks.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_global_lmac_if_open(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_global_lmac_if_rx_ops_register() - UMAC rx handler register
 * @rx_ops: Pointer to rx_ops structure to be populated
 *
 * Register umac RX callabacks which will be called by DA/OL/WMA/WMI
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_global_lmac_if_rx_ops_register
		(struct wlan_lmac_if_rx_ops *rx_ops);

/**
 * wlan_global_lmac_if_close() - Close global lmac_if
 * @psoc: psoc context
 *
 * Deregister global lmac_if TX and RX handlers
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_global_lmac_if_close(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_global_lmac_if_set_txops_registration_cb() -tx
 * registration callback assignment
 * @dev_type: Dev type can be either Direct attach or Offload
 * @handler: handler to be called for LMAC tx ops registration
 *
 * API to assign appropriate tx registration callback handler based on the
 * device type(Offload or Direct attach)
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_global_lmac_if_set_txops_registration_cb(WLAN_DEV_TYPE dev_type,
		QDF_STATUS (*handler)(struct wlan_lmac_if_tx_ops *));

#ifdef WLAN_CONV_SPECTRAL_ENABLE
/**
 * wlan_lmac_if_sptrl_set_rx_ops_register_cb ()- Spectral LMAC Rx ops
 * registration callback assignment
 * @handler: Handler to be called for spectral LMAC rx ops registration
 *
 * API to assign appropriate Spectral LMAC rx ops registration callback handler
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_lmac_if_sptrl_set_rx_ops_register_cb(void (*handler)
				(struct wlan_lmac_if_rx_ops *));
#endif /* WLAN_CONV_SPECTRAL_ENABLE */
#endif /* _WLAN_LMAC_IF_API_H */
