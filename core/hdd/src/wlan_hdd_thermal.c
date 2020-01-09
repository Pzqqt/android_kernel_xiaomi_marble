/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_thermal.c
 *
 * WLAN Host Device Driver implementation for thermal mitigation handling
 */

#include <wlan_hdd_includes.h>
#include <net/cfg80211.h>
#include "wlan_osif_priv.h"
#include "qdf_trace.h"
#include "wlan_hdd_main.h"
#include "osif_sync.h"
#include <linux/limits.h>
#include <wlan_hdd_object_manager.h>
#include "sme_api.h"
#include "wlan_hdd_thermal.h"
#include "wlan_hdd_cfg80211.h"
#include <qca_vendor.h>
#include "wlan_fwol_ucfg_api.h"

static const struct nla_policy
	wlan_hdd_thermal_mitigation_policy
	[QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_MAX + 1] = {
		[QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_VALUE] = {.type = NLA_U32},
		[QCA_WLAN_VENDOR_ATTR_THERMAL_LEVEL] = {
						.type = NLA_U32},
};

/**
 * __wlan_hdd_cfg80211_set_thermal_mitigation_policy() - Set the thermal policy
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_set_thermal_mitigation_policy(struct wiphy *wiphy,
						  struct wireless_dev *wdev,
						  const void *data,
						  int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_MAX + 1];
	bool enable = true;
	uint32_t dc, dc_off_percent, level, cmd_type;
	uint32_t prio = 0, target_temp = 0;
	struct wlan_fwol_thermal_temp thermal_temp = {0};
	QDF_STATUS status;

	hdd_enter();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err_rl("Command not allowed in FTM mode");
		return -EPERM;
	}

	if (wlan_cfg80211_nla_parse(tb,
				    QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_MAX,
				    (struct nlattr *)data, data_len,
				    wlan_hdd_thermal_mitigation_policy)) {
		hdd_err_rl("Invalid attribute");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_VALUE]) {
		hdd_err_rl("attr thermal cmd value failed");
		return -EINVAL;
	}

	cmd_type = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_VALUE]);
	if (cmd_type != QCA_WLAN_VENDOR_ATTR_THERMAL_CMD_TYPE_SET_LEVEL) {
		hdd_err_rl("invalid thermal cmd value");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_THERMAL_LEVEL]) {
		hdd_err_rl("attr thermal throttle set failed");
		return -EINVAL;
	}
	level =
	    nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_THERMAL_LEVEL]);

	hdd_debug("thermal mitigation level %d", level);

	status = ucfg_fwol_get_thermal_temp(hdd_ctx->psoc, &thermal_temp);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err_rl("Failed to get fwol thermal obj");
		return qdf_status_to_os_return(status);
	}

	switch (level) {
	case QCA_WLAN_VENDOR_THERMAL_LEVEL_EMERGENCY:
		dc_off_percent = thermal_temp.throttle_dutycycle_level[5];
		break;
	case QCA_WLAN_VENDOR_THERMAL_LEVEL_CRITICAL:
		dc_off_percent = thermal_temp.throttle_dutycycle_level[4];
		break;
	case QCA_WLAN_VENDOR_THERMAL_LEVEL_SEVERE:
		dc_off_percent = thermal_temp.throttle_dutycycle_level[3];
		break;
	case QCA_WLAN_VENDOR_THERMAL_LEVEL_MODERATE:
		dc_off_percent = thermal_temp.throttle_dutycycle_level[2];
		break;
	case QCA_WLAN_VENDOR_THERMAL_LEVEL_LIGHT:
		dc_off_percent = thermal_temp.throttle_dutycycle_level[1];
		break;
	case QCA_WLAN_VENDOR_THERMAL_LEVEL_NONE:
		enable = false;
		dc_off_percent = thermal_temp.throttle_dutycycle_level[0];
		break;
	default:
		hdd_debug("Invalid thermal state");
		return -EINVAL;
	}

	dc = thermal_temp.thermal_sampling_time;
	hdd_debug("dc %d dc_off_per %d", dc, dc_off_percent);

	status = sme_set_thermal_throttle_cfg(hdd_ctx->mac_handle,
					      enable,
					      dc,
					      dc_off_percent,
					      prio,
					      target_temp);
	if (QDF_IS_STATUS_ERROR(status))
		hdd_err_rl("Failed to set throttle configuration %d", status);

	return qdf_status_to_os_return(status);
}

/**
 * wlan_hdd_cfg80211_set_thermal_mitigation_policy() - set thermal
 * mitigation policy
 * @wiphy: wiphy pointer
 * @wdev: pointer to struct wireless_dev
 * @data: pointer to incoming NL vendor data
 * @data_len: length of @data
 *
 * Return: 0 on success; error number otherwise.
 */
int
wlan_hdd_cfg80211_set_thermal_mitigation_policy(struct wiphy *wiphy,
						struct wireless_dev *wdev,
						const void *data, int data_len)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_thermal_mitigation_policy(wiphy, wdev,
								  data,
								  data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}

bool wlan_hdd_thermal_config_support(void)
{
	return true;
}

