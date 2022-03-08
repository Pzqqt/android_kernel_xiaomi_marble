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

	curr_mode = hdd_get_conparam();
	if (curr_mode == QDF_GLOBAL_FTM_MODE ||
	    curr_mode == QDF_GLOBAL_MONITOR_MODE) {
		hdd_debug("Command not allowed in FTM/MONITOR mode");
		return -EINVAL;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (hdd_is_connection_in_progress(NULL, NULL)) {
		hdd_debug("Update chan list refused: conn in progress");
		ret = -EPERM;
		goto out;
	}

	qdf_mem_zero(&avoid_freq_list, sizeof(struct ch_avoid_ind_type));

	if (!data && data_len == 0) {
		hdd_debug("Clear extended avoid frequency list");
		goto process_avoid_channel_ext;
	}

	ret = wlan_cfg80211_nla_parse(tb,
				      QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_MAX,
				      data,
				      data_len,
				      avoid_freq_ext_policy);
	if (ret) {
		hdd_err("Invalid avoid freq ext ATTR");
		ret = -EINVAL;
		goto out;
	}

	id = QCA_WLAN_VENDOR_ATTR_AVOID_FREQUENCY_RANGE;

	if (!tb[id]) {
		hdd_err("Attr avoid frequency ext range failed");
		ret = -EINVAL;
		goto out;
	}

	i = 0;
	avoid_freq_list.ch_avoid_range_cnt = CH_AVOID_MAX_RANGE;

	nla_for_each_nested(freq_ext, tb[id], rem) {
		if (i == CH_AVOID_MAX_RANGE) {
			hdd_warn("Ignoring excess range number");
			break;
		}

		if (wlan_cfg80211_nla_parse(tb2, AVOID_FREQ_EXT_MAX,
					    nla_data(freq_ext),
					    nla_len(freq_ext),
					    avoid_freq_ext_policy)) {
			hdd_err("nla_parse failed");
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

		if (!wlan_reg_is_same_band_freqs(avoid_freq_range->start_freq,
						 avoid_freq_range->end_freq)) {
			hdd_debug("Not in same band");
			ret = -EINVAL;
			goto out;
		}

		if (avoid_freq_range->end_freq <
		    avoid_freq_range->start_freq) {
			ret = -EINVAL;
			goto out;
		}
		hdd_debug("ext avoid freq start: %u end: %u",
			  avoid_freq_range->start_freq,
			  avoid_freq_range->end_freq);
		i++;
	}

	if (i < CH_AVOID_MAX_RANGE) {
		hdd_warn("Number of freq range %u less than expected %u",
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
