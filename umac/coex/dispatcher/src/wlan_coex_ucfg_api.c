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
 * DOC: contains coex north bound interface definitions
 */

#include <wlan_coex_main.h>
#include <wlan_coex_ucfg_api.h>
#include "wmi_unified.h"

QDF_STATUS
ucfg_coex_register_cfg_updated_handler(struct wlan_objmgr_psoc *psoc,
				       enum coex_config_type type,
				       update_coex_cb handler)
{
	struct coex_psoc_obj *coex_obj;

	if (type >= COEX_CONFIG_TYPE_MAX) {
		coex_err("invalid coex type: %d", type);
		return QDF_STATUS_E_INVAL;
	}

	coex_obj = wlan_psoc_get_coex_obj(psoc);
	if (!coex_obj)
		return QDF_STATUS_E_INVAL;

	coex_obj->coex_config_updated[type] = handler;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
ucfg_coex_psoc_set_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t val)
{
	return wlan_coex_psoc_set_btc_chain_mode(psoc, val);
}

QDF_STATUS
ucfg_coex_psoc_get_btc_chain_mode(struct wlan_objmgr_psoc *psoc, uint8_t *val)
{
	return wlan_coex_psoc_get_btc_chain_mode(psoc, val);
}

QDF_STATUS
ucfg_coex_send_btc_chain_mode(struct wlan_objmgr_vdev *vdev, uint8_t mode)
{
	struct coex_config_params param = {0};

	if (mode != WLAN_COEX_BTC_CHAIN_MODE_SHARED &&
	    mode != WLAN_COEX_BTC_CHAIN_MODE_SEPARATED)
		return QDF_STATUS_E_INVAL;

	param.vdev_id = wlan_vdev_get_id(vdev);
	param.config_type = WMI_COEX_CONFIG_BTCOEX_SEPARATE_CHAIN_MODE;
	param.config_arg1 = mode;

	coex_debug("send btc chain mode %d for vdev %d", mode, param.vdev_id);

	return wlan_coex_config_send(vdev, &param);
}
