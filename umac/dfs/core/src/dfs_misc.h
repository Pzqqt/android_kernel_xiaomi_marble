/*
 * Copyright (c) 2013, 2016-2020 The Linux Foundation.  All rights reserved.
 * Copyright (c) 2005-2006 Atheros Communications, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
e* copyright notice and this permission notice appear in all copies.
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
 * DOC: This file has header files of dfs_misc.c
 *
 */

#include "dfs.h"
#include "wlan_dfs_ucfg_api.h"
#include "wlan_lmac_if_def.h"

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
/**
 * dfs_set_postnol_freq() - DFS API to set postNOL frequency.
 * @dfs: Pointer to wlan_dfs object.
 * @postnol_freq: PostNOL frequency value configured by the user.
 */
void dfs_set_postnol_freq(struct wlan_dfs *dfs, qdf_freq_t postnol_freq);

/**
 * dfs_set_postnol_mode() - DFS API to set postNOL mode.
 * @dfs: Pointer to wlan_dfs object.
 * @postnol_mode: PostNOL frequency value configured by the user.
 */
void dfs_set_postnol_mode(struct wlan_dfs *dfs, uint8_t postnol_mode);

/**
 * dfs_set_postnol_cfreq2() - DFS API to set postNOL secondary center frequency.
 * @dfs: Pointer to wlan_dfs object.
 * @postnol_cfreq2: PostNOL secondary center frequency value configured by the
 * user.
 */
void dfs_set_postnol_cfreq2(struct wlan_dfs *dfs, qdf_freq_t postnol_cfreq2);

/**
 * dfs_get_postnol_freq() - DFS API to get postNOL frequency.
 * @dfs: Pointer to wlan_dfs object.
 * @postnol_freq: PostNOL frequency value configured by the user.
 */
void dfs_get_postnol_freq(struct wlan_dfs *dfs, qdf_freq_t *postnol_freq);

/**
 * dfs_get_postnol_mode() - DFS API to get postNOL mode.
 * @dfs: Pointer to wlan_dfs object.
 * @postnol_mode: PostNOL frequency value configured by the user.
 */
void dfs_get_postnol_mode(struct wlan_dfs *dfs, uint8_t *postnol_mode);

/**
 * dfs_get_postnol_cfreq2() - DFS API to get postNOL secondary center frequency.
 * @dfs: Pointer to wlan_dfs object.
 * @postnol_cfreq2: PostNOL secondary center frequency value configured by the
 * user.
 */
void dfs_get_postnol_cfreq2(struct wlan_dfs *dfs, qdf_freq_t *postnol_cfreq2);
#else
static inline void
dfs_set_postnol_freq(struct wlan_dfs *dfs, qdf_freq_t postnol_freq)
{
}

static inline void
dfs_set_postnol_mode(struct wlan_dfs *dfs, uint8_t postnol_mode)
{
}

static inline void
dfs_set_postnol_cfreq2(struct wlan_dfs *dfs, qdf_freq_t postnol_cfreq2)
{
}

static inline void
dfs_get_postnol_freq(struct wlan_dfs *dfs, qdf_freq_t *postnol_freq)
{
	*postnol_freq = 0;
}

static inline void
dfs_get_postnol_mode(struct wlan_dfs *dfs, uint8_t *postnol_mode)
{
	*postnol_mode = CH_WIDTH_INVALID;
}

static inline void
dfs_get_postnol_cfreq2(struct wlan_dfs *dfs, qdf_freq_t *postnol_cfreq2)
{
	*postnol_cfreq2 = 0;
}
#endif /* QCA_SUPPORT_DFS_CHAN_POSTNOL */
