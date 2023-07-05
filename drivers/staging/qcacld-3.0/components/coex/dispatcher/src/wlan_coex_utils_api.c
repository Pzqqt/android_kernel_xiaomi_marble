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
 * DOC: wlan_coex_utils_api.c
 *
 * This file provides definitions of public APIs exposed to other UMAC
 * components.
 */

#include <wlan_coex_main.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_coex_utils_api.h>
#include "cfg_ucfg_api.h"

QDF_STATUS wlan_coex_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_COEX,
			wlan_coex_psoc_created_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		coex_err("Failed to register psoc create handler");
		goto fail_create_psoc;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_COEX,
			wlan_coex_psoc_destroyed_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		coex_err("Failed to create psoc delete handler");
		goto fail_psoc_destroy;
	}

	coex_debug("coex psoc create and delete handler registered");
	return status;

fail_psoc_destroy:
	wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_COEX,
			wlan_coex_psoc_created_notification, NULL);
fail_create_psoc:
	return status;
}

QDF_STATUS wlan_coex_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_COEX,
			wlan_coex_psoc_destroyed_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		coex_err("Failed to unregister psoc delete handler");

	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_COEX,
			wlan_coex_psoc_created_notification, NULL);
	if (status != QDF_STATUS_SUCCESS)
		coex_err("Failed to unregister psoc create handler");

	return status;
}

#ifdef FEATURE_BTC_CHAIN_MODE
/**
 * wlan_coex_set_btc_chain_mode_with_ini() - set BTC init chain mode
 * with ini
 * @psoc: pointer to psoc object
 *
 * This function is used to set BTC init chain mode with ini
 *
 * Return: None
 */
static void
wlan_coex_set_btc_chain_mode_with_ini(struct wlan_objmgr_psoc *psoc)
{
	uint8_t btc_chain_mode;
	QDF_STATUS status;

	status = wlan_coex_psoc_get_btc_chain_mode(psoc, &btc_chain_mode);
	if (QDF_IS_STATUS_ERROR(status)) {
		coex_err("error for getting btc chain mode");
		return;
	}

	if (btc_chain_mode == WLAN_COEX_BTC_CHAIN_MODE_UNSETTLED) {
		btc_chain_mode = cfg_get(psoc, CFG_SET_INIT_CHAIN_MODE_FOR_BTC);
		if (btc_chain_mode != WLAN_COEX_BTC_CHAIN_MODE_SHARED &&
		    btc_chain_mode != WLAN_COEX_BTC_CHAIN_MODE_SEPARATED &&
		    btc_chain_mode != WLAN_COEX_BTC_CHAIN_MODE_UNSETTLED) {
			coex_err("invalid ini config %d for btc chain mode",
				 btc_chain_mode);
			return;
		}

		status = wlan_coex_psoc_set_btc_chain_mode(psoc,
							   btc_chain_mode);
		if (QDF_IS_STATUS_ERROR(status))
			coex_err("error for setting btc init chain mode from ini");
	}
}
#else
static void
wlan_coex_set_btc_chain_mode_with_ini(struct wlan_objmgr_psoc *psoc)
{
}
#endif

QDF_STATUS
wlan_coex_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	wlan_coex_set_btc_chain_mode_with_ini(psoc);
	return wlan_coex_psoc_init(psoc);
}

QDF_STATUS
wlan_coex_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_coex_psoc_deinit(psoc);
}
