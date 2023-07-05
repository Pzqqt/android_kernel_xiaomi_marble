/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * DOC:  wlan_hdd_mdns_offload.c
 *
 * WLAN Host Device Driver MDNS offload interface implementation.
 */

/* Include Files */

#include <osif_vdev_sync.h>
#include "os_if_fwol.h"
#include "wlan_hdd_mdns_offload.h"
#include "wlan_hdd_main.h"

#define MDNS_ENABLE \
	QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_ENABLE
#define MDNS_TABLE \
	QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_TABLE
#define MDNS_ENTRY \
	QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_ENTRY
#define MDNS_FQDN \
	QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_FQDN
#define MDNS_RESOURCE_RECORDS_COUNT \
	QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_ANSWER_RESOURCE_RECORDS_COUNT
#define MDNS_ANSWER_PAYLOAD \
	QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_ANSWER_PAYLOAD

const struct nla_policy wlan_hdd_set_mdns_offload_policy[
			QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX + 1] = {
	[MDNS_ENABLE] = {.type = NLA_FLAG},
	[MDNS_TABLE] = {.type = NLA_NESTED},
	[MDNS_ENTRY] = {.type = NLA_NESTED},
	[MDNS_FQDN] = {.type = NLA_STRING, .len = MAX_FQDN_LEN - 1 },
	[MDNS_RESOURCE_RECORDS_COUNT] = {.type = NLA_U16},
	[MDNS_ANSWER_PAYLOAD] = {.type = NLA_BINARY, .len = MAX_MDNS_RESP_LEN },

};

static int hdd_extract_mdns_attr(struct nlattr *tb[],
				 struct mdns_config_info *mdns_info)
{
	int len, count;

	/* Configure mDNS FQDN*/

	if (!tb[MDNS_FQDN]) {
		hdd_err_rl("mDNS_FQDN attr failed");
		return -EINVAL;
	}

	len = nla_len(tb[MDNS_FQDN]);
	mdns_info->fqdn_len = len;
	mdns_info->fqdn_type = MDNS_FQDN_TYPE_GENERAL;
	wlan_cfg80211_nla_strscpy(mdns_info->fqdn_data, tb[MDNS_FQDN],
				  sizeof(mdns_info->fqdn_data));

	/* Configure mDNS Answer Payload*/

	if (!tb[MDNS_RESOURCE_RECORDS_COUNT]) {
		hdd_err_rl("mDNS_RR count attr failed");
		return -EINVAL;
	}

	count = nla_get_u16(tb[MDNS_RESOURCE_RECORDS_COUNT]);
	mdns_info->resource_record_count = count;

	if (!tb[MDNS_ANSWER_PAYLOAD]) {
		hdd_err_rl("mDNS_Response attr failed");
		return -EINVAL;
	}

	len = nla_len(tb[MDNS_ANSWER_PAYLOAD]);
	mdns_info->answer_payload_len = len;
	nla_memcpy(mdns_info->answer_payload_data,
		   nla_data(tb[MDNS_ANSWER_PAYLOAD]),
		   sizeof(mdns_info->answer_payload_data));

	return 0;
}

/**
 * __wlan_hdd_cfg80211_set_mdns_offload() - mDNS Offload configuration
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX.
 *
 * Return: Error code.
 */
static int
__wlan_hdd_cfg80211_set_mdns_offload(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_context *hdd_ctx  = wiphy_priv(wiphy);
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct nlattr *curr_attr;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX + 1];
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX + 1];
	struct mdns_config_info *mdns_info;
	bool is_mdns_enable;
	int errno, rem;

	hdd_enter_dev(dev);

	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno)
		return errno;

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err_rl("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (!(adapter->device_mode == QDF_STA_MODE ||
	      adapter->device_mode == QDF_P2P_CLIENT_MODE)) {
		hdd_err_rl("mDNS is only supported in STA or P2P CLI modes!");
		return -ENOTSUPP;
	}
	if (wlan_cfg80211_nla_parse(tb, QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX,
				    data, data_len,
				    wlan_hdd_set_mdns_offload_policy)) {
		hdd_err_rl("invalid attr");
		return -EINVAL;
	}

	if (!tb[MDNS_ENABLE]) {
		os_if_fwol_disable_mdns_offload(hdd_ctx->psoc);
		return -EINVAL;
	}

	is_mdns_enable = nla_get_flag(tb[MDNS_ENABLE]);

	if (is_mdns_enable && !tb[MDNS_TABLE]) {
		hdd_err_rl("Invalid mDNS table of records");
		return -EINVAL;
	}

	mdns_info = qdf_mem_malloc(sizeof(*mdns_info));
	if (!mdns_info)
		return -ENOMEM;

	mdns_info->enable = is_mdns_enable;
	mdns_info->vdev_id = adapter->vdev_id;

	nla_for_each_nested(curr_attr, tb[MDNS_TABLE], rem) {
		if (wlan_cfg80211_nla_parse(tb2,
					QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX,
					nla_data(curr_attr),
					nla_len(curr_attr),
					wlan_hdd_set_mdns_offload_policy)) {
			hdd_err_rl("Failed to parse mDNS table of records");
			errno = -EINVAL;
			goto out;
		}

		errno = hdd_extract_mdns_attr(tb2, mdns_info);
		if (errno) {
			hdd_err_rl("Failed to extract MDNS nested attrs");
			goto out;
		}
	}

	os_if_fwol_enable_mdns_offload(hdd_ctx->psoc, mdns_info);

out:
	qdf_mem_free(mdns_info);
	hdd_exit();

	return errno;
}

/**
 * wlan_hdd_cfg80211_set_mdns_offload() - mDNS Offload configuration
 * @wiphy: wiphy device pointer
 * @wdev: wireless device pointer
 * @data: Vendor command data buffer
 * @data_len: Buffer length
 *
 * Handles QCA_WLAN_VENDOR_ATTR_MDNS_OFFLOAD_MAX.
 *
 * Return: EOK or other error codes.
 */
int wlan_hdd_cfg80211_set_mdns_offload(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       const void *data, int data_len)
{
	int errno;
	struct osif_vdev_sync *vdev_sync;

	errno = osif_vdev_sync_op_start(wdev->netdev, &vdev_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_set_mdns_offload(wiphy, wdev,
						     data, data_len);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno;
}
