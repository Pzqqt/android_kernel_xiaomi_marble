/*
 * Copyright (c) 2012-2015, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: osif_cm_req.h
 *
 * This header file maintains declarations of connect, disconnect, roam
 * request apis.
 */

#ifndef __OSIF_CM_REQ_H
#define __OSIF_CM_REQ_H

#ifdef FEATURE_CM_ENABLE
#include "qdf_status.h"
#include "wlan_cm_public_struct.h"
#include <net/cfg80211.h>
#include "wlan_objmgr_vdev_obj.h"

/**
 * osif_cm_connect() - Connect start request
 * @dev: net dev
 * @vdev: vdev pointer
 * @req: connect req
 *
 * Return: int
 */
int osif_cm_connect(struct net_device *dev,
		    struct wlan_objmgr_vdev *vdev,
		    struct cfg80211_connect_params *req);

/**
 * osif_cm_disconnect() - Disconnect start request
 * @dev: net dev
 * @vdev: vdev pointer
 * @reason: disconnect reason
 *
 * Return: int
 */
int osif_cm_disconnect(struct net_device *dev,
		       struct wlan_objmgr_vdev *vdev,
		       uint16_t reason);
#endif
#endif /* __OSIF_CM_REQ_H */
