/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
 * DOC: contains interface manager public api
 */

#ifndef _WLAN_IF_MGR_API_H_
#define _WLAN_IF_MGR_API_H_

#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_if_mgr_public_struct.h"

#ifdef WLAN_FEATURE_INTERFACE_MGR

/**
 * if_mgr_deliver_event() - interface mgr event handler
 * @vdev: vdev object
 * @event: interface mangaer event
 * @event_data: Interface mgr event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_deliver_event(struct wlan_objmgr_vdev *vdev,
				enum wlan_if_mgr_evt event,
				struct if_mgr_event_data *event_data);
/**
 * if_mgr_get_event_str() - get event string
 * @event: interface manager event
 *
 * Return: Interface event string
 */
const char *if_mgr_get_event_str(enum wlan_if_mgr_evt event);

/**
 * if_mgr_deliver_mbss_event() - interface mgr MBSS event handler
 * @vdev: vdev object
 * @event: interface mangaer event
 * @event_data: Interface mgr event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_deliver_mbss_event(struct wlan_objmgr_vdev *vdev,
				     enum wlan_if_mgr_evt event,
				     struct if_mgr_event_data *event_data);

#else
static inline
QDF_STATUS if_mgr_deliver_event(struct wlan_objmgr_vdev *vdev,
				enum wlan_if_mgr_evt event,
				struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline const char *if_mgr_get_event_str(enum wlan_if_mgr_evt event)
{
	return "";
}

#endif /* WLAN_FEATURE_INTERFACE_MGR */
#endif
