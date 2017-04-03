/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: defines nan component os interface APIs
 */

#include "qdf_types.h"
#include "qdf_trace.h"
#include "os_if_nan.h"
#include "wlan_nlink_srv.h"
#include "nan_public_structs.h"
#include "wlan_cfg80211.h"

/* NLA policy */
static const struct nla_policy
vendor_attr_policy[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR] = { .type = NLA_STRING,
					.len = IFNAMSIZ },
	[QCA_WLAN_VENDOR_ATTR_NDP_SERVICE_INSTANCE_ID] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PEER_DISCOVERY_MAC_ADDR] = {
					.type = NLA_BINARY,
					.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_SECURITY] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_CONFIG_QOS] = { .type = NLA_BINARY,
					.len = NDP_QOS_INFO_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_APP_INFO] = { .type = NLA_BINARY,
					.len = NDP_APP_INFO_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_RESPONSE_CODE] = { .type = NLA_U16 },
	[QCA_WLAN_VENDOR_ATTR_NDP_NDI_MAC_ADDR] = { .type = NLA_BINARY,
					.len = QDF_MAC_ADDR_SIZE },
	[QCA_WLAN_VENDOR_ATTR_NDP_INSTANCE_ID_ARRAY] = { .type = NLA_BINARY,
					.len = NDP_NUM_INSTANCE_ID },
	[QCA_WLAN_VENDOR_ATTR_NDP_CHANNEL_CONFIG] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_NCS_SK_TYPE] = { .type = NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_PMK] = { .type = NLA_BINARY,
					.len = NDP_PMK_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_SCID] = { .type = NLA_BINARY,
					.len = NDP_SCID_BUF_LEN },
	[QCA_WLAN_VENDOR_ATTR_NDP_DRV_RESPONSE_STATUS_TYPE] = { .type =
					NLA_U32 },
	[QCA_WLAN_VENDOR_ATTR_NDP_DRV_RETURN_VALUE] = { .type = NLA_U32 },
};

int os_if_nan_process_ndp_cmd(void *ctx, struct wlan_objmgr_psoc *psoc,
			      const void *data, int data_len)
{
	uint32_t ndp_cmd_type;
	uint16_t transaction_id;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX + 1];
	char *iface_name;

	if (nla_parse(tb, QCA_WLAN_VENDOR_ATTR_NDP_PARAMS_MAX,
			data, data_len, vendor_attr_policy)) {
		cfg80211_err("Invalid NDP vendor command attributes");
		return -EINVAL;
	}

	/* Parse and fetch NDP Command Type*/
	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]) {
		cfg80211_err("NAN datapath cmd type failed");
		return -EINVAL;
	}
	ndp_cmd_type = nla_get_u32(tb[QCA_WLAN_VENDOR_ATTR_NDP_SUBCMD]);

	if (!tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]) {
		cfg80211_err("attr transaction id failed");
		return -EINVAL;
	}
	transaction_id = nla_get_u16(
			tb[QCA_WLAN_VENDOR_ATTR_NDP_TRANSACTION_ID]);

	if (tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]) {
		iface_name = nla_data(tb[QCA_WLAN_VENDOR_ATTR_NDP_IFACE_STR]);
		cfg80211_err("Transaction Id: %d NDPCmd: %d iface_name: %s",
			transaction_id, ndp_cmd_type, iface_name);
	} else {
		cfg80211_err("Transaction Id: %d NDPCmd: %d iface_name: unspecified",
			transaction_id, ndp_cmd_type);
	}

	switch (ndp_cmd_type) {
	default:
		cfg80211_err("Unrecognized NDP vendor cmd %d", ndp_cmd_type);
		return -EINVAL;
	}

	return -EINVAL;
}

void os_if_nan_event_handler(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_vdev *vdev,
			     uint32_t type, void *msg)
{
	switch (type) {
	default:
		break;
	}
}
