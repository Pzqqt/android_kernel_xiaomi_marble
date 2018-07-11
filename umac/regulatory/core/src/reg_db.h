/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#define REGULATORY_CHAN_DISABLED     (1<<0)
#define REGULATORY_CHAN_NO_IR        (1<<1)
#define REGULATORY_CHAN_RADAR        (1<<3)
#define REGULATORY_CHAN_NO_OFDM      (1<<6)
#define REGULATORY_CHAN_INDOOR_ONLY  (1<<9)

#define REGULATORY_CHAN_NO_HT40      (1<<4)
#define REGULATORY_CHAN_NO_80MHZ     (1<<7)
#define REGULATORY_CHAN_NO_160MHZ    (1<<8)
#define REGULATORY_CHAN_NO_20MHZ     (1<<11)
#define REGULATORY_CHAN_NO_10MHZ     (1<<12)

#define REGULATORY_PHYMODE_NO11A     (1<<0)
#define REGULATORY_PHYMODE_NO11B     (1<<1)
#define REGULATORY_PHYMODE_NO11G     (1<<2)
#define REGULATORY_CHAN_NO11N        (1<<3)
#define REGULATORY_PHYMODE_NO11AC    (1<<4)
#define REGULATORY_PHYMODE_NO11AX    (1<<5)

#define MAX_REG_RULES 10
#define REG_ALPHA2_LEN 2

/**
 * enum dfs_reg - DFS region
 * @DFS_UNINIT_REG: un-initialized region
 * @DFS_FCC_REG: FCC region
 * @DFS_ETSI_REG: ETSI region
 * @DFS_MKK_REG: MKK region
 * @DFS_CN_REG: China region
 * @DFS_KR_REG: Korea region
 * @DFS_UNDEF_REG: Undefined region
 */
enum dfs_reg {
	DFS_UNINIT_REG = 0,
	DFS_FCC_REG = 1,
	DFS_ETSI_REG = 2,
	DFS_MKK_REG = 3,
	DFS_CN_REG = 4,
	DFS_KR_REG = 5,
	DFS_UNDEF_REG = 0xFFFF,
};

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
 * @num_reg_rules: number of regulatory rules
 * @reg_rules_id: regulatory rule index
 */
struct regdomain   {
	uint8_t ctl_val;
	enum dfs_reg dfs_region;
	uint16_t min_bw;
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

/**
 * enum ctl_value - CTL value
 * @CTL_FCC: CTL FCC
 * @CTL_MKK: CTL MKK
 * @CTL_ETSI: CTL ETSI
 * @CTL_KOR: CTL KOR
 * @CTL_CHN: CTL CHINA
 * @CTL_USER_DEF: CTL USER_DEF
 * @CTL_NONE: CTL NONE
 */
enum ctl_value {
	CTL_FCC = 0x10,
	CTL_ETSI = 0x30,
	CTL_MKK = 0x40,
	CTL_KOR = 0x50,
	CTL_CHN = 0x60,
	CTL_USER_DEF = 0x70,
	CTL_NONE = 0xff
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
#endif
