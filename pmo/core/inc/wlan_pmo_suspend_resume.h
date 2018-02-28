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
 * DOC: Declare suspend / resume related API's
 */

#ifndef _WLAN_PMO_SUSPEND_RESUME_H_
#define _WLAN_PMO_SUSPEND_RESUME_H_

#include "wlan_pmo_common_public_struct.h"
#include "wlan_pmo_wow.h"

/**
 * pmo_core_configure_dynamic_wake_events(): configure dyanmic wake events
 * @wma: wma handle
 *
 * Some wake events need to be enabled dynamically. Control those here.
 *
 * Return: none
 */
void pmo_core_configure_dynamic_wake_events(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_core_get_wow_bus_suspend(): API to get wow bus is suspended or not
 * @psoc: objmgr psoc handle
 *
 * Return: True if bus suspende else false
 */
static inline bool pmo_core_get_wow_bus_suspend(struct wlan_objmgr_psoc *psoc)
{
	bool value;
	struct pmo_psoc_priv_obj *psoc_ctx;

	pmo_psoc_with_ctx(psoc, psoc_ctx) {
		value = psoc_ctx->wow.is_wow_bus_suspended;
	}

	return value;
}

/**
 * pmo_core_psoc_user_space_suspend_req() -  Core handle user space suspend req
 * @psoc: objmgr psoc handle
 * @type: type of suspend
 *
 * Pmo core Handles user space suspend request for psoc
 *
 * Return: QDF status
 */
QDF_STATUS pmo_core_psoc_user_space_suspend_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type);

/**
 * pmo_core_psoc_user_space_resume_req() - Core handle user space resume req
 * @psoc: objmgr psoc handle
 * @type: type of suspend from resume required
 *
 * Pmo core Handles user space resume request for psoc
 *
 * Return: QDF status
 */
QDF_STATUS pmo_core_psoc_user_space_resume_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type);

/**
 * pmo_core_psoc_bus_suspend_req(): handles bus suspend for psoc
 * @psoc: objmgr psoc
 * @type: is this suspend part of runtime suspend or system suspend?
 * @wow_params: collection of wow enable override parameters
 *
 * Bails if a scan is in progress.
 * Calls the appropriate handlers based on configuration and event.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_core_psoc_bus_suspend_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type,
		struct pmo_wow_enable_params *wow_params);

#ifdef FEATURE_RUNTIME_PM
/**
 * pmo_core_psoc_bus_runtime_suspend(): handles bus runtime suspend
 * @psoc: objmgr psoc
 * @pld_cb: callback to do link auto suspend
 *
 * Suspend the wlan bus without apps suspend.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_core_psoc_bus_runtime_suspend(struct wlan_objmgr_psoc *psoc,
					     pmo_pld_auto_suspend_cb pld_cb);

/**
 * pmo_core_psoc_bus_runtime_resume(): handles bus runtime resume
 * @psoc: objmgr psoc
 * @pld_cb: callback to do link auto resume
 *
 * Resume the wlan bus from runtime suspend.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_core_psoc_bus_runtime_resume(struct wlan_objmgr_psoc *psoc,
					     pmo_pld_auto_resume_cb pld_cb);
#endif

/**
 * pmo_core_psoc_suspend_target() -Send suspend target command
 * @psoc: objmgr psoc handle
 * @disable_target_intr: disable target interrupt
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS pmo_core_psoc_suspend_target(struct wlan_objmgr_psoc *psoc,
		int disable_target_intr);

/**
 * pmo_core_psoc_bus_resume() -handle bus resume request for psoc
 * @psoc: objmgr psoc handle
 * @type: is this suspend part of runtime suspend or system suspend?
 *
 * Return:QDF_STATUS_SUCCESS on success else error code
 */
QDF_STATUS pmo_core_psoc_bus_resume_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type);

/**
 * pmo_core_get_vdev_dtim_period() - Get vdev dtim period
 * @vdev: objmgr vdev handle
 *
 * Return: Vdev dtim period
 */
static inline
uint8_t pmo_core_get_vdev_dtim_period(struct wlan_objmgr_vdev *vdev)
{
	uint8_t dtim_period = 0;

	if (!vdev) {
		pmo_err("vdev is null");
		QDF_ASSERT(0);
		return 0;
	}
	/* TODO */
	/* dtim_period = wlan_vdev_mlme_get_dtim_period(vdev); */

	return dtim_period;
}

/**
 * pmo_core_get_vdev_beacon_interval() - Get vdev beacon interval
 * @vdev: objmgr vdev handle
 *
 * Return: Vdev beacon interval
 */
static inline
uint16_t pmo_core_get_vdev_beacon_interval(struct wlan_objmgr_vdev *vdev)
{
	uint16_t beacon_interval = 0;

	if (!vdev) {
		pmo_err("vdev is null");
		QDF_ASSERT(0);
		return 0;
	}
	/* TODO */
	/* beacon_interval = wlan_vdev_mlme_get_beacon_interval(vdev); */

	return beacon_interval;
}

/**
 * pmo_core_update_alt_modulated_dtim_enable() - update alt modulatate dtim
 * @vdev: objmgr vdev handle
 * @value: true when alt modulated dtim enable else false
 *
 * Return: None
 */
static inline
void pmo_core_update_alt_modulated_dtim_enable(struct wlan_objmgr_vdev *vdev,
	bool value)
{
	struct pmo_vdev_priv_obj *vdev_ctx;

	vdev_ctx = pmo_vdev_get_priv(vdev);
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->alt_modulated_dtim_enable = value;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

/**
 * pmo_core_vdev_set_dtim_policy() - Set vdev beacon dtim policy
 * @vdev: objmgr vdev handle
 * @value: carry vdev dtim policy
 *
 * Return: None
 */
static inline
void pmo_core_vdev_set_dtim_policy(struct wlan_objmgr_vdev *vdev,
	uint32_t value)
{
	struct pmo_vdev_priv_obj *vdev_ctx;

	vdev_ctx = pmo_vdev_get_priv(vdev);
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->dtim_policy = value;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

/**
 * pmo_core_vdev_get_dtim_policy() - Get vdev beacon dtim policy
 * @vdev: objmgr vdev handle
 *
 * Return: vdev dtim policy
 */
static inline
uint32_t pmo_core_vdev_get_dtim_policy(struct wlan_objmgr_vdev *vdev)
{
	uint32_t value;
	struct pmo_vdev_priv_obj *vdev_ctx;

	vdev_ctx = pmo_vdev_get_priv(vdev);
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	value = vdev_ctx->dtim_policy;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

	return value;
}

/**
 * pmo_core_update_power_save_mode() - update power save mode
 * @vdev: objmgr vdev handle
 * @value:describe vdev power save mode
 *
 * Return: None
 */
static inline void
pmo_core_psoc_update_power_save_mode(struct wlan_objmgr_psoc *psoc,
				     uint8_t value)
{
	struct pmo_psoc_priv_obj *psoc_ctx;

	pmo_psoc_with_ctx(psoc, psoc_ctx) {
		psoc_ctx->psoc_cfg.power_save_mode = value;
	}
}

/**
 * pmo_core_psoc_get_power_save_mode() - Get psoc power save mode
 * @psoc: objmgr psoc handle
 *
 * Return: vdev psoc power save mode value
 */
static inline uint8_t
pmo_core_psoc_get_power_save_mode(struct wlan_objmgr_psoc *psoc)
{
	uint8_t value;
	struct pmo_psoc_priv_obj *psoc_ctx;

	pmo_psoc_with_ctx(psoc, psoc_ctx) {
		value = psoc_ctx->psoc_cfg.power_save_mode;
	}

	return value;
}

/**
 * pmo_core_psoc_get_qpower_config() - get qpower configuration
 * @psoc: objmgr psoc handle
 *
 * Power Save Offload configuration:
 * 0 -> Power save offload is disabled
 * 1 -> Legacy Power save enabled + Deep sleep Disabled
 * 2 -> QPower enabled + Deep sleep Disabled
 * 3 -> Legacy Power save enabled + Deep sleep Enabled
 * 4 -> QPower enabled + Deep sleep Enabled
 * 5 -> Duty cycling QPower enabled
 *
 * Return: enum powersave_qpower_mode with below values
 * QPOWER_DISABLED if QPOWER is disabled
 * QPOWER_ENABLED if QPOWER is enabled
 * QPOWER_DUTY_CYCLING if DUTY CYCLING QPOWER is enabled
 */
static inline
enum pmo_power_save_qpower_mode pmo_core_psoc_get_qpower_config(
		struct wlan_objmgr_psoc *psoc)
{
	uint8_t ps_mode = pmo_core_psoc_get_power_save_mode(psoc);

	switch (ps_mode) {
	case pmo_ps_qpower_no_deep_sleep:
	case pmo_ps_qpower_deep_sleep:
		pmo_debug("QPOWER is enabled in power save mode %d", ps_mode);
		return pmo_qpower_enabled;
	case pmo_ps_duty_cycling_qpower:
		pmo_debug("DUTY cycling QPOWER is enabled in power save mode %d",
			ps_mode);
		return pmo_qpower_duty_cycling;
	default:
		pmo_debug("QPOWER is disabled in power save mode %d",
			ps_mode);
		return pmo_qpower_disabled;
	}
}

/**
 * pmo_core_vdev_get_pause_bitmap() - Get vdev pause bitmap
 * @psoc_ctx: psoc priv ctx
 * @vdev_id: vdev id
 *
 * Return: vdev pause bitmap
 */
static inline
uint16_t pmo_core_vdev_get_pause_bitmap(struct pmo_psoc_priv_obj *psoc_ctx,
		uint8_t vdev_id)
{
	uint16_t value = 0;
	pmo_get_pause_bitmap handler;

	qdf_spin_lock_bh(&psoc_ctx->lock);
	handler = psoc_ctx->get_pause_bitmap;
	qdf_spin_unlock_bh(&psoc_ctx->lock);

	if (handler)
		value = handler(vdev_id);

	return value;
}

/**
 * wma_is_vdev_in_ap_mode() - check that vdev is in ap mode or not
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Helper function to know whether given vdev id
 * is in AP mode or not.
 *
 * Return: True/False
 */
static inline
bool pmo_is_vdev_in_ap_mode(struct wlan_objmgr_vdev *vdev)
{
	enum QDF_OPMODE mode;

	mode = pmo_get_vdev_opmode(vdev);

	return (mode == QDF_SAP_MODE || mode == QDF_P2P_GO_MODE) == 1 ? 1 : 0;
}

#ifdef QCA_IBSS_SUPPORT
/**
 * pmo_is_vdev_in_ibss_mode() - check that vdev is in ibss mode or not
 * @vdev: objmgr vdev handle
 * @vdev_id: vdev id
 *
 * Helper function to know whether given vdev id
 * is in IBSS mode or not.
 *
 * Return: True/False
 */
static inline
bool pmo_is_vdev_in_ibss_mode(struct wlan_objmgr_vdev *vdev)
{
	enum QDF_OPMODE mode;

	mode = pmo_get_vdev_opmode(vdev);

	return (mode == QDF_IBSS_MODE) ? true : false;
}
#else
static inline bool pmo_is_vdev_in_ibss_mode(struct wlan_objmgr_vdev *vdev)
{
	return false;
}
#endif /* QCA_IBSS_SUPPORT */

/**
 * pmo_handle_initial_wake_up() - handle initial wake up
 * @cb_ctx: callback context
 *
 * Return: None
 */
void pmo_core_psoc_handle_initial_wake_up(void *cb_ctx);

/**
 * pmo_core_psoc_is_target_wake_up_received() - check for initial wake up
 *
 * Check if target initial wake up is received and fail PM suspend gracefully
 *
 * Return: -EAGAIN if initial wake up is received else 0
 */
int pmo_core_psoc_is_target_wake_up_received(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_core_psoc_clear_target_wake_up() - clear initial wake up
 *
 * Clear target initial wake up reason
 *
 * Return: 0 for success and negative error code for failure
 */
int pmo_core_psoc_clear_target_wake_up(struct wlan_objmgr_psoc *psoc);

/**
 * pmo_core_psoc_target_suspend_acknowledge() - update target susspend status
 * @context: HTC_INIT_INFO->context
 * @wow_nack: true when wow is rejected
 *
 * Return: none
 */
void pmo_core_psoc_target_suspend_acknowledge(void *context, bool wow_nack);

/**
 * pmo_core_psoc_wakeup_host_event_received() - received host wake up event
 * @psoc: objmgr psoc handle
 *
 * Return: None
 */
void pmo_core_psoc_wakeup_host_event_received(struct wlan_objmgr_psoc *psoc);

#endif /* end  of _WLAN_PMO_SUSPEND_RESUME_H_ */
