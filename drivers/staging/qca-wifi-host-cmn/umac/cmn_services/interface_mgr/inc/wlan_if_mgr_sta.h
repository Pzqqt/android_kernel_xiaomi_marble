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
 * DOC: contains interface manager public file containing STA event handlers
 */

#ifndef _WLAN_IF_MGR_STA_H_
#define _WLAN_IF_MGR_STA_H_

/**
 * if_mgr_connect_start() - connect start event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Interface manager connect start event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_connect_start(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data);

/**
 * if_mgr_connect_complete() - connect complete event handler
 * @vdev: vdev object
 * @event_data: Interface manager complete event data
 *
 * Interface manager connect complete event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_connect_complete(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data);

/**
 * if_mgr_disconnect_start() - Disconnect start event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the disconnect start event for interface manager
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_disconnect_start(struct wlan_objmgr_vdev *vdev,
				   struct if_mgr_event_data *event_data);

/**
 * if_mgr_disconnect_complete() - Disconnect complete event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the disconnect complete event for interface
 * manager
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_disconnect_complete(struct wlan_objmgr_vdev *vdev,
				      struct if_mgr_event_data *event_data);

/**
 * if_mgr_validate_candidate() - Validate the selected candidate
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles candidate validation as part of connection
 * manager
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_validate_candidate(struct wlan_objmgr_vdev *vdev,
				     struct if_mgr_event_data *event_data);

#endif
