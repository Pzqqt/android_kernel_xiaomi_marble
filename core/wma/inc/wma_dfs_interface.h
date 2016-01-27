/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include "ath_dfs_structs.h"
#include <qdf_lock.h>
#include "cds_reg_service.h"
#include "cds_regdomain.h"
#include "cds_ieee80211_common.h"

#define IEEE80211_CHAN_MAX      255

/* channel attributes */

/* Turbo channel */
#define IEEE80211_CHAN_TURBO            0x00000010
/* CCK channel */
#define IEEE80211_CHAN_CCK              0x00000020
/* OFDM channel */
#define IEEE80211_CHAN_OFDM             0x00000040
/* 2 GHz spectrum channel. */
#define IEEE80211_CHAN_2GHZ             0x00000080
/* 5 GHz spectrum channel */
#define IEEE80211_CHAN_5GHZ             0x00000100
/* Only passive scan allowed */
#define IEEE80211_CHAN_PASSIVE          0x00000200
/* Dynamic CCK-OFDM channel */
#define IEEE80211_CHAN_DYN              0x00000400
/* GFSK channel (FHSS PHY) */
#define IEEE80211_CHAN_GFSK             0x00000800
/* 11a static turbo channel only */
#define IEEE80211_CHAN_STURBO           0x00002000
/* Half rate channel */
#define IEEE80211_CHAN_HALF             0x00004000
/* Quarter rate channel */
#define IEEE80211_CHAN_QUARTER          0x00008000
/* HT 20 channel */
#define IEEE80211_CHAN_HT20             0x00010000
/* HT 40 with extension channel above */
#define IEEE80211_CHAN_HT40PLUS         0x00020000
/* HT 40 with extension channel below */
#define IEEE80211_CHAN_HT40MINUS        0x00040000
/* HT 40 Intolerant */
#define IEEE80211_CHAN_HT40INTOL        0x00080000
/* VHT 20 channel */
#define IEEE80211_CHAN_VHT20            0x00100000
/* VHT 40 with extension channel above */
#define IEEE80211_CHAN_VHT40PLUS        0x00200000
/* VHT 40 with extension channel below */
#define IEEE80211_CHAN_VHT40MINUS       0x00400000
/* VHT 80 channel */
#define IEEE80211_CHAN_VHT80            0x00800000
/* VHT 80+80 Channel */
#define IEEE80211_CHAN_VHT80P80         0x01000000
/* VHT 160 Channel */
#define IEEE80211_CHAN_VHT160           0x02000000

/* token for ``any channel'' */
#define DFS_IEEE80211_CHAN_ANY      (-1)
#define DFS_IEEE80211_CHAN_ANYC \
	((struct dfs_ieee80211_channel *) DFS_IEEE80211_CHAN_ANY)

#define IEEE80211_IS_CHAN_11N_HT40MINUS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40MINUS) != 0)
#define IEEE80211_IS_CHAN_11N_HT40PLUS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40PLUS) != 0)
#define IEEE80211_CHAN_11AC_VHT80 \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_VHT80)

#define IEEE80211_IS_CHAN_11AC_VHT80(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_11AC_VHT80) == \
	 IEEE80211_CHAN_11AC_VHT80)
#define IEEE80211_IS_CHAN_11AC_VHT80P80(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_VHT80P80) == \
	IEEE80211_CHAN_VHT80P80)
#define IEEE80211_IS_CHAN_11AC_VHT160(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_VHT160) == \
	IEEE80211_CHAN_VHT160)
#define CHANNEL_108G \
	(IEEE80211_CHAN_2GHZ|IEEE80211_CHAN_OFDM|IEEE80211_CHAN_TURBO)

/*
 * Software use: channel interference
 * used for as AR as well as RADAR
 * interference detection
 */
#define CHANNEL_INTERFERENCE    0x01
/* In case of VHT160, we can have 8 20Mhz channels */
#define IEE80211_MAX_20M_SUB_CH 8

#define WMA_DFS2_PHYERROR_CODE    0x5
#define WMA_DFS2_FALSE_RADAR_EXT  0x24

/**
 * struct dfs_ieee80211_channel - channel info
 * @ic_freq: frequency in MHz
 * @ic_flags: channel flags
 * @ic_flagext: flags extension
 * @ic_ieee: IEEE channel number
 * @ic_maxregpower: max regulatory power in dbm
 * @ic_maxpower: max tx power in dbm
 * @ic_minpower: min tx power in dbm
 * @ic_regClassId: reg class id of this channel
 * @ic_antennamax: max antenna gain from regulatory
 * @ic_vhtop_ch_freq_seg1: channel center frequency
 * @ic_vhtop_ch_freq_seg2: Channel Center frequency applicable
 * @ic_pri_freq_center_freq_mhz_separation: separation b/w pri and center freq
 * @ic_80p80_both_dfs: Flag indicating if both 80p80 segments are dfs
 * @ic_radar_found_segid: Indicates seg ID on which radar is found in 80p80 mode
 */
struct dfs_ieee80211_channel {
	uint32_t ic_freq;
	uint32_t ic_freq_ext;
	uint32_t ic_flags;
	uint8_t ic_flagext;
	uint8_t ic_ieee;
	uint8_t ic_ieee_ext;
	int8_t ic_maxregpower;
	int8_t ic_maxpower;
	int8_t ic_minpower;
	uint8_t ic_regClassId;
	uint8_t ic_antennamax;
	uint32_t ic_vhtop_ch_freq_seg1;
	uint32_t ic_vhtop_ch_freq_seg2;
	int ic_pri_freq_center_freq_mhz_separation;
	bool ic_80p80_both_dfs;
	int ic_radar_found_segid;
};

/**
 * struct ieee80211_channel_list - channel list
 * @cl_nchans: number of channels
 * @cl_channels: channel info
 */
struct ieee80211_channel_list {
	int cl_nchans;
	struct dfs_ieee80211_channel *cl_channels[IEE80211_MAX_20M_SUB_CH];
};

/**
 * struct ieee80211_dfs_state - dfs state
 * @nol_event: nol event list
 * @nol_timer: nol list processing
 * @cac_timer: cac timer
 * @cureps: current events/second
 * @lastchan: chan w/ last radar event
 * @newchan: chan selected next
 * @cac_timeout_override: overridden cac timeout
 * @flags: dfs flags
 */
struct ieee80211_dfs_state {
	int nol_event[IEEE80211_CHAN_MAX];
	os_timer_t nol_timer;
	os_timer_t cac_timer;
	int cureps;
	const struct dfs_ieee80211_channel *lastchan;
	struct dfs_ieee80211_channel *newchan;
	int cac_timeout_override;
	uint8_t enable:1, cac_timer_running:1, ignore_dfs:1, ignore_cac:1;
};

/**
 * enum DFS_HWBD_ID - Board ID to differentiate between DFS-2 and DFS-3
 * @DFS_HWBD_NONE: No hw board information/currently used for adreastea FPGA
 * @DFS_HWBD_QCA6174: Rome(AR6320)
 * @DFS_HWBD_QCA2582: Killer 1525
 */
typedef enum {
	DFS_HWBD_NONE = 0,
	DFS_HWBD_QCA6174 = 1,
	DFS_HWBD_QCA2582 = 2,
} DFS_HWBD_ID;


/**
 * struct ieee80211com - per device structure
 * @ic_opmode: operation mode
 * @ic_channels: ieee80211 channel list
 * @ic_nchans: number of channels
 * @ic_curchan: current channel
 * @ic_isdfsregdomain: is opearting in dfs region
 * @current_dfs_regdomain: current dfs regulatory domain
 * @vdev_id: vdev id
 * @last_radar_found_chan: last radar found channel
 * @dfs_pri_multiplier: dfs multiplier
 */
typedef struct ieee80211com {
	void (*ic_start_csa)(struct ieee80211com *ic, uint8_t ieeeChan);
	void (*ic_get_ext_chan_info)(struct ieee80211com *ic,
				     struct ieee80211_channel_list *chan);
	enum ieee80211_opmode ic_opmode;
	struct dfs_ieee80211_channel *(*ic_find_channel)
			(struct ieee80211com *ic, int freq, uint32_t flags);
	uint64_t (*ic_get_TSF64)(struct ieee80211com *ic);
	unsigned int (*ic_ieee2mhz)(u_int chan, u_int flags);
	struct dfs_ieee80211_channel ic_channels[IEEE80211_CHAN_MAX + 1];
	int ic_nchans;
	struct dfs_ieee80211_channel *ic_curchan;
	uint8_t ic_isdfsregdomain;
	int (*ic_get_dfsdomain)(struct ieee80211com *);
	uint16_t (*ic_dfs_usenol)(struct ieee80211com *ic);
	uint16_t (*ic_dfs_isdfsregdomain)(struct ieee80211com *ic);
	int (*ic_dfs_attached)(struct ieee80211com *ic);
	void *ic_dfs;
	struct ieee80211_dfs_state ic_dfs_state;
	int (*ic_dfs_attach)(struct ieee80211com *ic,
			     void *pCap, void *radar_info);
	int (*ic_dfs_detach)(struct ieee80211com *ic);
	int (*ic_dfs_enable)(struct ieee80211com *ic, int *is_fastclk,
			     void *);
	int (*ic_dfs_disable)(struct ieee80211com *ic);
	int (*ic_get_ext_busy)(struct ieee80211com *ic);
	int (*ic_get_mib_cycle_counts_pct)(struct ieee80211com *ic,
					   uint32_t *rxc_pcnt,
					   uint32_t *rxf_pcnt,
					   uint32_t *txf_pcnt);
	int (*ic_dfs_get_thresholds)(struct ieee80211com *ic, void *pe);

	int (*ic_dfs_debug)(struct ieee80211com *ic, int type, void *data);
	/*
	 * Update the channel list with the current set of DFS
	 * NOL entries.
	 *
	 * + 'cmd' indicates what to do; for now it should just
	 *   be DFS_NOL_CLIST_CMD_UPDATE which will update all
	 *   channels, given the _entire_ NOL. (Rather than
	 *   the earlier behaviour with clist_update, which
	 *   was to either add or remove a set of channel
	 *   entries.)
	 */
	void (*ic_dfs_clist_update)(struct ieee80211com *ic, int cmd,
				    struct dfs_nol_chan_entry *, int nentries);
	void (*ic_dfs_notify_radar)(struct ieee80211com *ic,
				    struct dfs_ieee80211_channel *chan);
	void (*ic_dfs_unmark_radar)(struct ieee80211com *ic,
				    struct dfs_ieee80211_channel *chan);
	int (*ic_dfs_control)(struct ieee80211com *ic,
			      u_int id, void *indata, uint32_t insize,
			      void *outdata, uint32_t *outsize);
	enum dfs_region current_dfs_regdomain;
	uint8_t vdev_id;
	uint8_t last_radar_found_chan;
	int32_t dfs_pri_multiplier;
	qdf_spinlock_t chan_lock;
	bool disable_phy_err_processing;
	DFS_HWBD_ID dfs_hw_bd_id;
} IEEE80211COM, *PIEEE80211COM;

/**
 * ieee80211_chan2freq() - Convert channel to frequency value.
 * @ic: ieee80211com ptr
 * @c: ieee80211 channel
 *
 * Return: freqency in MHz
 */
static inline u_int
ieee80211_chan2freq(struct ieee80211com *ic,
			const struct dfs_ieee80211_channel *c)
{
	if (c == NULL) {
		return 0;
	}
	return (c == DFS_IEEE80211_CHAN_ANYC) ?
			DFS_IEEE80211_CHAN_ANY : c->ic_freq;
}
