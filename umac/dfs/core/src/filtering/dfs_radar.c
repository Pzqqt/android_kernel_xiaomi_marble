/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2011, Atheros Communications Inc.
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

#include "../dfs.h"
#include "../dfs_zero_cac.h"
#include "../dfs_filter_init.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_lmac_api.h"
#include "../dfs_partial_offload_radar.h"
#include "../dfs_internal.h"

void dfs_get_radars(struct wlan_dfs *dfs)
{
	struct wlan_objmgr_psoc *psoc;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		return;
	}

	if (wlan_objmgr_psoc_get_dev_type(psoc) == WLAN_DEV_OL) {
		/* For Partial offload */
		dfs_get_po_radars(dfs);
	}
}

int dfs_radar_disable(struct wlan_dfs *dfs)
{
	dfs->dfs_proc_phyerr &= ~DFS_AR_EN;
	dfs->dfs_proc_phyerr &= ~DFS_RADAR_EN;

	return 0;
}

void dfs_phyerr_param_copy(struct wlan_dfs_phyerr_param *dst,
		struct wlan_dfs_phyerr_param *src)
{
	qdf_mem_copy(dst, src, sizeof(*dst));
}

#ifdef CONFIG_CHAN_FREQ_API
struct dfs_state *dfs_getchanstate(struct wlan_dfs *dfs, uint8_t *index,
				   int ext_chan_flag)
{
	struct dfs_state *rs = NULL;
	struct dfs_channel *ch, cmp_ch1;
	int i;
	QDF_STATUS err;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return NULL;
	}
	ch = &cmp_ch1;
	if (ext_chan_flag) {
		err = dfs_mlme_get_extchan_for_freq(
					dfs->dfs_pdev_obj,
					&ch->dfs_ch_freq,
					&ch->dfs_ch_flags,
					&ch->dfs_ch_flagext,
					&ch->dfs_ch_ieee,
					&ch->dfs_ch_vhtop_ch_freq_seg1,
					&ch->dfs_ch_vhtop_ch_freq_seg2,
					&ch->dfs_ch_mhz_freq_seg1,
					&ch->dfs_ch_mhz_freq_seg2);

		if (err == QDF_STATUS_SUCCESS) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
				  "Extension channel freq = %u flags=0x%x",
				  ch->dfs_ch_freq,
				  ch->dfs_ch_flagext);
		} else {
			return NULL;
		}
	} else {
		ch = dfs->dfs_curchan;
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
			  "Primary channel freq = %u flags=0x%x",
			  ch->dfs_ch_freq, ch->dfs_ch_flagext);
	}

	for (i = 0; i < DFS_NUM_RADAR_STATES; i++) {
		if ((dfs->dfs_radar[i].rs_chan.dfs_ch_freq ==
			 ch->dfs_ch_freq) &&
			(dfs->dfs_radar[i].rs_chan.dfs_ch_flags ==
			 ch->dfs_ch_flags)) {
			if (index)
				*index = (uint8_t)i;
			return &dfs->dfs_radar[i];
		}
	}
	/* No existing channel found, look for first free channel state entry.*/
	for (i = 0; i < DFS_NUM_RADAR_STATES; i++) {
		if (dfs->dfs_radar[i].rs_chan.dfs_ch_freq == 0) {
			rs = &dfs->dfs_radar[i];
			/* Found one, set channel info and default thresholds.*/
			rs->rs_chan = *ch;

			/* Copy the parameters from the default set. */
			dfs_phyerr_param_copy(&rs->rs_param,
					      &dfs->dfs_defaultparams);

			if (index)
				*index = (uint8_t)i;

			return rs;
		}
	}
	dfs_debug(dfs, WLAN_DEBUG_DFS2, "No more radar states left.");

	return NULL;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
void dfs_radar_enable(struct wlan_dfs *dfs, int no_cac, uint32_t opmode)
{
	int is_ext_ch;
	int is_fastclk = 0;
	struct dfs_channel *exch, extchan;
	QDF_STATUS err = QDF_STATUS_E_FAILURE;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	is_ext_ch = WLAN_IS_CHAN_11N_HT40(dfs->dfs_curchan);
	lmac_dfs_disable(dfs->dfs_pdev_obj, no_cac);
	/*
	 * In all modes, if the primary is DFS then we have to
	 * enable radar detection. In HT80_80, we can have
	 * primary non-DFS 80MHz with extension 80MHz DFS.
	 */
	if ((WLAN_IS_CHAN_DFS(dfs->dfs_curchan) ||
	     ((WLAN_IS_CHAN_11AC_VHT160(dfs->dfs_curchan) ||
	       WLAN_IS_CHAN_11AC_VHT80_80(dfs->dfs_curchan)) &&
	      WLAN_IS_CHAN_DFS_CFREQ2(dfs->dfs_curchan))) ||
	    (dfs_is_precac_timer_running(dfs))) {
		struct dfs_state *rs_pri = NULL, *rs_ext = NULL;
		uint8_t index_pri, index_ext;

		dfs->dfs_proc_phyerr |= DFS_AR_EN;
		dfs->dfs_proc_phyerr |= DFS_RADAR_EN;
		dfs->dfs_proc_phyerr |= DFS_SECOND_SEGMENT_RADAR_EN;

		exch = &extchan;
		if (is_ext_ch) {
			err = dfs_mlme_get_extchan_for_freq
				(
				 dfs->dfs_pdev_obj,
				 &exch->dfs_ch_freq,
				 &exch->dfs_ch_flags,
				 &exch->dfs_ch_flagext,
				 &exch->dfs_ch_ieee,
				 &exch->dfs_ch_vhtop_ch_freq_seg1,
				 &exch->dfs_ch_vhtop_ch_freq_seg2,
				 &exch->dfs_ch_mhz_freq_seg1,
				 &exch->dfs_ch_mhz_freq_seg2);
		}
		dfs_reset_alldelaylines(dfs);

		rs_pri = dfs_getchanstate(dfs, &index_pri, 0);
		if (err == QDF_STATUS_SUCCESS)
			rs_ext = dfs_getchanstate(dfs, &index_ext, 1);

		if (rs_pri && ((err == QDF_STATUS_E_FAILURE) || (rs_ext))) {
			struct wlan_dfs_phyerr_param pe;

			qdf_mem_set(&pe, sizeof(pe), '\0');

			if (index_pri != dfs->dfs_curchan_radindex)
				dfs_reset_alldelaylines(dfs);

			dfs->dfs_curchan_radindex = (int16_t)index_pri;

			if (rs_ext)
				dfs->dfs_extchan_radindex = (int16_t)index_ext;

			dfs_phyerr_param_copy(&pe, &rs_pri->rs_param);
			dfs_debug(dfs, WLAN_DEBUG_DFS3,
				  "firpwr=%d, rssi=%d, height=%d, prssi=%d, inband=%d, relpwr=%d, relstep=%d, maxlen=%d",
				  pe.pe_firpwr,
				  pe.pe_rrssi, pe.pe_height,
				  pe.pe_prssi, pe.pe_inband,
				  pe.pe_relpwr, pe.pe_relstep,
				  pe.pe_maxlen);

			lmac_dfs_enable(dfs->dfs_pdev_obj, &is_fastclk,
					&pe, dfs->dfsdomain);
			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "Enabled radar detection on channel %d",
				  dfs->dfs_curchan->dfs_ch_freq);

			dfs->dur_multiplier = is_fastclk ?
				DFS_FAST_CLOCK_MULTIPLIER :
				DFS_NO_FAST_CLOCK_MULTIPLIER;

			dfs_debug(dfs, WLAN_DEBUG_DFS3,
				  "duration multiplier is %d",
				  dfs->dur_multiplier);
		} else
			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "No more radar states left");
	}
}
#endif

int dfs_set_thresholds(struct wlan_dfs *dfs, const uint32_t threshtype,
		const uint32_t value)
{
	int16_t chanindex;
	struct dfs_state *rs;
	struct wlan_dfs_phyerr_param pe;
	int is_fastclk = 0;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return 0;
	}

	chanindex = dfs->dfs_curchan_radindex;
	if ((chanindex < 0) || (chanindex >= DFS_NUM_RADAR_STATES)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			  "chanindex = %d, DFS_NUM_RADAR_STATES=%d",
			  chanindex,
			  DFS_NUM_RADAR_STATES);
		return 0;
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS,
			"threshtype=%d, value=%d", threshtype, value);

	wlan_dfs_phyerr_init_noval(&pe);

	rs = &(dfs->dfs_radar[chanindex]);
	switch (threshtype) {
	case DFS_PARAM_FIRPWR:
		rs->rs_param.pe_firpwr = (int32_t) value;
		pe.pe_firpwr = value;
		break;
	case DFS_PARAM_RRSSI:
		rs->rs_param.pe_rrssi = value;
		pe.pe_rrssi = value;
		break;
	case DFS_PARAM_HEIGHT:
		rs->rs_param.pe_height = value;
		pe.pe_height = value;
		break;
	case DFS_PARAM_PRSSI:
		rs->rs_param.pe_prssi = value;
		pe.pe_prssi = value;
		break;
	case DFS_PARAM_INBAND:
		rs->rs_param.pe_inband = value;
		pe.pe_inband = value;
		break;
		/* 5413 specific */
	case DFS_PARAM_RELPWR:
		rs->rs_param.pe_relpwr = value;
		pe.pe_relpwr = value;
		break;
	case DFS_PARAM_RELSTEP:
		rs->rs_param.pe_relstep = value;
		pe.pe_relstep = value;
		break;
	case DFS_PARAM_MAXLEN:
		rs->rs_param.pe_maxlen = value;
		pe.pe_maxlen = value;
		break;
	default:
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
				"unknown threshtype (%d)", threshtype);
		break;
	}


	/*
	 * The driver layer dfs_enable routine is tasked with translating
	 * values from the global format to the per-device (HAL, offload)
	 * format.
	 */
	lmac_dfs_enable(dfs->dfs_pdev_obj, &is_fastclk,
			&pe, dfs->dfsdomain);

	return 1;
}

int dfs_get_thresholds(struct wlan_dfs *dfs,
		struct wlan_dfs_phyerr_param *param)
{
	lmac_dfs_get_thresholds(dfs->dfs_pdev_obj, param);

	return 1;
}

uint16_t dfs_chan2freq(struct dfs_channel *chan)
{
	if (!chan)
		return 0;

	return chan == WLAN_CHAN_ANYC ? WLAN_CHAN_ANY : chan->dfs_ch_freq;
}
