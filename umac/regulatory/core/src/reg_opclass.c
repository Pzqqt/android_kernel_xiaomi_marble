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

#ifdef HOST_OPCLASS
static struct reg_dmn_supp_op_classes reg_dmn_curr_supp_opp_classes = { 0 };
#endif

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

#ifdef HOST_OPCLASS
/**
 * reg_get_class_from_country()- Get Class from country
 * @country- Country
 *
 * Return: class.
 */
static
const struct reg_dmn_op_class_map_t *reg_get_class_from_country(uint8_t
								   *country)
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
					return class->ch_spacing;
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
#endif
