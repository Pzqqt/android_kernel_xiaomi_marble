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
 * DOC: reg_db.h
 * This file contains regulatory component data structures
 */

#ifndef __REG_DB_H
#define __REG_DB_H

/* Alpha2 code for world reg domain */
#define REG_WORLD_ALPHA2 "00"

/**
 * struct regulatory_rule
 * @start_freq: start frequency
 * @end_freq: end frequency
 * @max_bw: maximum bandwidth
 * @reg_power: regulatory power
 * @flags: regulatory flags
 */
struct regulatory_rule {
	uint16_t start_freq;
	uint16_t end_freq;
	uint16_t max_bw;
	uint8_t reg_power;
	uint16_t flags;
};

/**
 * struct regdomain
 * @ctl_val: CTL value
 * @dfs_region: dfs region
 * @min_bw: minimum bandwidth
 * @max_bw: maximum bandwidth
 * @num_reg_rules: number of regulatory rules
 * @reg_rules_id: regulatory rule index
 */
struct regdomain   {
	uint8_t ctl_val;
	enum dfs_reg dfs_region;
	uint16_t min_bw;
	uint16_t max_bw;
	uint8_t ant_gain;
	uint8_t num_reg_rules;
	uint8_t reg_rule_id[MAX_REG_RULES];
};

/**
 * struct country_code_to_reg_domain
 * @country_code: country code
 * @reg_dmn_pair_id: reg domainpair id
 * @alpha2: iso-3166 alpha2
 * @max_bw_2g: maximum 2g bandwidth
 * @max_bw_5g: maximum 5g bandwidth
 * @phymode_bitmap: phymodes not supported
 */
struct country_code_to_reg_domain   {
	uint16_t country_code;
	uint16_t reg_dmn_pair_id;
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
	uint16_t max_bw_2g;
	uint16_t max_bw_5g;
	uint16_t phymode_bitmap;
};

/**
 * struct reg_domain_pair
 * @reg_dmn_pair_id: reg domainpiar value
 * @dmn_id_5g: 5g reg domain value
 * @dmn_id_2g: 2g regdomain value
 */
struct reg_domain_pair {
	uint16_t reg_dmn_pair_id;
	uint8_t dmn_id_5g;
	uint8_t dmn_id_2g;
};

QDF_STATUS reg_get_num_countries(int *num_countries);

QDF_STATUS reg_get_num_reg_dmn_pairs(int *num_reg_dmn);

QDF_STATUS reg_get_default_country(uint16_t *default_country);

/**
 * reg_etsi13_regdmn () - Checks if the reg domain is ETSI13 or not
 * @reg_dmn: reg domain
 *
 * Return: true or false
 */
bool reg_etsi13_regdmn(uint8_t reg_dmn);

/**
 * reg_fcc_regdmn () - Checks if the reg domain is FCC3/FCC8/FCC15/FCC16 or not
 * @reg_dmn: reg domain
 *
 * Return: true or false
 */
bool reg_fcc_regdmn(uint8_t reg_dmn);

/**
 * reg_en302_502_regdmn() - Check if the reg domain is en302_502 applicable.
 * @reg_dmn: Regulatory domain pair ID.
 *
 * Return: True if EN302_502 applicable, else false.
 */
bool reg_en302_502_regdmn(uint16_t reg_dmn);
#endif
