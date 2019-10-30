/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: This file has support to inject synthetic radar pulses and
 * enabling/disabling it.
 */

#include "dfs.h"
#include "dfs_partial_offload_radar.h"

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(WLAN_DFS_SYNTHETIC_RADAR)
void dfs_allow_hw_pulses(struct wlan_dfs *dfs, bool allow_hw_pulses)
{
	dfs->dfs_allow_hw_pulses = allow_hw_pulses;
}

bool dfs_is_hw_pulses_allowed(struct wlan_dfs *dfs)
{
	return dfs->dfs_allow_hw_pulses;
}

QDF_STATUS
dfs_inject_synthetic_pulse_sequence(struct wlan_dfs *dfs, unsigned char *buf)
{
	unsigned short j;
	struct synthetic_seq *cur_seq;
	struct synthetic_pulse *cur_pulse;
	unsigned char *pulse_ptr;

	if (!dfs) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (dfs->dfs_allow_hw_pulses) {
		dfs_debug(dfs,
			  WLAN_DEBUG_DFS_ALWAYS,
			  "Hardware pulses are enabled");
		return QDF_STATUS_E_NOSUPPORT;
	}

	if (!buf) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "Seq is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dfs_false_radarfound_reset_vars(dfs);
	cur_seq = (struct synthetic_seq *)buf;
	cur_pulse = (struct synthetic_pulse *)(buf +
		     sizeof(cur_seq->num_pulses) +
		     sizeof(cur_seq->total_len_seq));
	pulse_ptr = (unsigned char *)cur_pulse;

	for (j = 0; j < cur_seq->num_pulses; j++) {
		unsigned char *fft;

		cur_pulse  =
			(struct synthetic_pulse *)pulse_ptr;
		/* fft to point to the beggining of the
		 * FFT buffer
		 */
		fft = (unsigned char *)(&cur_pulse->total_len_pulse + 1);
		dfs_process_phyerr(dfs,
				   fft,
				   cur_pulse->fft_datalen,
				   cur_pulse->r_rssi,
				   cur_pulse->r_ext_rssi,
				   cur_pulse->r_rs_tstamp,
				   cur_pulse->r_fulltsf);
		pulse_ptr += cur_pulse->total_len_pulse;
	}

	return QDF_STATUS_SUCCESS;
}
#endif

