/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: contains core nan function definitions
 */

#include "nan_main_i.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"

struct nan_vdev_priv_obj *nan_get_vdev_priv_obj(
				struct wlan_objmgr_vdev *vdev)
{
	struct nan_vdev_priv_obj *obj;

	if (!vdev) {
		nan_err("vdev is null");
		return NULL;
	}
	wlan_vdev_obj_lock(vdev);
	obj = wlan_objmgr_vdev_get_comp_private_obj(vdev, WLAN_UMAC_COMP_NAN);
	wlan_vdev_obj_unlock(vdev);

	return obj;
}

struct nan_psoc_priv_obj *nan_get_psoc_priv_obj(
				struct wlan_objmgr_psoc *psoc)
{
	struct nan_psoc_priv_obj *obj;

	if (!psoc) {
		nan_err("psoc is null");
		return NULL;
	}
	wlan_psoc_obj_lock(psoc);
	obj = wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_NAN);
	wlan_psoc_obj_unlock(psoc);

	return obj;
}