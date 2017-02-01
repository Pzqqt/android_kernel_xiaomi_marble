/*
* Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
* DOC: Declare various api/struct which shall be used
* by pmo component for wmi cmd (tx path) and
* event (rx) handling.
*/

#ifndef _TARGET_IF_PMO_H_
#define _TARGET_IF_PMO_H_

#include "target_if.h"
#include "wlan_pmo_tgt_api.h"

/**
 * target_if_pmo_enable_wow_wakeup_event() - Enable wow wakeup events.
 * @vdev:objmgr vdev handle
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: QDF status
 */
QDF_STATUS target_if_pmo_enable_wow_wakeup_event(struct wlan_objmgr_vdev *vdev,
		uint32_t bitmap);

/**
 * target_if_pmo_disable_wow_wakeup_event() -  Disable wow wakeup events.
 * @vdev:objmgr vdev handle
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: QDF status
 */
QDF_STATUS target_if_pmo_disable_wow_wakeup_event(
		struct wlan_objmgr_vdev *vdev, uint32_t bitmap);

/**
 * target_if_pmo_send_wow_patterns_to_fw() - Sends WOW patterns to FW.
 * @vdev: objmgr vdev handle
 * @ptrn_id: pattern id
 * @ptrn: pattern
 * @ptrn_len: pattern length
 * @ptrn_offset: pattern offset
 * @mask: mask
 * @mask_len: mask length
 * @user: true for user configured pattern and false for default pattern
 *
 * Return: QDF status
 */
QDF_STATUS target_if_pmo_send_wow_patterns_to_fw(struct wlan_objmgr_vdev *vdev,
		uint8_t ptrn_id,
		const uint8_t *ptrn, uint8_t ptrn_len,
		uint8_t ptrn_offset, const uint8_t *mask,
		uint8_t mask_len, bool user);

/**
 * target_if_pmo_send_enhance_mc_offload_req() - send enhance mc offload req
 * @vdev: objmgr vdev
 * @action: enable or disable enhance multicast offload
 *
 * Return: QDF_STATUS_SUCCESS on success else error code
 */
QDF_STATUS target_if_pmo_send_enhance_mc_offload_req(
		struct wlan_objmgr_vdev *vdev,
		bool enable);

/**
 * target_if_pmo_set_mc_filter_req() - set mcast filter command to fw
 * @vdev: objmgr vdev handle
 * @multicastAddr: mcast address
 *
 * Return: 0 for success or error code
 */
QDF_STATUS target_if_pmo_set_mc_filter_req(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr);

/**
 * target_if_pmo_clear_mc_filter_req() - clear mcast filter command to fw
 * @vdev: objmgr vdev handle
 * @multicastAddr: mcast address
 *
 * Return: 0 for success or error code
 */
QDF_STATUS target_if_pmo_clear_mc_filter_req(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr);

/**
 * target_if_pmo_send_ra_filter_req() - set RA filter pattern in fw
 * @vdev: objmgr vdev handle
 * @default_pattern: default pattern id
 * @rate_limit_interval: ra rate limit interval
 *
 * Return: QDF status
 */
QDF_STATUS target_if_pmo_send_ra_filter_req(struct wlan_objmgr_vdev *vdev,
		uint8_t default_pattern, uint16_t rate_limit_interval);

/**
 * target_if_pmo_send_action_frame_patterns() - register action frame map to fw
 * @handle: Pointer to wma handle
 * @vdev_id: VDEV ID
 *
 * This is called to push action frames wow patterns from local
 * cache to firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_pmo_send_action_frame_patterns(
		struct wlan_objmgr_vdev *vdev,
		struct pmo_action_wakeup_set_params *ip_cmd);

/**
 * target_if_pmo_send_arp_offload_req() - sends arp request to fwr
 * @vdev: objmgr vdev
 * @arp_offload_req: arp offload req
 * @ns_offload_req: ns offload request
 *
 * This functions sends arp request to fwr.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS target_if_pmo_send_arp_offload_req(
		struct wlan_objmgr_vdev *vdev,
		struct pmo_arp_offload_params *arp_offload_req,
		struct pmo_ns_offload_params *ns_offload_req);

/**
 * target_if_pmo_send_ns_offload_req() - sends ns request to fwr
 * @vdev: objmgr vdev
 * @arp_offload_req: arp offload req
 * @ns_offload_req: ns offload request
 *
 * This functions sends ns request to fwr.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS target_if_pmo_send_ns_offload_req(
		struct wlan_objmgr_vdev *vdev,
		struct pmo_arp_offload_params *arp_offload_req,
		struct pmo_ns_offload_params *ns_offload_req);

/**
 * target_if_pmo_send_gtk_offload_req() - send gtk offload request in fwr
 * @vdev: objmgr vdev handle
 * @gtk_offload_req: gtk offload request
 *
 * Return: QDF status
 */
QDF_STATUS target_if_pmo_send_gtk_offload_req(struct wlan_objmgr_vdev *vdev,
		struct pmo_gtk_req *gtk_offload_req);

/**
 * target_if_pmo_send_gtk_response_req() - send gtk response request in fwr
 * @vdev: objmgr vdev handle
 *
 * Return: QDF status
 */
QDF_STATUS target_if_pmo_send_gtk_response_req(struct wlan_objmgr_vdev *vdev);

/**
 * target_if_pmo_gtk_offload_status_event() - GTK offload status event handler
 * @scn_handle: scn handle
 * @event: event buffer
 * @len: buffer length
 *
 * Return: 0 for success or error code
 */
int target_if_pmo_gtk_offload_status_event(void *scn_handle,
	uint8_t *event, uint32_t len);


#endif

