/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: reg_opclass.c
 * This file defines regulatory opclass functions.
 */

#include <qdf_types.h>
#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include "reg_priv_objs.h"
#include "reg_utils.h"
#include "reg_db.h"
#include "reg_db_parser.h"
#include "reg_host_11d.h"
#include <scheduler_api.h>
#include "reg_build_chan_list.h"
#include "reg_opclass.h"
#include "reg_services_common.h"

#ifdef HOST_OPCLASS
static struct reg_dmn_supp_op_classes reg_dmn_curr_supp_opp_classes = { 0 };
#endif

static const struct reg_dmn_op_class_map_t global_op_class[] = {
	{81, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{82, 25, BW20, BIT(BEHAV_NONE), 2414,
	 {14} },
	{83, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9} },
	{84, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{115, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48} },
	{116, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44} },
	{117, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48} },
	{118, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64} },
	{119, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60} },
	{120, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64} },
	{121, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144} },
	{122, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132, 140} },
	{123, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136, 144} },
	{125, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165, 169} },
	{126, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {149, 157} },
	{127, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {153, 161} },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64,
	  100, 104, 108, 112, 116, 120, 124,
	  128, 132, 136, 140, 144,
	  149, 153, 157, 161} },

#ifdef CONFIG_BAND_6GHZ
	{131, 20, BW20, BIT(BEHAV_NONE), 5940,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33,
	  37, 41, 45, 49, 53, 57, 61, 65, 69,
	  73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125,
	  129, 133, 137, 141, 145, 149, 153,
	  157, 161, 165, 169, 173, 177, 181,
	  185, 189, 193, 197, 201, 205, 209,
	  213, 217, 221, 225, 229, 233} },

	{132, 40, BW40_LOW_PRIMARY, BIT(BEHAV_NONE), 5940,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49,
	  53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125, 129, 133, 137,
	  141, 145, 149, 153, 157, 161, 165, 169, 173, 177,
	  181, 185, 189, 193, 197, 201, 205, 209, 213, 217,
	  221, 225, 229, 233} },

	{133, 80, BW80, BIT(BEHAV_NONE), 5940,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49,
	  53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125, 129, 133, 137,
	  141, 145, 149, 153, 157, 161, 165, 169, 173,
	  177, 181, 185, 189, 193, 197, 201, 205, 209, 213,
	  217, 221, 225, 229, 233} },

	{134, 160, BW80, BIT(BEHAV_NONE), 5940,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45,
	  49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89,
	  93, 97, 101, 105, 109, 113, 117, 121, 125,
	  129, 133, 137, 141, 145, 149, 153, 157, 161,
	  165, 169, 173, 177, 181, 185, 189, 193, 197,
	  201, 205, 209, 213, 217, 221, 225, 229, 233} },

	{135, 80, BW80, BIT(BEHAV_BW80_PLUS), 5940,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41,
	  45, 49, 53, 57, 61, 65, 69, 73, 77, 81,
	  85, 89, 93, 97, 101, 105, 109, 113, 117,
	  121, 125, 129, 133, 137, 141, 145, 149,
	  153, 157, 161, 165, 169, 173, 177, 181,
	  185, 189, 193, 197, 201, 205, 209, 213,
	  217, 221, 225, 229, 233} },
#endif
	{0, 0, 0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t us_op_class[] = {
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48} },
	{2, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64} },
	{4, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144} },
	{5, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165} },
	{12, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11} },
	{22, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44} },
	{23, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60} },
	{24, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132} },
	{26, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {149, 157} },
	{27, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48} },
	{28, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64} },
	{29, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136} },
	{30, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {153, 161} },
	{31, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {153, 161} },
	{32, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7} },
	{33, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11} },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100,
	  104, 108, 112, 116, 120, 124, 128, 132,
	  136, 140, 144, 149, 153, 157, 161} },
	{0, 0, 0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t euro_op_class[] = {
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48} },
	{2, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64} },
	{3, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120,
	  124, 128, 132, 136, 140} },
	{4, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{5, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44} },
	{6, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60} },
	{7, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132} },
	{8, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48} },
	{9, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64} },
	{10, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136} },
	{11, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9} },
	{12, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{17, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165, 169} },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120,
	  124, 128} },
	{0, 0, 0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t japan_op_class[] = {
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48} },
	{30, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{31, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {14} },
	{32, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64} },
	{34, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{36, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44} },
	{37, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60} },
	{39, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132} },
	{41, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48} },
	{42, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64} },
	{44, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136} },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120,
	  124, 128} },
	{0, 0, 0, 0, 0, {0} },
};

#ifdef HOST_OPCLASS
/**
 * reg_get_class_from_country()- Get Class from country
 * @country- Country
 *
 * Return: class.
 */
static const
struct reg_dmn_op_class_map_t *reg_get_class_from_country(uint8_t *country)
{
	const struct reg_dmn_op_class_map_t *class = NULL;

	qdf_debug("Country %c%c 0x%x",
		  country[0], country[1], country[2]);

	switch (country[2]) {
	case OP_CLASS_US:
		class = us_op_class;
		break;

	case OP_CLASS_EU:
		class = euro_op_class;
		break;

	case OP_CLASS_JAPAN:
		class = japan_op_class;
		break;

	case OP_CLASS_GLOBAL:
		class = global_op_class;
		break;

	default:
		if (!qdf_mem_cmp(country, "US", 2))
			class = us_op_class;
		else if (!qdf_mem_cmp(country, "EU", 2))
			class = euro_op_class;
		else if (!qdf_mem_cmp(country, "JP", 2))
			class = japan_op_class;
		else
			class = global_op_class;
	}
	return class;
}

uint16_t reg_dmn_get_chanwidth_from_opclass(uint8_t *country, uint8_t channel,
					    uint8_t opclass)
{
	const struct reg_dmn_op_class_map_t *class;
	uint16_t i;

	class = reg_get_class_from_country(country);

	while (class->op_class) {
		if (opclass == class->op_class) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     class->channels[i]); i++) {
				if (channel == class->channels[i])
					return class->chan_spacing;
			}
		}
		class++;
	}

	return 0;
}

uint16_t reg_dmn_get_opclass_from_channel(uint8_t *country, uint8_t channel,
					  uint8_t offset)
{
	const struct reg_dmn_op_class_map_t *class = NULL;
	uint16_t i = 0;

	class = reg_get_class_from_country(country);
	while (class && class->op_class) {
		if ((offset == class->offset) || (offset == BWALL)) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     class->channels[i]); i++) {
				if (channel == class->channels[i])
					return class->op_class;
			}
		}
		class++;
	}

	return 0;
}

void reg_dmn_print_channels_in_opclass(uint8_t *country, uint8_t op_class)
{
	const struct reg_dmn_op_class_map_t *class = NULL;
	uint16_t i = 0;

	class = reg_get_class_from_country(country);

	if (!class) {
		reg_err("class is NULL");
		return;
	}

	while (class->op_class) {
		if (class->op_class == op_class) {
			for (i = 0;
			     (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
			      class->channels[i]); i++) {
				reg_debug("Valid channel(%d) in requested RC(%d)",
					  class->channels[i], op_class);
			}
			break;
		}
		class++;
	}
	if (!class->op_class)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Invalid requested RC (%d)", op_class);
}

uint16_t reg_dmn_set_curr_opclasses(uint8_t num_classes, uint8_t *class)
{
	uint8_t i;

	if (num_classes > REG_MAX_SUPP_OPER_CLASSES) {
		reg_err("invalid num classes %d", num_classes);
		return 0;
	}

	for (i = 0; i < num_classes; i++)
		reg_dmn_curr_supp_opp_classes.classes[i] = class[i];

	reg_dmn_curr_supp_opp_classes.num_classes = num_classes;

	return 0;
}

uint16_t reg_dmn_get_curr_opclasses(uint8_t *num_classes, uint8_t *class)
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

#ifdef CONFIG_CHAN_FREQ_API
void reg_freq_width_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq,
				     uint16_t chan_width,
				     bool global_tbl_lookup,
				     uint16_t behav_limit,
				     uint8_t *op_class,
				     uint8_t *chan_num)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;
	enum channel_enum chan_enum;
	uint16_t i;

	chan_enum = reg_get_chan_enum_for_freq(freq);

	if (chan_enum == INVALID_CHANNEL) {
		reg_err(" channel enumeration is invalid %d", chan_enum);
		return;
	}

	if (global_tbl_lookup) {
		op_class_tbl = global_op_class;
	} else {
		if (channel_map == channel_map_us)
			op_class_tbl = us_op_class;
		else if (channel_map == channel_map_eu)
			op_class_tbl = euro_op_class;
		else if (channel_map == channel_map_china)
			op_class_tbl = us_op_class;
		else if (channel_map == channel_map_jp)
			op_class_tbl = japan_op_class;
		else
			op_class_tbl = global_op_class;
	}

	while (op_class_tbl->op_class) {
		if (op_class_tbl->chan_spacing >= chan_width) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     op_class_tbl->channels[i]); i++) {
				if ((op_class_tbl->start_freq +
				     FREQ_TO_CHAN_SCALE *
				     op_class_tbl->channels[i] == freq) &&
				    (behav_limit & op_class_tbl->behav_limit ||
				     behav_limit == BIT(BEHAV_NONE))) {
					*chan_num = op_class_tbl->channels[i];
					*op_class = op_class_tbl->op_class;
					return;
				}
			}
		}
		op_class_tbl++;
	}

	reg_err_rl("invalid frequency %d", freq);
}

void reg_freq_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
			       qdf_freq_t freq,
			       bool global_tbl_lookup,
			       uint16_t behav_limit,
			       uint8_t *op_class,
			       uint8_t *chan_num)
{
	enum channel_enum chan_enum;
	uint16_t chan_width;
	struct regulatory_channel *cur_chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	chan_enum = reg_get_chan_enum_for_freq(freq);

	if (chan_enum == INVALID_CHANNEL) {
		reg_err(" channel enumeration is invalid %d", chan_enum);
		return;
	}

	chan_width = cur_chan_list[chan_enum].max_bw;

	reg_freq_width_to_chan_op_class(pdev, freq,
					chan_width,
					global_tbl_lookup,
					behav_limit,
					op_class,
					chan_num);
}
#endif

uint16_t reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
				uint8_t op_class,
				bool global_tbl_lookup)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;

	if (global_tbl_lookup) {
		op_class_tbl = global_op_class;
	} else {
		if (channel_map == channel_map_us)
			op_class_tbl = us_op_class;
		else if (channel_map == channel_map_eu)
			op_class_tbl = euro_op_class;
		else if (channel_map == channel_map_china)
			op_class_tbl = us_op_class;
		else if (channel_map == channel_map_jp)
			op_class_tbl = japan_op_class;
		else
			op_class_tbl = global_op_class;
	}

	while (op_class_tbl->op_class) {
		if  (op_class_tbl->op_class == op_class)
			return op_class_tbl->chan_spacing;
		op_class_tbl++;
	}

	return 0;
}

uint16_t reg_chan_opclass_to_freq(uint8_t chan,
				  uint8_t op_class,
				  bool global_tbl_lookup)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl = NULL;
	uint8_t i = 0;

	if (global_tbl_lookup) {
		op_class_tbl = global_op_class;
	} else {
		if (channel_map == channel_map_global) {
			op_class_tbl = global_op_class;
		} else if (channel_map == channel_map_us) {
			op_class_tbl = us_op_class;
		} else if (channel_map == channel_map_eu) {
			op_class_tbl = euro_op_class;
		} else if (channel_map == channel_map_china) {
			op_class_tbl = us_op_class;
		} else if (channel_map == channel_map_jp) {
			op_class_tbl = japan_op_class;
		} else {
			reg_err_rl("Invalid channel map");
			return 0;
		}
	}

	while (op_class_tbl->op_class) {
		if  (op_class_tbl->op_class == op_class) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     op_class_tbl->channels[i]); i++) {
				if (op_class_tbl->channels[i] == chan) {
					chan = op_class_tbl->channels[i];
					return (op_class_tbl->start_freq +
						(chan * FREQ_TO_CHAN_SCALE));
				}
			}
			reg_err_rl("Channel not found");
			return 0;
		}
		op_class_tbl++;
	}
	reg_err_rl("Invalid opclass given as input");
	return 0;
}
#endif
