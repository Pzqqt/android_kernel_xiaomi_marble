/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: reg_services.c
 * This file defines regulatory component service functions
 */

#include "reg_services.h"
#include "reg_priv.h"
#include "reg_db_parser.h"
#include <scheduler_api.h>

#define MAX_PWR_FCC_CHAN_12 8
#define MAX_PWR_FCC_CHAN_13 2

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (NULL != psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (NULL != pdev_priv_obj)


const struct bonded_channel bonded_chan_40mhz_list[] = {
	{36, 40},
	{44, 48},
	{52, 56},
	{60, 64},
	{100, 104},
	{108, 112},
	{116, 120},
	{124, 128},
	{132, 136},
	{140, 144},
	{149, 153},
	{157, 161}
};

const struct bonded_channel bonded_chan_80mhz_list[] = {
	{36, 48},
	{52, 64},
	{100, 112},
	{116, 128},
	{132, 144},
	{149, 161}
};

const struct bonded_channel bonded_chan_160mhz_list[] = {
	{36, 64},
	{100, 128}
};

const enum phy_ch_width get_next_lower_bw[] = {
	[CH_WIDTH_80P80MHZ] = CH_WIDTH_160MHZ,
	[CH_WIDTH_160MHZ] = CH_WIDTH_80MHZ,
	[CH_WIDTH_80MHZ] = CH_WIDTH_40MHZ,
	[CH_WIDTH_40MHZ] = CH_WIDTH_20MHZ,
	[CH_WIDTH_20MHZ] = CH_WIDTH_10MHZ,
	[CH_WIDTH_10MHZ] = CH_WIDTH_5MHZ,
	[CH_WIDTH_5MHZ] = CH_WIDTH_INVALID
};

const struct chan_map channel_map[NUM_CHANNELS] = {
	[CHAN_ENUM_1] = {2412, 1},
	[CHAN_ENUM_2] = {2417, 2},
	[CHAN_ENUM_3] = {2422, 3},
	[CHAN_ENUM_4] = {2427, 4},
	[CHAN_ENUM_5] = {2432, 5},
	[CHAN_ENUM_6] = {2437, 6},
	[CHAN_ENUM_7] = {2442, 7},
	[CHAN_ENUM_8] = {2447, 8},
	[CHAN_ENUM_9] = {2452, 9},
	[CHAN_ENUM_10] = {2457, 10},
	[CHAN_ENUM_11] = {2462, 11},
	[CHAN_ENUM_12] = {2467, 12},
	[CHAN_ENUM_13] = {2472, 13},
	[CHAN_ENUM_14] = {2484, 14},

	[CHAN_ENUM_36] = {5180, 36},
	[CHAN_ENUM_40] = {5200, 40},
	[CHAN_ENUM_44] = {5220, 44},
	[CHAN_ENUM_48] = {5240, 48},
	[CHAN_ENUM_52] = {5260, 52},
	[CHAN_ENUM_56] = {5280, 56},
	[CHAN_ENUM_60] = {5300, 60},
	[CHAN_ENUM_64] = {5320, 64},

	[CHAN_ENUM_100] = {5500, 100},
	[CHAN_ENUM_104] = {5520, 104},
	[CHAN_ENUM_108] = {5540, 108},
	[CHAN_ENUM_112] = {5560, 112},
	[CHAN_ENUM_116] = {5580, 116},
	[CHAN_ENUM_120] = {5600, 120},
	[CHAN_ENUM_124] = {5620, 124},
	[CHAN_ENUM_128] = {5640, 128},
	[CHAN_ENUM_132] = {5660, 132},
	[CHAN_ENUM_136] = {5680, 136},
	[CHAN_ENUM_140] = {5700, 140},
	[CHAN_ENUM_144] = {5720, 144},

	[CHAN_ENUM_149] = {5745, 149},
	[CHAN_ENUM_153] = {5765, 153},
	[CHAN_ENUM_157] = {5785, 157},
	[CHAN_ENUM_161] = {5805, 161},
	[CHAN_ENUM_165] = {5825, 165},

	[CHAN_ENUM_170] = {5852, 170},
	[CHAN_ENUM_171] = {5855, 171},
	[CHAN_ENUM_172] = {5860, 172},
	[CHAN_ENUM_173] = {5865, 173},
	[CHAN_ENUM_174] = {5870, 174},
	[CHAN_ENUM_175] = {5875, 175},
	[CHAN_ENUM_176] = {5880, 176},
	[CHAN_ENUM_177] = {5885, 177},
	[CHAN_ENUM_178] = {5890, 178},
	[CHAN_ENUM_179] = {5895, 179},
	[CHAN_ENUM_180] = {5900, 180},
	[CHAN_ENUM_181] = {5905, 181},
	[CHAN_ENUM_182] = {5910, 182},
	[CHAN_ENUM_183] = {5915, 183},
	[CHAN_ENUM_184] = {5920, 184},
};


static struct reg_dmn_supp_op_classes reg_dmn_curr_supp_opp_classes = { 0 };

static const struct reg_dmn_op_class_map_t global_op_class[] = {
	{81, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{82, 25, BW20, {14} },
	{83, 40, BW40_LOW_PRIMARY, {1, 2, 3, 4, 5, 6, 7, 8, 9} },
	{84, 40, BW40_HIGH_PRIMARY, {5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{115, 20, BW20, {36, 40, 44, 48} },
	{116, 40, BW40_LOW_PRIMARY, {36, 44} },
	{117, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{118, 20, BW20, {52, 56, 60, 64} },
	{119, 40, BW40_LOW_PRIMARY, {52, 60} },
	{120, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{121, 20, BW20,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{122, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{123, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{125, 20, BW20, {149, 153, 157, 161, 165, 169} },
	{126, 40, BW40_LOW_PRIMARY, {149, 157} },
	{127, 40, BW40_HIGH_PRIMARY, {153, 161} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108,
			   112, 116, 120, 124, 128, 132, 136, 140, 144,
			   149, 153, 157, 161} },
	{0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t us_op_class[] = {
	{1, 20, BW20, {36, 40, 44, 48} },
	{2, 20, BW20, {52, 56, 60, 64} },
	{4, 20, BW20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
		       144} },
	{5, 20, BW20, {149, 153, 157, 161, 165} },
	{12, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11} },
	{22, 40, BW40_LOW_PRIMARY, {36, 44} },
	{23, 40, BW40_LOW_PRIMARY, {52, 60} },
	{24, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{26, 40, BW40_LOW_PRIMARY, {149, 157} },
	{27, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{28, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{29, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{31, 40, BW40_HIGH_PRIMARY, {153, 161} },
	{32, 40, BW40_LOW_PRIMARY, {1, 2, 3, 4, 5, 6, 7} },
	{33, 40, BW40_HIGH_PRIMARY, {5, 6, 7, 8, 9, 10, 11} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108,
			 112, 116, 120, 124, 128, 132, 136, 140, 144,
			 149, 153, 157, 161} },
	{0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t euro_op_class[] = {
	{1, 20, BW20, {36, 40, 44, 48} },
	{2, 20, BW20, {52, 56, 60, 64} },
	{3, 20, BW20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{4, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{5, 40, BW40_LOW_PRIMARY, {36, 44} },
	{6, 40, BW40_LOW_PRIMARY, {52, 60} },
	{7, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{8, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{9, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{10, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{11, 40, BW40_LOW_PRIMARY, {1, 2, 3, 4, 5, 6, 7, 8, 9} },
	{12, 40, BW40_HIGH_PRIMARY, {5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{17, 20, BW20, {149, 153, 157, 161, 165, 169} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
			 116, 120, 124, 128} },
	{0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t japan_op_class[] = {
	{1, 20, BW20, {36, 40, 44, 48} },
	{30, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{31, 25, BW20, {14} },
	{32, 20, BW20, {52, 56, 60, 64} },
	{34, 20, BW20,
		{100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{36, 40, BW40_LOW_PRIMARY, {36, 44} },
	{37, 40, BW40_LOW_PRIMARY, {52, 60} },
	{39, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{41, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{42, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{44, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
			 116, 120, 124, 128} },
	{0, 0, 0, {0} },
};

/**
 * reg_get_psoc_obj() - Provides the reg component object pointer
 * @psoc: pointer to psoc object.
 *
 * Return: reg component object pointer
 */
static struct wlan_regulatory_psoc_priv_obj *reg_get_psoc_obj(
		struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	if (!psoc) {
		reg_alert("psoc is NULL");
		return NULL;
	}
	wlan_psoc_obj_lock(psoc);
	soc_reg = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	return soc_reg;
}

/**
 * reg_get_pdev_obj() - Provides the reg component object pointer
 * @psoc: pointer to psoc object.
 *
 * Return: reg component object pointer
 */
static struct wlan_regulatory_pdev_priv_obj *reg_get_pdev_obj(
		struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_reg;

	if (!pdev) {
		reg_alert("pdev is NULL");
		return NULL;
	}
	wlan_pdev_obj_lock(pdev);
	pdev_reg = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_REGULATORY);
	wlan_pdev_obj_unlock(pdev);

	return pdev_reg;
}
/**
 * reg_get_bw_value() - give bandwidth value
 * bw: bandwidth enum
 *
 * Return: uint16_t
 */
uint16_t reg_get_bw_value(enum phy_ch_width bw)
{
	switch (bw) {
	case CH_WIDTH_20MHZ:
		return 20;
	case CH_WIDTH_40MHZ:
		return 40;
	case CH_WIDTH_80MHZ:
		return 80;
	case CH_WIDTH_160MHZ:
		return 160;
	case CH_WIDTH_80P80MHZ:
		return 160;
	case CH_WIDTH_INVALID:
		return 0;
	case CH_WIDTH_5MHZ:
		return 5;
	case CH_WIDTH_10MHZ:
		return 10;
	case CH_WIDTH_MAX:
		return 160;
	default:
		return 0;
	}
}

/**
 * reg_get_channel_list_with_power() - Provides the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
					   struct channel_power *ch_list,
					   uint8_t *num_chan)
{
	int i, count;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	if (!num_chan || !ch_list) {
		reg_err("chan_list or num_ch is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* set the current channel list */
	reg_channels = pdev_priv_obj->cur_chan_list;

	for (i = 0, count = 0; i < NUM_CHANNELS; i++) {
		if (reg_channels[i].state) {
			ch_list[count].chan_num =
				channel_map[i].chan_num;
			ch_list[count++].tx_power =
				reg_channels[i].tx_power;
		}
	}

	*num_chan = count;

	return QDF_STATUS_SUCCESS;
}

enum channel_enum reg_get_chan_enum(uint32_t chan_num)
{
	uint32_t count;

	for (count = 0; count < NUM_CHANNELS; count++)
		if (channel_map[count].chan_num == chan_num)
			return count;

	reg_err("invalid channel number %d", chan_num);

	return INVALID_CHANNEL;
}
/**
 * reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
		uint32_t ch)
{
	enum channel_enum ch_idx;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	ch_idx = reg_get_chan_enum(ch);

	if (INVALID_CHANNEL == ch_idx)
		return CHANNEL_STATE_INVALID;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return pdev_priv_obj->cur_chan_list[ch_idx].state;
}

/**
 * reg_get_5g_bonded_chan_array() - get ptr to bonded channel
 * @oper_ch: operating channel number
 * @bonded_chan_ar: bonded channel array
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: bonded channel state
 */
static enum channel_state reg_get_5g_bonded_chan_array(
		struct wlan_objmgr_pdev *pdev,
		uint32_t oper_chan,
		const struct bonded_channel bonded_chan_ar[],
		uint16_t array_size,
		const struct bonded_channel **bonded_chan_ptr_ptr)
{
	int i;
	uint8_t chan_num;
	const struct bonded_channel *bonded_chan_ptr = NULL;
	enum channel_state chan_state = CHANNEL_STATE_INVALID;
	enum channel_state temp_chan_state;

	for (i = 0; i < array_size; i++) {
		if ((oper_chan >= bonded_chan_ar[i].start_ch) &&
		    (oper_chan <= bonded_chan_ar[i].end_ch)) {
			bonded_chan_ptr =  &(bonded_chan_ar[i]);
			break;
		}
	}

	if (NULL == bonded_chan_ptr)
		return chan_state;

	*bonded_chan_ptr_ptr = bonded_chan_ptr;
	chan_num =  bonded_chan_ptr->start_ch;
	while (chan_num <= bonded_chan_ptr->end_ch) {
		temp_chan_state = reg_get_channel_state(pdev, chan_num);
		if (temp_chan_state < chan_state)
			chan_state = temp_chan_state;
		chan_num = chan_num + 4;
	}

	return chan_state;
}

/**
 * reg_get_5g_bonded_channel() - get the 5G bonded channel state
 * @chan_num: channel number
 * @ch_width: channel width
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: channel state
 */
static enum channel_state reg_get_5g_bonded_channel(
		struct wlan_objmgr_pdev *pdev, uint32_t chan_num,
		enum phy_ch_width ch_width,
		const struct bonded_channel **bonded_chan_ptr_ptr)
{
	if (CH_WIDTH_80P80MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_160MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_160mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_160mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_80MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_40MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_40mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
				bonded_chan_ptr_ptr);
	else
		return reg_get_channel_state(pdev, chan_num);
}
/**
 * reg_get_5g_bonded_channel_state() - Get channel state for 5G bonded channel
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev,
		uint8_t ch, enum phy_ch_width bw)
{
	enum channel_enum ch_indx;
	enum channel_state chan_state;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	bool bw_enabled = false;
	const struct bonded_channel *bonded_chan_ptr = NULL;

	if (CH_WIDTH_80P80MHZ < bw) {
		reg_err("bw passed is not good");
		return CHANNEL_STATE_INVALID;
	}

	chan_state = reg_get_5g_bonded_channel(pdev, ch, bw, &bonded_chan_ptr);

	if ((CHANNEL_STATE_INVALID == chan_state) ||
			(CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return CHANNEL_STATE_INVALID;
	}
	reg_channels = pdev_priv_obj->cur_chan_list;

	ch_indx = reg_get_chan_enum(ch);
	if (CH_WIDTH_5MHZ == bw)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 10) &&
			(reg_channels[ch_indx].max_bw >= 10);
	else if (CH_WIDTH_20MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 20) &&
			(reg_channels[ch_indx].max_bw >= 20);
	else if (CH_WIDTH_40MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 40) &&
			(reg_channels[ch_indx].max_bw >= 40);
	else if (CH_WIDTH_80MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 80) &&
			(reg_channels[ch_indx].max_bw >= 80);
	else if (CH_WIDTH_160MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 160) &&
			(reg_channels[ch_indx].max_bw >= 160);
	else if (CH_WIDTH_80P80MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 80) &&
			(reg_channels[ch_indx].max_bw >= 80);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;
}

/**
 * reg_get_2g_bonded_channel_state() - Get channel state for 2G bonded channel
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev,
		uint8_t oper_ch, uint8_t sec_ch,
		enum phy_ch_width bw)
{
	enum channel_enum chan_idx;
	enum channel_state chan_state;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	bool bw_enabled = false;
	enum channel_state chan_state2 = CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ < bw)
		return CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ == bw) {
		if ((sec_ch + 4 != oper_ch) &&
		    (oper_ch + 4 != sec_ch))
			return CHANNEL_STATE_INVALID;
		chan_state2 = reg_get_channel_state(pdev, sec_ch);
		if (CHANNEL_STATE_INVALID == chan_state2)
			return chan_state2;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return CHANNEL_STATE_INVALID;
	}

	reg_channels = pdev_priv_obj->cur_chan_list;

	chan_state = reg_get_channel_state(pdev, oper_ch);
	if (chan_state2 < chan_state)
		chan_state = chan_state2;

	if ((CHANNEL_STATE_INVALID == chan_state) ||
	    (CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	chan_idx = reg_get_chan_enum(oper_ch);
	if (CH_WIDTH_5MHZ == bw)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == bw)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 10) &&
			(reg_channels[chan_idx].max_bw >= 10);
	else if (CH_WIDTH_20MHZ == bw)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 20) &&
			(reg_channels[chan_idx].max_bw >= 20);
	else if (CH_WIDTH_40MHZ == bw)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 40) &&
			(reg_channels[chan_idx].max_bw >= 40);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;

	return CHANNEL_STATE_ENABLE;
}

static enum channel_state reg_combine_channel_states(
	enum channel_state chan_state1,
	enum channel_state chan_state2)
{
	if ((CHANNEL_STATE_INVALID == chan_state1) ||
	    (CHANNEL_STATE_INVALID == chan_state2))
		return CHANNEL_STATE_INVALID;
	else
		return min(chan_state1, chan_state2);
}

/**
 * reg_set_5g_channel_params () - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
static void reg_set_5g_channel_params(struct wlan_objmgr_pdev *pdev,
			       uint8_t ch,
			       struct ch_params *ch_params)
{
	/*
	 * Set channel parameters like center frequency for a bonded channel
	 * state. Also return the maximum bandwidth supported by the channel.
	 */

	enum phy_ch_width next_lower_bw;
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;
	enum channel_state chan_state2 = CHANNEL_STATE_ENABLE;
	const struct bonded_channel *bonded_chan_ptr = NULL;
	const struct bonded_channel *bonded_chan_ptr2 = NULL;

	if (NULL == ch_params) {
		reg_err("ch_params is NULL");
		return;
	}

	if (CH_WIDTH_MAX <= ch_params->ch_width) {
		if (0 != ch_params->center_freq_seg1)
			ch_params->ch_width = CH_WIDTH_80P80MHZ;
		else
			ch_params->ch_width = CH_WIDTH_160MHZ;
	}
	next_lower_bw = ch_params->ch_width;

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		ch_params->ch_width = next_lower_bw;
		next_lower_bw = get_next_lower_bw[ch_params->ch_width];
		bonded_chan_ptr = NULL;
		bonded_chan_ptr2 = NULL;
		chan_state = reg_get_5g_bonded_channel(pdev, ch,
				ch_params->ch_width, &bonded_chan_ptr);

		chan_state = reg_get_5g_bonded_channel_state(pdev, ch,
				ch_params->ch_width);

		if (CH_WIDTH_80P80MHZ == ch_params->ch_width) {
			chan_state2 = reg_get_5g_bonded_channel_state(pdev,
					   ch_params->center_freq_seg1 - 2,
					   CH_WIDTH_80MHZ);

			chan_state = reg_combine_channel_states(chan_state,
					chan_state2);
		}

		if ((CHANNEL_STATE_ENABLE != chan_state) &&
		    (CHANNEL_STATE_DFS != chan_state))
			continue;
		if (CH_WIDTH_20MHZ >= ch_params->ch_width) {
			ch_params->sec_ch_offset = NO_SEC_CH;
			ch_params->center_freq_seg0 = ch;
			break;
		} else if (CH_WIDTH_40MHZ <= ch_params->ch_width) {
			reg_get_5g_bonded_chan_array(pdev, ch,
					bonded_chan_40mhz_list,
					QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
					&bonded_chan_ptr2);
			if (!bonded_chan_ptr || !bonded_chan_ptr2)
				continue;
			if (ch == bonded_chan_ptr2->start_ch)
				ch_params->sec_ch_offset = LOW_PRIMARY_CH;
			else
				ch_params->sec_ch_offset = HIGH_PRIMARY_CH;

			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
			break;
		}
	}

	if (CH_WIDTH_160MHZ == ch_params->ch_width) {
		ch_params->center_freq_seg1 = ch_params->center_freq_seg0;
		chan_state = reg_get_5g_bonded_channel(pdev, ch,
				    CH_WIDTH_80MHZ, &bonded_chan_ptr);
		if (bonded_chan_ptr)
			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
	}

	reg_debug("ch %d ch_wd %d freq0 %d freq1 %d", ch,
		  ch_params->ch_width, ch_params->center_freq_seg0,
		  ch_params->center_freq_seg1);
}

/**
 * reg_set_2g_channel_params() - set the 2.4G bonded channel parameters
 * @oper_ch: operating channel
 * @ch_params: channel parameters
 * @sec_ch_2g: 2.4G secondary channel
 *
 * Return: void
 */
static void reg_set_2g_channel_params(struct wlan_objmgr_pdev *pdev,
		uint16_t oper_ch, struct ch_params *ch_params,
		uint16_t sec_ch_2g)
{
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;

	if (CH_WIDTH_MAX <= ch_params->ch_width)
		ch_params->ch_width = CH_WIDTH_40MHZ;
	if ((reg_get_bw_value(ch_params->ch_width) > 20) && !sec_ch_2g) {
		if (oper_ch >= 1 && oper_ch <= 5)
			sec_ch_2g = oper_ch + 4;
		else if (oper_ch >= 6 && oper_ch <= 13)
			sec_ch_2g = oper_ch - 4;
	}

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		chan_state = reg_get_2g_bonded_channel_state(pdev, oper_ch,
							  sec_ch_2g,
							  ch_params->ch_width);
		if (CHANNEL_STATE_ENABLE == chan_state) {
			if (CH_WIDTH_40MHZ == ch_params->ch_width) {
				if (oper_ch < sec_ch_2g)
					ch_params->sec_ch_offset =
						LOW_PRIMARY_CH;
				else
					ch_params->sec_ch_offset =
						HIGH_PRIMARY_CH;
				ch_params->center_freq_seg0 =
					(oper_ch + sec_ch_2g)/2;
			} else
				ch_params->sec_ch_offset = NO_SEC_CH;
			break;
		}

		ch_params->ch_width = get_next_lower_bw[ch_params->ch_width];
	}
}

/**
 * reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params(struct wlan_objmgr_pdev *pdev,
			    uint8_t ch, uint8_t sec_ch_2g,
			    struct ch_params *ch_params)
{
	if (REG_IS_5GHZ_CH(ch))
		reg_set_5g_channel_params(pdev, ch, ch_params);
	else if  (REG_IS_24GHZ_CH(ch))
		reg_set_2g_channel_params(pdev, ch, ch_params,
					  sec_ch_2g);
}

QDF_STATUS reg_get_curr_band(struct wlan_objmgr_pdev *pdev,
			     enum band_info *band)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_reg;

	pdev_reg = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_reg)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	*band = pdev_reg->band_capability;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country_code) {
		reg_err("country_code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(country_code,
		     psoc_reg->default_country,
		     sizeof(psoc_reg->default_country));

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_set_default_country() - Read the default country for the regdomain
 * @country: country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country) {
		reg_err("country is NULL");
		return QDF_STATUS_E_INVAL;
	}
	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting default_country:n%s", country);
	qdf_mem_copy(psoc_reg->default_country,
		     country, sizeof(psoc_reg->default_country));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_set_country(struct wlan_objmgr_pdev *pdev,
			   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct set_country country_code;
	struct wlan_objmgr_psoc *psoc;

	if (!country) {
		reg_err("country code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);

	psoc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(psoc_reg->current_country,
			country, REG_ALPHA2_LEN)) {
		reg_err("country is not different");
		return QDF_STATUS_E_INVAL;
	}

	reg_debug("programming new country:%s to firmware", country);

	qdf_mem_copy(country_code.country,
		     country, REG_ALPHA2_LEN + 1);
	country_code.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	psoc_reg->new_user_ctry_pending = true;

	tx_ops = reg_get_psoc_tx_ops(psoc);

	if (tx_ops->set_country_code) {
		tx_ops->set_country_code(psoc, &country_code);
	} else {
		reg_err("country set fw handler not present");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_reset_country(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("re-setting current_country to default");
	qdf_mem_copy(psoc_reg->current_country,
		     psoc_reg->default_country,
		     sizeof(psoc_reg->current_country));

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_get_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
			enum dfs_reg *dfs_reg)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return;
	}

	*dfs_reg = soc_reg->dfs_region;
}

/**
 * reg_set_dfs_region () - Set the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_set_dfs_region(struct wlan_objmgr_psoc *psoc,
			enum dfs_reg dfs_reg)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return;
	}
	soc_reg->dfs_region = dfs_reg;
}

QDF_STATUS reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
					    const uint8_t *country_alpha2,
					    enum country_src source)
{
	if (NULL == reg_domain_ptr) {
		reg_err("Invalid reg domain pointer");
		return QDF_STATUS_E_FAULT;
	}

	*reg_domain_ptr = 0;

	if (NULL == country_alpha2) {
		reg_err("Country code array is NULL");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

uint32_t reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
				   uint32_t chan_num)
{
	enum channel_enum chan_enum;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *reg_channels;

	chan_enum = reg_get_chan_enum(chan_num);

	if (chan_enum == INVALID_CHANNEL) {
		reg_err("channel is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_channels = pdev_priv_obj->cur_chan_list;

	return reg_channels[chan_enum].tx_power;
}

uint32_t reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
			      uint32_t chan_num)
{
	enum channel_enum chan_enum;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *reg_channels;

	chan_enum = reg_get_chan_enum(chan_num);

	if (chan_enum == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_channels = pdev_priv_obj->cur_chan_list;

	return reg_channels[chan_enum].center_freq;
}


bool reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev,
		   uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return ch_state == CHANNEL_STATE_DFS;
}

bool reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				  uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return (ch_state == CHANNEL_STATE_DFS) ||
		(ch_state == CHANNEL_STATE_DISABLE);
}


uint32_t reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			  uint32_t freq)
{
	uint32_t count;
	struct regulatory_channel *chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	chan_list = pdev_priv_obj->cur_chan_list;

	for (count = 0; count < NUM_CHANNELS; count++)
		if (chan_list[count].center_freq == freq)
			return chan_list[count].chan_num;

	reg_err("invalid frequency %d", freq);

	return 0;
}

uint32_t reg_chan_to_freq(struct wlan_objmgr_pdev *pdev,
			  uint32_t chan_num)
{
	uint32_t count;
	struct regulatory_channel *chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	chan_list = pdev_priv_obj->cur_chan_list;

	for (count = 0; count < NUM_CHANNELS; count++)
		if (chan_list[count].chan_num == chan_num)
			return chan_list[count].center_freq;

	reg_err("invalid channel %d", chan_num);

	return 0;
}

enum band_info reg_chan_to_band(uint32_t chan_num)
{
	if (chan_num <= 14)
		return BAND_2G;

	return BAND_5G;
}

uint16_t reg_dmn_get_chanwidth_from_opclass(uint8_t *country,
					    uint8_t channel,
					    uint8_t opclass)
{
	const struct reg_dmn_op_class_map_t *class;
	uint16_t i;

	if (!qdf_mem_cmp(country, "US", 2))
		class = us_op_class;
	else if (!qdf_mem_cmp(country, "EU", 2))
		class = euro_op_class;
	else if (!qdf_mem_cmp(country, "JP", 2))
		class = japan_op_class;
	else
		class = global_op_class;

	while (class->op_class) {
		if (opclass == class->op_class) {
			for (i = 0;
			     (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
			      class->channels[i]);
			     i++) {
				if (channel == class->channels[i])
					return class->ch_spacing;
			}
		}
		class++;
	}

	return 0;
}


uint16_t reg_dmn_get_opclass_from_channel(uint8_t *country,
					  uint8_t channel,
					  uint8_t offset)
{
	const struct reg_dmn_op_class_map_t *class = NULL;
	uint16_t i = 0;

	if (!qdf_mem_cmp(country, "US", 2))
		class = us_op_class;
	else if (!qdf_mem_cmp(country, "EU", 2))
		class = euro_op_class;
	else if (!qdf_mem_cmp(country, "JP", 2))
		class = japan_op_class;
	else
		class = global_op_class;

	while (class->op_class) {
		if ((offset == class->offset) || (offset == BWALL)) {
			for (i = 0;
			     (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
			      class->channels[i]); i++) {
				if (channel == class->channels[i])
					return class->op_class;
			}
		}
		class++;
	}

	return 0;
}

uint16_t reg_dmn_set_curr_opclasses(uint8_t num_classes,
				    uint8_t *class)
{
	uint8_t i;

	if (REG_MAX_SUPP_OPER_CLASSES < num_classes) {
		reg_err("invalid num classes %d", num_classes);
		return 0;
	}

	for (i = 0; i < num_classes; i++)
		reg_dmn_curr_supp_opp_classes.classes[i] = class[i];

	reg_dmn_curr_supp_opp_classes.num_classes = num_classes;

	return 0;
}

uint16_t reg_dmn_get_curr_opclasses(uint8_t *num_classes,
				    uint8_t *class)
{
	uint8_t i;

	if (!num_classes || !class) {
		reg_err("either num_classes or class is null");
		return 0;
	}

	for (i = 0; i < reg_dmn_curr_supp_opp_classes.num_classes; i++)
		class[i] = reg_dmn_curr_supp_opp_classes.classes[i];

	*num_classes = reg_dmn_curr_supp_opp_classes.num_classes;

	return 0;
}


static void reg_fill_channel_info(enum channel_enum chan_enum,
				  struct cur_reg_rule *reg_rule,
				  struct regulatory_channel *master_list,
				  uint16_t min_bw)
{

	master_list[chan_enum].chan_flags &=
		~REGULATORY_CHAN_DISABLED;

	master_list[chan_enum].tx_power = reg_rule->reg_power;
	master_list[chan_enum].ant_gain = reg_rule->ant_gain;
	master_list[chan_enum].state = CHANNEL_STATE_ENABLE;

	if (reg_rule->flags & REGULATORY_CHAN_NO_IR) {

		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_NO_IR;

		master_list[chan_enum].state =
			CHANNEL_STATE_DFS;
	}

	if (reg_rule->flags & REGULATORY_CHAN_RADAR) {
		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_RADAR;

		master_list[chan_enum].state =
			CHANNEL_STATE_DFS;
	}

	if (reg_rule->flags & REGULATORY_CHAN_INDOOR_ONLY)
		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_INDOOR_ONLY;

	if (reg_rule->flags & REGULATORY_CHAN_NO_OFDM)
		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_NO_OFDM;

	master_list[chan_enum].min_bw = min_bw;
	if (20 == master_list[chan_enum].max_bw)
		master_list[chan_enum].max_bw = reg_rule->max_bw;
}


static void populate_band_channels(enum channel_enum start_chan,
				   enum channel_enum end_chan,
				   struct cur_reg_rule *rule_start_ptr,
				   uint32_t num_reg_rules,
				   uint16_t min_bw,
				   struct regulatory_channel *mas_chan_list)
{
	struct cur_reg_rule *found_rule_ptr;
	struct cur_reg_rule *cur_rule_ptr;
	struct regulatory_channel;
	enum channel_enum chan_enum;
	uint32_t rule_num, bw;

	for (chan_enum = start_chan; chan_enum <= end_chan; chan_enum++) {
		found_rule_ptr = NULL;
		for (bw = 20; ((bw >= 5) && (NULL == found_rule_ptr));
		     bw = bw/2) {
			for (rule_num = 0, cur_rule_ptr =
				     rule_start_ptr;
			     rule_num < num_reg_rules;
			     cur_rule_ptr++, rule_num++) {
				if ((cur_rule_ptr->start_freq <=
				     channel_map[chan_enum].center_freq -
				     bw/2) &&
				    (cur_rule_ptr->end_freq >=
				     channel_map[chan_enum].center_freq +
				     bw/2) && (min_bw <= bw)) {
					found_rule_ptr = cur_rule_ptr;
					break;
				}
			}
			break;
		}

		if (found_rule_ptr) {
			mas_chan_list[chan_enum].max_bw = bw;
			reg_fill_channel_info(chan_enum, found_rule_ptr,
					      mas_chan_list, min_bw);
		}
	}
}

static void update_max_bw_per_rule(uint32_t num_reg_rules,
				   struct cur_reg_rule *reg_rule_start,
				   uint16_t max_bw)
{
	uint32_t count;

	for (count = 0; count < num_reg_rules; count++)
		reg_rule_start[count].max_bw =
			min(reg_rule_start[count].max_bw, max_bw);
}

static void do_auto_bw_correction(uint32_t num_reg_rules,
				  struct cur_reg_rule *reg_rule_ptr,
				  uint16_t max_bw)
{
	uint32_t count;
	uint16_t new_bw;

	for (count = 0; count < num_reg_rules - 1; count++) {
		if ((reg_rule_ptr[count].end_freq ==
		     reg_rule_ptr[count+1].start_freq) &&
		    ((reg_rule_ptr[count].max_bw + reg_rule_ptr[count+1].max_bw)
		     <= max_bw)) {
			new_bw = reg_rule_ptr[count].max_bw +
				reg_rule_ptr[count+1].max_bw;
			reg_rule_ptr[count].max_bw = new_bw;
			reg_rule_ptr[count+1].max_bw = new_bw;
		}
	}
}

static void modify_chan_list_for_dfs_channels(struct regulatory_channel
					      *chan_list,
					      bool dfs_enabled)
{
	enum channel_enum chan_enum;

	if (dfs_enabled)
		return;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		if (CHANNEL_STATE_DFS == chan_list[chan_enum].state) {
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
		}
	}
}

static void modify_chan_list_for_indoor_channels(struct regulatory_channel
						 *chan_list, bool
						 indoor_chan_enabled)
{
	enum channel_enum chan_enum;

	if (indoor_chan_enabled)
		return;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		if (REGULATORY_CHAN_INDOOR_ONLY &
		    chan_list[chan_enum].chan_flags) {
			chan_list[chan_enum].state =
				CHANNEL_STATE_PASSIVE;
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_NO_IR;
		}
	}
}

static void modify_chan_list_for_band(struct regulatory_channel *chan_list,
				      enum band_info band_val)
{
	enum channel_enum chan_enum;

	if (BAND_2G == band_val) {
		for (chan_enum = MIN_5GHZ_CHANNEL;
		     chan_enum <= MAX_5GHZ_CHANNEL;
		     chan_enum++) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state = CHANNEL_STATE_DISABLE;
		}
	}

	if (BAND_5G == band_val) {
		for (chan_enum = MIN_24GHZ_CHANNEL;
		     chan_enum <= MAX_24GHZ_CHANNEL;
		     chan_enum++) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state = CHANNEL_STATE_DISABLE;
		}
	}
}

static void modify_chan_list_for_fcc_channel(struct regulatory_channel
					     *chan_list,
					     bool set_fcc_channel)
{
	if (set_fcc_channel) {
		chan_list[CHAN_ENUM_12].tx_power = MAX_PWR_FCC_CHAN_12;
		chan_list[CHAN_ENUM_13].tx_power = MAX_PWR_FCC_CHAN_13;
	}
}

static void
modify_chan_list_for_nol_list(struct regulatory_channel *chan_list)
{
	enum channel_enum chan_enum;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS;
	     chan_enum++) {
		if (chan_list[chan_enum].nol_chan) {
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
		}
	}
}

static void
modify_chan_list_for_freq_range(struct regulatory_channel *chan_list,
				uint32_t low_freq_2g,
				uint32_t high_freq_2g,
				uint32_t low_freq_5g,
				uint32_t high_freq_5g)
{
	uint32_t low_limit_2g = NUM_CHANNELS;
	uint32_t high_limit_2g = NUM_CHANNELS;
	uint32_t low_limit_5g = NUM_CHANNELS;
	uint32_t high_limit_5g = NUM_CHANNELS;
	enum channel_enum chan_enum;
	bool chan_in_range;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		if ((chan_list[chan_enum].center_freq - 10) >= low_freq_2g) {
			low_limit_2g = chan_enum;
			break;
		}
	}

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		if ((chan_list[chan_enum].center_freq - 10) >= low_freq_5g) {
			low_limit_5g = chan_enum;
			break;
		}
	}

	for (chan_enum = NUM_CHANNELS - 1; chan_enum >= 0; chan_enum--) {
		if (chan_list[chan_enum].center_freq + 10 <= high_freq_2g) {
			high_limit_2g = chan_enum;
			break;
		}
		if (chan_enum == 0)
			break;
	}

	for (chan_enum = NUM_CHANNELS - 1; chan_enum >= 0; chan_enum--) {
		if (chan_list[chan_enum].center_freq + 10 <= high_freq_5g) {
			high_limit_5g = chan_enum;
			break;
		}
		if (chan_enum == 0)
			break;
	}

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		chan_in_range = false;
		if  ((low_limit_2g <= chan_enum) &&
		     (high_limit_2g >= chan_enum) &&
		     (low_limit_2g != NUM_CHANNELS) &&
		     (high_limit_2g != NUM_CHANNELS))
			chan_in_range = true;
		if  ((low_limit_5g <= chan_enum) &&
		     (high_limit_5g >= chan_enum) &&
		     (low_limit_5g != NUM_CHANNELS) &&
		     (high_limit_5g != NUM_CHANNELS))
			chan_in_range = true;
		if (!chan_in_range) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
		}
	}
}

static void init_pdev_chan_list(struct wlan_regulatory_psoc_priv_obj
				*psoc_priv_obj,
				struct wlan_regulatory_pdev_priv_obj
				*pdev_priv_obj)
{
	qdf_mem_copy(pdev_priv_obj->cur_chan_list,
		     psoc_priv_obj->mas_chan_list,
		     NUM_CHANNELS * sizeof(struct regulatory_channel));
}


static void compute_pdev_current_chan_list(struct wlan_regulatory_pdev_priv_obj
					   *pdev_priv_obj)
{
	modify_chan_list_for_freq_range(pdev_priv_obj->cur_chan_list,
					pdev_priv_obj->range_2g_low,
					pdev_priv_obj->range_2g_high,
					pdev_priv_obj->range_5g_low,
					pdev_priv_obj->range_5g_high);

	modify_chan_list_for_band(pdev_priv_obj->cur_chan_list,
				  pdev_priv_obj->band_capability);

	modify_chan_list_for_dfs_channels(pdev_priv_obj->cur_chan_list,
					  pdev_priv_obj->dfs_enabled);

	modify_chan_list_for_nol_list(pdev_priv_obj->cur_chan_list);

	modify_chan_list_for_indoor_channels(pdev_priv_obj->cur_chan_list,
					 pdev_priv_obj->indoor_chan_enabled);

	modify_chan_list_for_fcc_channel(pdev_priv_obj->cur_chan_list,
					 pdev_priv_obj->set_fcc_channel);
}

static void call_chan_change_cbks(struct wlan_objmgr_psoc *psoc,
				  struct wlan_objmgr_pdev *pdev)
{
	struct chan_change_cbk_entry *cbk_list;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	uint32_t ctr;
	reg_chan_change_callback callback;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_alert("psoc reg component is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_alert("pdev reg component is NULL");
		return;
	}

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(*cur_chan_list));
	if (NULL == cur_chan_list) {
		reg_alert("Mem alloc failed for current channel list");
		return;
	}

	qdf_mem_copy(cur_chan_list,
		     pdev_priv_obj->cur_chan_list,
		     NUM_CHANNELS *
		     sizeof(struct regulatory_channel));

	cbk_list = psoc_priv_obj->cbk_list;

	for (ctr = 0; ctr < REG_MAX_CHAN_CHANGE_CBKS; ctr++) {
		callback  = NULL;
		qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
		if (cbk_list[ctr].cbk != NULL)
			callback = cbk_list[ctr].cbk;
		qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);
		if (callback != NULL)
			callback(psoc, pdev, cur_chan_list,
				 cbk_list[ctr].arg);
	}
	qdf_mem_free(cur_chan_list);
}

static struct reg_sched_payload
*reg_alloc_and_fill_payload(struct wlan_objmgr_psoc *psoc,
			    struct wlan_objmgr_pdev *pdev)
{
	struct reg_sched_payload *payload;

	payload = qdf_mem_malloc(sizeof(*payload));
	if (payload != NULL) {
		payload->psoc = psoc;
		payload->pdev = pdev;
	}

	return payload;
}

static QDF_STATUS chan_change_flush_cbk_sb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS sched_chan_change_cbks_sb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	call_chan_change_cbks(psoc, pdev);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS chan_change_flush_cbk_nb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS sched_chan_change_cbks_nb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	call_chan_change_cbks(psoc, pdev);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS send_scheduler_msg_sb(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev)
{
	struct scheduler_msg msg = {0};
	struct reg_sched_payload *payload;
	QDF_STATUS status;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		reg_err("error taking pdev ref cnt");
		return status;
	}

	payload = reg_alloc_and_fill_payload(psoc, pdev);
	if (payload == NULL) {
		reg_err("payload memory alloc failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		return QDF_STATUS_E_NOMEM;
	}
	msg.bodyptr = payload;
	msg.callback = sched_chan_change_cbks_sb;
	msg.flush_callback = chan_change_flush_cbk_sb;

	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		reg_err("scheduler msg posting failed");
		qdf_mem_free(payload);
	}

	return status;
}

static QDF_STATUS send_scheduler_msg_nb(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev)
{
	struct scheduler_msg msg = {0};
	struct reg_sched_payload *payload;
	QDF_STATUS status;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_REGULATORY_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		reg_err("error taking pdev ref cnt");
		return status;
	}

	payload = reg_alloc_and_fill_payload(psoc, pdev);
	if (payload == NULL) {
		reg_err("payload memory alloc failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		return QDF_STATUS_E_NOMEM;
	}
	msg.bodyptr = payload;
	msg.callback = sched_chan_change_cbks_nb;
	msg.flush_callback = chan_change_flush_cbk_nb;

	status = scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		reg_err("scheduler msg posting failed");
		qdf_mem_free(payload);
	}

	return status;
}

static void reg_propagate_mas_chan_list_to_pdev(struct wlan_objmgr_psoc *psoc,
						void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	enum direction *dir = arg;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				       WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	init_pdev_chan_list(psoc_priv_obj,
			    pdev_priv_obj);

	compute_pdev_current_chan_list(pdev_priv_obj);

	if (*dir == NORTHBOUND)
		send_scheduler_msg_nb(psoc, pdev);
	else
		send_scheduler_msg_sb(psoc, pdev);
}

QDF_STATUS reg_process_master_chan_list(struct cur_regulatory_info
					*regulat_info)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;
	struct cur_reg_rule *reg_rule_2g, *reg_rule_5g;
	uint16_t min_bw_2g, max_bw_2g, min_bw_5g, max_bw_5g;
	struct regulatory_channel *mas_chan_list;
	struct wlan_objmgr_psoc *psoc;
	enum channel_enum chan_enum;
	QDF_STATUS status;
	wlan_objmgr_ref_dbgid dbg_id;
	enum direction dir;

	reg_debug("process reg master chan list");

	if (regulat_info->status_code == REG_CURRENT_ALPHA2_NOT_FOUND) {
		reg_err("current country not present in regdb");
		soc_reg->new_user_ctry_pending = false;
		return QDF_STATUS_SUCCESS;
	}

	psoc = regulat_info->psoc;
	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mas_chan_list = soc_reg->mas_chan_list;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS;
	     chan_enum++) {
		mas_chan_list[chan_enum].chan_num =
			channel_map[chan_enum].chan_num;
		mas_chan_list[chan_enum].center_freq =
			channel_map[chan_enum].center_freq;
		mas_chan_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_DISABLED;
		mas_chan_list[chan_enum].state =
			CHANNEL_STATE_DISABLE;
		mas_chan_list[chan_enum].nol_chan = false;
	}

	soc_reg->offload_enabled = regulat_info->offload_enabled;
	soc_reg->phybitmap = regulat_info->phybitmap;
	soc_reg->dfs_region = regulat_info->dfs_region;
	soc_reg->num_phy = regulat_info->num_phy;
	soc_reg->ctry_code = regulat_info->ctry_code;
	soc_reg->reg_dmn_pair = regulat_info->reg_dmn_pair;
	qdf_mem_copy(soc_reg->default_country, regulat_info->alpha2,
		     REG_ALPHA2_LEN + 1);
	qdf_mem_copy(soc_reg->current_country, regulat_info->alpha2,
		     REG_ALPHA2_LEN + 1);

	min_bw_2g = regulat_info->min_bw_2g;
	max_bw_2g = regulat_info->max_bw_2g;
	reg_rule_2g = regulat_info->reg_rules_2g_ptr;
	num_2g_reg_rules = regulat_info->num_2g_reg_rules;
	update_max_bw_per_rule(num_2g_reg_rules,
			       reg_rule_2g, max_bw_2g);

	min_bw_5g = regulat_info->min_bw_5g;
	max_bw_5g = regulat_info->max_bw_5g;
	reg_rule_5g = regulat_info->reg_rules_5g_ptr;
	num_5g_reg_rules = regulat_info->num_5g_reg_rules;
	update_max_bw_per_rule(num_5g_reg_rules,
			       reg_rule_5g, max_bw_5g);

	if (num_5g_reg_rules != 0)
		do_auto_bw_correction(num_5g_reg_rules,
				      reg_rule_5g, max_bw_5g);

	if (num_2g_reg_rules != 0)
		populate_band_channels(MIN_24GHZ_CHANNEL, MAX_24GHZ_CHANNEL,
				       reg_rule_2g, num_2g_reg_rules,
				       min_bw_2g, mas_chan_list);

	if (num_5g_reg_rules != 0)
		populate_band_channels(MIN_5GHZ_CHANNEL, MAX_5GHZ_CHANNEL,
				       reg_rule_5g,
				       num_5g_reg_rules,
				       min_bw_5g, mas_chan_list);

	soc_reg->cc_src = SOURCE_DRIVER;
	if (soc_reg->new_user_ctry_pending == true) {
		soc_reg->new_user_ctry_pending = false;
		soc_reg->cc_src = SOURCE_USERSPACE;

	} else if (soc_reg->new_11d_ctry_pending == true) {
		soc_reg->new_11d_ctry_pending = false;
		soc_reg->cc_src = SOURCE_11D;
	}

	if (soc_reg->offload_enabled) {
		dbg_id = WLAN_REGULATORY_NB_ID;
		dir = NORTHBOUND;
	} else {
		dbg_id = WLAN_REGULATORY_SB_ID;
		dir = SOUTHBOUND;
	}

	status = wlan_objmgr_psoc_try_get_ref(psoc, dbg_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					 reg_propagate_mas_chan_list_to_pdev,
					 &dir, 1, dbg_id);
	wlan_objmgr_psoc_release_ref(psoc, dbg_id);

	return status;
}

/**
 * wlan_regulatory_psoc_obj_created_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg_obj;
	struct regulatory_channel *mas_chan_list;
	enum channel_enum chan_enum;
	QDF_STATUS status;

	soc_reg_obj = qdf_mem_malloc(sizeof(*soc_reg_obj));
	if (NULL == soc_reg_obj) {
		reg_alert("Mem alloc failed for reg psoc priv obj");
		return QDF_STATUS_E_NOMEM;
	}

	soc_reg_obj->offload_enabled = false;
	soc_reg_obj->psoc_ptr = psoc;
	soc_reg_obj->dfs_enabled = true;
	soc_reg_obj->set_fcc_channel = false;
	soc_reg_obj->band_capability = BAND_ALL;
	soc_reg_obj->indoor_chan_enabled = true;
	qdf_spinlock_create(&soc_reg_obj->cbk_list_lock);

	mas_chan_list = soc_reg_obj->mas_chan_list;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS;
	     chan_enum++) {
		mas_chan_list[chan_enum].chan_num =
			channel_map[chan_enum].chan_num;
		mas_chan_list[chan_enum].center_freq =
			channel_map[chan_enum].center_freq;
		mas_chan_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_DISABLED;
		mas_chan_list[chan_enum].state =
			CHANNEL_STATE_DISABLE;
		mas_chan_list[chan_enum].nol_chan = false;
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
			WLAN_UMAC_COMP_REGULATORY, soc_reg_obj,
			QDF_STATUS_SUCCESS);

	reg_debug("reg psoc obj created with status %d", status);

	return status;
}

/**
 * wlan_regulatory_psoc_obj_destroyed_notification() - PSOC obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is deleted.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_psoc_obj_destroyed_notification(
	struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	wlan_psoc_obj_lock(psoc);
	soc_reg = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					    WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	soc_reg->psoc_ptr = NULL;
	qdf_spinlock_destroy(&soc_reg->cbk_list_lock);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
			WLAN_UMAC_COMP_REGULATORY,
			soc_reg);
	if (status != QDF_STATUS_SUCCESS)
		reg_err("soc_reg private obj detach failed");

	reg_debug("reg psoc obj detached with status %d", status);

	qdf_mem_free(soc_reg);

	return status;
}

QDF_STATUS reg_set_band(struct wlan_objmgr_pdev *pdev,
			enum band_info band)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->band_capability == band) {
		reg_info("band is already set to %d", band);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting band_info: %d", band);
	pdev_priv_obj->band_capability = band;

	init_pdev_chan_list(psoc_priv_obj,
			    pdev_priv_obj);

	compute_pdev_current_chan_list(pdev_priv_obj);

	status = send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_set_fcc_constraint(struct wlan_objmgr_pdev *pdev,
				  bool fcc_constraint)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->set_fcc_channel == fcc_constraint) {
		reg_info("fcc_constraint is already set to %d", fcc_constraint);
		return QDF_STATUS_SUCCESS;
	}

	reg_info("setting set_fcc_channel: %d", fcc_constraint);
	pdev_priv_obj->set_fcc_channel = fcc_constraint;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	init_pdev_chan_list(psoc_priv_obj,
			    pdev_priv_obj);

	compute_pdev_current_chan_list(pdev_priv_obj);

	status = send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_enable_dfs_channels(struct wlan_objmgr_pdev *pdev,
				   bool enable)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->dfs_enabled == enable) {
		reg_info("dfs_enabled is already set to %d", enable);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting dfs_enabled: %d", enable);

	pdev_priv_obj->dfs_enabled = enable;

	init_pdev_chan_list(psoc_priv_obj,
			    pdev_priv_obj);

	compute_pdev_current_chan_list(pdev_priv_obj);

	status = send_scheduler_msg_sb(psoc, pdev);

	return status;
}

/**
 * wlan_regulatory_pdev_obj_created_notification() - PDEV obj create callback
 * @pdev: pdev object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the pdev object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_pdev_obj_created_notification(
	struct wlan_objmgr_pdev *pdev, void *arg_list)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap_ptr;
	struct wlan_objmgr_psoc *parent_psoc;
	uint32_t pdev_id;
	uint32_t cnt;
	uint32_t range_2g_low, range_2g_high,
		range_5g_low, range_5g_high;
	QDF_STATUS status;

	pdev_priv_obj = qdf_mem_malloc(sizeof(*pdev_priv_obj));
	if (NULL == pdev_priv_obj) {
		reg_alert("Mem alloc failed for pdev priv obj");
		return QDF_STATUS_E_NOMEM;
	}

	parent_psoc = wlan_pdev_get_psoc(pdev);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	wlan_psoc_obj_lock(parent_psoc);
	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(parent_psoc,
				     WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(parent_psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	pdev_priv_obj->pdev_ptr = pdev;
	pdev_priv_obj->dfs_enabled = psoc_priv_obj->dfs_enabled;
	pdev_priv_obj->set_fcc_channel = psoc_priv_obj->set_fcc_channel;
	pdev_priv_obj->band_capability =  psoc_priv_obj->band_capability;
	pdev_priv_obj->indoor_chan_enabled =
		psoc_priv_obj->indoor_chan_enabled;

	reg_cap_ptr = parent_psoc->ext_service_param.reg_cap;

	for (cnt = 0; cnt < PSOC_MAX_PHY_REG_CAP; cnt++) {
		if (reg_cap_ptr == NULL) {
			reg_err(" reg cap ptr is NULL");
			return QDF_STATUS_E_FAULT;
		}

		if (reg_cap_ptr->phy_id == pdev_id)
			break;
		reg_cap_ptr++;
	}

	if (cnt == PSOC_MAX_PHY_REG_CAP) {
		reg_err("extended capabilities not found for pdev");
		return QDF_STATUS_E_FAULT;
	}

	range_2g_low = reg_cap_ptr->low_2ghz_chan;
	range_2g_high = reg_cap_ptr->high_2ghz_chan;
	range_5g_low = reg_cap_ptr->low_5ghz_chan;
	range_5g_high = reg_cap_ptr->high_5ghz_chan;

	pdev_priv_obj->range_2g_low = range_2g_low;
	pdev_priv_obj->range_2g_high = range_2g_high;
	pdev_priv_obj->range_5g_low = range_5g_low;
	pdev_priv_obj->range_5g_high = range_5g_high;

	init_pdev_chan_list(psoc_priv_obj,
			    pdev_priv_obj);

	compute_pdev_current_chan_list(pdev_priv_obj);

	status = send_scheduler_msg_sb(parent_psoc, pdev);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
						     WLAN_UMAC_COMP_REGULATORY,
						     pdev_priv_obj,
						     QDF_STATUS_SUCCESS);

	reg_debug("reg pdev obj created with status %d", status);

	return status;
}

QDF_STATUS wlan_regulatory_pdev_obj_destroyed_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj->pdev_ptr = NULL;

	status = wlan_objmgr_pdev_component_obj_detach(pdev,
			WLAN_UMAC_COMP_REGULATORY,
			pdev_priv_obj);

	if (status != QDF_STATUS_SUCCESS)
		reg_err("reg pdev private obj detach failed");

	reg_debug("reg pdev obj deleted with status %d", status);

	qdf_mem_free(pdev_priv_obj);

	return status;
}

QDF_STATUS reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
			  struct regulatory_channel *chan_list)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(chan_list, pdev_priv_obj->cur_chan_list,
		     NUM_CHANNELS * sizeof(struct regulatory_channel));

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_update_nol_ch () - Updates NOL channels in current channel list
 * @pdev: pointer to pdev object
 * @ch_list: pointer to NOL channel list
 * @num_ch: No.of channels in list
 * @update_nol: set/reset the NOL status
 *
 * Return: None
 */
void reg_update_nol_ch(struct wlan_objmgr_pdev *pdev,
		       uint8_t *chan_list,
		       uint8_t num_chan,
		       bool nol_chan)
{
	enum channel_enum chan_enum;
	struct regulatory_channel *mas_chan_list;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint16_t i;
	enum direction dir;
	QDF_STATUS status;

	if (!num_chan || !chan_list) {
		reg_err("chan_list or num_ch is NULL");
		return;
	}

	psoc = wlan_pdev_get_psoc(pdev);

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						  WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	mas_chan_list = psoc_priv_obj->mas_chan_list;
	for (i = 0; i < num_chan; i++) {
		chan_enum = reg_get_chan_enum(chan_list[i]);
		if (chan_enum == INVALID_CHANNEL) {
			reg_err("Invalid ch in nol list, chan %d",
				chan_list[i]);
			continue;
		}
		mas_chan_list[chan_enum].nol_chan = nol_chan;
	}

	dir =  SOUTHBOUND;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					  reg_propagate_mas_chan_list_to_pdev,
					  &dir, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
}

static void reg_change_pdev_for_config(struct wlan_objmgr_psoc *psoc,
				       void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
					     WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return;
	}

	pdev_priv_obj->dfs_enabled =
		psoc_priv_obj->dfs_enabled;
	pdev_priv_obj->indoor_chan_enabled =
		psoc_priv_obj->indoor_chan_enabled;
	pdev_priv_obj->band_capability = psoc_priv_obj->band_capability;

	init_pdev_chan_list(psoc_priv_obj,
			    pdev_priv_obj);

	compute_pdev_current_chan_list(pdev_priv_obj);

	send_scheduler_msg_sb(psoc, pdev);
}

QDF_STATUS reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
			    struct reg_config_vars config_vars)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				       WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj->enable_11d_supp_original =
		config_vars.enable_11d_support;
	psoc_priv_obj->enable_11d_supp_current =
		config_vars.enable_11d_support;
	psoc_priv_obj->userspace_country_priority =
		config_vars.userspace_ctry_priority;
	psoc_priv_obj->dfs_enabled =
		config_vars.dfs_enabled;
	psoc_priv_obj->indoor_chan_enabled =
		config_vars.indoor_chan_enabled;
	psoc_priv_obj->band_capability = config_vars.band_capability;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					      reg_change_pdev_for_config,
					      NULL, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);

	return status;
}

bool reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return ch_state == CHANNEL_STATE_DISABLE;
}

bool reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return false;
	}

	return psoc_priv_obj->offload_enabled;
}

void reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
				    struct regulatory_channel *reg_channels,
				    uint8_t *alpha2,
				    enum dfs_reg dfs_region)
{

	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;
	uint32_t count;
	enum direction dir;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	psoc_priv_obj->dfs_region = dfs_region;
	qdf_mem_copy(psoc_priv_obj->current_country, alpha2,
		     REG_ALPHA2_LEN);

	for (count = 0; count < NUM_CHANNELS; count++) {
		reg_channels[count].chan_num =
			channel_map[count].chan_num;
		reg_channels[count].center_freq =
			channel_map[count].center_freq;
		reg_channels[count].nol_chan = false;
	}

	qdf_mem_copy(psoc_priv_obj->mas_chan_list,
		     reg_channels,
		     NUM_CHANNELS * sizeof(struct regulatory_channel));

	dir = SOUTHBOUND;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					  reg_propagate_mas_chan_list_to_pdev,
					  &dir, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
}


void reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
				       reg_chan_change_callback cbk,
				       void *arg)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint32_t count;


	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
	for (count = 0; count < REG_MAX_CHAN_CHANGE_CBKS; count++)
		if (psoc_priv_obj->cbk_list[count].cbk == NULL) {
			psoc_priv_obj->cbk_list[count].cbk = cbk;
			psoc_priv_obj->cbk_list[count].arg = arg;
			psoc_priv_obj->num_chan_change_cbks++;
			break;
		}
	qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);

	if (count == REG_MAX_CHAN_CHANGE_CBKS)
		reg_err("callback list is full, could not add the cbk");
}

void reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
				       reg_chan_change_callback cbk)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint32_t count;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
	for (count = 0; count < REG_MAX_CHAN_CHANGE_CBKS; count++)
		if (psoc_priv_obj->cbk_list[count].cbk == cbk) {
			psoc_priv_obj->cbk_list[count].cbk = NULL;
			psoc_priv_obj->num_chan_change_cbks--;
			break;
		}
	qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);

	if (count == REG_MAX_CHAN_CHANGE_CBKS)
		reg_err("callback not found in the list");
}

enum country_src reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
				    uint8_t *alpha2)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	wlan_psoc_obj_lock(psoc);
	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);
	wlan_psoc_obj_unlock(psoc);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return false;
	}

	qdf_mem_copy(alpha2, psoc_priv_obj->current_country,
		     REG_ALPHA2_LEN + 1);

	return psoc_priv_obj->cc_src;
}

QDF_STATUS reg_program_default_cc(struct wlan_objmgr_psoc *psoc,
		uint16_t regdmn)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	struct cur_regulatory_info *reg_info;
	uint16_t cc = -1;
	uint16_t country_index = -1, regdmn_pair = -1;
	QDF_STATUS err;

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("reg soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (soc_reg->offload_enabled)
		return QDF_STATUS_E_FAILURE;

	reg_info = (struct cur_regulatory_info *)qdf_mem_malloc
		(sizeof(struct cur_regulatory_info));
	if (reg_info == NULL) {
		reg_err("reg info is NULL");
		return QDF_STATUS_E_NOMEM;
	}

	reg_info->psoc = psoc;

	if (regdmn == 0) {
		reg_get_default_country(&regdmn);
		regdmn |= COUNTRY_ERD_FLAG;
	}

	if (regdmn & COUNTRY_ERD_FLAG) {
		cc = regdmn & ~COUNTRY_ERD_FLAG;

		reg_get_rdpair_from_country_code(cc,
				&country_index,
				&regdmn_pair);

		err = reg_get_cur_reginfo(reg_info, country_index, regdmn_pair);
		if (err == QDF_STATUS_E_FAILURE) {
			reg_err("%s : Unable to set country code\n", __func__);
			qdf_mem_free(reg_info->reg_rules_2g_ptr);
			qdf_mem_free(reg_info->reg_rules_5g_ptr);
			qdf_mem_free(reg_info);
			return QDF_STATUS_E_FAILURE;
		}

		soc_reg->ctry_code = cc;

	} else {
		reg_get_rdpair_from_regdmn_id(regdmn,
				&regdmn_pair);

		err = reg_get_cur_reginfo(reg_info, country_index, regdmn_pair);
		if (err == QDF_STATUS_E_FAILURE) {
			reg_err("%s : Unable to set country code\n", __func__);
			qdf_mem_free(reg_info->reg_rules_2g_ptr);
			qdf_mem_free(reg_info->reg_rules_5g_ptr);
			qdf_mem_free(reg_info);
			return QDF_STATUS_E_FAILURE;
		}

		soc_reg->reg_dmn_pair = regdmn;
	}

	reg_info->offload_enabled = false;
	reg_process_master_chan_list(reg_info);

	qdf_mem_free(reg_info->reg_rules_2g_ptr);
	qdf_mem_free(reg_info->reg_rules_5g_ptr);
	qdf_mem_free(reg_info);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_program_chan_list(struct wlan_objmgr_psoc *psoc,
		struct cc_regdmn_s *rd)
{
	struct cur_regulatory_info *reg_info;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	uint16_t country_index = -1, regdmn_pair = -1;
	QDF_STATUS err;

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("soc_reg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_info = (struct cur_regulatory_info *)qdf_mem_malloc
		(sizeof(struct cur_regulatory_info));
	if (reg_info == NULL) {
		reg_err("reg info is NULL");
		return QDF_STATUS_E_NOMEM;
	}

	reg_info->psoc = psoc;

	if (rd->flags == CC_IS_SET) {
		reg_get_rdpair_from_country_code(rd->cc.country_code,
				&country_index,
				&regdmn_pair);
	} else if (rd->flags == ALPHA_IS_SET) {
		reg_get_rdpair_from_country_iso(rd->cc.alpha,
				&country_index,
				&regdmn_pair);
	} else if (rd->flags == REGDMN_IS_SET) {
		reg_get_rdpair_from_regdmn_id(rd->cc.regdmn_id,
				&regdmn_pair);
	}

	err = reg_get_cur_reginfo(reg_info, country_index, regdmn_pair);
	if (err == QDF_STATUS_E_FAILURE) {
		reg_err("%s : Unable to set country code\n", __func__);
		qdf_mem_free(reg_info->reg_rules_2g_ptr);
		qdf_mem_free(reg_info->reg_rules_5g_ptr);
		qdf_mem_free(reg_info);
		return QDF_STATUS_E_FAILURE;
	}

	if (rd->flags == CC_IS_SET ||
			rd->flags == ALPHA_IS_SET) {
		soc_reg->ctry_code =
			g_all_countries[country_index].country_code;
	}

	if (rd->flags == REGDMN_IS_SET)
		soc_reg->reg_dmn_pair = rd->cc.regdmn_id;

	reg_info->offload_enabled = false;
	reg_process_master_chan_list(reg_info);

	qdf_mem_free(reg_info->reg_rules_2g_ptr);
	qdf_mem_free(reg_info->reg_rules_5g_ptr);
	qdf_mem_free(reg_info);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_current_cc(struct wlan_objmgr_psoc *psoc,
		struct cc_regdmn_s *rd)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("reg soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (rd->flags == CC_IS_SET) {
		rd->cc.country_code = soc_reg->ctry_code;
	} else if (rd->flags == ALPHA_IS_SET) {
		qdf_mem_copy(rd->cc.alpha, soc_reg->current_country,
				sizeof(rd->cc.alpha));
	} else if (rd->flags == REGDMN_IS_SET) {
		rd->cc.regdmn_id = soc_reg->reg_dmn_pair;
	}

	return QDF_STATUS_SUCCESS;
}
