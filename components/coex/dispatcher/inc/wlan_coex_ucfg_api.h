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

/*
 * DOC: contains coex north bound interface declarations
 */

#ifndef _WLAN_COEX_UCFG_API_H_
#define _WLAN_COEX_UCFG_API_H_

#include "qdf_status.h"
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include "qca_vendor.h"

#define WLAN_COEX_BTC_CHAIN_MODE_SHARED QCA_BTC_CHAIN_SHARED
#define WLAN_COEX_BTC_CHAIN_MODE_SEPARATED QCA_BTC_CHAIN_SEPARATED
#define WLAN_COEX_BTC_CHAIN_MODE_UNSETTLED 0xFF

/**
 * enum coex_config_type - coex config type definitions
 * @COEX_CONFIG_BTC_CHAIN_MODE: config BT coex chain mode
 * @COEX_CONFIG_TYPE_MAX: max value
 */
enum coex_config_type {
	COEX_CONFIG_BTC_CHAIN_MODE,
	/* keep last */
	COEX_CONFIG_TYPE_MAX,
};

/**
 * typedef update_coex_cb() - cb to inform coex config
 * @vdev: vdev pointer
 *
 * Return: void
 */
typedef QDF_STATUS (*update_coex_cb)(struct wlan_objmgr_vdev *vdev);

#ifdef FEATURE_COEX
/**
 * ucfg_coex_register_cfg_updated_handler() - API to register coex config
 * updated handler.
 * @psoc: pointer to psoc object
 * @type: type of coex config
 * @handler: handler to be registered
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coex_register_cfg_updated_handler(struct wlan_objmgr_psoc *psoc,
				       enum coex_config_type type,
				       update_coex_cb handler);

/**
 * ucfg_coex_psoc_set_btc_chain_mode() - API to set BT coex chain mode for psoc
 * @psoc: pointer to psoc object
 * @val: BT coex chain mode
 *
 * Return : status of operation
 */
QDF_STATUS
ucfg_coex_psoc_set_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t val);

/**
 * ucfg_coex_psoc_get_btc_chain_mode() - API to get BT coex chain mode from psoc
 * @psoc: pointer to psoc object
 * @val: pointer to BT coex chain mode
 *
 * Return : status of operation
 */
QDF_STATUS
ucfg_coex_psoc_get_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t *val);

/**
 * ucfg_coex_send_btc_chain_mode() - API to send BT coex config to target if
 * @vdev: pointer to vdev object
 * @mode: BT coex chain mode
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coex_send_btc_chain_mode(struct wlan_objmgr_vdev *vdev, uint8_t mode);
#else
static inline QDF_STATUS
ucfg_coex_register_cfg_updated_handler(struct wlan_objmgr_psoc *psoc,
				       enum coex_config_type type,
				       update_coex_cb handler)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_coex_psoc_get_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t *val)
{
	if (val)
		*val = WLAN_COEX_BTC_CHAIN_MODE_UNSETTLED;

	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_coex_send_btc_chain_mode(struct wlan_objmgr_vdev *vdev, uint8_t mode)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
