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
 * DOC: wlan_gpio_priv_api.h
 *
 * This header file provide API declarations required for gpio cfg
 * that called internally
 */

#ifndef __WLAN_GPIO_CFG_PRIV_API_H__
#define __WLAN_GPIO_CFG_PRIV_API_H__

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_lmac_if_def.h>
#include <qdf_lock.h>

#define gpio_debug(args ...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_GPIO, ## args)
#define gpio_err(args ...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_GPIO, ## args)

/**
 * struct gpio_psoc_priv_obj - psoc private object
 * @lock: qdf spin lock
 * @soc: pointer to psoc object
 */
struct gpio_psoc_priv_obj {
	qdf_spinlock_t lock;
	struct wlan_objmgr_psoc *soc;
};

/**
 * gpio_get_psoc_priv_obj() - get priv object from psoc object
 * @psoc: pointer to psoc object
 *
 * Return: pointer to gpio psoc private object
 */
static inline
struct gpio_psoc_priv_obj *
gpio_get_psoc_priv_obj(struct wlan_objmgr_psoc *psoc)
{
	struct gpio_psoc_priv_obj *obj;

	if (!psoc)
		return NULL;
	obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_UMAC_COMP_GPIO);

	return obj;
}

/**
 * wlan_psoc_get_gpio_txops() - get TX ops from the private object
 * @psoc: pointer to psoc object
 *
 * Return: pointer to TX op callback
 */

static inline struct wlan_lmac_if_gpio_tx_ops *
wlan_psoc_get_gpio_txops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		gpio_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->gpio_ops;
}
#endif /*__WLAN_GPIO_CFG_PRIV_API_H__*/
