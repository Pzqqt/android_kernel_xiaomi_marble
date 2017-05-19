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
 * DOC: Define API's for suspend / resume handling
 */

#include "wlan_pmo_wow.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_main.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"
#include "wlan_pmo_lphb.h"
#include "wlan_pmo_suspend_resume.h"
#include "cdp_txrx_ops.h"
#include "cdp_txrx_misc.h"
#include "cdp_txrx_flow_ctrl_legacy.h"
#include "hif.h"
#include "htc_api.h"
#include "wlan_pmo_obj_mgmt_api.h"
#include <wlan_scan_ucfg_api.h>


/**
 * pmo_core_calculate_listen_interval() - Calculate vdev listen interval
 * @vdev: objmgr vdev handle
 * @vdev_ctx: pmo vdev priv ctx
 * @listen_interval: listen interval which is computed for vdev
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS pmo_core_calculate_listen_interval(
			struct wlan_objmgr_vdev *vdev,
			struct pmo_vdev_priv_obj *vdev_ctx,
			uint32_t *listen_interval)
{
	uint32_t max_mod_dtim;
	uint32_t beacon_interval_mod;
	struct pmo_psoc_cfg *psoc_cfg = &vdev_ctx->pmo_psoc_ctx->psoc_cfg;

	if (psoc_cfg->sta_dynamic_dtim) {
		*listen_interval = psoc_cfg->sta_dynamic_dtim;
	} else if ((psoc_cfg->sta_mod_dtim) &&
		   (psoc_cfg->sta_max_li_mod_dtim)) {
		/*
		 * When the system is in suspend
		 * (maximum beacon will be at 1s == 10)
		 * If maxModulatedDTIM ((MAX_LI_VAL = 10) / AP_DTIM)
		 * equal or larger than MDTIM
		 * (configured in WCNSS_qcom_cfg.ini)
		 * Set LI to MDTIM * AP_DTIM
		 * If Dtim = 2 and Mdtim = 2 then LI is 4
		 * Else
		 * Set LI to maxModulatedDTIM * AP_DTIM
		 */
		beacon_interval_mod =
			pmo_core_get_vdev_beacon_interval(vdev) / 100;
		if (beacon_interval_mod == 0)
			beacon_interval_mod = 1;

		max_mod_dtim = psoc_cfg->sta_max_li_mod_dtim /
			(pmo_core_get_vdev_dtim_period(vdev)
			 * beacon_interval_mod);

		if (max_mod_dtim <= 0)
			max_mod_dtim = 1;

		if (max_mod_dtim >= psoc_cfg->sta_mod_dtim) {
			*listen_interval =
				(psoc_cfg->sta_mod_dtim *
				pmo_core_get_vdev_dtim_period(vdev));
		} else {
			*listen_interval =
				(max_mod_dtim *
				pmo_core_get_vdev_dtim_period(vdev));
		}
	} else {
		return QDF_STATUS_E_FAULT;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * pmo_core_set_vdev_suspend_dtim() - set suspend dtim parameters in fw
 * @psoc: objmgr psoc handle
 * @vdev: objmgr vdev handle
 * @vdev_ctx: pmo vdev priv ctx
 *
 * Return: none
 */
static void pmo_core_set_vdev_suspend_dtim(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_vdev *vdev,
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	uint32_t listen_interval;
	QDF_STATUS ret;
	uint8_t vdev_id;
	enum pmo_power_save_qpower_mode qpower_config;
	enum tQDF_ADAPTER_MODE opmode = pmo_core_get_vdev_op_mode(vdev);

	qpower_config = pmo_core_psoc_get_qpower_config(psoc);
	vdev_id = pmo_vdev_get_id(vdev);
	if (PMO_VDEV_IN_STA_MODE(opmode) &&
	    pmo_core_get_vdev_dtim_period(vdev) != 0) {
		/* calculate listen interval */
		ret = pmo_core_calculate_listen_interval(vdev, vdev_ctx,
				&listen_interval);
		if (ret != QDF_STATUS_SUCCESS) {
			/* even it fails continue fwr will take default LI */
			pmo_info("Fail to calculate listen interval");
		}

		ret = pmo_tgt_vdev_update_param_req(vdev,
				pmo_vdev_param_listen_interval,
				listen_interval);
		if (QDF_IS_STATUS_ERROR(ret)) {
			/* even it fails continue fwr will take default LI */
			pmo_info("Failed to Set Listen Interval vdevId %d",
				 vdev_id);
		}
		pmo_debug("Set Listen Interval vdevId %d Listen Intv %d",
			 vdev_id, listen_interval);

		if (qpower_config) {
			pmo_debug("disable Qpower in suspend mode!");
			ret = pmo_tgt_send_vdev_sta_ps_param(vdev,
					pmo_sta_ps_enable_qpower, 0);
			if (QDF_IS_STATUS_ERROR(ret))
				pmo_info("Failed to disable Qpower in suspend mode!");
		}

		ret = pmo_tgt_vdev_update_param_req(vdev,
				pmo_vdev_param_dtim_policy,
				pmo_normal_dtim);
		if (QDF_IS_STATUS_ERROR(ret))
			pmo_info("Failed to Set to Normal DTIM vdevId %d",
				vdev_id);

		/* Set it to Normal DTIM */
		pmo_core_vdev_set_dtim_policy(vdev, pmo_normal_dtim);
		pmo_debug("Set DTIM Policy to Normal Dtim vdevId %d", vdev_id);
	}
}

/**
 * pmo_core_set_suspend_dtim() - set suspend dtim
 * @psoc: objmgr psoc handle
 *
 * Return: none
 */
static void pmo_core_set_suspend_dtim(struct wlan_objmgr_psoc *psoc)
{
	uint8_t vdev_id;
	struct wlan_objmgr_vdev *vdev;
	struct pmo_vdev_priv_obj *vdev_ctx;
	bool alt_mdtim_enabled;
	QDF_STATUS status;

	/* Iterate through VDEV list */
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) {
		vdev = pmo_psoc_get_vdev(psoc, vdev_id);
		if (!vdev)
			continue;

		status = pmo_vdev_get_ref(vdev);
		if (QDF_IS_STATUS_ERROR(status))
			continue;

		vdev_ctx = pmo_vdev_get_priv(vdev);
		qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
		alt_mdtim_enabled = vdev_ctx->alt_modulated_dtim_enable;
		qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

		if (!alt_mdtim_enabled)
			pmo_core_set_vdev_suspend_dtim(psoc, vdev, vdev_ctx);

		pmo_vdev_put_ref(vdev);
	}
}

/**
 * pmo_core_update_wow_bus_suspend() - set wow bus suspend flag
 * @psoc: objmgr psoc handle
 * @psoc_ctx: pmo psoc priv ctx
 * @val: true for enable else false
 * Return: none
 */
static inline
void pmo_core_update_wow_bus_suspend(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_priv_obj *psoc_ctx, int val)
{
	qdf_spin_lock_bh(&psoc_ctx->lock);
	psoc_ctx->wow.is_wow_bus_suspended = val;
	qdf_spin_unlock_bh(&psoc_ctx->lock);
	pmo_tgt_psoc_update_wow_bus_suspend_state(psoc, val);
}

void pmo_core_configure_dynamic_wake_events(struct wlan_objmgr_psoc *psoc)
{
	int vdev_id;
	int enable_mask;
	int disable_mask;
	struct wlan_objmgr_psoc_objmgr *objmgr;
	struct wlan_objmgr_vdev *vdev;

	/* Iterate through VDEV list */
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) {
		wlan_psoc_obj_lock(psoc);
		objmgr = &psoc->soc_objmgr;
		if (!objmgr->wlan_vdev_list[vdev_id]) {
			wlan_psoc_obj_unlock(psoc);
			continue;
		}
		vdev = objmgr->wlan_vdev_list[vdev_id];
		wlan_psoc_obj_unlock(psoc);

		enable_mask = 0;
		disable_mask = 0;
		if (ucfg_scan_get_pno_in_progress(vdev)) {
			if (ucfg_scan_get_pno_match(vdev))
				enable_mask |=
					(1 << WOW_NLO_SCAN_COMPLETE_EVENT);
			else
				disable_mask |=
					(1 << WOW_NLO_SCAN_COMPLETE_EVENT);
		}

		if (enable_mask != 0)
			pmo_core_enable_wakeup_event(psoc, vdev_id,
				enable_mask);
		if (disable_mask != 0)
			pmo_core_disable_wakeup_event(psoc, vdev_id,
					disable_mask);
	}
}

/**
 * pmo_core_psoc_configure_suspend(): configure suspend req events
 * @psoc: objmgr psoc
 *
 * Responsibility of the caller to take the psoc reference.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS pmo_core_psoc_configure_suspend(struct wlan_objmgr_psoc *psoc)
{
	struct pmo_psoc_priv_obj *psoc_ctx;

	PMO_ENTER();

	psoc_ctx = pmo_psoc_get_priv(psoc);

	if (pmo_core_is_wow_applicable(psoc)) {
		pmo_info("WOW Suspend");
		pmo_core_apply_lphb(psoc);
		pmo_core_configure_dynamic_wake_events(psoc);
		pmo_core_update_wow_enable(psoc_ctx, true);
		pmo_core_update_wow_enable_cmd_sent(psoc_ctx, false);
	}

	pmo_core_set_suspend_dtim(psoc);

	/*
	 * To handle race between hif_pci_suspend and unpause/pause tx handler.
	 * This happens when host sending WMI_WOW_ENABLE_CMDID to FW and receive
	 * WMI_TX_PAUSE_EVENT with ACTON_UNPAUSE almost at same time.
	 */
	pmo_core_update_wow_bus_suspend(psoc, psoc_ctx, true);

	PMO_EXIT();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_psoc_user_space_suspend_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type)
{
	QDF_STATUS status;

	PMO_ENTER();

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("pmo cannot get the reference out of psoc");
		goto out;
	}

	/* Suspend all components before sending target suspend command */
	status = pmo_suspend_all_components(psoc, type);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to suspend all component");
		goto dec_psoc_ref;
	}

	status = pmo_core_psoc_configure_suspend(psoc);
	if (status != QDF_STATUS_SUCCESS)
		pmo_err("Failed to configure suspend");

dec_psoc_ref:
	pmo_psoc_put_ref(psoc);
out:
	PMO_EXIT();

	return status;
}

/**
 * pmo_core_set_vdev_resume_dtim() - set resume dtim parameters in fw
 * @psoc: objmgr psoc handle
 * @vdev: objmgr vdev handle
 * @vdev_ctx: pmo vdev priv ctx
 *
 * Return: none
 */
static void pmo_core_set_vdev_resume_dtim(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_vdev *vdev,
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	enum pmo_power_save_qpower_mode qpower_config;
	QDF_STATUS ret;
	uint8_t vdev_id;
	enum tQDF_ADAPTER_MODE opmode = pmo_core_get_vdev_op_mode(vdev);
	uint32_t cfg_data_val = 0;

	qpower_config = pmo_core_psoc_get_qpower_config(psoc);
	vdev_id = pmo_vdev_get_id(vdev);
	if ((PMO_VDEV_IN_STA_MODE(opmode)) &&
	    (pmo_core_vdev_get_dtim_policy(vdev) == pmo_normal_dtim)) {
/*
		if (!mac) {
			WMA_LOGE(FL("Failed to get mac context"));
			return;
		}
		if ((wlan_cfg_get_int(mac, WNI_CFG_LISTEN_INTERVAL,
				      &cfg_data_val) != eSIR_SUCCESS)) {
			pmo_err("Failed to get value for listen interval");
			cfg_data_val = POWERSAVE_DEFAULT_LISTEN_INTERVAL;
		}
*/
		cfg_data_val = 1;
		ret = pmo_tgt_vdev_update_param_req(vdev,
				pmo_vdev_param_listen_interval, cfg_data_val);
		if (QDF_IS_STATUS_ERROR(ret)) {
			/* Even it fails continue Fw will take default LI */
			pmo_err("Failed to Set Listen Interval vdevId %d",
				 vdev_id);
		}
		pmo_debug("Set Listen Interval vdevId %d Listen Intv %d",
			 vdev_id, cfg_data_val);

		ret = pmo_tgt_vdev_update_param_req(vdev,
				pmo_vdev_param_dtim_policy,
				pmo_stick_dtim);
		if (QDF_IS_STATUS_ERROR(ret)) {
			/* Set it back to Stick DTIM */
			pmo_err("Failed to Set to Stick DTIM vdevId %d",
				 vdev_id);
		}
		pmo_core_vdev_set_dtim_policy(vdev, pmo_stick_dtim);
		pmo_debug("Set DTIM Policy to Stick Dtim vdevId %d", vdev_id);

		if (qpower_config) {
			pmo_debug("enable Qpower in resume mode!");
			ret = pmo_tgt_send_vdev_sta_ps_param(vdev,
				pmo_sta_ps_enable_qpower, qpower_config);
			if (QDF_IS_STATUS_ERROR(ret))
				pmo_err("Failed to enable Qpower in resume");
		}
	}
}

/**
 * pmo_core_set_resume_dtim() - set resume time dtim
 * @psoc: objmgr psoc handle
 *
 * Return: none
 */
static void pmo_core_set_resume_dtim(struct wlan_objmgr_psoc *psoc)
{
	uint8_t vdev_id;
	struct wlan_objmgr_vdev *vdev;
	struct pmo_vdev_priv_obj *vdev_ctx;
	bool alt_mdtim_enabled;
	QDF_STATUS status;

	/* Iterate through VDEV list */
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) {
		vdev = pmo_psoc_get_vdev(psoc, vdev_id);
		if (!vdev)
			continue;

		status = pmo_vdev_get_ref(vdev);
		if (QDF_IS_STATUS_ERROR(status))
			continue;

		vdev_ctx = pmo_vdev_get_priv(vdev);
		qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
		alt_mdtim_enabled = vdev_ctx->alt_modulated_dtim_enable;
		qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

		if (!alt_mdtim_enabled)
			pmo_core_set_vdev_resume_dtim(psoc, vdev, vdev_ctx);

		pmo_vdev_put_ref(vdev);
	}
}

/**
 * pmo_unpause_vdev - unpause all vdev
 * @psoc: objmgr psoc handle
 *
 * unpause all vdev aftter resume/coming out of wow mode
 *
 * Return: none
 */
static void pmo_unpause_all_vdev(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_priv_obj *psoc_ctx)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc_objmgr *objmgr;
	struct wlan_objmgr_vdev *vdev;

	/* Iterate through VDEV list */
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) {
		wlan_psoc_obj_lock(psoc);
		objmgr = &psoc->soc_objmgr;
		if (!objmgr->wlan_vdev_list[vdev_id]) {
			wlan_psoc_obj_unlock(psoc);
			continue;
		}
		vdev = objmgr->wlan_vdev_list[vdev_id];
		wlan_psoc_obj_unlock(psoc);
		if (vdev) {
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
			/*
			 * When host resume, by default,
			 * unpause all active vdev
			 */
			if (pmo_core_vdev_get_pause_bitmap(psoc_ctx, vdev_id)) {
				cdp_fc_vdev_unpause(
					pmo_core_psoc_get_dp_handle(psoc),
					pmo_core_vdev_get_dp_handle(vdev),
					0xffffffff);
				if (psoc_ctx->pause_bitmap_notifier)
					psoc_ctx->pause_bitmap_notifier(vdev_id,
							0);
			}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
		}
	}
}

/**
 * pmo_core_psoc_configure_resume(): configure events after bus resume
 * @psoc: objmgr psoc
 *
 * Responsibility of the caller to take the psoc reference.
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS pmo_core_psoc_configure_resume(struct wlan_objmgr_psoc *psoc)
{
	struct pmo_psoc_priv_obj *psoc_ctx;

	PMO_ENTER();

	psoc_ctx = pmo_psoc_get_priv(psoc);

	pmo_core_set_resume_dtim(psoc);
	pmo_core_update_wow_bus_suspend(psoc, psoc_ctx, false);
	pmo_unpause_all_vdev(psoc, psoc_ctx);

	PMO_EXIT();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS pmo_core_psoc_user_space_resume_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("pmo cannot get the reference out of psoc");
		goto out;
	}

	/* Resume all components */
	status = pmo_resume_all_components(psoc, type);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to resume all the components");
		goto dec_psoc_ref;
	}

	status = pmo_core_psoc_configure_resume(psoc);
	if (status != QDF_STATUS_SUCCESS)
		pmo_err("Failed to configure resume");

dec_psoc_ref:
	pmo_psoc_put_ref(psoc);
out:
	PMO_EXIT();

	return status;
}

/**
 * pmo_core_enable_wow_in_fw() - enable wow in fw
 * @psoc: objmgr psoc handle
 * @psoc_ctx: pmo psoc private ctx
 * @wow_params: collection of wow enable override parameters
 *
 * Return: QDF status
 */
static
QDF_STATUS pmo_core_enable_wow_in_fw(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_priv_obj *psoc_ctx,
		struct pmo_wow_enable_params *wow_params)
{
	int host_credits, wmi_pending_cmds;
	struct pmo_wow_cmd_params param = {0};
	QDF_STATUS status;

	PMO_ENTER();
	qdf_event_reset(&psoc_ctx->wow.target_suspend);
	pmo_core_set_wow_nack(psoc_ctx, false);
	host_credits = pmo_tgt_psoc_get_host_credits(psoc);
	wmi_pending_cmds = pmo_tgt_psoc_get_pending_cmnds(psoc);
	pmo_debug("Credits:%d; Pending_Cmds: %d",
		host_credits, wmi_pending_cmds);

	param.enable = true;
	if (wow_params->is_unit_test)
		param.flags = WMI_WOW_FLAG_UNIT_TEST_ENABLE;

	switch (wow_params->interface_pause) {
	default:
		pmo_err("Invalid interface pause setting: %d",
			 wow_params->interface_pause);
		/* intentional fall-through to default */
	case PMO_WOW_INTERFACE_PAUSE_DEFAULT:
		param.can_suspend_link =
			htc_can_suspend_link(
				pmo_core_psoc_get_htc_handle(psoc));
		break;
	case PMO_WOW_INTERFACE_PAUSE_ENABLE:
		param.can_suspend_link = true;
		break;
	case PMO_WOW_INTERFACE_PAUSE_DISABLE:
		param.can_suspend_link = false;
		break;
	}

	switch (wow_params->resume_trigger) {
	default:
		pmo_err("Invalid resume trigger setting: %d",
			 wow_params->resume_trigger);
		/* intentional fall-through to default */
	case PMO_WOW_RESUME_TRIGGER_DEFAULT:
	case PMO_WOW_RESUME_TRIGGER_GPIO:
		/*
		 * GPIO is currently implicit. This means you can't actually
		 * force GPIO if a platform's default wake trigger is HTC wakeup
		 */
		break;
	case PMO_WOW_RESUME_TRIGGER_HTC_WAKEUP:
		param.flags |= WMI_WOW_FLAG_DO_HTC_WAKEUP;
		break;
	}

	status = pmo_tgt_psoc_send_wow_enable_req(psoc, &param);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to enable wow in fw");
		goto out;
	}

	pmo_tgt_update_target_suspend_flag(psoc, true);

	if (qdf_wait_single_event(&psoc_ctx->wow.target_suspend,
				  PMO_TGT_SUSPEND_COMPLETE_TIMEOUT)
	    != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to receive WoW Enable Ack from FW");
		pmo_err("Credits:%d; Pending_Cmds: %d",
			pmo_tgt_psoc_get_host_credits(psoc),
			pmo_tgt_psoc_get_pending_cmnds(psoc));
		pmo_tgt_update_target_suspend_flag(psoc, false);
		status = QDF_STATUS_E_FAILURE;
		QDF_BUG(0);
		goto out;
	}

	if (pmo_core_get_wow_nack(psoc_ctx)) {
		pmo_err("FW not ready to WOW");
		pmo_tgt_update_target_suspend_flag(psoc, false);
		status = QDF_STATUS_E_AGAIN;
		goto out;
	}

	host_credits = pmo_tgt_psoc_get_host_credits(psoc);
	wmi_pending_cmds = pmo_tgt_psoc_get_pending_cmnds(psoc);

	if (host_credits < PMO_WOW_REQUIRED_CREDITS) {
		pmo_err("No Credits after HTC ACK:%d, pending_cmds:%d,"
			 "cannot resume back", host_credits, wmi_pending_cmds);
		htc_dump_counter_info(pmo_core_psoc_get_htc_handle(psoc));
/*
		if (!cds_is_driver_recovering())
			QDF_BUG(0);
		else
			pmo_err("SSR in progress, ignore no credit issue");
*/
	}
	pmo_debug("WOW enabled successfully in fw: credits:%d pending_cmds: %d",
		host_credits, wmi_pending_cmds);

	pmo_core_update_wow_enable_cmd_sent(psoc_ctx, true);
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_psoc_suspend_target(struct wlan_objmgr_psoc *psoc,
		int disable_target_intr)
{
	QDF_STATUS status;
	struct pmo_suspend_params param;
	struct pmo_psoc_priv_obj *psoc_ctx;

	PMO_ENTER();

	psoc_ctx = pmo_psoc_get_priv(psoc);

	qdf_event_reset(&psoc_ctx->wow.target_suspend);
	param.disable_target_intr = disable_target_intr;
	status = pmo_tgt_psoc_send_supend_req(psoc, &param);
	if (status != QDF_STATUS_SUCCESS)
		goto out;

	pmo_tgt_update_target_suspend_flag(psoc, true);

	if (qdf_wait_single_event(&psoc_ctx->wow.target_suspend,
				  PMO_TGT_SUSPEND_COMPLETE_TIMEOUT)
	    != QDF_STATUS_SUCCESS) {
		status = QDF_STATUS_E_TIMEOUT;
		pmo_err("Failed to get ACK from firmware for pdev suspend");
		pmo_tgt_update_target_suspend_flag(psoc, false);
		/* wma_suspend_target_timeout(pmac->sme.enableSelfRecovery); */
	}
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_psoc_bus_suspend_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type,
		struct pmo_wow_enable_params *wow_params)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	QDF_STATUS status;
	bool wow_mode_selected = false;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	if (!wow_params) {
		pmo_err("wow_params is NULL");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("pmo cannot get the reference out of psoc");
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);

/* TODO - scan manager need to provide the below public api
	if (wma_check_scan_in_progress(handle)) {
		pmo_err("Scan in progress. Aborting suspend");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}
*/

	wow_mode_selected = pmo_core_is_wow_enabled(psoc_ctx);
	pmo_info("wow mode selected %d", wow_mode_selected);

	if (wow_mode_selected)
		status = pmo_core_enable_wow_in_fw(psoc, psoc_ctx, wow_params);
	else
		status = pmo_core_psoc_suspend_target(psoc, 0);

	pmo_psoc_put_ref(psoc);
out:
	PMO_EXIT();

	return status;
}

#ifdef FEATURE_RUNTIME_PM
QDF_STATUS pmo_core_psoc_bus_runtime_suspend(struct wlan_objmgr_psoc *psoc,
					     pmo_pld_auto_suspend_cb pld_cb)
{
	void *hif_ctx;
	void *dp_soc;
	void *txrx_pdev;
	void *htc_ctx;
	QDF_STATUS status;
	struct pmo_wow_enable_params wow_params = {0};

	PMO_ENTER();

	if (!psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("pmo cannot get the reference out of psoc");
		goto out;
	}

	hif_ctx = pmo_core_psoc_get_hif_handle(psoc);
	dp_soc = pmo_core_psoc_get_dp_handle(psoc);
	txrx_pdev = pmo_core_psoc_get_txrx_handle(psoc);
	htc_ctx = pmo_core_psoc_get_htc_handle(psoc);
	if (!hif_ctx || !dp_soc || !txrx_pdev || !htc_ctx) {
		pmo_err("Invalid hif: %p, dp: %p, txrx: %p, htc: %p",
			hif_ctx, dp_soc, txrx_pdev, htc_ctx);
		status = QDF_STATUS_E_INVAL;
		goto dec_psoc_ref;
	}

	wow_params.interface_pause = PMO_WOW_INTERFACE_PAUSE_ENABLE;
	wow_params.resume_trigger = PMO_WOW_RESUME_TRIGGER_GPIO;

	if (hif_pre_runtime_suspend(hif_ctx))
		goto runtime_failure;

	status = cdp_runtime_suspend(dp_soc, txrx_pdev);
	if (status != QDF_STATUS_SUCCESS)
		goto runtime_failure;

	if (htc_runtime_suspend(htc_ctx))
		goto cdp_runtime_resume;

	status = pmo_tgt_psoc_set_runtime_pm_inprogress(psoc, true);
	if (status != QDF_STATUS_SUCCESS)
		goto resume_htc;

	status = pmo_core_psoc_configure_suspend(psoc);
	if (status != QDF_STATUS_SUCCESS)
		goto resume_htc;

	status = pmo_core_psoc_bus_suspend_req(psoc, QDF_RUNTIME_SUSPEND,
					       &wow_params);
	if (status != QDF_STATUS_SUCCESS)
		goto pmo_resume_configure;

	if (hif_runtime_suspend(hif_ctx))
		goto pmo_bus_resume;

	if (pld_cb && pld_cb())
		goto resume_hif;

	hif_process_runtime_suspend_success(hif_ctx);

	goto dec_psoc_ref;

resume_hif:
	QDF_BUG(!hif_runtime_resume(hif_ctx));

pmo_bus_resume:
	QDF_BUG(QDF_STATUS_SUCCESS ==
		pmo_core_psoc_bus_resume_req(psoc, QDF_RUNTIME_SUSPEND));

pmo_resume_configure:
	QDF_BUG(QDF_STATUS_SUCCESS ==
		pmo_core_psoc_configure_resume(psoc));

resume_htc:
	QDF_BUG(QDF_STATUS_SUCCESS ==
		pmo_tgt_psoc_set_runtime_pm_inprogress(psoc, false));

	QDF_BUG(!htc_runtime_resume(htc_ctx));

cdp_runtime_resume:
	QDF_BUG(QDF_STATUS_SUCCESS ==
		cdp_runtime_resume(dp_soc, txrx_pdev));

runtime_failure:
	hif_process_runtime_suspend_failure(hif_ctx);

dec_psoc_ref:
	pmo_psoc_put_ref(psoc);

out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_psoc_bus_runtime_resume(struct wlan_objmgr_psoc *psoc,
					    pmo_pld_auto_resume_cb pld_cb)
{
	void *hif_ctx;
	void *dp_soc;
	void *txrx_pdev;
	void *htc_ctx;
	QDF_STATUS status;

	PMO_ENTER();

	if (!psoc) {
		pmo_err("psoc is NULL");
		status = QDF_STATUS_E_INVAL;
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("pmo cannot get the reference out of psoc");
		goto out;
	}

	hif_ctx = pmo_core_psoc_get_hif_handle(psoc);
	dp_soc = pmo_core_psoc_get_dp_handle(psoc);
	txrx_pdev = pmo_core_psoc_get_txrx_handle(psoc);
	htc_ctx = pmo_core_psoc_get_htc_handle(psoc);
	if (!hif_ctx || !dp_soc || !txrx_pdev || !htc_ctx) {
		pmo_err("Invalid hif: %p, dp: %p, txrx: %p, htc: %p",
			hif_ctx, dp_soc, txrx_pdev, htc_ctx);
		status = QDF_STATUS_E_INVAL;
		goto dec_psoc_ref;
	}

	hif_pre_runtime_resume(hif_ctx);

	if (pld_cb)
		QDF_BUG(!pld_cb());

	QDF_BUG(!hif_runtime_resume(hif_ctx));

	status = pmo_core_psoc_bus_resume_req(psoc, QDF_RUNTIME_SUSPEND);
	QDF_BUG(status == QDF_STATUS_SUCCESS);

	status = pmo_core_psoc_configure_resume(psoc);
	QDF_BUG(status == QDF_STATUS_SUCCESS);

	status = pmo_tgt_psoc_set_runtime_pm_inprogress(psoc, false);
	QDF_BUG(status == QDF_STATUS_SUCCESS);

	QDF_BUG(!htc_runtime_resume(htc_ctx));

	status = cdp_runtime_resume(dp_soc, txrx_pdev);
	QDF_BUG(status == QDF_STATUS_SUCCESS);

	hif_process_runtime_resume_success(hif_ctx);

dec_psoc_ref:
	pmo_psoc_put_ref(psoc);

out:
	PMO_EXIT();

	return status;
}
#endif

/**
 * pmo_core_psoc_send_host_wakeup_ind_to_fw() - send wakeup ind to fw
 * @psoc: objmgr psoc handle
 * @psoc_ctx: pmo psoc private context
 *
 * Sends host wakeup indication to FW. On receiving this indication,
 * FW will come out of WOW.
 *
 * Return: QDF status
 */
static
QDF_STATUS pmo_core_psoc_send_host_wakeup_ind_to_fw(
			struct wlan_objmgr_psoc *psoc,
			struct pmo_psoc_priv_obj *psoc_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();
	qdf_event_reset(&psoc_ctx->wow.target_resume);

	status = pmo_tgt_psoc_send_host_wakeup_ind(psoc);
	if (status) {
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}
	pmo_debug("Host wakeup indication sent to fw");

	status = qdf_wait_single_event(&psoc_ctx->wow.target_resume,
					PMO_RESUME_TIMEOUT);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Timeout waiting for resume event from FW");
		pmo_err("Pending commands %d credits %d",
			pmo_tgt_psoc_get_pending_cmnds(psoc),
			pmo_tgt_psoc_get_host_credits(psoc));
		QDF_BUG(0);
	} else {
		pmo_debug("Host wakeup received");
	}

	if (status == QDF_STATUS_SUCCESS)
		pmo_tgt_update_target_suspend_flag(psoc, false);
out:
	PMO_EXIT();

	return status;
}

/**
 * pmo_core_psoc_disable_wow_in_fw() -  Disable wow in bus resume context.
 * @psoc: objmgr psoc handle
 * @psoc_ctx: pmo psoc private context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static
QDF_STATUS pmo_core_psoc_disable_wow_in_fw(struct wlan_objmgr_psoc *psoc,
			struct pmo_psoc_priv_obj *psoc_ctx)
{
	QDF_STATUS ret;

	PMO_ENTER();
	ret = pmo_core_psoc_send_host_wakeup_ind_to_fw(psoc, psoc_ctx);
	if (ret != QDF_STATUS_SUCCESS)
		goto out;

	pmo_core_update_wow_enable(psoc_ctx, false);
	pmo_core_update_wow_enable_cmd_sent(psoc_ctx, false);

	/* To allow the tx pause/unpause events */
	pmo_core_update_wow_bus_suspend(psoc, psoc_ctx, false);
	/* Unpause the vdev as we are resuming */
	pmo_unpause_all_vdev(psoc, psoc_ctx);
out:
	PMO_EXIT();

	return ret;
}

/**
 * pmo_core_psoc_resume_target() - resume target
 * @psoc: objmgr psoc handle
 * @psoc_ctx: pmo psoc private context
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static
QDF_STATUS pmo_core_psoc_resume_target(struct wlan_objmgr_psoc *psoc,
		struct pmo_psoc_priv_obj *psoc_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	PMO_ENTER();
	qdf_event_reset(&psoc_ctx->wow.target_resume);

	status = pmo_tgt_psoc_send_target_resume_req(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	status = qdf_wait_single_event(&psoc_ctx->wow.target_resume,
			PMO_RESUME_TIMEOUT);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_fatal("Timeout waiting for resume event from FW");
		pmo_fatal("Pending commands %d credits %d",
			pmo_tgt_psoc_get_pending_cmnds(psoc),
			pmo_tgt_psoc_get_host_credits(psoc));
		QDF_BUG(0);
	} else {
		pmo_debug("Host wakeup received");
	}

	if (status == QDF_STATUS_SUCCESS)
		pmo_tgt_update_target_suspend_flag(psoc, false);
out:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_psoc_bus_resume_req(struct wlan_objmgr_psoc *psoc,
		enum qdf_suspend_type type)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	bool wow_mode;
	QDF_STATUS status;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		status = QDF_STATUS_E_NULL_VALUE;
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("pmo cannot get the reference out of psoc");
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);
	wow_mode = pmo_core_is_wow_enabled(psoc_ctx);
	pmo_info("wow mode %d", wow_mode);

	pmo_core_update_wow_initial_wake_up(psoc_ctx, false);

	if (wow_mode)
		status = pmo_core_psoc_disable_wow_in_fw(psoc, psoc_ctx);
	else
		status = pmo_core_psoc_resume_target(psoc, psoc_ctx);

	pmo_psoc_put_ref(psoc);

out:
	PMO_EXIT();

	return status;
}

void pmo_core_psoc_target_suspend_acknowledge(void *context, bool wow_nack)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	struct wlan_objmgr_psoc *psoc = (struct wlan_objmgr_psoc *)context;
	QDF_STATUS status;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to get psoc reference");
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);

	pmo_core_set_wow_nack(psoc_ctx, wow_nack);
	qdf_event_set(&psoc_ctx->wow.target_suspend);
	if (wow_nack && !pmo_tgt_psoc_get_runtime_pm_in_progress(psoc)) {
		qdf_wake_lock_timeout_acquire(&psoc_ctx->wow.wow_wake_lock,
						PMO_WAKE_LOCK_TIMEOUT);
	}

	pmo_psoc_put_ref(psoc);
out:
	PMO_EXIT();
}

void pmo_core_psoc_wakeup_host_event_received(struct wlan_objmgr_psoc *psoc)
{
	struct pmo_psoc_priv_obj *psoc_ctx;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("psoc is null");
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);
	qdf_event_set(&psoc_ctx->wow.target_resume);
out:
	PMO_EXIT();
}

int pmo_core_psoc_is_target_wake_up_received(struct wlan_objmgr_psoc *psoc)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	int ret = 0;
	QDF_STATUS status;

	if (!psoc) {
		pmo_err("psoc is NULL");
		ret = -EAGAIN;
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to get psoc reference");
		ret = -EAGAIN;
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);
	if (pmo_core_get_wow_initial_wake_up(psoc_ctx)) {
		pmo_err("Target initial wake up received try again");
		ret = -EAGAIN;
	}

	pmo_psoc_put_ref(psoc);
out:
	PMO_EXIT();

	return ret;
}


int pmo_core_psoc_clear_target_wake_up(struct wlan_objmgr_psoc *psoc)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	int ret = 0;
	QDF_STATUS status;

	if (!psoc) {
		pmo_err("psoc is NULL");
		ret = -EAGAIN;
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to get psoc reference");
		ret = -EAGAIN;
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);
	pmo_core_update_wow_initial_wake_up(psoc_ctx, false);

	pmo_psoc_put_ref(psoc);
out:
	PMO_EXIT();

	return ret;
}

void pmo_core_psoc_handle_initial_wake_up(void *cb_ctx)
{
	struct pmo_psoc_priv_obj *psoc_ctx;
	struct wlan_objmgr_psoc *psoc = (struct wlan_objmgr_psoc *)cb_ctx;
	QDF_STATUS status;

	PMO_ENTER();
	if (!psoc) {
		pmo_err("cb ctx/psoc is null");
		goto out;
	}

	status = pmo_psoc_get_ref(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		pmo_err("Failed to get psoc reference");
		goto out;
	}

	psoc_ctx = pmo_psoc_get_priv(psoc);
	pmo_core_update_wow_initial_wake_up(psoc_ctx, true);

	pmo_psoc_put_ref(psoc);

out:
	PMO_EXIT();
}

