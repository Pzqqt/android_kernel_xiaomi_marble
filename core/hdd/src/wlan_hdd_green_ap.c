/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 *  DOC: wlan_hdd_green_ap.c
 *
 *  WLAN Host Device Driver Green AP implementation
 *
 */

#include <wlan_hdd_green_ap.h>
#include <wlan_hdd_main.h>
#include <wlan_policy_mgr_api.h>
#include <wlan_green_ap_ucfg_api.h>

/**
 * hdd_green_ap_check_enable() - to check whether to enable green ap or not
 * @hdd_ctx: hdd context
 * @enable_green_ap: 1 - enable green ap enabled, 0 - disbale green ap
 *
 * Return: 0 - success, < 0 - failure
 */
static int hdd_green_ap_check_enable(struct hdd_context *hdd_ctx,
				     bool *enable_green_ap)
{
	uint8_t num_sessions, mode;
	QDF_STATUS status;

	for (mode = 0;
	     mode < QDF_MAX_NO_OF_MODE;
	     mode++) {
		if (mode == QDF_SAP_MODE || mode == QDF_P2P_GO_MODE)
			continue;

		status = policy_mgr_mode_specific_num_active_sessions(
					hdd_ctx->hdd_psoc, mode, &num_sessions);
		hdd_debug("No. of active sessions for mode: %d is %d",
			  mode, num_sessions);
		if (status != QDF_STATUS_SUCCESS) {
			hdd_err("Failed to get num sessions for mode: %d",
				mode);
			return -EINVAL;
		} else if (num_sessions) {
			*enable_green_ap = false;
			return 0;
		}
	}
	*enable_green_ap = true;
	return 0;
}

void hdd_green_ap_add_sta(struct hdd_context *hdd_ctx)
{
	wlan_green_ap_add_sta(hdd_ctx->hdd_pdev);
}

void hdd_green_ap_del_sta(struct hdd_context *hdd_ctx)
{
	wlan_green_ap_del_sta(hdd_ctx->hdd_pdev);
}

int hdd_green_ap_enable_egap(struct hdd_context *hdd_ctx)
{
	QDF_STATUS status;

	status = ucfg_green_ap_enable_egap(hdd_ctx->hdd_pdev);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_debug("enhance green ap is not enabled, status %d",
			  status);
		return qdf_status_to_os_return(status);
	}

	return 0;
}

void hdd_green_ap_print_config(struct hdd_context *hdd_ctx)
{
	hdd_debug("Name = [gEnableGreenAp] Value = [%u] ",
		  hdd_ctx->config->enable_green_ap);
	hdd_debug("Name = [gEenableEGAP] Value = [%u] ",
		  hdd_ctx->config->enable_egap);
	hdd_debug("Name = [gEGAPInactTime] Value = [%u] ",
		  hdd_ctx->config->egap_inact_time);
	hdd_debug("Name = [gEGAPWaitTime] Value = [%u] ",
		  hdd_ctx->config->egap_wait_time);
	hdd_debug("Name = [gEGAPFeatures] Value = [%u] ",
		  hdd_ctx->config->egap_feature_flag);
}

int hdd_green_ap_update_config(struct hdd_context *hdd_ctx)
{
	struct green_ap_user_cfg green_ap_cfg;
	struct hdd_config *cfg = hdd_ctx->config;
	QDF_STATUS status;

	green_ap_cfg.host_enable_egap = cfg->enable_egap;
	green_ap_cfg.egap_inactivity_time = cfg->egap_inact_time;
	green_ap_cfg.egap_wait_time = cfg->egap_wait_time;
	green_ap_cfg.egap_feature_flags = cfg->egap_feature_flag;

	status = ucfg_green_ap_update_user_config(hdd_ctx->hdd_pdev,
						  &green_ap_cfg);
	if (status != QDF_STATUS_SUCCESS) {
		hdd_err("failed to update green ap user configuration");
		return -EINVAL;
	}

	return 0;
}

int hdd_green_ap_start_state_mc(struct hdd_context *hdd_ctx,
				enum QDF_OPMODE mode, bool is_session_start)
{
	struct hdd_config *cfg;
	bool enable_green_ap = false;
	uint8_t num_sap_sessions = 0, num_p2p_go_sessions = 0, ret = 0;

	cfg = hdd_ctx->config;
	if (!cfg) {
		hdd_err("NULL hdd config");
		return -EINVAL;
	}

	if (!cfg->enable2x2 || !cfg->enable_green_ap) {
		hdd_debug("Green AP not enabled: enable2x2:%d, enable_green_ap:%d",
			  cfg->enable2x2, cfg->enable_green_ap);
		return 0;
	}

	policy_mgr_mode_specific_num_active_sessions(hdd_ctx->hdd_psoc,
						     QDF_SAP_MODE,
						     &num_sap_sessions);
	policy_mgr_mode_specific_num_active_sessions(hdd_ctx->hdd_psoc,
						     QDF_P2P_GO_MODE,
						     &num_p2p_go_sessions);

	switch (mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_IBSS_MODE:
		if (!num_sap_sessions && !num_p2p_go_sessions)
			return 0;

		if (is_session_start) {
			hdd_debug("Disabling Green AP");
			ucfg_green_ap_set_ps_config(hdd_ctx->hdd_pdev,
						    false);
			wlan_green_ap_stop(hdd_ctx->hdd_pdev);
		} else {
			ret = hdd_green_ap_check_enable(hdd_ctx,
							&enable_green_ap);
			if (!ret) {
				if (enable_green_ap) {
					hdd_debug("Enabling Green AP");
					ucfg_green_ap_set_ps_config(
						hdd_ctx->hdd_pdev, true);
					wlan_green_ap_start(hdd_ctx->hdd_pdev);
				}
			} else {
				hdd_err("Failed to check Green AP enable status");
			}
		}
		break;
	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
		if (is_session_start) {
			ret = hdd_green_ap_check_enable(hdd_ctx,
							&enable_green_ap);
			if (!ret) {
				if (enable_green_ap) {
					hdd_debug("Enabling Green AP");
					ucfg_green_ap_set_ps_config(
						hdd_ctx->hdd_pdev, true);
					wlan_green_ap_start(hdd_ctx->hdd_pdev);
				}
			} else {
				hdd_err("Failed to check Green AP enable status");
			}
		} else {
			if (!num_sap_sessions && !num_p2p_go_sessions) {
				hdd_debug("Disabling Green AP");
				ucfg_green_ap_set_ps_config(hdd_ctx->hdd_pdev,
							    false);
				wlan_green_ap_stop(hdd_ctx->hdd_pdev);
			}
		}
		break;
	default:
		break;
	}
	return ret;
}
