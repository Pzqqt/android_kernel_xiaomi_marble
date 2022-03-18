/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cfg80211_wifi_pos.c
 * defines wifi-pos module related driver functions interfacing with linux
 * kernel
 */
#include "wlan_cfg80211.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_cfg80211_wifi_pos.h"

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)) || \
	defined(CFG80211_PASN_SUPPORT)) && \
	defined(WIFI_POS_CONVERGED)
void
wlan_wifi_pos_cfg80211_set_wiphy_ext_feature(struct wiphy *wiphy,
					     struct wlan_objmgr_psoc *psoc)
{
	if (wlan_psoc_nif_fw_ext2_cap_get(psoc,
					  WLAN_RTT_11AZ_MAC_SEC_SUPPORT)) {
		wiphy_ext_feature_set(wiphy,
				      NL80211_EXT_FEATURE_PROT_RANGE_NEGO_AND_MEASURE);
	}

	if (wlan_psoc_nif_fw_ext2_cap_get(psoc,
					  WLAN_RTT_11AZ_MAC_PHY_SEC_SUPPORT))
		wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_SECURE_LTF);

	if (wlan_psoc_nif_fw_ext_cap_get(psoc, WLAN_RTT_11AZ_NTB_SUPPORT))
		wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_NTB_RANGING);

	if (wlan_psoc_nif_fw_ext2_cap_get(psoc, WLAN_RTT_11AZ_TB_SUPPORT))
		wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_TB_RANGING);
}
#endif
