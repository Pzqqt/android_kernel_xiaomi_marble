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
 * DOC: Define VDEV MLME public APIs
 */

#ifndef _WLAN_VDEV_MLME_API_H_
#define _WLAN_VDEV_MLME_API_H_

/**
 * wlan_vdev_mlme_get_cmpt_obj - Returns MLME component object
 *
 * Retrieves MLME component object from VDEV object
 *
 * Return: comp handle on SUCCESS
 *         NULL, if it fails to retrieve
 */
struct vdev_mlme_obj *wlan_vdev_mlme_get_cmpt_obj(
						struct wlan_objmgr_vdev *vdev);
/**
 * wlan_vdev_mlme_get_ext_hdl - Returns legacy handle
 *
 * Retrieves legacy handle from vdev mlme component object
 *
 * Return: legacy handle on SUCCESS
 *         NULL, if it fails to retrieve
 */
void *wlan_vdev_mlme_get_ext_hdl(struct wlan_objmgr_vdev *vdev);

#ifdef CMN_VDEV_MLME_SM_ENABLE
/**
 * wlan_vdev_mlme_sm_deliver_evt() - Delivers event to VDEV MLME SM
 * @vdev: Object manager VDEV object
 * @event: MLME event
 * @event_data_len: data size
 * @event_data: event data
 *
 * API to dispatch event to VDEV MLME SM with lock acquired
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
QDF_STATUS wlan_vdev_mlme_sm_deliver_evt(struct wlan_objmgr_vdev *vdev,
					 enum wlan_vdev_sm_evt event,
					 uint16_t event_data_len,
					 void *event_data);

/**
 * wlan_vdev_mlme_sm_deliver_evt_sync() - Delivers event to VDEV MLME SM sync
 * @vdev: Object manager VDEV object
 * @event: MLME event
 * @event_data_len: data size
 * @event_data: event data
 *
 * API to dispatch event to VDEV MLME SM with lock acquired
 *
 * Return: SUCCESS: on handling event
 *         FAILURE: on ignoring the event
 */
QDF_STATUS wlan_vdev_mlme_sm_deliver_evt_sync(struct wlan_objmgr_vdev *vdev,
					      enum wlan_vdev_sm_evt event,
					      uint16_t event_data_len,
					      void *event_data);

#ifdef SM_ENG_HIST_ENABLE
/**
 * wlan_vdev_mlme_sm_history_print() - Prints SM history
 * @vdev: Object manager VDEV object
 *
 * API to print SM history
 *
 * Return: void
 */
void wlan_vdev_mlme_sm_history_print(struct wlan_objmgr_vdev *vdev);

#endif

/**
 * wlan_vdev_allow_connect_n_tx() - Checks whether VDEV is in operational state
 * @vdev: Object manager VDEV object
 *
 * API to checks the VDEV MLME SM state to allow tx or connections
 *
 * Return: SUCCESS: to allow tx or connection
 *         FAILURE: otherwise failure
 */
QDF_STATUS wlan_vdev_allow_connect_n_tx(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_vdev_mlme_is_active() - Checks whether VDEV is in active state
 * @vdev: Object manager VDEV object
 *
 * API to checks the VDEV MLME SM state to check channel is configured in FW
 *
 * Return: SUCCESS: valid channel is configured
 *         FAILURE: otherwise failure
 */
QDF_STATUS wlan_vdev_mlme_is_active(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_vdev_chan_config_valid() - Checks whether VDEV chan config valid
 * @vdev: Object manager VDEV object
 *
 * API to checks the VDEV MLME SM state to check channel is configured in Host
 *
 * Return: SUCCESS: valid channel is configured
 *         FAILURE: otherwise failure
 */
QDF_STATUS wlan_vdev_chan_config_valid(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_vdev_is_going_down() - Checks whether VDEV is being brought down
 * @vdev: Object manager VDEV object
 *
 * API to checks the VDEV MLME SM state to check VDEV is being brought down
 *
 * Return: SUCCESS: valid channel is configured
 *         FAILURE: otherwise failure
 */
QDF_STATUS wlan_vdev_is_going_down(struct wlan_objmgr_vdev *vdev);
#endif
#endif
