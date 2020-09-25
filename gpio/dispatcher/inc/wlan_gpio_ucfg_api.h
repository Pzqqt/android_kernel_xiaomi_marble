/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_gpio_ucfg_api.h
 *
 * This header file maintain API declaration required for northbound interaction
 */

#ifndef __WLAN_GPIO_CFG_UCFG_API_H__
#define __WLAN_GPIO_CFG_UCFG_API_H__

#include <qdf_status.h>
#include <wmi_unified_param.h>
struct wlan_objmgr_psoc;

#ifdef WLAN_FEATURE_GPIO_CFG

/**
 * ucfg_set_gpio_config() - API to set gpio config
 * @psoc: the pointer of psoc object
 * @param: the pointer of gpio configuration info
 *
 * Return:QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS ucfg_set_gpio_config(struct wlan_objmgr_psoc *psoc,
				struct gpio_config_params *param);

/**
 * ucfg_set_gpio_output() - API to set gpio output
 * @psoc: the pointer of psoc object
 * @param: the pointer of gpio output info
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS ucfg_set_gpio_output(struct wlan_objmgr_psoc *psoc,
				struct gpio_output_params *param);
#endif /* WLAN_FEATURE_GPIO_CFG */
#endif /* __WLAN_GPIO_CFG_UCFG_API_H__ */
