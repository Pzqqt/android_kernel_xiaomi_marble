/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: defines driver functions interfacing with linux kernel
 */
#include <wmi_unified_param.h>
#include <wlan_osif_request_manager.h>
#include <osif_sync.h>
#include <wlan_objmgr_psoc_obj_i.h>
#include <wlan_coex_main.h>
#include <wlan_coex_ucfg_api.h>
#include <wlan_cfg80211_coex.h>

static const struct nla_policy
btc_chain_mode_policy[QCA_VENDOR_ATTR_BTC_CHAIN_MODE_MAX + 1] = {
	[QCA_VENDOR_ATTR_BTC_CHAIN_MODE] = {.type = NLA_U32},
	[QCA_VENDOR_ATTR_BTC_CHAIN_MODE_RESTART] = {.type = NLA_FLAG},
};

static int
__wlan_cfg80211_coex_set_btc_chain_mode(struct wlan_objmgr_vdev *vdev,
					uint8_t mode, bool do_restart)
{
	QDF_STATUS status;
	uint8_t cur_mode;
	int err;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev_tmp;
	int vdev_id;
	struct coex_psoc_obj *coex_obj;

	if (!vdev) {
		coex_err("Null vdev");
		return -EINVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		coex_err("NULL psoc");
		return -EINVAL;
	}

	coex_obj = wlan_psoc_get_coex_obj(psoc);
	if (!coex_obj)
		return -EINVAL;

	status = ucfg_coex_psoc_get_btc_chain_mode(psoc, &cur_mode);
	if (QDF_IS_STATUS_ERROR(status)) {
		coex_err("failed to get cur BTC chain mode, status %d", status);
		return -EFAULT;
	}

	if (cur_mode == mode)
		return -EALREADY;

	status = ucfg_coex_psoc_set_btc_chain_mode(psoc, mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		coex_err("unable to set BTC chain mode to %d", mode);
		return -EFAULT;
	}

	wlan_objmgr_for_each_psoc_vdev(psoc, vdev_id, vdev_tmp) {
		status = ucfg_coex_send_btc_chain_mode(vdev, mode);
		err = qdf_status_to_os_return(status);
		if (err) {
			coex_err("Failed to set btc chain mode to %d for vdev %d",
				 mode, vdev_id);
			return err;
		}
		coex_debug("Set btc chain mode to %d for vdev %d",
			   mode, vdev_id);

		if (!do_restart)
			continue;

		wlan_coex_config_updated(vdev, COEX_CONFIG_BTC_CHAIN_MODE);
	}

	return 0;
}

/**
 * wlan_hdd_cfg80211_set_btc_chain_mode() - set btc chain mode
 * @wiphy: pointer to wireless wiphy structure.
 * @wdev: pointer to wireless_dev structure.
 * @data: pointer to btc chain mode command parameters.
 * @data_len: the length in byte of btc chain mode command parameters.
 *
 * Return: An error code or 0 on success.
 */
int wlan_cfg80211_coex_set_btc_chain_mode(struct wlan_objmgr_vdev *vdev,
					  const void *data, int data_len)
{
	struct nlattr *tb[QCA_VENDOR_ATTR_BTC_CHAIN_MODE_MAX + 1];
	uint32_t mode;
	bool restart;

	if (wlan_cfg80211_nla_parse(tb, QCA_VENDOR_ATTR_BTC_CHAIN_MODE_MAX,
				    data, data_len, btc_chain_mode_policy)) {
		coex_err("Invalid btc chain mode ATTR");
		return -EINVAL;
	}

	if (!tb[QCA_VENDOR_ATTR_BTC_CHAIN_MODE]) {
		coex_err("btc chain mode - no attr mode");
		return -EINVAL;
	}

	mode = nla_get_u32(tb[QCA_VENDOR_ATTR_BTC_CHAIN_MODE]);
	if (mode < QCA_BTC_CHAIN_SHARED || mode > QCA_BTC_CHAIN_SEPARATED) {
		coex_err("Invalid btc chain mode %d", mode);
		return -EINVAL;
	}

	restart = nla_get_flag(tb[QCA_VENDOR_ATTR_BTC_CHAIN_MODE_RESTART]);

	coex_debug("vdev_id %u mode %u restart %u",
		   wlan_vdev_get_id(vdev), mode, restart);

	return __wlan_cfg80211_coex_set_btc_chain_mode(vdev, mode, restart);
}
