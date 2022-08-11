/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC:  wlan_hdd_avoid_freq_ext.c
 *
 * WLAN Host Device Driver extended avoid frequency interface implementation.
 */

/* Include Files */

#include <osif_psoc_sync.h>
#include "wlan_hdd_avoid_freq_ext.h"
#include "wlan_reg_ucfg_api.h"
#include "wlan_reg_services_api.h"

#define AVOID_FREQ_EXT_MAX QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX

const struct nla_policy
avoid_freq_ext_policy [QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_RANGE] = { .type = NLA_NESTED },
	[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_START] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_END] = {.type = NLA_U32},
	[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_POWER_CAP_DBM] = {.type =
								NLA_S32},
	[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_IFACES_BITMASK] = {.type =
								NLA_U32},
};

/**
 * __wlan_hdd_cfg80211_avoid_freq_ext() - exclude channels from upper layer.
 * @wiphy: wiphy structure pointer
 * @wdev: Wireless device structure pointer
 * @data: Pointer to the data received
 * @data_len: Length of @data
 *
 * __wlan_hdd_cfg80211_avoid_freq_ext() extract the valid avoid frequency
 * list from upper layer and prepared one extended avoid frequency list for
 * regulatory component.
 *
 * Return: 0 on success; errno on failure
 */
static int
__wlan_hdd_cfg80211_avoid_freq_ext(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
		const void *data, int data_len)
{
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	int ret = 0;
	struct ch_avoid_ind_type avoid_freq_list;
	enum QDF_GLOBAL_MODE curr_mode;
	struct nlattr *tb[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX + 1];
	struct nlattr *tb2[QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX + 1];
	struct nlattr *freq_ext;
	int id, rem, i, sub_id;
	struct ch_avoid_freq_type *avoid_freq_range;

	hdd_enter_dev(wdev->netdev);

	if (!ucfg_mlme_get_coex_unsafe_chan_nb_user_prefer(hdd_ctx->psoc)) {
		hdd_debug_rl("Coex unsafe chan nb user prefer is not set");
		return -EOPNOTSUPP;
	}

	curr_mode = hdd_get_conparam();
	if (curr_mode == QDF_GLOBAL_FTM_MODE ||
	    curr_mode == QDF_GLOBAL_MONITOR_MODE) {
		hdd_debug_rl("Command not allowed in FTM/MONITOR mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (hdd_is_connection_in_progress(NULL, NULL)) {
		hdd_debug_rl("Update chan list refused: conn in progress");
		ret = -EPERM;
		goto out;
	}

	qdf_mem_zero(&avoid_freq_list, sizeof(struct ch_avoid_ind_type));

	if (!data && data_len == 0) {
		hdd_debug_rl("Clear extended avoid frequency list");
		goto process_avoid_channel_ext;
	}

	ret = wlan_cfg80211_nla_parse(tb,
				      QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX,
				      data,
				      data_len,
				      avoid_freq_ext_policy);
	if (ret) {
		hdd_err_rl("Invalid avoid freq ext ATTR");
		ret = -EINVAL;
		goto out;
	}

	id = QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_RANGE;

	if (!tb[id]) {
		hdd_err_rl("Attr avoid frequency ext range failed");
		ret = -EINVAL;
		goto out;
	}

	i = 0;
	avoid_freq_list.ch_avoid_range_cnt = CH_AVOID_MAX_RANGE;

	/* restriction mask */
	sub_id = QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_IFACES_BITMASK;

	if (tb[sub_id])
		avoid_freq_list.restriction_mask = nla_get_u32(tb[sub_id]);

	if (avoid_freq_list.restriction_mask & BIT(NL80211_IFTYPE_AP))
		avoid_freq_list.restriction_mask = (1 << QDF_SAP_MODE);

	nla_for_each_nested(freq_ext, tb[id], rem) {
		if (i == CH_AVOID_MAX_RANGE) {
			hdd_warn_rl("Ignoring excess range number");
			break;
		}

		if (wlan_cfg80211_nla_parse(tb2, AVOID_FREQ_EXT_MAX,
					    nla_data(freq_ext),
					    nla_len(freq_ext),
					    avoid_freq_ext_policy)) {
			hdd_err_rl("nla_parse failed");
			ret = -EINVAL;
			goto out;
		}

		avoid_freq_range = &avoid_freq_list.avoid_freq_range[i];

		/* ext avoid freq start */
		sub_id = QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_START;
		if (!tb2[sub_id]) {
			ret = -EINVAL;
			goto out;
		}
		avoid_freq_range->start_freq = nla_get_u32(tb2[sub_id]);

		/* ext avoid freq end */
		sub_id = QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_END;
		if (!tb2[sub_id]) {
			ret = -EINVAL;
			goto out;
		}
		avoid_freq_range->end_freq = nla_get_u32(tb2[sub_id]);

		if (!avoid_freq_range->start_freq &&
		    !avoid_freq_range->end_freq && (i < 1)) {
			hdd_debug_rl("Clear unsafe channel list");
		} else if (!wlan_reg_is_same_band_freqs(
			   avoid_freq_range->start_freq,
			   avoid_freq_range->end_freq)) {
			hdd_debug_rl("start freq %d end freq %d not in same band",
				     avoid_freq_range->start_freq,
				     avoid_freq_range->end_freq);
			ret = -EINVAL;
			goto out;
		}

		if (avoid_freq_range->end_freq <
		    avoid_freq_range->start_freq) {
			ret = -EINVAL;
			goto out;
		}

		/* ext txpower */
		sub_id = QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_POWER_CAP_DBM;

		if (tb2[sub_id]) {
			avoid_freq_range->txpower = nla_get_s32(tb2[sub_id]);
			avoid_freq_range->is_valid_txpower = true;
		}

		hdd_debug_rl("ext avoid freq start: %u end: %u txpower %d mask %d",
			     avoid_freq_range->start_freq,
			     avoid_freq_range->end_freq,
			     avoid_freq_range->txpower,
			     avoid_freq_list.restriction_mask);
		i++;
	}

	if (i < CH_AVOID_MAX_RANGE) {
		hdd_warn_rl("Number of freq range %u less than expected %u",
			    i, CH_AVOID_MAX_RANGE);
		avoid_freq_list.ch_avoid_range_cnt = i;
	}

process_avoid_channel_ext:
	ucfg_reg_ch_avoid_ext(hdd_ctx->psoc, &avoid_freq_list);
out:
	return ret;
}

/**
 * wlan_hdd_cfg80211_avoid_freq_ext() - exclude channels from upper layer.
 * @wiphy: wiphy structure pointer
 * @wdev: Wireless device structure pointer
 * @data: Pointer to the data received
 * @data_len: Length of @data
 *
 * wlan_hdd_cfg80211_avoid_freq_ext() will pass the extended avoid frequency
 * list from upper layer to regulatory component to compute one new channel
 * list.
 *
 * Return: 0 on success; errno on failure
 */
int wlan_hdd_cfg80211_avoid_freq_ext(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data, int data_len)
{
	struct osif_psoc_sync *psoc_sync;
	int errno;

	errno = osif_psoc_sync_op_start(wiphy_dev(wiphy), &psoc_sync);
	if (errno)
		return errno;

	errno = __wlan_hdd_cfg80211_avoid_freq_ext(wiphy, wdev, data, data_len);

	osif_psoc_sync_op_stop(psoc_sync);

	return errno;
}
