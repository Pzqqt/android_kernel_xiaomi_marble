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
 * DOC: wlan_gpio_tgt_api.h
 *
 * This header file provide with API declarations to interface with Southbound
 */
#ifndef __WLAN_GPIO_CFG_TGT_API_H__
#define __WLAN_GPIO_CFG_TGT_API_H__

#ifdef WLAN_FEATURE_GPIO_CFG
#include <qdf_status.h>
#include <wmi_unified_param.h>
struct wlan_objmgr_psoc;

/**
 * tgt_set_gpio_config_req(): API to set GPIO configuration to lmac
 * @psoc: the pointer to psoc object manager
 * @param: the pointer to gpio cfg info
 *
 * Return: status of operation
 */
QDF_STATUS
tgt_set_gpio_config_req(struct wlan_objmgr_psoc *psoc,
			struct gpio_config_params *param);

/**
 * tgt_set_gpio_output_req(): API to set GPIO output info to lmac
 * @psoc: the pointer to psoc object manager
 * @param: the pointer to gpio output info
 *
 * Return: status of operation
 */

QDF_STATUS
tgt_set_gpio_output_req(struct wlan_objmgr_psoc *psoc,
			struct gpio_output_params *param);

/**
 * tgt_if_gpio_config() - API to send gpio config request
 * @psoc: pointer to psoc object
 * @gpio_num: gpio pin number
 * @input: enable/disable the gpio pin
 * @pull_type: gpio pull type
 * @intr_mode: gpio interrupt mode
 * @mux_config_val: gpio MUX value
 * @drive: gpio drive
 * @init_enable: gpio init_enable
 *
 * Return: status of operation
 */
QDF_STATUS tgt_gpio_config(struct wlan_objmgr_psoc *psoc, uint32_t gpio_num,
			   uint32_t input, uint32_t pull_type,
			   uint32_t intr_mode,  uint32_t mux_config_val,
			   uint32_t drive, uint32_t init_enable);
/**
 * tgt_if_gpio_output() - API to send gpio output request
 * @psoc: pointer to psoc object
 * @gpio_num: gpio pin number
 * @set: enable/disable the gpio pin
 *
 * Return: status of operation
 */
QDF_STATUS tgt_gpio_output(struct wlan_objmgr_psoc *psoc, uint32_t gpio_num,
			   uint32_t set);
#else
static QDF_STATUS tgt_gpio_config(struct wlan_objmgr_psoc *psoc,
				  uint32_t gpio_num, uint32_t input,
				  uint32_t pull_type, uint32_t intr_mode)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tgt_gpio_output(struct wlan_objmgr_psoc *psoc,
				  uint32_t gpio_num, uint32_t set)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_GPIO_CFG */
#endif /* __WLAN_GPIO_TGT_API_H__ */
