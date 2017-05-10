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

#define PMO_VDEV_IN_STA_MODE(mode)\
	((mode == QDF_STA_MODE || mode == QDF_P2P_CLIENT_MODE) == 1 ? 1 : 0)

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


/**
 * pmo_core_get_vdev_op_mode(): API to get the vdev operation mode
 * @vdev: objmgr vdev handle
 *
 * API to get the vdev operation mode
 *
 * Return QDF_MAX_NO_OF_MODE - in case of error else return vdev opmode
 */
static inline enum tQDF_ADAPTER_MODE pmo_core_get_vdev_op_mode(
					struct wlan_objmgr_vdev *vdev)
{
	enum tQDF_ADAPTER_MODE op_mode = QDF_MAX_NO_OF_MODE;

	if (!vdev)
		return op_mode;
	wlan_vdev_obj_lock(vdev);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	wlan_vdev_obj_unlock(vdev);

	return op_mode;
}

/**
 * pmo_core_psoc_update_dp_handle() - update psoc data path handle
 * @psoc: objmgr psoc handle
 * @dp_hdl: psoc data path handle
 *
 * Return: None
 */
static inline
void pmo_core_psoc_update_dp_handle(struct wlan_objmgr_psoc *psoc,
	void *dp_hdl)
{
	struct pmo_psoc_priv_obj *psoc_ctx;

	psoc_ctx = pmo_get_psoc_priv_ctx(psoc);
	if (!psoc_ctx)
		return;
	qdf_spin_lock_bh(&psoc_ctx->lock);
	psoc_ctx->dp_hdl = dp_hdl;
	qdf_spin_unlock_bh(&psoc_ctx->lock);
}

/**
 * pmo_core_psoc_get_dp_handle() - Get psoc data path handle
 * @psoc: objmgr psoc handle
 *
 * Return: psoc data path handle
 */
static inline
void *pmo_core_psoc_get_dp_handle(struct wlan_objmgr_psoc *psoc)
{
	void *dp_hdl;
	struct pmo_psoc_priv_obj *psoc_ctx;

	psoc_ctx = pmo_get_psoc_priv_ctx(psoc);
	if (!psoc_ctx)
		return NULL;
	qdf_spin_lock_bh(&psoc_ctx->lock);
	dp_hdl = psoc_ctx->dp_hdl;
	qdf_spin_unlock_bh(&psoc_ctx->lock);

	return dp_hdl;
}

/**
 * pmo_core_vdev_update_dp_handle() - update vdev data path handle
 * @vdev: objmgr vdev handle
 * @dp_hdl: Vdev data path handle
 *
 * Return: None
 */
static inline
void pmo_core_vdev_update_dp_handle(struct wlan_objmgr_vdev *vdev,
	void *dp_hdl)
{
	struct pmo_vdev_priv_obj *vdev_ctx;

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx)
		return;
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->vdev_dp_hdl = dp_hdl;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

/**
 * pmo_core_vdev_get_dp_handle() - Get vdev data path handle
 * @vdev: objmgr vdev handle
 *
 * Return: Vdev data path handle
 */
static inline
void *pmo_core_vdev_get_dp_handle(struct wlan_objmgr_vdev *vdev)
{
	void *dp_hdl;
	struct pmo_vdev_priv_obj *vdev_ctx;

	vdev_ctx = pmo_get_vdev_priv_ctx(vdev);
	if (!vdev_ctx)
		return NULL;
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	dp_hdl = vdev_ctx->vdev_dp_hdl;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

	return dp_hdl;
}

/**
 * pmo_core_psoc_update_htc_handle() - update psoc htc layer handle
 * @psoc: objmgr psoc handle
 * @htc_hdl: psoc htc layer handle
 *
 * Return: None
 */
static inline
void pmo_core_psoc_update_htc_handle(struct wlan_objmgr_psoc *psoc,
	void *htc_hdl)
{
	struct pmo_psoc_priv_obj *psoc_ctx;

	psoc_ctx = pmo_get_psoc_priv_ctx(psoc);
	if (!psoc_ctx)
		return;
	qdf_spin_lock_bh(&psoc_ctx->lock);
	psoc_ctx->htc_hdl = htc_hdl;
	qdf_spin_unlock_bh(&psoc_ctx->lock);
}

/**
 * pmo_core_psoc_get_htc_handle() - Get psoc htc layer handle
 * @psoc: objmgr psoc handle
 *
 * Return: psoc htc layer handle
 */
static inline
void *pmo_core_psoc_get_htc_handle(struct wlan_objmgr_psoc *psoc)
{
	void *htc_hdl;
	struct pmo_psoc_priv_obj *psoc_ctx;

	psoc_ctx = pmo_get_psoc_priv_ctx(psoc);
	if (!psoc_ctx)
		return NULL;
	qdf_spin_lock_bh(&psoc_ctx->lock);
	htc_hdl = psoc_ctx->htc_hdl;
	qdf_spin_unlock_bh(&psoc_ctx->lock);

	return htc_hdl;
}

/**
 * pmo_core_psoc_set_hif_handle() - update psoc hif layer handle
 * @psoc: objmgr psoc handle
 * @hif_hdl: hif context handle
 *
 * Return: None
 */
void pmo_core_psoc_set_hif_handle(struct wlan_objmgr_psoc *psoc,
				  void *hif_hdl);

/**
 * pmo_core_psoc_get_hif_handle() - Get psoc hif layer handle
 * @psoc: objmgr psoc handle
 *
 * Return: psoc hif layer handle
 */
void *pmo_core_psoc_get_hif_handle(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_core_psoc_set_txrx_handle() - update psoc pdev txrx layer handle
 * @psoc: objmgr psoc handle
 * @txrx_hdl: pdev txrx context handle
 *
 * Return: None
 */
void pmo_core_psoc_set_txrx_handle(struct wlan_objmgr_psoc *psoc,
				   void *txrx_hdl);

/**
 * pmo_core_psoc_get_txrx_handle() - Get psoc pdev txrx handle
 * @psoc: objmgr psoc handle
 *
 * Return: pdev txrx handle
 */
void *pmo_core_psoc_get_txrx_handle(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_is_vdev_up() - API to check whether vdev is UP
 * @vdev: objmgr vdev handle
 *
 * Return:true if vdev is up else false
 */
static inline
bool pmo_is_vdev_up(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state = WLAN_VDEV_S_INIT;

	if (!vdev) {
		pmo_err("vdev context is invalid!");
		return false;
	}
	wlan_vdev_obj_lock(vdev);
	state = wlan_vdev_mlme_get_state(vdev);
	wlan_vdev_obj_unlock(vdev);

	return (state == WLAN_VDEV_S_RUN) ? true : false;
}

#endif /* end  of _WLAN_PMO_MAIN_H_ */
