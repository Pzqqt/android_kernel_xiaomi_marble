/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
#include "wlan_pmo_mc_addr_filtering.h"
#include "wlan_pmo_mc_addr_filtering_public_struct.h"
#include "wlan_pmo_wow_public_struct.h"
#include "wlan_pmo_common_public_struct.h"
#include "wlan_pmo_obj_mgmt_api.h"
#include "wlan_pmo_pkt_filter_public_struct.h"
#include "wlan_pmo_hw_filter_public_struct.h"

/**
 * ucfg_pmo_is_ap_mode_supports_arp_ns() - Check ap mode support arp&ns offload
 * @psoc: objmgr psoc
 * @vdev_opmode: vdev opmode
 *
 * Return: true in case support else false
 */
bool ucfg_pmo_is_ap_mode_supports_arp_ns(struct wlan_objmgr_psoc *psoc,
	enum QDF_OPMODE vdev_opmode);

/**
 * ucfg_pmo_is_vdev_connected() -  to check whether peer is associated or not
 * @vdev: objmgr vdev
 *
 * Return: true in case success else false
 */
bool ucfg_pmo_is_vdev_connected(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_pmo_is_vdev_supports_offload() - check offload is supported on vdev
 * @vdev: objmgr vdev
 *
 * Return: true in case success else false
 */
bool ucfg_pmo_is_vdev_supports_offload(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_pmo_get_psoc_config(): API to get the psoc user configurations of pmo
 * @psoc: objmgr psoc handle
 * @psoc_cfg: fill the current psoc user configurations.
 *
 * Return pmo psoc configurations
 */
QDF_STATUS ucfg_pmo_get_psoc_config(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_cfg *psoc_cfg);

/**
 * ucfg_pmo_update_psoc_config(): API to update the psoc user configurations
 * @psoc: objmgr psoc handle
 * @psoc_cfg: pmo psoc configurations
 *
 * This api shall be used for soc config initialization as well update.
 * In case of update caller must first call pmo_get_psoc_cfg to get
 * current config and then apply changes on top of current config.
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS ucfg_pmo_update_psoc_config(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_cfg *psoc_cfg);

/**
 * ucfg_pmo_psoc_set_caps() - overwrite configured device capability flags
 * @psoc: the psoc for which the capabilities apply
 * @caps: the cabability information to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_pmo_psoc_set_caps(struct wlan_objmgr_psoc *psoc,
				  struct pmo_device_caps *caps);

/**
 * pmo_ucfg_enable_wakeup_event() -  enable wow wakeup events
 * @psoc: objmgr psoc
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 *
 * Return: none
 */
void pmo_ucfg_enable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t *bitmap);

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
 * pmo_ucfg_enable_hw_filter_in_fwr() - enable previously configured hw filter
 * @vdev: objmgr vdev to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS pmo_ucfg_enable_hw_filter_in_fwr(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_ucfg_disable_hw_filter_in_fwr() - disable previously configured hw filter
 * @vdev: objmgr vdev to configure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS pmo_ucfg_disable_hw_filter_in_fwr(struct wlan_objmgr_vdev *vdev);

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
 * pmo_ucfg_enhance_mc_filter_enable() - enable enhanced multicast filtering
 * @vdev: the vdev to enable enhanced multicast filtering for
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
pmo_ucfg_enhanced_mc_filter_enable(struct wlan_objmgr_vdev *vdev)
{
	return pmo_core_enhanced_mc_filter_enable(vdev);
}

/**
 * pmo_ucfg_enhance_mc_filter_disable() - disable enhanced multicast filtering
 * @vdev: the vdev to disable enhanced multicast filtering for
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
pmo_ucfg_enhanced_mc_filter_disable(struct wlan_objmgr_vdev *vdev)
{
	return pmo_core_enhanced_mc_filter_disable(vdev);
}

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
 * pmo_ucfg_set_pkt_filter() - Set packet filter
 * @psoc: objmgr psoc handle
 * @pmo_set_pkt_fltr_req:
 * @vdev_id: vdev id
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_set_pkt_filter(struct wlan_objmgr_psoc *psoc,
	struct pmo_rcv_pkt_fltr_cfg *pmo_set_pkt_fltr_req,
	uint8_t vdev_id);

/**
 * pmo_ucfg_clear_pkt_filter() - Clear packet filter
 * @psoc: objmgr psoc handle
 * @pmo_clr_pkt_fltr_req:
 * @vdev_id: vdev id
 *
 * Return QDF_STATUS_SUCCESS -in case of success else return error
 */
QDF_STATUS pmo_ucfg_clear_pkt_filter(struct wlan_objmgr_psoc *psoc,
	struct pmo_rcv_pkt_fltr_clear_param *pmo_clr_pkt_fltr_param,
	uint8_t vdev_id);

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

/**
 * pmo_ucfg_update_extscan_in_progress(): update extscan is in progress flags
 * @vdev: objmgr vdev handle
 * @value:true if extscan is in progress else false
 *
 * Return: TRUE/FALSE
 */
void pmo_ucfg_update_extscan_in_progress(struct wlan_objmgr_vdev *vdev,
	bool value);

/**
 * pmo_ucfg_update_p2plo_in_progress(): update p2plo is in progress flags
 * @vdev: objmgr vdev handle
 * @value:true if p2plo is in progress else false
 *
 * Return: TRUE/FALSE
 */
void pmo_ucfg_update_p2plo_in_progress(struct wlan_objmgr_vdev *vdev,
	bool value);

/**
 * pmo_ucfg_lphb_config_req() -  Handles lphb config request for psoc
 * @psoc: objmgr psoc handle
 * @lphb_req: low power heart beat request
 * @lphb_cb_ctx: Context which needs to pass to soif when lphb callback called
 * @callback: upon receiving of lphb indication from fwr call lphb callback
 *
 * Return: QDF status
 */
QDF_STATUS pmo_ucfg_lphb_config_req(struct wlan_objmgr_psoc *psoc,
		struct pmo_lphb_req *lphb_req, void *lphb_cb_ctx,
		pmo_lphb_callback callback);

/**
 * pmo_ucfg_update_alt_modulated_dtim_enable() - update alt modulatate dtim
 * @vdev: objmgr vdev handle
 * @value: true for alt_modulated_dtim enable else false
 *
 * Return: QDF status
 */
void pmo_ucfg_update_alt_modulated_dtim_enable(struct wlan_objmgr_vdev *vdev,
	bool value);

/**
 * pmo_ucfg_psoc_update_power_save_mode() - update power save mode
 * @vdev: objmgr vdev handle
 * @value:vdev power save mode
 *
 * Return: None
 */
void pmo_ucfg_psoc_update_power_save_mode(struct wlan_objmgr_psoc *psoc,
	uint8_t value);

/**
 * pmo_ucfg_psoc_update_dp_handle() - update psoc data path handle
 * @psoc: objmgr psoc handle
 * @dp_hdl: psoc data path handle
 *
 * Return: None
 */
void pmo_ucfg_psoc_update_dp_handle(struct wlan_objmgr_psoc *psoc,
	void *dp_hdl);

/**
 * pmo_ucfg_vdev_update_dp_handle() - update vdev data path handle
 * @vdev: objmgr vdev handle
 * @dp_hdl: vdev data path handle
 *
 * Return: None
 */
void pmo_ucfg_vdev_update_dp_handle(struct wlan_objmgr_vdev *vdev,
	void *dp_hdl);

/**
 * pmo_ucfg_psoc_update_htc_handle() - update psoc htc layer handle
 * @psoc: objmgr psoc handle
 * @htc_handle: psoc host-to-tagret layer (htc) handle
 *
 * Return: None
 */
void pmo_ucfg_psoc_update_htc_handle(struct wlan_objmgr_psoc *psoc,
		void *htc_handle);

/**
 * pmo_ucfg_psoc_set_hif_handle() - Set psoc hif layer handle
 * @psoc: objmgr psoc handle
 * @hif_handle: hif context handle
 *
 * Return: None
 */
void pmo_ucfg_psoc_set_hif_handle(struct wlan_objmgr_psoc *psoc,
				  void *hif_handle);

/**
 * pmo_ucfg_psoc_set_txrx_handle() - Set psoc pdev txrx layer handle
 * @psoc: objmgr psoc handle
 * @txrx_handle: pdev txrx context handle
 *
 * Return: None
 */
void pmo_ucfg_psoc_set_txrx_handle(struct wlan_objmgr_psoc *psoc,
				   void *txrx_handle);

/**
 * pmo_ucfg_psoc_user_space_suspend_req() -  Handles user space suspend req
 * @psoc: objmgr psoc handle
 * @type: type of suspend
 *
 * Handles user space suspend indication for psoc
 *
 * Return: QDF status
 */
QDF_STATUS pmo_ucfg_psoc_user_space_suspend_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type);

/**
 * pmo_ucfg_psoc_user_space_resume_req() -  Handles user space resume req
 * @psoc: objmgr psoc handle
 * @type: type of suspend from which resume needed
 *
 * Handles user space resume indication for psoc
 *
 * Return: QDF status
 */
QDF_STATUS pmo_ucfg_psoc_user_space_resume_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type);

/**
 * pmo_ucfg_psoc_bus_suspend_req(): handles bus suspend for psoc
 * @psoc: objmgr psoc
 * @type: is this suspend part of runtime suspend or system suspend?
 * @wow_params: collection of wow enable override parameters
 *
 * Bails if a scan is in progress.
 * Calls the appropriate handlers based on configuration and event.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_ucfg_psoc_bus_suspend_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type,
		struct pmo_wow_enable_params *wow_params);

#ifdef FEATURE_RUNTIME_PM
/**
 * pmo_ucfg_psoc_bus_runtime_suspend(): handles bus runtime suspend for psoc
 * @psoc: objmgr psoc
 * @pld_cb: callback to call link auto suspend
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_ucfg_psoc_bus_runtime_suspend(struct wlan_objmgr_psoc *psoc,
					     pmo_pld_auto_suspend_cb pld_cb);

/**
 * pmo_ucfg_psoc_bus_runtime_resume(): handles bus runtime resume for psoc
 * @psoc: objmgr psoc
 * @pld_cb: callback to call link auto resume
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_ucfg_psoc_bus_runtime_resume(struct wlan_objmgr_psoc *psoc,
					    pmo_pld_auto_resume_cb pld_cb);
#endif

/**
 * pmo_ucfg_psoc_suspend_target() -Send suspend target command
 * @psoc: objmgr psoc handle
 * @disable_target_intr: disable target interrupt
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_ucfg_psoc_suspend_target(struct wlan_objmgr_psoc *psoc,
		int disable_target_intr);

QDF_STATUS pmo_ucfg_add_wow_user_pattern(struct wlan_objmgr_vdev *vdev,
		struct pmo_wow_add_pattern *ptrn);

QDF_STATUS pmo_ucfg_del_wow_user_pattern(struct wlan_objmgr_vdev *vdev,
		uint8_t pattern_id);

/**
 * pmo_ucfg_psoc_bus_resume() -handle bus resume request for psoc
 * @psoc: objmgr psoc handle
 * @type: is this suspend part of runtime suspend or system suspend?
 *
 * Return:QDF_STATUS_SUCCESS on success else error code
 */
QDF_STATUS pmo_ucfg_psoc_bus_resume_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type);

/**
 * pmo_ucfg_get_wow_bus_suspend(): API to check if wow bus is suspended or not
 * @psoc: objmgr psoc handle
 *
 * Return: True if bus suspende else false
 */
bool pmo_ucfg_get_wow_bus_suspend(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_ucfg_psoc_handle_initial_wake_up() - update initial wake up
 * @cb_ctx: objmgr psoc handle as void * due to htc layer is not aware psoc
 *
 * Return: None
 */
void pmo_ucfg_psoc_handle_initial_wake_up(void *cb_ctx);

/**
 * pmo_ucfg_psoc_is_target_wake_up_received() - Get initial wake up status
 * @psoc: objmgr psoc handle
 *
 * Return: 0 on success else error code
 */
int pmo_ucfg_psoc_is_target_wake_up_received(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_ucfg_psoc_is_target_wake_up_received() - Clear initial wake up status
 * @psoc: objmgr psoc handle
 *
 * Return: 0 on success else error code
 */
int pmo_ucfg_psoc_clear_target_wake_up(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_ucfg_psoc_target_suspend_acknowledge() - Clear initial wake up status
 * @psoc: objmgr psoc handle
 *
 * Return: None
 */
void pmo_ucfg_psoc_target_suspend_acknowledge(void *context, bool wow_nack);

/**
 * pmo_ucfg_psoc_wakeup_host_event_received() - got host wake up evennt from fwr
 * @psoc: objmgr psoc handle
 *
 * Return: None
 */
void pmo_ucfg_psoc_wakeup_host_event_received(struct wlan_objmgr_psoc *psoc);

#endif /* end  of _WLAN_PMO_UCFG_API_H_ */
