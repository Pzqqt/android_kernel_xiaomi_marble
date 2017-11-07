/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_SPECTRAL_UTILS_API_H_
#define _WLAN_SPECTRAL_UTILS_API_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_lmac_if_def.h>

/**
 * wlan_spectral_init(): API to init spectral component
 *
 * This API is invoked from dispatcher init during all component init.
 * This API will register all required handlers for pdev and peer object
 * create/delete notification.
 *
 * Return: SUCCESS,
 *         Failure
 */
QDF_STATUS wlan_spectral_init(void);

/**
 * wlan_spectral_deinit(): API to deinit spectral component
 *
 * This API is invoked from dispatcher deinit during all component deinit.
 * This API will unregister all registered handlers for pdev and peer object
 * create/delete notification.
 *
 * Return: SUCCESS,
 *         Failure
 */
QDF_STATUS wlan_spectral_deinit(void);

/**
 * wlan_lmac_if_sptrl_register_rx_ops(): Register lmac interface Rx operations
 * @rx_ops: Pointer to lmac interface Rx operations structure
 *
 * Return: None
 */
void wlan_lmac_if_sptrl_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);

#endif /* _WLAN_SPECTRAL_UTILS_API_H_*/
