/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Implements VDEV MLME SM
 */

#include <wlan_mlme_dbg.h>
#include <wlan_sm_engine.h>
#include "include/wlan_vdev_mlme.h"
#include "vdev_mlme_sm.h"

QDF_STATUS mlme_vdev_sm_create(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_sm *sm;

	sm = wlan_sm_create("VDEV MLME", vdev_mlme, 0, NULL, 0, NULL, 0);
	if (!sm) {
		mlme_err("VDEV MLME SM allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	vdev_mlme->sm_hdl = sm;

	mlme_vdev_sm_spinlock_create(vdev_mlme);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS mlme_vdev_sm_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_vdev_sm_spinlock_destroy(vdev_mlme);

	wlan_sm_delete(vdev_mlme->sm_hdl);

	return QDF_STATUS_SUCCESS;
}
