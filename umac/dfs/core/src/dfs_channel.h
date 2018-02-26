/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2008 Atheros Communications, Inc.
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
 * DOC: This file has channel related information.
 */

#ifndef _DFS_CHANNEL_H_
#define _DFS_CHANNEL_H_

/* Channel attributes */

/* OFDM channel */
#define WLAN_CHAN_OFDM             0x0000000000000040

/* 2 GHz spectrum channel. */
#define WLAN_CHAN_2GHZ             0x0000000000000080

/* 5 GHz spectrum channel */
#define WLAN_CHAN_5GHZ             0x0000000000000100

/* Radar found on channel */
#define WLAN_CHAN_DFS_RADAR        0x0000000000001000

/* HT 20 channel */
#define WLAN_CHAN_HT20             0x0000000000010000

/* HT 40 with extension channel above */
#define WLAN_CHAN_HT40PLUS         0x0000000000020000

/* HT 40 with extension channel below */
#define WLAN_CHAN_HT40MINUS        0x0000000000040000

/* VHT 20 channel */
#define WLAN_CHAN_VHT20            0x0000000000100000

/* VHT 40 with extension channel above */
#define WLAN_CHAN_VHT40PLUS        0x0000000000200000

/* VHT 40 with extension channel below */
#define WLAN_CHAN_VHT40MINUS       0x0000000000400000

/* VHT 80 channel */
#define WLAN_CHAN_VHT80            0x0000000000800000

/* VHT 160 channel */
#define WLAN_CHAN_VHT160           0x0000000004000000

/* VHT 80_80 channel */
#define WLAN_CHAN_VHT80_80         0x0000000008000000

/* HE 20 channel */
#define WLAN_CHAN_HE20             0x0000000010000000

/* HE 40 with extension channel above */
#define WLAN_CHAN_HE40PLUS         0x0000000020000000

/* HE 40 with extension channel below */
#define WLAN_CHAN_HE40MINUS        0x0000000040000000

/* HE 80 channel */
#define WLAN_CHAN_HE80             0x0000000200000000

/* HE 160 channel */
#define WLAN_CHAN_HE160            0x0000000400000000

/* HE 80_80 channel */
#define WLAN_CHAN_HE80_80          0x0000000800000000

/* flagext */
#define WLAN_CHAN_DFS_RADAR_FOUND    0x01

/* DFS required on channel */
#define WLAN_CHAN_DFS              0x0002

/* DFS required on channel for 2nd band of 80+80*/
#define WLAN_CHAN_DFS_CFREQ2       0x0004

/* if channel has been checked for DFS */
#define WLAN_CHAN_DFS_CLEAR        0x0008

/* DFS radar history for slave device(STA mode) */
#define WLAN_CHAN_HISTORY_RADAR    0x0100

/* DFS CAC valid for  slave device(STA mode) */
#define WLAN_CHAN_CAC_VALID        0x0200

#define WLAN_IS_CHAN_2GHZ(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_2GHZ) != 0)

#define WLAN_IS_CHAN_5GHZ(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_5GHZ) != 0)

#define WLAN_IS_CHAN_11N_HT40(_c) \
	(((_c)->dfs_ch_flags & (WLAN_CHAN_HT40PLUS | \
					WLAN_CHAN_HT40MINUS)) != 0)

#define WLAN_IS_CHAN_11N_HT40PLUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_HT40PLUS) != 0)

#define WLAN_IS_CHAN_11N_HT40MINUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_HT40MINUS) != 0)

#define WLAN_CHAN_A \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_OFDM)

#define WLAN_IS_CHAN_A(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_A) == WLAN_CHAN_A)

#define WLAN_CHAN_11NA_HT20 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HT20)

#define WLAN_CHAN_11NA_HT40PLUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HT40PLUS)

#define WLAN_CHAN_11NA_HT40MINUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HT40MINUS)

#define WLAN_IS_CHAN_11NA_HT20(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11NA_HT20) == \
	 WLAN_CHAN_11NA_HT20)

#define WLAN_IS_CHAN_11NA_HT40PLUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11NA_HT40PLUS) == \
	WLAN_CHAN_11NA_HT40PLUS)

#define WLAN_IS_CHAN_11NA_HT40MINUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11NA_HT40MINUS) == \
	 WLAN_CHAN_11NA_HT40MINUS)

#define WLAN_CHAN_11AC_VHT20 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_VHT20)

#define WLAN_CHAN_11AC_VHT40PLUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_VHT40PLUS)

#define WLAN_CHAN_11AC_VHT40MINUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_VHT40MINUS)

#define WLAN_CHAN_11AC_VHT80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_VHT80)

#define WLAN_CHAN_11AC_VHT160 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_VHT160)

#define WLAN_CHAN_11AC_VHT80_80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_VHT80_80)

#define WLAN_IS_CHAN_11AC_VHT20(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AC_VHT20) == \
	 WLAN_CHAN_11AC_VHT20)

#define WLAN_IS_CHAN_11AC_VHT40(_c) \
	(((_c)->dfs_ch_flags & (WLAN_CHAN_VHT40PLUS | \
			    WLAN_CHAN_VHT40MINUS)) != 0)

#define WLAN_IS_CHAN_11AC_VHT40PLUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AC_VHT40PLUS) == \
	WLAN_CHAN_11AC_VHT40PLUS)

#define WLAN_IS_CHAN_11AC_VHT40MINUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AC_VHT40MINUS) == \
	WLAN_CHAN_11AC_VHT40MINUS)

#define WLAN_IS_CHAN_11AC_VHT80(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AC_VHT80) == \
	 WLAN_CHAN_11AC_VHT80)

#define WLAN_IS_CHAN_11AC_VHT160(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AC_VHT160) == \
	 WLAN_CHAN_11AC_VHT160)

#define WLAN_IS_CHAN_11AC_VHT80_80(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AC_VHT80_80) == \
	WLAN_CHAN_11AC_VHT80_80)

#define WLAN_CHAN_11AXA_HE20 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE20)

#define WLAN_CHAN_11AXA_HE40PLUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE40PLUS)

#define WLAN_CHAN_11AXA_HE40MINUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE40MINUS)

#define WLAN_CHAN_11AXA_HE80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE80)

#define WLAN_CHAN_11AXA_HE160 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE160)

#define WLAN_CHAN_11AXA_HE80_80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE80_80)

#define WLAN_IS_CHAN_11AXA_HE20(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE20) == \
	 WLAN_CHAN_11AXA_HE20)

#define WLAN_IS_CHAN_11AXA_HE40PLUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE40PLUS) == \
	 WLAN_CHAN_11AXA_HE40PLUS)

#define WLAN_IS_CHAN_11AXA_HE40MINUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE40MINUS) == \
	 WLAN_CHAN_11AXA_HE40MINUS)

#define WLAN_IS_CHAN_11AXA_HE80(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE80) == \
	 WLAN_CHAN_11AXA_HE80)

#define WLAN_IS_CHAN_11AXA_HE160(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE160) == \
	 WLAN_CHAN_11AXA_HE160)

#define WLAN_IS_CHAN_11AXA_HE80_80(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE80_80) == \
	 WLAN_CHAN_11AXA_HE80_80)

#define WLAN_IS_CHAN_DFS(_c) \
	(((_c)->dfs_ch_flagext & \
	(WLAN_CHAN_DFS | WLAN_CHAN_DFS_CLEAR)) == WLAN_CHAN_DFS)

#define WLAN_IS_CHAN_DFS_CFREQ2(_c) \
	(((_c)->dfs_ch_flagext & \
	(WLAN_CHAN_DFS_CFREQ2|WLAN_CHAN_DFS_CLEAR)) == \
	WLAN_CHAN_DFS_CFREQ2)

#define WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(_c) \
	(WLAN_IS_CHAN_DFS(_c) || \
	 ((WLAN_IS_CHAN_11AC_VHT160(_c) || \
	 WLAN_IS_CHAN_11AC_VHT80_80(_c) || \
	 WLAN_IS_CHAN_11AXA_HE160(_c) || \
	 WLAN_IS_CHAN_11AXA_HE80_80(_c)) \
	&& WLAN_IS_CHAN_DFS_CFREQ2(_c)))

#define WLAN_IS_CHAN_RADAR(_c)    \
	(((_c)->dfs_ch_flags & WLAN_CHAN_DFS_RADAR) == \
	 WLAN_CHAN_DFS_RADAR)

#define WLAN_IS_CHAN_HISTORY_RADAR(_c)    \
	(((_c)->dfs_ch_flagext & WLAN_CHAN_HISTORY_RADAR) == \
	WLAN_CHAN_HISTORY_RADAR)

#define WLAN_CHAN_CLR_HISTORY_RADAR(_c)    \
	((_c)->dfs_ch_flagext &= ~WLAN_CHAN_HISTORY_RADAR)

#define WLAN_CHAN_ANY      (-1)    /* token for ``any channel'' */

#define WLAN_CHAN_ANYC \
	((struct dfs_channel *) WLAN_CHAN_ANY)

#define WLAN_IS_CHAN_MODE_20(_c)      \
	(WLAN_IS_CHAN_A(_c)        ||    \
	 WLAN_IS_CHAN_11NA_HT20(_c)  ||  \
	 WLAN_IS_CHAN_11AC_VHT20(_c) ||  \
	 WLAN_IS_CHAN_11AXA_HE20(_c))

#define WLAN_IS_CHAN_MODE_40(_c)          \
	(WLAN_IS_CHAN_11AC_VHT40PLUS(_c)  || \
	 WLAN_IS_CHAN_11AC_VHT40MINUS(_c) || \
	 WLAN_IS_CHAN_11NA_HT40PLUS(_c)   || \
	 WLAN_IS_CHAN_11NA_HT40MINUS(_c)  || \
	 WLAN_IS_CHAN_11AXA_HE40PLUS(_c)  || \
	 WLAN_IS_CHAN_11AXA_HE40MINUS(_c))

#define WLAN_IS_CHAN_MODE_80(_c)          \
	(WLAN_IS_CHAN_11AC_VHT80(_c)      || \
	 WLAN_IS_CHAN_11AXA_HE80(_c))

#define WLAN_IS_CHAN_MODE_160(_c)         \
	(WLAN_IS_CHAN_11AC_VHT160(_c)     || \
	 WLAN_IS_CHAN_11AXA_HE160(_c))

#define WLAN_IS_CHAN_MODE_80_80(_c)       \
	(WLAN_IS_CHAN_11AC_VHT80_80(_c)   || \
	 WLAN_IS_CHAN_11AXA_HE80_80(_c))

#endif /* _DFS_CHANNEL_H_ */
