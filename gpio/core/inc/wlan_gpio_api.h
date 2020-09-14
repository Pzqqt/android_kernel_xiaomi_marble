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
 * DOC: wlan_gpio_api.h
 *
 * This header file provide API declarations required for gpio cfg
 * that called by other components
 */

#ifndef __WLAN_GPIO_CFG_API_H__
#define __WLAN_GPIO_CFG_API_H__

#include <qdf_types.h>

#ifdef WLAN_FEATURE_GPIO_CFG

/**
 * wlan_gpio_init() - API to init component
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_gpio_init(void);

/**
 * wlan_gpio_deinit() - API to deinit component
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_gpio_deinit(void);

#else
static inline
QDF_STATUS wlan_gpio_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS wlan_gpio_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_GPIO_CFG */
#endif /*__WLAN_GPIO_CFG_API_H__*/

