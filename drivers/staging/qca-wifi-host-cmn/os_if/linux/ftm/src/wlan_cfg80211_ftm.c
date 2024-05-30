/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: implementation of the driver FTM functions interfacing with linux kernel
 */

#include <net/cfg80211.h>
#include <qdf_util.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_cfg80211.h>
#include <wlan_cfg80211_ftm.h>
#include <wlan_ftm_ucfg_api.h>
#include <wlan_osif_priv.h>
#include <qdf_types.h>
#include <qdf_module.h>

static const struct nla_policy
wlan_cfg80211_ftm_policy[WLAN_CFG80211_FTM_ATTR_MAX + 1] = {
	[WLAN_CFG80211_FTM_ATTR_CMD] = {.type = NLA_U32},
	[WLAN_CFG80211_FTM_ATTR_DATA] = {.type = NLA_BINARY,
		.len = WLAN_FTM_DATA_MAX_LEN},
};

static int
wlan_cfg80211_process_ftm_cmd(struct wlan_objmgr_pdev *pdev,
				struct nlattr *tb[])
{
	int buf_len;
	void *buf;
	QDF_STATUS status;

	if (!tb[WLAN_CFG80211_FTM_ATTR_DATA]) {
		ftm_err("WLAN_CFG80211_FTM_ATTR_DATA attribute is invalid");
		return -EINVAL;
	}

	buf = nla_data(tb[WLAN_CFG80211_FTM_ATTR_DATA]);
	buf_len = nla_len(tb[WLAN_CFG80211_FTM_ATTR_DATA]);

	if (buf_len > WLAN_FTM_DATA_MAX_LEN)
		return -EINVAL;

	ftm_debug("****FTM Tx cmd len = %d*****", buf_len);

	status = ucfg_wlan_ftm_testmode_cmd(pdev, buf, buf_len);

	if (QDF_IS_STATUS_ERROR(status))
		status = QDF_STATUS_E_BUSY;

	return qdf_status_to_os_return(status);
}

int
wlan_cfg80211_ftm_testmode_cmd(struct wlan_objmgr_pdev *pdev,
				void *data, uint32_t len)
{
	struct nlattr *tb[WLAN_CFG80211_FTM_ATTR_MAX + 1];
	int err = 0, cmd;
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj;

	ftm_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							WLAN_UMAC_COMP_FTM);
	if (!ftm_pdev_obj) {
		ftm_err("Failed to get ftm pdev component");
		return -EINVAL;
	}

	ftm_pdev_obj->cmd_type = WIFI_FTM_CMD_NL80211;

	err = wlan_cfg80211_nla_parse(tb, WLAN_CFG80211_FTM_ATTR_MAX - 1, data,
					len, wlan_cfg80211_ftm_policy);
	if (err) {
		ftm_err("Testmode INV ATTR");
		return err;
	}

	if (!tb[WLAN_CFG80211_FTM_ATTR_CMD]) {
		ftm_err("Testmode INV CMD");
		return -EINVAL;
	}
	cmd = nla_get_u32(tb[WLAN_CFG80211_FTM_ATTR_CMD]);

	switch (cmd) {
	case WLAN_CFG80211_FTM_CMD_WLAN_FTM:
		err = wlan_cfg80211_process_ftm_cmd(pdev, tb);
		break;

	default:
		ftm_err("unknown command: %d", cmd);
		return -ENOENT;
	}

	return err;
}

qdf_export_symbol(wlan_cfg80211_ftm_testmode_cmd);

QDF_STATUS
wlan_cfg80211_ftm_rx_event(struct wlan_objmgr_pdev *pdev,
				uint8_t *data, uint32_t len)
{
	struct pdev_osif_priv *pdev_ospriv;
	qdf_nbuf_t skb = NULL;

	if (!data || !len) {
		ftm_err("Null data or invalid length");
		return QDF_STATUS_E_INVAL;
	}

	pdev_ospriv = wlan_pdev_get_ospriv(pdev);
	if (!pdev_ospriv) {
		ftm_err("pdev_ospriv is NULL");
		return QDF_STATUS_E_INVAL;
	}

	ftm_debug("Testmode response event generated");
	skb = cfg80211_testmode_alloc_event_skb(pdev_ospriv->wiphy,
						len, GFP_ATOMIC);
	if (!skb)
		return QDF_STATUS_E_NOMEM;

	if (nla_put_u32(skb, WLAN_CFG80211_FTM_ATTR_CMD,
			WLAN_CFG80211_FTM_CMD_WLAN_FTM) ||
		nla_put(skb, WLAN_CFG80211_FTM_ATTR_DATA, len, data)) {
		goto nla_put_failure;
	}
	cfg80211_testmode_event(skb, GFP_ATOMIC);

	return QDF_STATUS_SUCCESS;

nla_put_failure:
	qdf_nbuf_free(skb);
	ftm_err("nla_put failed on testmode rx skb!");

	return QDF_STATUS_E_INVAL;
}
