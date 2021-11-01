/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <wlan_lmac_if_def.h>
#include "wifi_pos_utils_i.h"

/* lock to protect use of psoc global pointer variable */
static qdf_spinlock_t psoc_ptr_lock;

/*
 * WIFI pos command are not associated with any pdev/psoc/vdev, so the callback
 * registered with GENL socket does not receive any pdev/pdev/vdev object.
 * Since PSOC is top most object, it was decided to keep WIFI POS private obj
 * within PSOC and hence, this module need to hang on to the first PSOC that
 * was created for all its internal usage.
 */
static struct wlan_objmgr_psoc *wifi_pos_psoc_obj;

void wifi_pos_lock_init(void)
{
	qdf_spinlock_create(&psoc_ptr_lock);
}

void wifi_pos_lock_deinit(void)
{
	qdf_spinlock_destroy(&psoc_ptr_lock);
}

struct wlan_objmgr_psoc *wifi_pos_get_psoc(void)
{
	struct wlan_objmgr_psoc  *tmp;

	qdf_spin_lock_bh(&psoc_ptr_lock);
	tmp = wifi_pos_psoc_obj;
	qdf_spin_unlock_bh(&psoc_ptr_lock);

	return tmp;
}

qdf_export_symbol(wifi_pos_get_psoc);

void wifi_pos_set_psoc(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_psoc *tmp;

	qdf_spin_lock_bh(&psoc_ptr_lock);
	tmp = wifi_pos_psoc_obj;
	if (!wifi_pos_psoc_obj)
		wifi_pos_psoc_obj = psoc;
	qdf_spin_unlock_bh(&psoc_ptr_lock);

	if (tmp)
		wifi_pos_warn("global psoc obj already set");
}

void wifi_pos_clear_psoc(void)
{
	struct wlan_objmgr_psoc *tmp;

	qdf_spin_lock_bh(&psoc_ptr_lock);
	tmp = wifi_pos_psoc_obj;
	if (wifi_pos_psoc_obj)
		wifi_pos_psoc_obj = NULL;
	qdf_spin_unlock_bh(&psoc_ptr_lock);

	if (!tmp)
		wifi_pos_warn("global psoc obj already cleared");
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

	obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_UMAC_COMP_WIFI_POS);

	return obj;
}

qdf_export_symbol(wifi_pos_get_psoc_priv_obj);
