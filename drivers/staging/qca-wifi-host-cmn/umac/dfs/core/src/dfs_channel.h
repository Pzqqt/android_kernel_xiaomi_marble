/*
 * Copyright (c) 2016-2018,2020-2021 The Linux Foundation. All rights reserved.
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
#ifdef WLAN_COMP_CHAN_MODE

#define WLAN_CHAN_20MHZ            0x0000000000000100

#define WLAN_CHAN_40PLUS           0x0000000000000200

#define WLAN_CHAN_40MINUS          0x0000000000000300

#define WLAN_CHAN_80MHZ            0x0000000000000400

#define WLAN_CHAN_160MHZ           0x0000000000000500

#define WLAN_CHAN_80_80MHZ         0x0000000000000600

#define WLAN_CHAN_320MHZ           0x0000000000000700

#define WLAN_CHAN_HTCAP            0x0000000000001000

#define WLAN_CHAN_VHTCAP           0x0000000000002000

#define WLAN_CHAN_HECAP            0x0000000000003000

#define WLAN_CHAN_EHTCAP           0x0000000000004000

/* 2 GHz spectrum channel. */
#define WLAN_CHAN_2GHZ             0x0000000000000010

/* 5 GHz spectrum channel */
#define WLAN_CHAN_5GHZ             0x0000000000000020

/* 6 GHz spectrum channel */
#define WLAN_CHAN_6GHZ             0x0000000000000030

/* HT 20 channel */
#define WLAN_CHAN_HT20             (WLAN_CHAN_HTCAP | WLAN_CHAN_20MHZ)

/* HT 40 with extension channel above */
#define WLAN_CHAN_HT40PLUS         (WLAN_CHAN_HTCAP | WLAN_CHAN_40PLUS)

/* HT 40 with extension channel below */
#define WLAN_CHAN_HT40MINUS        (WLAN_CHAN_HTCAP | WLAN_CHAN_40MINUS)

/* VHT 20 channel */
#define WLAN_CHAN_VHT20            (WLAN_CHAN_VHTCAP | WLAN_CHAN_20MHZ)

/* VHT 40 with extension channel above */
#define WLAN_CHAN_VHT40PLUS        (WLAN_CHAN_VHTCAP | WLAN_CHAN_40PLUS)

/* VHT 40 with extension channel below */
#define WLAN_CHAN_VHT40MINUS       (WLAN_CHAN_VHTCAP | WLAN_CHAN_40MINUS)

/* VHT 80 channel */
#define WLAN_CHAN_VHT80            (WLAN_CHAN_VHTCAP | WLAN_CHAN_80MHZ)

/* VHT 160 channel */
#define WLAN_CHAN_VHT160           (WLAN_CHAN_VHTCAP | WLAN_CHAN_160MHZ)

/* VHT 80_80 channel */
#define WLAN_CHAN_VHT80_80         (WLAN_CHAN_VHTCAP | WLAN_CHAN_80_80MHZ)

/* HE 20 channel */
#define WLAN_CHAN_HE20             (WLAN_CHAN_HECAP | WLAN_CHAN_20MHZ)

/* HE 40 with extension channel above */
#define WLAN_CHAN_HE40PLUS         (WLAN_CHAN_HECAP | WLAN_CHAN_40PLUS)

/* HE 40 with extension channel below */
#define WLAN_CHAN_HE40MINUS        (WLAN_CHAN_HECAP | WLAN_CHAN_40MINUS)

/* HE 80 channel */
#define WLAN_CHAN_HE80             (WLAN_CHAN_HECAP | WLAN_CHAN_80MHZ)

/* HE 160 channel */
#define WLAN_CHAN_HE160            (WLAN_CHAN_HECAP | WLAN_CHAN_160MHZ)

/* HE 80_80 channel */
#define WLAN_CHAN_HE80_80          (WLAN_CHAN_HECAP | WLAN_CHAN_80_80MHZ)

/* EHT 20 channel */
#define WLAN_CHAN_EHT20            (WLAN_CHAN_EHTCAP | WLAN_CHAN_20MHZ)

/* EHT 40 with extension channel above */
#define WLAN_CHAN_EHT40PLUS        (WLAN_CHAN_EHTCAP | WLAN_CHAN_40PLUS)

/* EHT 40 with extension channel below */
#define WLAN_CHAN_EHT40MINUS       (WLAN_CHAN_EHTCAP | WLAN_CHAN_40MINUS)

/* EHT 80 channel */
#define WLAN_CHAN_EHT80            (WLAN_CHAN_EHTCAP | WLAN_CHAN_80MHZ)

/* EHT 160 channel */
#define WLAN_CHAN_EHT160           (WLAN_CHAN_EHTCAP | WLAN_CHAN_160MHZ)

/* EHT 320 channel */
#define WLAN_CHAN_EHT320           (WLAN_CHAN_EHTCAP | WLAN_CHAN_320MHZ)

/* OFDM channel */
#define WLAN_CHAN_OFDM             0x0000000000400000

/* Turbo Channel */
#define WLAN_CHAN_TURBO            0x0000000020000000

/* Radar found on channel */
#define WLAN_CHAN_DFS_RADAR        0x0000000002000000

#define WLAN_CHAN_BAND_MASK        0x00000000000000F0

#define WLAN_CHAN_BW_MASK          0x000000000000FF00

#define WLAN_IS_CHAN_2GHZ(_c)        WLAN_IS_FLAG_2GHZ((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_5GHZ(_c)        WLAN_IS_FLAG_5GHZ((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_6GHZ(_c)        WLAN_IS_FLAG_6GHZ((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HT20(_c)        WLAN_IS_FLAG_HT20((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HT40PLUS(_c)    WLAN_IS_FLAG_HT40PLUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HT40MINUS(_c)   WLAN_IS_FLAG_HT40MINUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HT40(_c)        WLAN_IS_FLAG_HT40((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT20(_c)       WLAN_IS_FLAG_VHT20((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT40PLUS(_c)   WLAN_IS_FLAG_VHT40PLUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT40MINUS(_c)  WLAN_IS_FLAG_VHT40MINUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT40(_c)       WLAN_IS_FLAG_VHT40((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT80(_c)       WLAN_IS_FLAG_VHT80((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT160(_c)      WLAN_IS_FLAG_VHT160((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT80_80(_c)    WLAN_IS_FLAG_VHT80_80((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_VHT40(_c)       WLAN_IS_FLAG_VHT40((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE20(_c)        WLAN_IS_FLAG_HE20((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE40PLUS(_c)    WLAN_IS_FLAG_HE40PLUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE40MINUS(_c)   WLAN_IS_FLAG_HE40MINUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE40(_c)        WLAN_IS_FLAG_HE40((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE80(_c)        WLAN_IS_FLAG_HE80((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE160(_c)       WLAN_IS_FLAG_HE160((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_HE80_80(_c)     WLAN_IS_FLAG_HE80_80((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT20(_c)       WLAN_IS_FLAG_EHT20((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT40PLUS(_c)   WLAN_IS_FLAG_EHT40PLUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT40MINUS(_c)  WLAN_IS_FLAG_EHT40MINUS((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT40(_c)       WLAN_IS_FLAG_EHT40((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT80(_c)       WLAN_IS_FLAG_EHT80((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT160(_c)      WLAN_IS_FLAG_EHT160((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_EHT320(_c)      WLAN_IS_FLAG_EHT320((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_OFDM(_c)        WLAN_IS_FLAG_OFDM((_c)->dfs_ch_flags)

#define WLAN_IS_CHAN_TURBO(_c)       WLAN_IS_FLAG_TURBO((_c)->dfs_ch_flags)

#define WLAN_IS_FLAG_2GHZ(_flag) \
	(((_flag) & WLAN_CHAN_BAND_MASK) == WLAN_CHAN_2GHZ)

#define WLAN_IS_FLAG_5GHZ(_flag) \
	(((_flag) & WLAN_CHAN_BAND_MASK) == WLAN_CHAN_5GHZ)

#define WLAN_IS_FLAG_6GHZ(_flag) \
	(((_flag) & WLAN_CHAN_BAND_MASK) == WLAN_CHAN_6GHZ)

#define WLAN_IS_FLAG_HT20(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HT20)

#define WLAN_IS_FLAG_HT40PLUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HT40PLUS)

#define WLAN_IS_FLAG_HT40MINUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HT40MINUS)

#define WLAN_IS_FLAG_VHT20(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_VHT20)

#define WLAN_IS_FLAG_VHT40PLUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_VHT40PLUS)

#define WLAN_IS_FLAG_VHT40MINUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_VHT40MINUS)

#define WLAN_IS_FLAG_VHT80(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_VHT80)

#define WLAN_IS_FLAG_VHT160(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_VHT160)

#define WLAN_IS_FLAG_VHT80_80(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_VHT80_80)

#define WLAN_IS_FLAG_HE20(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HE20)

#define WLAN_IS_FLAG_HE40PLUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HE40PLUS)

#define WLAN_IS_FLAG_HE40MINUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HE40MINUS)

#define WLAN_IS_FLAG_HE80(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HE80)

#define WLAN_IS_FLAG_HE160(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HE160)

#define WLAN_IS_FLAG_HE80_80(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_HE80_80)

#define WLAN_IS_FLAG_EHT20(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_EHT20)

#define WLAN_IS_FLAG_EHT40PLUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_EHT40PLUS)

#define WLAN_IS_FLAG_EHT40MINUS(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_EHT40MINUS)

#define WLAN_IS_FLAG_EHT40(_flag) \
	((WLAN_IS_FLAG_EHT40PLUS(flag)) || (WLAN_IS_FLAG_EHT40MINUS(flag)))

#define WLAN_IS_FLAG_EHT80(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_EHT80)

#define WLAN_IS_FLAG_EHT160(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_EHT160)

#define WLAN_IS_FLAG_EHT320(_flag) \
	(((_flag) & WLAN_CHAN_BW_MASK) == WLAN_CHAN_EHT320)

#define WLAN_IS_FLAG_OFDM(_flag) \
	((_flag) & WLAN_CHAN_OFDM)

#define WLAN_IS_FLAG_TURBO(_flag) \
	((_flag) & WLAN_CHAN_TURBO)

#define WLAN_IS_CHAN_11N_HT40(_c) \
	(WLAN_IS_CHAN_HT40PLUS(_c) || WLAN_IS_CHAN_HT40MINUS(_c))

#define WLAN_IS_CHAN_11N_HT40PLUS(_c) WLAN_IS_CHAN_HT40PLUS(_c)

#define WLAN_IS_CHAN_11N_HT40MINUS(_c) WLAN_IS_CHAN_HT40MINUS(_c)

#define WLAN_IS_CHAN_A(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_OFDM(_c))

#define WLAN_IS_CHAN_11NA_HT20(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HT20(_c))

#define WLAN_IS_CHAN_11NA_HT40PLUS(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HT40PLUS(_c))

#define WLAN_IS_CHAN_11NA_HT40MINUS(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HT40MINUS(_c))

#define WLAN_IS_CHAN_11AC_VHT20(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_VHT20(_c))

#define WLAN_IS_CHAN_11AC_VHT40(_c) \
	(WLAN_IS_CHAN_11AC_VHT40MINUS(_c) || WLAN_IS_CHAN_11AC_VHT40PLUS(_c))

#define WLAN_IS_CHAN_11AC_VHT40PLUS(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_VHT40PLUS(_c))

#define WLAN_IS_CHAN_11AC_VHT40MINUS(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_VHT40MINUS(_c))

#define WLAN_IS_CHAN_11AC_VHT80(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_VHT80(_c))

#define WLAN_IS_CHAN_11AC_VHT160(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_VHT160(_c))

#define WLAN_IS_CHAN_11AC_VHT80_80(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_VHT80_80(_c))

#define WLAN_IS_CHAN_11AXA_HE20_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HE20(_c))

#define WLAN_IS_CHAN_11AXA_HE40_5G(_c) \
	(WLAN_IS_CHAN_11AXA_HE40MINUS_5G(_c) ||  \
	 WLAN_IS_CHAN_11AXA_HE40PLUS_5G(_c))

#define WLAN_IS_CHAN_11AXA_HE40PLUS_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HE40PLUS(_c))

#define WLAN_IS_CHAN_11AXA_HE40MINUS_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HE40MINUS(_c))

#define WLAN_IS_CHAN_11AXA_HE80_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HE80(_c))

#define WLAN_IS_CHAN_11AXA_HE160_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HE160(_c))

#define WLAN_IS_CHAN_11AXA_HE80_80_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_HE80_80(_c))

#define WLAN_IS_CHAN_11BE_EHT20_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_EHT20(_c))

#define WLAN_IS_CHAN_11BE_EHT40PLUS_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_EHT40PLUS(_c))

#define WLAN_IS_CHAN_11BE_EHT40MINUS_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_EHT40MINUS(_c))

#define WLAN_IS_CHAN_11BE_EHT40_5G(_c) \
	(WLAN_IS_CHAN_11BE_EHT40MINUS_5G(_c) ||  \
	 WLAN_IS_CHAN_11BE_EHT40PLUS_5G(_c))

#define WLAN_IS_CHAN_11BE_EHT80_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_EHT80(_c))

#define WLAN_IS_CHAN_11BE_EHT160_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_EHT160(_c))

#define WLAN_IS_CHAN_11BE_EHT320_5G(_c) \
	(WLAN_IS_CHAN_5GHZ(_c) && WLAN_IS_CHAN_EHT320(_c))

#define WLAN_IS_CHAN_11AXA_HE20_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_HE20(_c))

#define WLAN_IS_CHAN_11AXA_HE40_6G(_c) \
	(WLAN_IS_CHAN_11AXA_HE40MINUS_6G(_c) || \
	 WLAN_IS_CHAN_11AXA_HE40PLUS_6G(_c))

#define WLAN_IS_CHAN_11AXA_HE40PLUS_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_HE40PLUS(_c))

#define WLAN_IS_CHAN_11AXA_HE40MINUS_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_HE40MINUS(_c))

#define WLAN_IS_CHAN_11AXA_HE80_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_HE80(_c))

#define WLAN_IS_CHAN_11AXA_HE160_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_HE160(_c))

#define WLAN_IS_CHAN_11AXA_HE80_80_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_HE80_80(_c))

#define WLAN_IS_CHAN_11BE_EHT20_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_EHT20(_c))

#define WLAN_IS_CHAN_11BE_EHT40PLUS_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_EHT40PLUS(_c))

#define WLAN_IS_CHAN_11BE_EHT40MINUS_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_EHT40MINUS(_c))

#define WLAN_IS_CHAN_11BE_EHT40_6G(_c) \
	(WLAN_IS_CHAN_11BE_EHT40MINUS_6G(_c) ||  \
	 WLAN_IS_CHAN_11BE_EHT40PLUS_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT80_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_EHT80(_c))

#define WLAN_IS_CHAN_11BE_EHT160_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_EHT160(_c))

#define WLAN_IS_CHAN_11BE_EHT320_6G(_c) \
	(WLAN_IS_CHAN_6GHZ(_c) && WLAN_IS_CHAN_EHT320(_c))

#define WLAN_IS_CHAN_11AXA_HE20(_c) \
	(WLAN_IS_CHAN_11AXA_HE20_5G(_c) || WLAN_IS_CHAN_11AXA_HE20_6G(_c))

#define WLAN_IS_CHAN_11AXA_HE40PLUS(_c) \
	(WLAN_IS_CHAN_11AXA_HE40PLUS_5G(_c) || \
	 WLAN_IS_CHAN_11AXA_HE40PLUS_6G(_c))

#define WLAN_IS_CHAN_11AXA_HE40MINUS(_c) \
	(WLAN_IS_CHAN_11AXA_HE40MINUS_5G(_c) || \
	 WLAN_IS_CHAN_11AXA_HE40MINUS_6G(_c))

#define WLAN_IS_CHAN_11AXA_HE80(_c) \
	(WLAN_IS_CHAN_11AXA_HE80_5G(_c) || WLAN_IS_CHAN_11AXA_HE80_6G(_c))

#define WLAN_IS_CHAN_11AXA_HE160(_c) \
	(WLAN_IS_CHAN_11AXA_HE160_5G(_c) || WLAN_IS_CHAN_11AXA_HE160_6G(_c))

#define WLAN_IS_CHAN_11AXA_HE80_80(_c) \
	(WLAN_IS_CHAN_11AXA_HE80_80_5G(_c) || WLAN_IS_CHAN_11AXA_HE80_80_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT20(_c) \
	(WLAN_IS_CHAN_11BE_EHT20_5G(_c) || WLAN_IS_CHAN_11BE_EHT20_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT40PLUS(_c) \
	(WLAN_IS_CHAN_11BE_EHT40PLUS_5G(_c) || \
	 WLAN_IS_CHAN_11BE_EHT40PLUS_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT40MINUS(_c) \
	(WLAN_IS_CHAN_11BE_EHT40MINUS_5G(_c) || \
	 WLAN_IS_CHAN_11BE_EHT40MINUS_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT40(_c) \
	(WLAN_IS_CHAN_11BE_EHT40PLUS(_c) || WLAN_IS_CHAN_11BE_EHT40MINUS(_c))

#define WLAN_IS_CHAN_11BE_EHT80(_c) \
	(WLAN_IS_CHAN_11BE_EHT80_5G(_c) || WLAN_IS_CHAN_11BE_EHT80_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT160(_c) \
	(WLAN_IS_CHAN_11BE_EHT160_5G(_c) || WLAN_IS_CHAN_11BE_EHT160_6G(_c))

#define WLAN_IS_CHAN_11BE_EHT320(_c) \
	(WLAN_IS_CHAN_11BE_EHT320_5G(_c) || WLAN_IS_CHAN_11BE_EHT320_6G(_c))
#else

/* OFDM channel */
#define WLAN_CHAN_OFDM             0x0000000000000040

/* 2 GHz spectrum channel. */
#define WLAN_CHAN_2GHZ             0x0000000000000080

/* 5 GHz spectrum channel */
#define WLAN_CHAN_5GHZ             0x0000000000000100

/* 6 GHz spectrum channel */
#define WLAN_CHAN_6GHZ             0x0000001000000000

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

/* EHT 20 channel */
#define WLAN_CHAN_EHT20            0x0000001000000000

/* EHT 40 with extension channel above */
#define WLAN_CHAN_EHT40PLUS        0x0000002000000000

/* EHT 40 with extension channel below */
#define WLAN_CHAN_EHT40MINUS       0x0000004000000000

/* EHT 80 channel */
#define WLAN_CHAN_EHT80            0x0000008000000000

/* HE 160 channel */
#define WLAN_CHAN_EHT160           0x0000010000000000

/* HE 320 channel */
#define WLAN_CHAN_EHT320           0x0000020000000000

/* Turbo Channel */
#define WLAN_CHAN_TURBO            0x0000000000000010

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

#define WLAN_IS_CHAN_VHT40PLUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_VHT40PLUS) != 0)

#define WLAN_IS_CHAN_VHT40MINUS(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_VHT40MINUS) != 0)

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

#define WLAN_CHAN_11AXA_HE20_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_HE20)

#define WLAN_CHAN_11AXA_HE40PLUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE40PLUS)

#define WLAN_CHAN_11AXA_HE40PLUS_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_HE40PLUS)

#define WLAN_CHAN_11AXA_HE40MINUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE40MINUS)

#define WLAN_CHAN_11AXA_HE40MINUS_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_HE40MINUS)

#define WLAN_CHAN_11AXA_HE80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE80)

#define WLAN_CHAN_11AXA_HE80_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_HE80)

#define WLAN_CHAN_11AXA_HE160 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE160)

#define WLAN_CHAN_11AXA_HE160_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_HE160)

#define WLAN_CHAN_11AXA_HE80_80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_HE80_80)

#define WLAN_CHAN_11AXA_HE80_80_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_HE80_80)

#define WLAN_CHAN_11BE_EHT20 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_EHT20)

#define WLAN_CHAN_11BE_EHT20_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_EHT20)

#define WLAN_CHAN_11BE_EHT40PLUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_EHT40PLUS)

#define WLAN_CHAN_11BE_EHT40PLUS_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_EHT40PLUS)

#define WLAN_CHAN_11BE_EHT40MINUS \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_EHT40MINUS)

#define WLAN_CHAN_11BE_EHT40MINUS_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_EHT40MINUS)

#define WLAN_CHAN_11BE_EHT80 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_EHT80)

#define WLAN_CHAN_11BE_EHT80_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_EHT80)

#define WLAN_CHAN_11BE_EHT160 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_EHT160)

#define WLAN_CHAN_11BE_EHT160_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_EHT160)

#define WLAN_CHAN_11BE_EHT320 \
	(WLAN_CHAN_5GHZ | WLAN_CHAN_EHT320)

#define WLAN_CHAN_11BE_EHT320_6G \
	(WLAN_CHAN_6GHZ | WLAN_CHAN_EHT320)

#define WLAN_CHAN_108G \
	(WLAN_CHAN_2GHZ | WLAN_CHAN_OFDM | WLAN_CHAN_TURBO)

#define WLAN_IS_CHAN_11AXA_HE20(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE20) == \
	 WLAN_CHAN_11AXA_HE20) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE20_6G) == \
	 WLAN_CHAN_11AXA_HE20_6G))

#define WLAN_IS_CHAN_11AXA_HE40PLUS(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE40PLUS) == \
	 WLAN_CHAN_11AXA_HE40PLUS) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE40PLUS_6G) == \
	  WLAN_CHAN_11AXA_HE40PLUS_6G))

#define WLAN_IS_CHAN_11AXA_HE40MINUS(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE40MINUS) == \
	 WLAN_CHAN_11AXA_HE40MINUS) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE40MINUS_6G) == \
	 WLAN_CHAN_11AXA_HE40MINUS_6G))

#define WLAN_IS_CHAN_11AXA_HE80(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE80) == \
	 WLAN_CHAN_11AXA_HE80) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE80_6G) == \
	  WLAN_CHAN_11AXA_HE80_6G))

#define WLAN_IS_CHAN_11AXA_HE160(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE160) == \
	 WLAN_CHAN_11AXA_HE160) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE160_6G) == \
	  WLAN_CHAN_11AXA_HE160_6G))

#define WLAN_IS_CHAN_11AXA_HE80_80(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE80_80) == \
	 WLAN_CHAN_11AXA_HE80_80) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11AXA_HE80_80_6G) == \
	  WLAN_CHAN_11AXA_HE80_80_6G))

#define WLAN_IS_CHAN_11BE_EHT20(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT20) == \
	 WLAN_CHAN_11BE_EHT20) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT20_6G) == \
	 WLAN_CHAN_11BE_EHT20_6G))

#define WLAN_IS_CHAN_11BE_EHT40PLUS(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT40PLUS) == \
	 WLAN_CHAN_11BE_EHT40PLUS) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT40PLUS_6G) == \
	  WLAN_CHAN_11BE_EHT40PLUS_6G))

#define WLAN_IS_CHAN_11BE_EHT40MINUS(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT40MINUS) == \
	 WLAN_CHAN_11BE_EHT40MINUS) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT40MINUS_6G) == \
	 WLAN_CHAN_11BE_EHT40MINUS_6G))

#define WLAN_IS_CHAN_11BE_EHT80(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT80) == \
	 WLAN_CHAN_11BE_EHT80) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT80_6G) == \
	  WLAN_CHAN_11BE_EHT80_6G))

#define WLAN_IS_CHAN_11BE_EHT160(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT160) == \
	 WLAN_CHAN_11BE_EHT160) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT160_6G) == \
	  WLAN_CHAN_11BE_EHT160_6G))

#define WLAN_IS_CHAN_11BE_EHT320(_c) \
	((((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT320) == \
	 WLAN_CHAN_11BE_EHT320) || \
	 (((_c)->dfs_ch_flags & WLAN_CHAN_11BE_EHT320_6G) == \
	  WLAN_CHAN_11BE_EHT320_6G))

#define WLAN_IS_CHAN_TURBO(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_TURBO) != 0)

#endif /* WLAN_COMP_CHAN_MODE */

#define WLAN_CHAN_108G \
	(WLAN_CHAN_2GHZ | WLAN_CHAN_OFDM | WLAN_CHAN_TURBO)

#define WLAN_IS_CHAN_108G(_c) \
	(((_c)->dfs_ch_flags & WLAN_CHAN_108G) == WLAN_CHAN_108G)

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
	 WLAN_IS_CHAN_11AXA_HE80_80(_c) || \
	 WLAN_IS_CHAN_11BE_EHT160(_c) || \
	 WLAN_IS_CHAN_11BE_EHT320(_c)) \
	&& WLAN_IS_CHAN_DFS_CFREQ2(_c)))

#ifdef CONFIG_HOST_FIND_CHAN
#define WLAN_IS_CHAN_RADAR wlan_is_chan_radar
#define WLAN_IS_CHAN_HISTORY_RADAR wlan_is_chan_history_radar
#else
#define WLAN_IS_CHAN_RADAR(_dfs, _c)    \
	(((_c)->dfs_ch_flags & WLAN_CHAN_DFS_RADAR) == \
	 WLAN_CHAN_DFS_RADAR)

#define WLAN_IS_CHAN_HISTORY_RADAR(_dfs, _c)    \
	(((_c)->dfs_ch_flagext & WLAN_CHAN_HISTORY_RADAR) == \
	WLAN_CHAN_HISTORY_RADAR)

#endif

#define WLAN_CHAN_CLR_HISTORY_RADAR(_c)    \
	((_c)->dfs_ch_flagext &= ~WLAN_CHAN_HISTORY_RADAR)

#define WLAN_CHAN_ANY      (-1)    /* token for ``any channel'' */

#define WLAN_CHAN_ANYC \
	((struct dfs_channel *) WLAN_CHAN_ANY)

#define WLAN_IS_CHAN_MODE_20(_c)      \
	(WLAN_IS_CHAN_A(_c)        ||    \
	 WLAN_IS_CHAN_11NA_HT20(_c)  ||  \
	 WLAN_IS_CHAN_11AC_VHT20(_c) ||  \
	 WLAN_IS_CHAN_11AXA_HE20(_c) ||  \
	 WLAN_IS_CHAN_11BE_EHT20(_c))

#define WLAN_IS_CHAN_MODE_40(_c)          \
	(WLAN_IS_CHAN_11AC_VHT40PLUS(_c)  || \
	 WLAN_IS_CHAN_11AC_VHT40MINUS(_c) || \
	 WLAN_IS_CHAN_11NA_HT40PLUS(_c)   || \
	 WLAN_IS_CHAN_11NA_HT40MINUS(_c)  || \
	 WLAN_IS_CHAN_11AXA_HE40PLUS(_c)  || \
	 WLAN_IS_CHAN_11AXA_HE40MINUS(_c) || \
	 WLAN_IS_CHAN_11BE_EHT40PLUS(_c)  || \
	 WLAN_IS_CHAN_11BE_EHT40MINUS(_c))

#define WLAN_IS_CHAN_MODE_80(_c)          \
	(WLAN_IS_CHAN_11AC_VHT80(_c)      || \
	 WLAN_IS_CHAN_11AXA_HE80(_c)      || \
	 WLAN_IS_CHAN_11BE_EHT80(_c))

#define WLAN_IS_CHAN_MODE_160(_c)         \
	(WLAN_IS_CHAN_11AC_VHT160(_c)     || \
	 WLAN_IS_CHAN_11AXA_HE160(_c)     || \
	 WLAN_IS_CHAN_11BE_EHT160(_c))

#define WLAN_IS_CHAN_MODE_80_80(_c)       \
	(WLAN_IS_CHAN_11AC_VHT80_80(_c)   || \
	 WLAN_IS_CHAN_11AXA_HE80_80(_c))

#define WLAN_IS_CHAN_MODE_165(_dfs, _c) \
	(dfs_is_restricted_80p80mhz_supported(_dfs) && \
	WLAN_IS_CHAN_MODE_80_80(_c))

#define WLAN_IS_CHAN_MODE_320(_c) WLAN_IS_CHAN_11BE_EHT320(_c)

#endif /* _DFS_CHANNEL_H_ */
