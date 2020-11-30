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
 * DOC: wlan_reg_channel_api.h
 * This file provides prototypes of the routines needed for the
 * external components to utilize the services provided by the
 * regulatory component with respect to channel list access.
 */

#ifndef __WLAN_REG_CHANNEL_API_H
#define __WLAN_REG_CHANNEL_API_H

#include <reg_services_public_struct.h>

#ifdef CONFIG_HOST_FIND_CHAN

#define WLAN_CHAN_DFS              0x0002  /* DFS set on primary segment */
#define WLAN_CHAN_DFS_CFREQ2       0x0004  /* DFS set on secondary segment */
#define WLAN_CHAN_DISALLOW_ADHOC   0x0040  /* ad-hoc is not allowed */
#define WLAN_CHAN_PSC              0x0400  /* 6GHz PSC frequency */

/**
 * wlan_reg_is_phymode_chwidth_allowed() - Check if requested phymode is allowed
 * @pdev: pdev pointer.
 * @phy_in: phymode that the user requested.
 * @ch_width: Channel width that the user requested.
 *
 * Return: true if phymode is allowed, else false.
 */
bool wlan_reg_is_phymode_chwidth_allowed(struct wlan_objmgr_pdev *pdev,
					 enum reg_phymode phy_in,
					 enum phy_ch_width ch_width,
					 qdf_freq_t primary_freq);

/**
 * wlan_reg_get_max_phymode_and_chwidth() - Find the maximum regmode and
 * channel width combo supported by the device.
 * @phy_in: Maximum reg_phymode.
 * @ch_width: Maximum channel width.
 * @primary_freq: Input primary frequency.
 *
 * Return QDF_STATUS_SUCCESS if a combination is found, else return failure.
 */
QDF_STATUS wlan_reg_get_max_phymode_and_chwidth(struct wlan_objmgr_pdev *pdev,
						enum reg_phymode *phy_in,
						enum phy_ch_width *ch_width);

/**
 * wlan_reg_get_txpow_ant_gain() - Find the tx power and antenna gain for
 * the given frequency.
 * @pdev: pdev pointer.
 * @freq: Given frequency.
 * @txpower: tx power to be filled.
 * @ant_gain: Antenna gain to be filled.
 *
 */
void wlan_reg_get_txpow_ant_gain(struct wlan_objmgr_pdev *pdev,
				 qdf_freq_t freq,
				 uint32_t *txpower,
				 uint8_t *ant_gain);

/**
 * wlan_reg_get_chan_flags() - Find the channel flags for freq1 and freq2.
 * @pdev: pdev pointer.
 * @freq1: Frequency in primary segment.
 * @freq2: Frequency in secondary segment.
 * @flags: Flags to be filled.
 *
 */
void wlan_reg_get_chan_flags(struct wlan_objmgr_pdev *pdev,
			     qdf_freq_t freq1,
			     qdf_freq_t freq2,
			     uint16_t *flags);
#else
static inline bool
wlan_reg_is_phymode_chwidth_allowed(struct wlan_objmgr_pdev *pdev,
				    enum reg_phymode phy_in,
				    enum phy_ch_width ch_width,
				    qdf_freq_t primary_freq)
{
	return false;
}

static inline QDF_STATUS
wlan_reg_get_max_phymode_and_chwidth(struct wlan_objmgr_pdev *pdev,
				     enum reg_phymode *phy_in,
				     enum phy_ch_width *ch_width)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void
wlan_reg_get_txpow_ant_gain(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t freq,
			    uint32_t *txpower,
			    uint8_t *ant_gain)
{
}

static inline void
wlan_reg_get_chan_flags(struct wlan_objmgr_pdev *pdev,
			qdf_freq_t freq1,
			qdf_freq_t freq2,
			uint16_t *flags)
{
}
#endif /* CONFIG_HOST_FIND_CHAN */

#endif /* __WLAN_REG_CHANNEL_API_H */
