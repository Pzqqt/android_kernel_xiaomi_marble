/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
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
 * @file wlan_reg_channel_api.c
 * @brief contains regulatory channel access functions
 */

#include <qdf_status.h>
#include <qdf_types.h>
#include <qdf_module.h>
#include <wlan_cmn.h>
#include <wlan_reg_channel_api.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <reg_priv_objs.h>
#include <reg_services_common.h>
#include <reg_channel.h>
#include <wlan_reg_services_api.h>

#ifdef CONFIG_HOST_FIND_CHAN
bool wlan_reg_is_phymode_chwidth_allowed(struct wlan_objmgr_pdev *pdev,
					 enum reg_phymode phy_in,
					 enum phy_ch_width ch_width,
					 qdf_freq_t primary_freq)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return false;
	}

	return reg_is_phymode_chwidth_allowed(pdev_priv_obj, phy_in, ch_width,
					      primary_freq);
}

QDF_STATUS wlan_reg_get_max_phymode_and_chwidth(struct wlan_objmgr_pdev *pdev,
						enum reg_phymode *phy_in,
						enum phy_ch_width *ch_width)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	*phy_in = pdev_priv_obj->max_phymode;
	*ch_width = pdev_priv_obj->max_chwidth;

	return QDF_STATUS_SUCCESS;
}

void wlan_reg_get_txpow_ant_gain(struct wlan_objmgr_pdev *pdev,
				 qdf_freq_t freq,
				 uint32_t *txpower,
				 uint8_t *ant_gain)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cur_chan_list[i].center_freq == freq) {
			*txpower = cur_chan_list[i].tx_power;
			*ant_gain = cur_chan_list[i].ant_gain;
		}
	}
}

void wlan_reg_get_chan_flags(struct wlan_objmgr_pdev *pdev,
			     qdf_freq_t freq1,
			     qdf_freq_t freq2,
			     uint16_t *flags)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	int i;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (cur_chan_list[i].center_freq == freq1 &&
		    cur_chan_list[i].chan_flags & REGULATORY_CHAN_RADAR) {
			*flags |= WLAN_CHAN_DFS;
			*flags |= WLAN_CHAN_DISALLOW_ADHOC;
		}

		if (cur_chan_list[i].center_freq == freq2 &&
		    cur_chan_list[i].chan_flags & REGULATORY_CHAN_RADAR) {
			*flags |= WLAN_CHAN_DFS_CFREQ2;
			*flags |= WLAN_CHAN_DISALLOW_ADHOC;
		}
	}

	if (WLAN_REG_IS_6GHZ_PSC_CHAN_FREQ(freq1))
		*flags |= WLAN_CHAN_PSC;
}

#endif /* CONFIG_HOST_FIND_CHAN */
