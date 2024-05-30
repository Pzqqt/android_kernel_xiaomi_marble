/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implements PDEV MLME public APIs
 */

#include <wlan_objmgr_pdev_obj.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_pdev_mlme.h"
#include <wlan_pdev_mlme_api.h>
#include <qdf_module.h>

struct pdev_mlme_obj *wlan_pdev_mlme_get_cmpt_obj(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_mlme_obj *pdev_mlme;

	if (!pdev) {
		mlme_err("pdev is NULL");
		return NULL;
	}

	pdev_mlme = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							  WLAN_UMAC_COMP_MLME);
	if (!pdev_mlme) {
		mlme_err(" MLME component object is NULL");
		return NULL;
	}

	return pdev_mlme;
}

mlme_pdev_ext_t *wlan_pdev_mlme_get_ext_hdl(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (pdev_mlme)
		return pdev_mlme->ext_pdev_ptr;

	return NULL;
}

qdf_export_symbol(wlan_pdev_mlme_get_ext_hdl);

