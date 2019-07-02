/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RATES_H_
#define _DP_RATES_H_

#define CMN_DP_ASSERT(__bool)

/*
 *Band Width Types
 */
enum CMN_BW_TYPES {
	CMN_BW_20MHZ,
	CMN_BW_40MHZ,
	CMN_BW_80MHZ,
	CMN_BW_160MHZ,
	CMN_BW_CNT,
	CMN_BW_IDLE = 0xFF, /*default BW state */
};

#define NUM_SPATIAL_STREAMS 8
#define SUPPORT_11AX 1
#define MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ 4
#define VHT_EXTRA_MCS_SUPPORT
#define CONFIG_160MHZ_SUPPORT 1
#define NUM_HT_MCS 8
#define NUM_VHT_MCS 12

#define NUM_HE_MCS 12

#define NUM_SPATIAL_STREAM 4
#define NUM_SPATIAL_STREAMS 8
#define WHAL_160MHZ_SUPPORT 1
#define MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ 4
#define RT_GET_RT(_rt)		    ((const struct DP_CMN_RATE_TABLE *)(_rt))
#define RT_GET_INFO(_rt, _index)	    RT_GET_RT(_rt)->info[(_index)]
#define RT_GET_RAW_KBPS(_rt, _index) \
	(RT_GET_INFO(_rt, (_index)).ratekbps)
#define RT_GET_SGI_KBPS(_rt, _index) \
	(RT_GET_INFO(_rt, (_index)).ratekbpssgi)

#define HW_RATECODE_CCK_SHORT_PREAM_MASK  0x4
#define RT_INVALID_INDEX (0xff)
/* pow2 to optimize out * and / */
#define DP_ATH_RATE_EP_MULTIPLIER     BIT(7)
#define DP_ATH_EP_MUL(a, b)	      ((a) * (b))
#define DP_ATH_RATE_LPF_LEN	      10	  /* Low pass filter length
						   * for averaging rates
						   */
#define DUMMY_MARKER	  0
#define DP_ATH_RATE_IN(c)  (DP_ATH_EP_MUL((c), DP_ATH_RATE_EP_MULTIPLIER))

static inline int dp_ath_rate_lpf(uint64_t _d, int _e)
{
	_e = DP_ATH_RATE_IN((_e));
	return (((_d) != DUMMY_MARKER) ? ((((_d) << 3) + (_e) - (_d)) >> 3) :
			(_e));
}

static inline int dp_ath_rate_out(uint64_t _i)
{
	int _mul = DP_ATH_RATE_EP_MULTIPLIER;

	return (((_i) != DUMMY_MARKER) ?
			((((_i) % (_mul)) >= ((_mul) / 2)) ?
			((_i) + ((_mul) - 1)) / (_mul) : (_i) / (_mul)) :
				DUMMY_MARKER);
}

#define RXDESC_GET_DATA_LEN(rx_desc) \
	(txrx_pdev->htt_pdev->ar_rx_ops->msdu_desc_msdu_length(rx_desc))
#define ASSEMBLE_HW_RATECODE(_rate, _nss, _pream)     \
	(((_pream) << 6) | ((_nss) << 4) | (_rate))
#define GET_HW_RATECODE_PREAM(_rcode)     (((_rcode) >> 6) & 0x3)
#define GET_HW_RATECODE_NSS(_rcode)       (((_rcode) >> 4) & 0x3)
#define GET_HW_RATECODE_RATE(_rcode)      (((_rcode) >> 0) & 0xF)

#define VHT_INVALID_MCS    (0xFF)  /* Certain MCSs are not valid in VHT mode */
#define VHT_INVALID_BCC_RATE  0
#define NUM_HT_SPATIAL_STREAM 4

#define NUM_HT_RIX_PER_BW (NUM_HT_MCS * NUM_HT_SPATIAL_STREAM)
#define NUM_VHT_RIX_PER_BW (NUM_VHT_MCS * NUM_SPATIAL_STREAMS)
#define NUM_HE_RIX_PER_BW (NUM_HE_MCS * NUM_SPATIAL_STREAMS)

#define NUM_VHT_RIX_FOR_160MHZ (NUM_VHT_MCS * \
		MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ)
#define NUM_HE_RIX_FOR_160MHZ (NUM_HE_MCS * \
		MAX_SPATIAL_STREAMS_SUPPORTED_AT_160MHZ)

#define CCK_RATE_TABLE_INDEX 0
#define CCK_RATE_11M_INDEX 0
#define CCK_FALLBACK_MIN_RATE 0x3 /** 1 Mbps */
#define CCK_FALLBACK_MAX_RATE 0x2 /** 2 Mbps */

#define OFDM_RATE_TABLE_INDEX 4
#define OFDMA_RATE_54M_INDEX 8

#define HT_20_RATE_TABLE_INDEX 12
#define HT_40_RATE_TABLE_INDEX (HT_20_RATE_TABLE_INDEX + NUM_HT_RIX_PER_BW)

#define VHT_20_RATE_TABLE_INDEX (HT_40_RATE_TABLE_INDEX + NUM_HT_RIX_PER_BW)
#define VHT_40_RATE_TABLE_INDEX (VHT_20_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)
#define VHT_80_RATE_TABLE_INDEX (VHT_40_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)

#define VHT_160_RATE_TABLE_INDEX (VHT_80_RATE_TABLE_INDEX + NUM_VHT_RIX_PER_BW)
#define VHT_LAST_RIX_PLUS_ONE (VHT_160_RATE_TABLE_INDEX + \
		NUM_VHT_RIX_FOR_160MHZ)

#define HE_20_RATE_TABLE_INDEX VHT_LAST_RIX_PLUS_ONE
#define HE_40_RATE_TABLE_INDEX (HE_20_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)
#define HE_80_RATE_TABLE_INDEX (HE_40_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)

#define HE_160_RATE_TABLE_INDEX (HE_80_RATE_TABLE_INDEX + NUM_HE_RIX_PER_BW)
#define DP_RATE_TABLE_SIZE (HE_160_RATE_TABLE_INDEX + NUM_HE_RIX_FOR_160MHZ)

/* The following would span more than one octet
 * when 160MHz BW defined for VHT
 * Also it's important to maintain the ordering of
 * this enum else it would break other rate adapation functions.
 */
enum DP_CMN_MODULATION_TYPE {
	   DP_CMN_MOD_IEEE80211_T_DS,   /* direct sequence spread spectrum */
	   DP_CMN_MOD_IEEE80211_T_OFDM, /* frequency division multiplexing */
	   DP_CMN_MOD_IEEE80211_T_HT_20,
	   DP_CMN_MOD_IEEE80211_T_HT_40,
	   DP_CMN_MOD_IEEE80211_T_VHT_20,
	   DP_CMN_MOD_IEEE80211_T_VHT_40,
	   DP_CMN_MOD_IEEE80211_T_VHT_80,
	   DP_CMN_MOD_IEEE80211_T_VHT_160,
	   DP_CMN_MOD_IEEE80211_T_HE_20, /* 11AX support enabled */
	   DP_CMN_MOD_IEEE80211_T_HE_40,
	   DP_CMN_MOD_IEEE80211_T_HE_80,
	   DP_CMN_MOD_IEEE80211_T_HE_160,
	   DP_CMN_MOD_IEEE80211_T_MAX_PHY
};

/* more common nomenclature */
#define DP_CMN_MOD_IEEE80211_T_CCK DP_CMN_MOD_IEEE80211_T_DS

enum HW_RATECODE_PREAM_TYPE {
	HW_RATECODE_PREAM_OFDM,
	HW_RATECODE_PREAM_CCK,
	HW_RATECODE_PREAM_HT,
	HW_RATECODE_PREAM_VHT,
};

enum DP_CMN_MODULATION_TYPE dp_getmodulation(
		uint16_t pream_type,
		uint8_t width);

uint32_t
dp_getrateindex(uint32_t gi, uint16_t mcs, uint8_t nss, uint8_t preamble,
		uint8_t bw, uint32_t *rix, uint16_t *ratecode);

int dp_rate_idx_to_kbps(uint8_t rate_idx, uint8_t gintval);

#endif /*_DP_RATES_H_*/
