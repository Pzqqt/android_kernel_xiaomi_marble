/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_hdd_txq_flush.c
 *
 * WLAN Host Device Driver Peer TX queue flush configuration APIs implementation
 *
 */

#include "osif_sync.h"
#include <wlan_hdd_includes.h>
#include "wlan_hdd_main.h"
#include "wlan_hdd_peer_txq_flush.h"
#include <qca_vendor.h>
#include "wlan_hdd_object_manager.h"

const struct nla_policy
peer_txq_flush_policy[QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_PEER_ADDR] = VENDOR_NLA_POLICY_MAC_ADDR,
	[QCA_WLAN_VENDOR_ATTR_AC] = { .type = NLA_U8 },
	[QCA_WLAN_VENDOR_ATTR_TID_MASK] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_POLICY] = { .type = NLA_U32 },
};

/**
 * map_txq_policy() - Map NL flush policy attribute value to DP
 * @policy: NL flush policy attribute value
 *
 * This function maps NL flush policy attribute value to DP
 *
 * Return: Valid DP policy value, else invalid
 */
static enum cdp_peer_txq_flush_policy
map_txq_policy(enum qca_wlan_vendor_flush_pending_policy policy)
{
	switch (policy) {
	case QCA_WLAN_VENDOR_FLUSH_PENDING_POLICY_NONE:
		return CDP_PEER_TXQ_FLUSH_POLICY_NONE;
	case QCA_WLAN_VENDOR_FLUSH_PENDING_POLICY_IMMEDIATE:
		return CDP_PEER_TXQ_FLUSH_POLICY_IMMEDIATE;
	case QCA_WLAN_VENDOR_FLUSH_PENDING_POLICY_TWT_SP_END:
		return CDP_PEER_TXQ_FLUSH_POLICY_TWT_SP_END;
	default:
		return CDP_PEER_TXQ_FLUSH_POLICY_INVALID;
	}
}

/**
 * hdd_peer_txq_flush_config() - Propagate txq flush config to DP
 * @adapter: Pointer to HDD adapter structure
 * @tb: NL attributes
 *
 * This function maps NL to DP attributes and proagates the configuration
 *
 * Return: 0 on success, negative errno on failure
 */
static int hdd_peer_txq_flush_config(struct hdd_adapter *adapter,
				     struct nlattr *tb[])
{
	void *dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint8_t addr[QDF_MAC_ADDR_SIZE];
	uint32_t ac, tid, cmd_id;
	enum qca_wlan_vendor_flush_pending_policy txq_policy;
	enum cdp_peer_txq_flush_policy cdp_policy;

	if (!tb || !dp_soc) {
		hdd_err("Invalid attributes");
		return -EINVAL;
	}

	nla_memcpy(addr, tb[QCA_WLAN_VENDOR_ATTR_PEER_ADDR], QDF_MAC_ADDR_SIZE);

	if (tb[QCA_WLAN_VENDOR_ATTR_TID_MASK]) {
		tid = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_TID_MASK]);

		cmd_id = QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_POLICY;
		if (!tb[cmd_id]) {
			hdd_err("Flush policy not provided");
			return -EINVAL;
		}
		txq_policy = nla_get_u32(tb[cmd_id]);
		cdp_policy = map_txq_policy(txq_policy);
		if (cdp_policy == CDP_PEER_TXQ_FLUSH_POLICY_INVALID) {
			hdd_err("Invalid dp flush policy %d", txq_policy);
			return -EINVAL;
		}
		ac = 0;
	} else if (tb[QCA_WLAN_VENDOR_ATTR_AC]) {
		ac = nla_get_u8(tb[QCA_WLAN_VENDOR_ATTR_AC]);
		cdp_policy = CDP_PEER_TXQ_FLUSH_POLICY_INVALID;
		tid = 0;
	} else {
		hdd_err("No ac/tid mask");
		return -EINVAL;
	}

	return cdp_set_peer_txq_flush_config(dp_soc, adapter->vdev_id,
					     addr, ac, tid, cdp_policy);
}

/**
 * __wlan_hdd_cfg80211_peer_txq_flush_config() - flush peer txq config
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Length of @data
 *
 * This function is used to flush peer pending packets using vendor commands
 *
 * Return: 0 on success, negative errno on failure
 */
static int
__wlan_hdd_cfg80211_peer_txq_flush_config(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_MAX + 1];
	int ret;

	hdd_enter();

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_FLUSH_PENDING_MAX,
				    data, data_len, peer_txq_flush_policy)) {
		hdd_err("Invalid attributes");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_PEER_ADDR]) {
		hdd_err("Peer mac not provided");
		return -EINVAL;
	}

	if (!tb[QCA_WLAN_VENDOR_ATTR_AC] &&
	    !tb[QCA_WLAN_VENDOR_ATTR_TID_MASK]) {
		hdd_err("AC/TID mask not provided");
		return -EINVAL;
	}

	ret = hdd_peer_txq_flush_config(adapter, tb);

	hdd_exit();

	return ret;
}

int wlan_hdd_cfg80211_peer_txq_flush_config(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    const void *attr,
					    int attr_len)
{
	int ret;
	struct osif_vdev_sync *vdev_sync;

	ret = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (ret)
		return ret;

	ret = __wlan_hdd_cfg80211_peer_txq_flush_config(wiphy, wdev,
							attr, attr_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return ret;
}
