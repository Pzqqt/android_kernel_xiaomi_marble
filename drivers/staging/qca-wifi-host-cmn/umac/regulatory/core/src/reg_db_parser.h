/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: reg_db.h
 * This file contains regulatory data base parser function declarations
 */

#ifndef __REG_DB_PARSER_H
#define __REG_DB_PARSER_H

extern const struct country_code_to_reg_domain g_all_countries[];
extern const struct reg_domain_pair g_reg_dmn_pairs[];
extern const struct regulatory_rule reg_rules_2g[];
extern const struct regdomain regdomains_2g[];
extern const struct regulatory_rule reg_rules_5g[];
extern const struct regdomain regdomains_5g[];

#ifdef CONFIG_REG_CLIENT
extern const uint32_t reg_2g_sub_dmn_code[];
extern const uint32_t reg_5g_sub_dmn_code[];
#endif

/**
 * reg_is_country_code_valid() - Check if the given country code is valid
 * @alpha2: Country string
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_is_country_code_valid(uint8_t *alpha2);

/**
 * reg_regrules_assign() - Get 2GHz and 5GHz regulatory rules from regdomain
 * structure.
 * @dmn_id_2g: 2GHz regdomain ID
 * @dmn_id_5g: 5GHz regdomain ID
 * @ant_gain_2g: 2GHz antenna gain
 * @ant_gain_5g: 5GHz antenna gain
 * @reg_info: Pointer to current regulatory info structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_regrules_assign(uint8_t dmn_id_2g, uint8_t dmn_id_5g,
			       uint8_t ant_gain_2g, uint8_t ant_gain_5g,
			       struct cur_regulatory_info *reg_info);

/**
 * reg_get_cur_reginfo() - Get current regulatory info for a given country code
 * @reg_info: Pointer to current regulatory info structure
 * @country_index: Country code index in the country code table
 * @regdmn_pair: Regdomain pair ID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_cur_reginfo(struct cur_regulatory_info *reg_info,
			       uint16_t country_index, uint16_t regdmn_pair);

/**
 * reg_get_rdpair_from_country_iso() - Get regdomain pair ID from country string
 * @alpha: Pointer to country code string
 * @country_index: Pointer to save country code index
 * @regdmn_pair: Pointer to save regdomain pair ID index
 *
 * Return: QDF_STATUS
 */
QDF_STATUS  reg_get_rdpair_from_country_iso(uint8_t *alpha,
					    uint16_t *country_index,
					    uint16_t *regdmn_pair);

/**
 * reg_get_rdpair_from_country_code() - Get regdomain pair ID from country code
 * @cc: Country code
 * @country_index: Pointer to save country code index
 * @regdmn_pair: Pointer to save regdomain pair ID index
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_rdpair_from_country_code(uint16_t cc,
					    uint16_t *country_index,
					    uint16_t *regdmn_pair);

/**
 * reg_get_rdpair_from_regdmn_id() - Get regdomain pair ID from regdomain ID
 * @regdmn_id: Regdomain ID
 * @regdmn_pair: Pointer to save regdomain pair ID index
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_rdpair_from_regdmn_id(uint16_t regdmn_id,
					 uint16_t *regdmn_pair);
#endif
