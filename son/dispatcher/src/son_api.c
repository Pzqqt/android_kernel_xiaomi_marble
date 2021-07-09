/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : contains interface prototypes for son api
 */

#include <son_api.h>
#include <wlan_reg_services_api.h>
#include <wlan_mlme_api.h>
#include <ieee80211_external.h>

/**
 * wlan_son_is_he_supported() - is he supported or not
 * @psoc: pointer to psoc
 * @he_supported: he supported or not
 *
 * Return: void
 */
#ifdef WLAN_FEATURE_11AX
static void wlan_son_is_he_supported(struct wlan_objmgr_psoc *psoc,
				     bool *he_supported)
{
	tDot11fIEhe_cap *he_cap = NULL;

	*he_supported = false;
	mlme_cfg_get_he_caps(psoc, he_cap);
	if (he_cap && he_cap->present)
		*he_supported = true;
}
#else
static void wlan_son_is_he_supported(struct wlan_objmgr_psoc *psoc,
				     bool *he_supported)
{
	*he_supported = false;
}
#endif /*WLAN_FEATURE_11AX*/

uint32_t wlan_son_get_chan_flag(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq, bool flag_160,
				struct ch_params *chan_params)
{
	uint32_t flags = 0;
	qdf_freq_t sec_freq;
	struct ch_params ch_width40_ch_params;
	uint8_t sub_20_channel_width = 0;
	enum phy_ch_width bandwidth = mlme_get_vht_ch_width();
	struct wlan_objmgr_psoc *psoc;
	bool is_he_enabled;

	if (!pdev) {
		qdf_err("invalid pdev");
		return flags;
	}
	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		qdf_err("invalid psoc");
		return flags;
	}

	wlan_son_is_he_supported(psoc, &is_he_enabled);

	wlan_mlme_get_sub_20_chan_width(wlan_pdev_get_psoc(pdev),
					&sub_20_channel_width);

	qdf_mem_zero(chan_params, sizeof(*chan_params));
	qdf_mem_zero(&ch_width40_ch_params, sizeof(ch_width40_ch_params));
	if (wlan_reg_is_24ghz_ch_freq(freq)) {
		if (bandwidth == CH_WIDTH_80P80MHZ ||
		    bandwidth == CH_WIDTH_160MHZ ||
		    bandwidth == CH_WIDTH_80MHZ)
			bandwidth = CH_WIDTH_40MHZ;
	}

	switch (bandwidth) {
	case CH_WIDTH_80P80MHZ:
		if (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								  bandwidth) !=
		    CHANNEL_STATE_INVALID) {
			if (!flag_160) {
				chan_params->ch_width = CH_WIDTH_80P80MHZ;
				wlan_reg_set_channel_params_for_freq(
					pdev, freq, 0, chan_params);
			}
			if (is_he_enabled)
				flags |= VENDOR_CHAN_FLAG2(
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80_80);
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80_80;
		}
		bandwidth = CH_WIDTH_160MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_160MHZ:
		if (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								  bandwidth) !=
		    CHANNEL_STATE_INVALID) {
			if (flag_160) {
				chan_params->ch_width = CH_WIDTH_160MHZ;
				wlan_reg_set_channel_params_for_freq(
					pdev, freq, 0, chan_params);
			}
			if (is_he_enabled)
				flags |= VENDOR_CHAN_FLAG2(
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE160);
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT160;
		}
		bandwidth = CH_WIDTH_80MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_80MHZ:
		if (wlan_reg_get_5g_bonded_channel_state_for_freq(pdev, freq,
								  bandwidth) !=
		    CHANNEL_STATE_INVALID) {
			if (!flag_160 &&
			    chan_params->ch_width != CH_WIDTH_80P80MHZ) {
				chan_params->ch_width = CH_WIDTH_80MHZ;
				wlan_reg_set_channel_params_for_freq(
					pdev, freq, 0, chan_params);
			}
			if (is_he_enabled)
				flags |= VENDOR_CHAN_FLAG2(
					QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE80);
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT80;
		}
		bandwidth = CH_WIDTH_40MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_40MHZ:
		ch_width40_ch_params.ch_width = bandwidth;
		wlan_reg_set_channel_params_for_freq(pdev, freq, 0,
						     &ch_width40_ch_params);

		if (ch_width40_ch_params.sec_ch_offset == LOW_PRIMARY_CH)
			sec_freq = freq + 20;
		else if (ch_width40_ch_params.sec_ch_offset == HIGH_PRIMARY_CH)
			sec_freq = freq - 20;
		else
			sec_freq = 0;

		if (wlan_reg_get_bonded_channel_state_for_freq(pdev, freq,
							       bandwidth,
							       sec_freq) !=
		    CHANNEL_STATE_INVALID) {
			if (ch_width40_ch_params.sec_ch_offset ==
			    LOW_PRIMARY_CH) {
				if (is_he_enabled)
				  flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40PLUS;
				flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40PLUS;
				flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40PLUS;
			} else if (ch_width40_ch_params.sec_ch_offset ==
				   HIGH_PRIMARY_CH) {
				if (is_he_enabled)
				  flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE40MINUS;
				flags |=
				   QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT40MINUS;
				flags |=
				    QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT40PLUS;
			}
		}
		bandwidth = CH_WIDTH_20MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_20MHZ:
		flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HT20;
		flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_VHT20;
		if (is_he_enabled)
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HE20;
		bandwidth = CH_WIDTH_10MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_10MHZ:
		if (wlan_reg_get_bonded_channel_state_for_freq(pdev, freq,
							       bandwidth,
							       0) !=
		     CHANNEL_STATE_INVALID &&
		     sub_20_channel_width == WLAN_SUB_20_CH_WIDTH_10)
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_HALF;
		bandwidth = CH_WIDTH_5MHZ;
	/* FALLTHROUGH */
	case CH_WIDTH_5MHZ:
		if (wlan_reg_get_bonded_channel_state_for_freq(pdev, freq,
							       bandwidth,
							       0) !=
		    CHANNEL_STATE_INVALID &&
		    sub_20_channel_width == WLAN_SUB_20_CH_WIDTH_5)
			flags |= QCA_WLAN_VENDOR_CHANNEL_PROP_FLAG_QUARTER;
		break;
	default:
		qdf_info("invalid channel width value %d", bandwidth);
	}

	return flags;
}
