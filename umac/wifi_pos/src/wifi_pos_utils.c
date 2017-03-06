/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
/**
 * DOC: wifi_pos_utils.c
 * This file defines the utility helper functions for wifi_pos component.
 */

#include "qdf_types.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wifi_pos_utils_i.h"

/*
 * WIFI pos command are not associated with any pdev/psoc/vdev, so the callback
 * registered with GENL socket does not receive any pdev/pdev/vdev object.
 * Since PSOC is top most object, it was decided to keep WIFI POS private obj
 * within PSOC and hence, this module need to hang on to the first PSOC that
 * was created for all its internal usage.
 */
static struct wlan_objmgr_psoc *wifi_pos_psoc_obj;

struct wlan_objmgr_psoc *wifi_pos_get_psoc(void)
{
	return wifi_pos_psoc_obj;
}

void wifi_pos_set_psoc(struct wlan_objmgr_psoc *psoc)
{
	if (wifi_pos_psoc_obj)
		wifi_pos_warn("global psoc obj already set");
	else
		wifi_pos_psoc_obj = psoc;
}

void wifi_pos_clear_psoc(void)
{
	if (!wifi_pos_psoc_obj)
		wifi_pos_warn("global psoc obj already cleared");
	else
		wifi_pos_psoc_obj = NULL;
}

/**
 * wifi_pos_get_psoc_priv_obj: returns wifi_pos priv object within psoc
 * @psoc: pointer to psoc object
 *
 * Return: wifi_pos_psoc_priv_obj
 */
struct wifi_pos_psoc_priv_obj *wifi_pos_get_psoc_priv_obj(
		struct wlan_objmgr_psoc *psoc)
{
	struct wifi_pos_psoc_priv_obj *obj;

	wlan_psoc_obj_lock(psoc);
	obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_UMAC_COMP_WIFI_POS);
	wlan_psoc_obj_unlock(psoc);

	return obj;
}

bool wifi_pos_is_app_registered(struct wlan_objmgr_psoc *psoc)
{
	struct wifi_pos_psoc_priv_obj *obj;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return false;
	}

	obj = wifi_pos_get_psoc_priv_obj(psoc);

	if (!obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return false;
	}

	return obj->is_app_registered;
}

uint32_t wifi_pos_get_app_pid(struct wlan_objmgr_psoc *psoc)
{
	struct wifi_pos_psoc_priv_obj *obj;

	if (!psoc) {
		wifi_pos_err("psoc is null");
		return 0;
	}

	obj = wifi_pos_get_psoc_priv_obj(psoc);

	if (!obj) {
		wifi_pos_err("wifi_pos priv obj is null");
		return 0;
	}

	return obj->app_pid;
}
