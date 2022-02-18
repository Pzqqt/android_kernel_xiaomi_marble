/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_objmgr.c
 * This file defines the APIs of TWT component.
 */
#include "wlan_twt_common.h"
#include "wlan_twt_priv.h"
#include "wlan_twt_objmgr_handler.h"
#include "wlan_objmgr_peer_obj.h"

QDF_STATUS
wlan_twt_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg)
{
	return QDF_STATUS_SUCCESS;
}

