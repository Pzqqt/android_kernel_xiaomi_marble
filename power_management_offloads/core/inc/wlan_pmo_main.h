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
 * DOC: declare various api which shall be used by
 * pmo user configuration and target interface
 */

#ifndef _WLAN_PMO_MAIN_H_
#define _WLAN_PMO_MAIN_H_

#include "wlan_cmn.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_objmgr_peer_obj.h"
#include "qdf_status.h"
#include "qdf_types.h"
#include "qdf_lock.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

#define pmo_log(level, args...) QDF_TRACE(QDF_MODULE_ID_HDD, level, ## args)
#define pmo_logfl(level, format, args...) pmo_log(level, FL(format), ## args)

#define pmo_fatal(format, args...) \
		pmo_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define pmo_err(format, args...) \
		pmo_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define pmo_warn(format, args...) \
		pmo_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define pmo_info(format, args...) \
		pmo_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define pmo_debug(format, args...) \
		pmo_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define PMO_ENTER() pmo_logfl(QDF_TRACE_LEVEL_INFO, "enter")
#define PMO_EXIT() pmo_logfl(QDF_TRACE_LEVEL_INFO, "exit")

static inline enum tQDF_ADAPTER_MODE pmo_get_vdev_opmode(
			struct wlan_objmgr_vdev *vdev)
{
	enum tQDF_ADAPTER_MODE opmode;

	wlan_vdev_obj_lock(vdev);
	opmode = wlan_vdev_mlme_get_opmode(vdev);
	wlan_vdev_obj_unlock(vdev);

	return opmode;
}

static inline uint8_t pmo_get_vdev_id(struct wlan_objmgr_vdev *vdev)
{
	uint8_t vdev_id;

	wlan_vdev_obj_lock(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	wlan_vdev_obj_unlock(vdev);

	return vdev_id;
}

/**
 * pmo_allocate_ctx() - Api to allocate pmo ctx
 *
 * Helper function to allocate pmo ctx
 *
 * Return: Success or failure.
 */
QDF_STATUS pmo_allocate_ctx(void);

/**
 * pmo_free_ctx() - to free pmo context
 *
 * Helper function to free pmo context
 *
 * Return: None.
 */
void pmo_free_ctx(void);

/**
 * pmo_get_context() - to get pmo context
 *
 * Helper function to get pmo context
 *
 * Return: pmo context.
 */
struct wlan_pmo_ctx *pmo_get_context(void);

/**
 * pmo_get_psoc_priv_ctx() - return pmo psoc priv ctx from objmgr psoc
 * @psoc: objmgr psoc
 *
 * Helper function to pmo psoc ctx from objmgr psoc
 *
 * Return: if success pmo psoc ctx else NULL
 */
struct pmo_psoc_priv_obj *pmo_get_psoc_priv_ctx(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_get_vdev_priv_ctx() - return pmo vdev priv ctx from objmgr vdev
 * @vdev: objmgr vdev
 *
 * Helper function to pmo vdev ctx from objmgr vdev
 *
 * Return: if success pmo vdev ctx else NULL
 */
struct pmo_vdev_priv_obj *pmo_get_vdev_priv_ctx(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_get_vdev_bss_peer_mac_addr() - API to get bss peer mac address
 * @vdev: objmgr vdev
 * @bss_peer_mac_address: bss peer mac address
 *.
 * Helper function to  get bss peer mac address
 *
 * Return: if success pmo vdev ctx else NULL
 */
QDF_STATUS pmo_get_vdev_bss_peer_mac_addr(struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr *bss_peer_mac_address);

/**
 * pmo_psoc_ctx_from_vdev_ctx() - return pmo psoc ctx from pmo vdev ctx
 * @vdev: pmo vdev ctx
 *
 * Helper function to get pmo psoc ctx from pmo vdev ctx
 *
 * Return: pmo psoc ctx
 */
struct pmo_psoc_priv_obj *pmo_psoc_ctx_from_vdev_ctx(
	struct pmo_vdev_priv_obj *vdev_ctx);

/**
 * pmo_is_vdev_in_beaconning_mode() - check if vdev is in a beaconning mode
 * @vdev_opmode: vdev opmode
 *
 * Helper function to know whether given vdev
 * is in a beaconning mode or not.
 *
 * Return: True if vdev needs to beacon.
 */
bool pmo_is_vdev_in_beaconning_mode(enum tQDF_ADAPTER_MODE vdev_opmode);

/**
 * pmo_core_is_ap_mode_supports_arp_ns() - To check ap mode supports arp/ns
 * @vdev_opmode: vdev opmode
 *
 * API to check if ap mode supports arp/ns offload
 *
 * Return: True  if ap mode supports arp/ns offload
 */

bool pmo_core_is_ap_mode_supports_arp_ns(struct wlan_objmgr_psoc *psoc,
	enum tQDF_ADAPTER_MODE vdev_opmode);

/**
 * pmo_core_is_vdev_connected() -  to check whether peer is associated or not
 * @vdev: objmgr vdev
 *
 * Return: true in case success else false
 */
bool pmo_core_is_vdev_connected(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_core_is_vdev_supports_offload() - Check offload is supported on vdev
 * @vdev: objmgr vdev
 *
 * Return: true in case success else false
 */
bool pmo_core_is_vdev_supports_offload(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_core_get_psoc_config(): API to get the psoc user configurations of pmo
 * @psoc: objmgr psoc handle
 * @psoc_cfg: fill the current psoc user configurations.
 *
 * Return pmo psoc configurations
 */
QDF_STATUS pmo_core_get_psoc_config(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_cfg *psoc_cfg);

/**
 * pmo_core_update_psoc_config(): API to update the psoc user configurations
 * @psoc: objmgr psoc handle
 * @psoc_cfg: pmo psoc configurations
 *
 * This api shall be used for soc config initialization as well update.
 * In case of update caller must first call pmo_get_psoc_cfg to get
 * current config and then apply changes on top of current config.
 *
 * Return QDF_STATUS -in case of success else return error
 */
QDF_STATUS pmo_core_update_psoc_config(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_cfg *psoc_cfg);

#endif /* end  of _WLAN_PMO_MAIN_H_ */
