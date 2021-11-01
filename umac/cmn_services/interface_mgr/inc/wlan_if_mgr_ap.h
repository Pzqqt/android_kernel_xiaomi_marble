/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains interface manager public file containing AP event handlers
 */

#ifndef _WLAN_IF_MGR_AP_H_
#define _WLAN_IF_MGR_AP_H_

/**
 * if_mgr_ap_start_bss() - Start BSS event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Interface manager start bss event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
if_mgr_ap_start_bss(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_start_bss_complete() - Start BSS complete event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Interface manager start bss complete event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
if_mgr_ap_start_bss_complete(struct wlan_objmgr_vdev *vdev,
			     struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_stop_bss() - Stop BSS event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Interface manager stop bss event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
if_mgr_ap_stop_bss(struct wlan_objmgr_vdev *vdev,
		   struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_stop_bss_complete() - Stop BSS event complete handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Interface manager stop bss complete event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
if_mgr_ap_stop_bss_complete(struct wlan_objmgr_vdev *vdev,
			    struct if_mgr_event_data *event_data);
#ifdef WLAN_FEATURE_P2P_P2P_STA
/**
 * if_mgr_csa_complete() - CSA event complete handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * Interface manager csa complete event handler
 *
 * Context: It should run in thread context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
if_mgr_csa_complete(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data);
#else
static inline QDF_STATUS
if_mgr_csa_complete(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined WLAN_MBSS
/**
 * if_mgr_ap_start_acs() - ACS start event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the ACS start event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_start_acs(struct wlan_objmgr_vdev *vdev,
			    struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_stop_acs() - ACS stop event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the ACS stop event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_stop_acs(struct wlan_objmgr_vdev *vdev,
			      struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_done_acs() - ACS done event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the ACS stop event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_done_acs(struct wlan_objmgr_vdev *vdev,
			      struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_cancel_acs() - ACS cancel event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the ACS cancel event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_cancel_acs(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_start_ht40() - HT40 scan start event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the HT40 scan start event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_start_ht40(struct wlan_objmgr_vdev *vdev,
				struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_stop_ht40() - HT40 scan stop event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the HT40 scan stop event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_stop_ht40(struct wlan_objmgr_vdev *vdev,
			       struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_done_ht40() - HT40 scan done event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the HT40 scan done event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_done_ht40(struct wlan_objmgr_vdev *vdev,
			       struct if_mgr_event_data *event_data);

/**
 * if_mgr_ap_cancel_ht40() - HT40 scan cancel event handler
 * @vdev: vdev object
 * @event_data: Interface mgr event data
 *
 * This function handles the HT40 scan cancel event for interface
 * manager
 *
 * Context: It can run in process/tasklet context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS if_mgr_ap_cancel_ht40(struct wlan_objmgr_vdev *vdev,
				 struct if_mgr_event_data *event_data);
#else
static inline QDF_STATUS
if_mgr_ap_start_acs(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
if_mgr_ap_stop_acs(struct wlan_objmgr_vdev *vdev,
		   struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
if_mgr_ap_cancel_acs(struct wlan_objmgr_vdev *vdev,
		     struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
if_mgr_ap_start_ht40(struct wlan_objmgr_vdev *vdev,
		     struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
if_mgr_ap_stop_ht40(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
if_mgr_ap_done_ht40(struct wlan_objmgr_vdev *vdev,
		    struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
if_mgr_ap_cancel_ht40(struct wlan_objmgr_vdev *vdev,
		      struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_MBSS */
#endif /* _WLAN_IF_MGR_AP_H_ */

