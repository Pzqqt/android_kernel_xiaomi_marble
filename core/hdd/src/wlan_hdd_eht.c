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
 * DOC : wlan_hdd_eht.c
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_eht.h"
#include "osif_sync.h"
#include "wlan_utility.h"
#include "wlan_mlme_ucfg_api.h"

void hdd_update_tgt_eht_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg)
{
	tDot11fIEeht_cap eht_cap_ini = {0};

	ucfg_mlme_update_tgt_eht_cap(hdd_ctx->psoc, cfg);
	sme_update_tgt_eht_cap(hdd_ctx->mac_handle, cfg, &eht_cap_ini);
}

void wlan_hdd_check_11be_support(struct hdd_beacon_data *beacon,
				 struct sap_config *config)
{
	const uint8_t *ie;

	ie = wlan_get_ext_ie_ptr_from_ext_id(EHT_CAP_OUI_TYPE, EHT_CAP_OUI_SIZE,
					     beacon->tail, beacon->tail_len);
	if (ie)
		config->SapHw_mode = eCSR_DOT11_MODE_11be;
}

static void
hdd_update_wiphy_eht_caps_6ghz(struct hdd_context *hdd_ctx,
			       struct wma_tgt_cfg *cfg)
{
	struct ieee80211_supported_band *band_6g =
		   hdd_ctx->wiphy->bands[HDD_NL80211_BAND_6GHZ];
	uint8_t *phy_info =
		    hdd_ctx->iftype_data_6g->eht_cap.eht_cap_elem.phy_cap_info;
	uint8_t max_fw_bw = sme_get_vht_ch_width();

	if (!band_6g || !phy_info) {
		hdd_debug("6ghz not supported in wiphy");
		return;
	}

	hdd_ctx->iftype_data_6g->types_mask =
		(BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP));
	hdd_ctx->iftype_data_6g->eht_cap.has_eht = true;
	hdd_ctx->iftype_data_6g->he_cap.has_he = true;
	band_6g->n_iftype_data = 1;

	if (max_fw_bw >= WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ)
		phy_info[0] |=
		      IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
	if (max_fw_bw >= WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ)
		phy_info[0] |=
			IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G;
	if (max_fw_bw >= WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ)
		phy_info[0] |=
		     IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G;

	band_6g->iftype_data = hdd_ctx->iftype_data_6g;
}

void hdd_update_wiphy_eht_cap(struct hdd_context *hdd_ctx,
			      struct wma_tgt_cfg *cfg)
{
	tDot11fIEeht_cap eht_cap_cfg;
	struct ieee80211_supported_band *band_2g =
			hdd_ctx->wiphy->bands[HDD_NL80211_BAND_2GHZ];
	struct ieee80211_supported_band *band_5g =
			hdd_ctx->wiphy->bands[HDD_NL80211_BAND_5GHZ];
	QDF_STATUS status;
	uint8_t *phy_info_5g =
		    hdd_ctx->iftype_data_5g->eht_cap.eht_cap_elem.phy_cap_info;
	uint8_t max_fw_bw = sme_get_vht_ch_width();
	uint32_t channel_bonding_mode_2g;
	uint8_t *phy_info_2g =
		    hdd_ctx->iftype_data_2g->eht_cap.eht_cap_elem.phy_cap_info;

	hdd_enter();

	status = ucfg_mlme_cfg_get_eht_caps(hdd_ctx->psoc, &eht_cap_cfg);

	if (QDF_IS_STATUS_ERROR(status))
		return;

	if (band_2g) {
		hdd_ctx->iftype_data_2g->types_mask =
			(BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP));
		hdd_ctx->iftype_data_2g->eht_cap.has_eht = eht_cap_cfg.present;
		hdd_ctx->iftype_data_2g->he_cap.has_he = true;
		band_2g->n_iftype_data = 1;
		band_2g->iftype_data = hdd_ctx->iftype_data_2g;

		ucfg_mlme_get_channel_bonding_24ghz(hdd_ctx->psoc,
						    &channel_bonding_mode_2g);
		if (channel_bonding_mode_2g)
			phy_info_2g[0] |=
			    IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_IN_2G;
		qdf_mem_copy(
		 &hdd_ctx->iftype_data_2g->eht_cap.eht_cap_elem.mac_cap_info[0],
		 &cfg->eht_cap.eht_mac_cap, 6);
		qdf_mem_copy(
		 &hdd_ctx->iftype_data_2g->eht_cap.eht_cap_elem.phy_cap_info[0],
		 &cfg->eht_cap.phy_cap_bytes, 11);
	}
	if (band_5g) {
		hdd_ctx->iftype_data_5g->types_mask =
			(BIT(NL80211_IFTYPE_STATION) | BIT(NL80211_IFTYPE_AP));
		hdd_ctx->iftype_data_5g->eht_cap.has_eht = eht_cap_cfg.present;
		hdd_ctx->iftype_data_5g->he_cap.has_he = true;
		band_5g->n_iftype_data = 1;
		band_5g->iftype_data = hdd_ctx->iftype_data_5g;
		if (max_fw_bw >= WNI_CFG_VHT_CHANNEL_WIDTH_80MHZ)
			phy_info_5g[0] |=
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_40MHZ_80MHZ_IN_5G;
		if (max_fw_bw >= WNI_CFG_VHT_CHANNEL_WIDTH_160MHZ)
			phy_info_5g[0] |=
				IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_160MHZ_IN_5G;
		if (max_fw_bw >= WNI_CFG_VHT_CHANNEL_WIDTH_80_PLUS_80MHZ)
			phy_info_5g[0] |=
			     IEEE80211_HE_PHY_CAP0_CHANNEL_WIDTH_SET_80PLUS80_MHZ_IN_5G;
	}

	hdd_update_wiphy_eht_caps_6ghz(hdd_ctx, cfg);

	hdd_exit();
}

