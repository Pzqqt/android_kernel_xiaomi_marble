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
 * DOC: Declare public API related to the pmo called by north bound HDD/OSIF
 */

#ifndef _WLAN_PMO_UCFG_API_H_
#define _WLAN_PMO_UCFG_API_H_

#include "wlan_pmo_arp_public_struct.h"
#include "wlan_pmo_ns_public_struct.h"
#include "wlan_pmo_gtk_public_struct.h"
#include "wlan_pmo_mc_addr_filtering_public_struct.h"
#include "wlan_pmo_wow_public_struct.h"
#include "wlan_pmo_common_public_struct.h"

/**
 * pmo_ucfg_is_ap_mode_supports_arp_ns() - Check ap mode support arp&ns offload
 * @psoc: objmgr psoc
 * @vdev_opmode: vdev opmode
 *
 * Return: true in case support else false
 */
bool pmo_ucfg_is_ap_mode_supports_arp_ns(struct wlan_objmgr_psoc *psoc,
	enum tQDF_ADAPTER_MODE vdev_opmode);

/**
 * pmo_ucfg_is_vdev_connected() -  to check whether peer is associated or not
 * @vdev: objmgr vdev
 *
 * Return: true in case success else false
 */
bool pmo_ucfg_is_vdev_connected(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_is_vdev_supports_offload() - check offload is supported on vdev
 * @vdev: objmgr vdev
 *
 * Return: true in case success else false
 */
bool pmo_ucfg_is_vdev_supports_offload(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_get_psoc_config(): API to get the psoc user configurations of pmo
 * @psoc: objmgr psoc handle
 * @psoc_cfg: fill the current psoc user configurations.
 *
 * Return pmo psoc configurations
 */
QDF_STATUS pmo_ucfg_get_psoc_config(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_cfg *psoc_cfg);

/**
 * pmo_ucfg_update_psoc_config(): API to update the psoc user configurations
 * @psoc: objmgr psoc handle
 * @psoc_cfg: pmo psoc configurations
 *
 * This api shall be used for soc config initialization as well update.
 * In case of update caller must first call pmo_get_psoc_cfg to get
 * current config and then apply changes on top of current config.
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_update_psoc_config(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_cfg *psoc_cfg);

/**
 * pmo_ucfg_enable_wakeup_event() -  enable wow wakeup events
 * @psoc: objmgr psoc
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 *
 * Return: none
 */
void pmo_ucfg_enable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t bitmap);

/**
 * pmo_ucfg_disable_wakeup_event() -  disable wow wakeup events
 * @psoc: objmgr psoc
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 *
 * Return: none
 */
void pmo_ucfg_disable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t bitmap);

/**
 * pmo_ucfg_cache_arp_offload_req(): API to cache arp req in pmo vdev priv ctx
 * @arp_req: pmo arp req param
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_cache_arp_offload_req(struct pmo_arp_req *arp_req);

/**
 * pmo_ucfg_flush_arp_offload_req(): API to flush arp req from pmo vdev priv ctx
 * @vdev: objmgr vdev param
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_flush_arp_offload_req(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_enable_arp_offload_in_fwr(): API to enable arp req in fwr
 * @vdev: objmgr vdev param
 * @trigger: triger reason for enable arp offload
 *
 *  API to enable cache arp req in fwr from pmo vdev priv ctx
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_enable_arp_offload_in_fwr(struct wlan_objmgr_vdev *vdev,
		enum pmo_offload_trigger trigger);

/**
 * pmo_ucfg_disable_arp_offload_in_fwr(): API to disable arp req in fwr
 * @vdev: objmgr vdev param
 * @trigger: triger reason  for disable arp offload
 *  API to disable cache arp req in fwr
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_disable_arp_offload_in_fwr(struct wlan_objmgr_vdev *vdev,
		enum pmo_offload_trigger trigger);

/**
 * pmo_ucfg_cache_ns_offload_req(): API to cache ns req in pmo vdev priv ctx
 * @ns_req: pmo ns req param
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_cache_ns_offload_req(struct pmo_ns_req *ns_req);

/**
 * pmo_ucfg_flush_ns_offload_req(): API to flush ns req from pmo vdev priv ctx
 * @vdev: vdev ojbmgr handle
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_flush_ns_offload_req(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_enable_ns_offload_in_fwr(): API to enable ns req in fwr
 * @arp_req: pmo arp req param
 * @trigger: trigger reason to enable ns offload
 *
 *  API to enable cache ns req in fwr from pmo vdev priv ctx
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_enable_ns_offload_in_fwr(struct wlan_objmgr_vdev *vdev,
		enum pmo_offload_trigger trigger);

/**
 * pmo_ucfg_disable_ns_offload_in_fwr(): API to disable ns req in fwr
 * @arp_req: pmo arp req param
 * @trigger: trigger reason to disable ns offload
 *
 *  API to disable ns req in fwr
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_disable_ns_offload_in_fwr(struct wlan_objmgr_vdev *vdev,
		enum pmo_offload_trigger trigger);

/**
 * pmo_ucfg_max_mc_addr_supported() -  to get max support mc address
 * @psoc: objmgr psoc
 *
 * Return: max mc addr supported count for all vdev in corresponding psoc
 */
uint8_t pmo_ucfg_max_mc_addr_supported(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_ucfg_cache_mc_addr_list(): API to cache mc addr list in pmo vdev priv obj
 * @psoc: objmgr psoc handle
 * @vdev_id: vdev id
 * @gtk_req: pmo gtk req param
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_cache_mc_addr_list(
		struct pmo_mc_addr_list_params *mc_list_config);

/**
 * pmo_ucfg_flush_mc_addr_list(): API to flush mc addr list in pmo vdev priv obj
 * @psoc: objmgr psoc handle
 * @vdev_id: vdev id
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_flush_mc_addr_list(struct wlan_objmgr_psoc *psoc,
	uint8_t vdev_id);

/**
 * pmo_ucfg_enable_mc_addr_filtering_in_fwr(): Enable cached mc add list in fwr
 * @psoc: objmgr psoc handle
 * @vdev_id: vdev id
 * @gtk_req: pmo gtk req param
 * @action: true for enable els false
 *
 * API to enable cached mc add list in fwr
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_enable_mc_addr_filtering_in_fwr(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id,
		enum pmo_offload_trigger trigger);

/**
 * pmo_ucfg_disable_mc_addr_filtering_in_fwr(): Disable cached mc addr list
 * @psoc: objmgr psoc handle
 * @vdev_id: vdev id
 * @gtk_req: pmo gtk req param
 * @action: true for enable els false
 *
 * API to disable cached mc add list in fwr
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_disable_mc_addr_filtering_in_fwr(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id,
		enum pmo_offload_trigger trigger);

/**
 * pmo_ucfg_cache_gtk_offload_req(): API to cache gtk req in pmo vdev priv obj
 * @vdev: objmgr vdev handle
 * @gtk_req: pmo gtk req param
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_cache_gtk_offload_req(struct wlan_objmgr_vdev *vdev,
		struct pmo_gtk_req *gtk_req);

/**
 * pmo_ucfg_flush_gtk_offload_req(): Flush saved gtk req from pmo vdev priv obj
 * @vdev: objmgr vdev handle
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_flush_gtk_offload_req(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_enable_gtk_offload_in_fwr(): enable cached gtk request in fwr
 * @vdev: objmgr vdev handle
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_enable_gtk_offload_in_fwr(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_disable_gtk_offload_in_fwr(): disable cached gtk request in fwr
 * @vdev: objmgr vdev handle
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_disable_gtk_offload_in_fwr(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_get_gtk_rsp(): API to send gtk response request to fwr
 * @vdev: objmgr vdev handle
 * @gtk_rsp: pmo gtk response request
 *
 * This api will send gtk response request to fwr
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_get_gtk_rsp(struct wlan_objmgr_vdev *vdev,
		struct pmo_gtk_rsp_req *gtk_rsp_req);

#endif /* end  of _WLAN_PMO_UCFG_API_H_ */
