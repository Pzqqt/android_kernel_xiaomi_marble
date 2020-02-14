/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: reg_db_parser.c
 * This file provides regulatory data base parser functions.
 */

#include <qdf_types.h>
#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include "reg_db.h"
#include "reg_db_parser.h"
#include <qdf_mem.h>
#include <wlan_objmgr_psoc_obj.h>
#include "reg_priv_objs.h"
#include "reg_utils.h"

QDF_STATUS reg_is_country_code_valid(uint8_t *alpha2)
{
	uint16_t i;
	int num_countries;

	reg_get_num_countries(&num_countries);

	for (i = 0; i < num_countries; i++) {
		if ((g_all_countries[i].alpha2[0] == alpha2[0]) &&
		    (g_all_countries[i].alpha2[1] == alpha2[1]))
			return QDF_STATUS_SUCCESS;
		else
			continue;
	}

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS reg_regrules_assign(uint8_t dmn_id_2g, uint8_t dmn_id_5g,
			       uint8_t ant_gain_2g, uint8_t ant_gain_5g,
			       struct cur_regulatory_info *reg_info)

{
	uint8_t k;
	uint8_t rule_index;
	struct cur_reg_rule *r_r_2g = reg_info->reg_rules_2g_ptr;
	struct cur_reg_rule *r_r_5g = reg_info->reg_rules_5g_ptr;

	for (k = 0; k < reg_info->num_2g_reg_rules; k++) {
		rule_index = regdomains_2g[dmn_id_2g].reg_rule_id[k];
		r_r_2g->start_freq = reg_rules_2g[rule_index].start_freq;
		r_r_2g->end_freq = reg_rules_2g[rule_index].end_freq;
		r_r_2g->max_bw = reg_rules_2g[rule_index].max_bw;
		r_r_2g->reg_power = reg_rules_2g[rule_index].reg_power;
		r_r_2g->flags = reg_rules_2g[rule_index].flags;
		r_r_2g->ant_gain = ant_gain_2g;
		r_r_2g++;
	}

	for (k = 0; k < reg_info->num_5g_reg_rules; k++) {
		rule_index = regdomains_5g[dmn_id_5g].reg_rule_id[k];
		r_r_5g->start_freq = reg_rules_5g[rule_index].start_freq;
		r_r_5g->end_freq = reg_rules_5g[rule_index].end_freq;
		r_r_5g->max_bw = reg_rules_5g[rule_index].max_bw;
		r_r_5g->reg_power = reg_rules_5g[rule_index].reg_power;
		r_r_5g->flags = reg_rules_5g[rule_index].flags;
		r_r_5g->ant_gain = ant_gain_5g;
		r_r_5g++;
	}

	if ((r_r_2g == reg_info->reg_rules_2g_ptr) &&
			(r_r_5g == reg_info->reg_rules_5g_ptr))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_rdpair_from_country_iso(uint8_t *alpha2,
					   uint16_t *country_index,
					   uint16_t *regdmn_pair)
{
	uint16_t i, j;
	int num_countries;
	int num_reg_dmn;

	reg_get_num_countries(&num_countries);
	reg_get_num_reg_dmn_pairs(&num_reg_dmn);

	for (i = 0; i < num_countries; i++) {
		if ((g_all_countries[i].alpha2[0] == alpha2[0]) &&
		    (g_all_countries[i].alpha2[1] == alpha2[1]))
			break;
	}

	if (i == num_countries) {
		*country_index = -1;
		return QDF_STATUS_E_FAILURE;
	}

	for (j = 0; j < num_reg_dmn; j++) {
		if (g_reg_dmn_pairs[j].reg_dmn_pair_id ==
				g_all_countries[i].reg_dmn_pair_id)
			break;
	}

	if (j == num_reg_dmn) {
		*regdmn_pair = -1;
		return QDF_STATUS_E_FAILURE;
	}

	*country_index = i;
	*regdmn_pair = j;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_rdpair_from_regdmn_id(uint16_t regdmn_id,
					 uint16_t *regdmn_pair)
{
	uint16_t j;
	int num_reg_dmn;

	reg_get_num_reg_dmn_pairs(&num_reg_dmn);

	for (j = 0; j < num_reg_dmn; j++) {
		if (g_reg_dmn_pairs[j].reg_dmn_pair_id == regdmn_id)
			break;
	}

	if (j == num_reg_dmn) {
		*regdmn_pair = -1;
		return QDF_STATUS_E_FAILURE;
	}

	*regdmn_pair = j;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_rdpair_from_country_code(uint16_t cc,
					    uint16_t *country_index,
					    uint16_t *regdmn_pair)
{
	uint16_t i, j;
	int num_countries;
	int num_reg_dmn;

	reg_get_num_countries(&num_countries);
	reg_get_num_reg_dmn_pairs(&num_reg_dmn);

	for (i = 0; i < num_countries; i++) {
		if (g_all_countries[i].country_code == cc)
			break;
	}

	if (i == num_countries) {
		*country_index = -1;
		return QDF_STATUS_E_FAILURE;
	}

	for (j = 0; j < num_reg_dmn; j++) {
		if (g_reg_dmn_pairs[j].reg_dmn_pair_id ==
				g_all_countries[i].reg_dmn_pair_id)
			break;
	}

	if (j == num_reg_dmn) {
		*regdmn_pair = -1;
		return QDF_STATUS_E_FAILURE;
	}

	*country_index = i;
	*regdmn_pair = j;

	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_get_reginfo_form_country_code_and_regdmn_pair(
		struct cur_regulatory_info *reg_info,
		uint16_t country_index,
		uint16_t regdmn_pair)
{
	uint8_t rule_size_2g, rule_size_5g;
	uint8_t dmn_id_5g, dmn_id_2g;
	uint8_t ant_gain_2g, ant_gain_5g;
	QDF_STATUS err;

	dmn_id_5g = g_reg_dmn_pairs[regdmn_pair].dmn_id_5g;
	dmn_id_2g = g_reg_dmn_pairs[regdmn_pair].dmn_id_2g;

	rule_size_2g = QDF_ARRAY_SIZE(regdomains_2g[dmn_id_2g].reg_rule_id);
	rule_size_5g = QDF_ARRAY_SIZE(regdomains_5g[dmn_id_5g].reg_rule_id);

	if (((rule_size_2g + rule_size_5g) >=
				regdomains_2g[dmn_id_2g].num_reg_rules +
				regdomains_5g[dmn_id_5g].num_reg_rules)) {

		qdf_mem_copy(reg_info->alpha2,
			g_all_countries[country_index].alpha2,
			sizeof(g_all_countries[country_index].alpha2));

		reg_info->ctry_code =
			g_all_countries[country_index].country_code;
		reg_info->reg_dmn_pair =
			g_reg_dmn_pairs[regdmn_pair].reg_dmn_pair_id;
		reg_info->dfs_region = regdomains_5g[dmn_id_5g].dfs_region;
		reg_info->phybitmap =
			g_all_countries[country_index].phymode_bitmap;

		reg_info->max_bw_2g = g_all_countries[country_index].max_bw_2g;
		reg_info->max_bw_5g = g_all_countries[country_index].max_bw_5g;

		reg_info->min_bw_2g = regdomains_2g[dmn_id_2g].min_bw;
		reg_info->min_bw_5g = regdomains_5g[dmn_id_5g].min_bw;

		ant_gain_2g = regdomains_2g[dmn_id_2g].ant_gain;
		ant_gain_5g = regdomains_5g[dmn_id_5g].ant_gain;

		reg_info->num_2g_reg_rules =
			regdomains_2g[dmn_id_2g].num_reg_rules;
		reg_info->num_5g_reg_rules =
			regdomains_5g[dmn_id_5g].num_reg_rules;

		reg_info->reg_rules_2g_ptr = (struct cur_reg_rule *)
			qdf_mem_malloc((reg_info->num_2g_reg_rules) *
					sizeof(struct cur_reg_rule));
		reg_info->reg_rules_5g_ptr = (struct cur_reg_rule *)
			qdf_mem_malloc((reg_info->num_5g_reg_rules) *
					sizeof(struct cur_reg_rule));

		err = reg_regrules_assign(dmn_id_2g, dmn_id_5g,
				ant_gain_2g, ant_gain_5g, reg_info);

		if (err == QDF_STATUS_E_FAILURE) {
			reg_err("No rule for country index = %d regdmn_pair = %d",
				country_index, regdmn_pair);
			return QDF_STATUS_E_FAILURE;
		}

		return QDF_STATUS_SUCCESS;
	} else if (!(((rule_size_2g + rule_size_5g) >=
				regdomains_2g[dmn_id_2g].num_reg_rules +
				regdomains_5g[dmn_id_5g].num_reg_rules)))
	    return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_REG_CLIENT
/**
 * reg_update_alpha2_from_domain() - Get country alpha2 code from reg domain
 * @reg_info: pointer to hold alpha2 code
 *
 * This function is used to populate alpha2 of @reg_info with:
 *	(a) "00" (REG_WORLD_ALPHA2) for WORLD domain and
 *	(b) alpha2 of first country matching with non WORLD domain.
 *
 * Return: None
 */
static void
reg_update_alpha2_from_domain(struct cur_regulatory_info *reg_info)
{
	uint16_t i;
	int num_countries;

	if (reg_is_world_ctry_code(reg_info->reg_dmn_pair)) {
		qdf_mem_copy(reg_info->alpha2, REG_WORLD_ALPHA2,
			     sizeof(reg_info->alpha2));
		return;
	}

	reg_get_num_countries(&num_countries);

	for (i = 0; i < (uint16_t)num_countries; i++)
		if (g_all_countries[i].reg_dmn_pair_id ==
		    reg_info->reg_dmn_pair)
			break;

	if (i == (uint16_t)num_countries)
		return;

	qdf_mem_copy(reg_info->alpha2, g_all_countries[i].alpha2,
		     sizeof(g_all_countries[i].alpha2));
	reg_info->ctry_code = g_all_countries[i].country_code;
}
#else
static inline void
reg_update_alpha2_from_domain(struct cur_regulatory_info *reg_info)
{
}
#endif

static inline QDF_STATUS reg_get_reginfo_form_regdmn_pair(
		struct cur_regulatory_info *reg_info,
		uint16_t regdmn_pair)
{
	uint8_t rule_size_2g, rule_size_5g;
	uint8_t dmn_id_5g, dmn_id_2g;
	uint8_t ant_gain_2g, ant_gain_5g;
	QDF_STATUS err;

	dmn_id_5g = g_reg_dmn_pairs[regdmn_pair].dmn_id_5g;
	dmn_id_2g = g_reg_dmn_pairs[regdmn_pair].dmn_id_2g;

	rule_size_2g = QDF_ARRAY_SIZE(regdomains_2g[dmn_id_2g].reg_rule_id);
	rule_size_5g = QDF_ARRAY_SIZE(regdomains_5g[dmn_id_5g].reg_rule_id);

	if (((rule_size_2g + rule_size_5g) >=
		    regdomains_2g[dmn_id_2g].num_reg_rules +
		    regdomains_5g[dmn_id_5g].num_reg_rules)) {

		qdf_mem_zero(reg_info->alpha2, sizeof(reg_info->alpha2));

		reg_info->reg_dmn_pair =
			g_reg_dmn_pairs[regdmn_pair].reg_dmn_pair_id;
		reg_info->ctry_code = 0;

		reg_update_alpha2_from_domain(reg_info);

		reg_info->dfs_region = regdomains_5g[dmn_id_5g].dfs_region;
		reg_info->phybitmap = 0;

		reg_info->max_bw_2g = 40;
		reg_info->max_bw_5g = 160;

		reg_info->min_bw_2g = regdomains_2g[dmn_id_2g].min_bw;
		reg_info->min_bw_5g = regdomains_5g[dmn_id_5g].min_bw;

		ant_gain_2g = regdomains_2g[dmn_id_2g].ant_gain;
		ant_gain_5g = regdomains_5g[dmn_id_5g].ant_gain;

		reg_info->num_2g_reg_rules =
			regdomains_2g[dmn_id_2g].num_reg_rules;
		reg_info->num_5g_reg_rules =
			regdomains_5g[dmn_id_5g].num_reg_rules;

		reg_info->reg_rules_2g_ptr = (struct cur_reg_rule *)
			qdf_mem_malloc((reg_info->num_2g_reg_rules) *
					sizeof(struct cur_reg_rule));
		reg_info->reg_rules_5g_ptr = (struct cur_reg_rule *)
			qdf_mem_malloc((reg_info->num_5g_reg_rules) *
					sizeof(struct cur_reg_rule));

		err = reg_regrules_assign(dmn_id_2g, dmn_id_5g,
			ant_gain_2g, ant_gain_5g, reg_info);
		if (err == QDF_STATUS_E_FAILURE) {
			reg_err("No rule for regdmn_pair = %d\n", regdmn_pair);
			return QDF_STATUS_E_FAILURE;
		}

		return QDF_STATUS_SUCCESS;
	} else if (!(((rule_size_2g + rule_size_5g) >=
			regdomains_2g[dmn_id_2g].num_reg_rules +
			regdomains_5g[dmn_id_5g].num_reg_rules)))
		return QDF_STATUS_E_NOMEM;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_cur_reginfo(struct cur_regulatory_info *reg_info,
			       uint16_t country_index,
			       uint16_t regdmn_pair)
{
	if ((country_index != (uint16_t)(-1)) &&
	    (regdmn_pair != (uint16_t)(-1)))
		return reg_get_reginfo_form_country_code_and_regdmn_pair(
				reg_info, country_index, regdmn_pair);
	else if (regdmn_pair != (uint16_t)(-1))
		return reg_get_reginfo_form_regdmn_pair(reg_info, regdmn_pair);
	else
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
