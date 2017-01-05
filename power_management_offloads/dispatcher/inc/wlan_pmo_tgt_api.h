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
 * DOC: Declare public API for pmo to interact with target/WMI
 */

#ifndef _WLAN_PMO_TGT_API_H_
#define _WLAN_PMO_TGT_API_H_

#include "wlan_pmo_main.h"

#define GET_PMO_TX_OPS_FROM_PSOC(psoc) (psoc->soc_cb.tx_ops.pmo_tx_ops)

/**
 * pmo_tgt_enable_arp_offload_req() - Enable arp offload req to target
 * @vdev: objmgr vdev
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_enable_arp_offload_req(struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id);

/**
 * pmo_tgt_disable_arp_offload_req() - Disable arp offload req to target
 * @vdev: objmgr vdev
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_disable_arp_offload_req(struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id);

/**
 * pmo_tgt_enable_ns_offload_req() -  Send ns offload req to targe
 * @vdev: objmgr vdev
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_enable_ns_offload_req(struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id);

/**
 * pmo_tgt_disable_ns_offload_req() - Disable arp offload req to target
 * @vdev: objmgr vdev
 * @vdev_id: vdev id
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_disable_ns_offload_req(struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id);

/**
 * pmo_tgt_enable_wow_wakeup_event() - Send Enable wow wakeup events req to fwr
 * @vdev: objmgr vdev handle
 * @bitmap: Event bitmap
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_enable_wow_wakeup_event(struct wlan_objmgr_vdev *vdev,
		uint32_t bitmap);

/**
 * pmo_tgt_disable_wow_wakeup_event() - Send Disable wow wakeup events to fwr
 * @vdev: objmgr vdev handle
 * @bitmap: Event bitmap
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_disable_wow_wakeup_event(struct wlan_objmgr_vdev *vdev,
		uint32_t bitmap);

/**
 * pmo_tgt_send_wow_patterns_to_fw() - Sends WOW patterns to FW.
 * @vdev: objmgr vdev
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
QDF_STATUS pmo_tgt_send_wow_patterns_to_fw(struct wlan_objmgr_vdev *vdev,
		uint8_t ptrn_id, const uint8_t *ptrn, uint8_t ptrn_len,
		uint8_t ptrn_offset, const uint8_t *mask,
		uint8_t mask_len, bool user);

/**
 * pmo_tgt_set_mc_filter_req() - Set mcast filter command to fw
 * @vdev: objmgr vdev
 * @multicastAddr: mcast address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_tgt_set_mc_filter_req(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr);

/**
 * pmo_tgt_clear_mc_filter_req() - Clear mcast filter command to fw
 * @vdev: objmgr vdev
 * @multicastAddr: mcast address
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_tgt_clear_mc_filter_req(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr);

/**
 * pmo_tgt_send_enhance_multicast_offload_req() - send enhance mc offload req
 * @wma_handle: wma handle
 * @vdev_id: vdev id
 * @action: enable or disable enhance multicast offload
 *
 * Return: QDF_STATUS_SUCCESS on success else error code
 */
QDF_STATUS pmo_tgt_send_enhance_multicast_offload_req(
		struct wlan_objmgr_vdev *vdev,
		uint8_t vdev_id,
		uint8_t action);

/**
 * pmo_tgt_send_ra_filter_req() - send ra filter request to target
 * @vdev: objmgr vdev handle
 *
 * Return: QDF_STATUS_SUCCESS on success else error code
 */
QDF_STATUS pmo_tgt_send_ra_filter_req(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_tgt_send_action_frame_pattern_req - send wow action frame patterns req
 * @vdev: objmgr vdev handle
 * @cmd: action frame pattern cmd
 *
 * Return: QDF_STATUS_SUCCESS on success else error code
 */
QDF_STATUS pmo_tgt_send_action_frame_pattern_req(
		struct wlan_objmgr_vdev *vdev,
		struct pmo_action_wakeup_set_params *cmd);

/**
 * pmo_tgt_send_gtk_offload_req() - send GTK offload command to fw
 * @vdev: objmgr vdev
 * @gtk_req: pmo gtk req
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_send_gtk_offload_req(struct wlan_objmgr_vdev *vdev,
		struct pmo_gtk_req *gtk_req);

/**
 * pmo_tgt_get_gtk_rsp() - send get gtk rsp command to fw
 * @vdev: objmgr vdev
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_get_gtk_rsp(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_tgt_gtk_rsp_evt() - receive gtk rsp event from fwr
 * @psoc: objmgr psoc
 * @gtk_rsp_param: gtk response parameters
 *
 * Return: QDF status
 */
QDF_STATUS pmo_tgt_gtk_rsp_evt(struct wlan_objmgr_psoc *psoc,
		struct pmo_gtk_rsp_params *rsp_param);

#endif /* end  of _WLAN_PMO_TGT_API_H_ */
