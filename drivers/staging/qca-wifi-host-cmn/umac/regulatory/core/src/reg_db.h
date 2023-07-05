/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * If COMPILE_REGDB_6G and CONFIG_BAND_6GHZ are defined, then
 * reg_6ghz_super_dmn_id and max_bw_6g are part of the
 * country_code_to_reg_domain table for a country
 * entry. If COMPILE_REGDB_6G and CONFIG_BAND_6GHZ are not defined, then they
 * are absent.
 *
 * COMPILE_REGDB_6G is not defined for the Partial offload platform.
 *
 * CE:- country entry
 */
#if defined(CONFIG_BAND_6GHZ) && defined(COMPILE_REGDB_6G)
#define CE(country_code, reg_dmn_pair_id, reg_6ghz_super_dmn_id,         \
	   alpha2, max_bw_2g, max_bw_5g, max_bw_6g, phymode_bitmap)      \
	{CTRY_ ## country_code, reg_dmn_pair_id, reg_6ghz_super_dmn_id,  \
	 #alpha2, max_bw_2g, max_bw_5g, max_bw_6g, phymode_bitmap}
#else
#define CE(country_code, reg_dmn_pair_id, reg_6ghz_super_dmn_id, alpha2, \
	   max_bw_2g, max_bw_5g, max_bw_6g, phymode_bitmap)              \
	{CTRY_ ## country_code, reg_dmn_pair_id, #alpha2, max_bw_2g,     \
	 max_bw_5g, phymode_bitmap}
#endif

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

#if defined(CONFIG_BAND_6GHZ) && defined(COMPILE_REGDB_6G)
/**
 * struct regulatory_rule_ext
 * @start_freq: start frequency in MHz
 * @end_freq: end frequency in MHz
 * @max_bw: maximum bandwidth in MHz
 * @eirp_power: EIRP power in dBm
 * @psd_power: Max PSD power in dBm per MHz
 * @flags: regulatory flags
 */
struct regulatory_rule_ext {
	uint16_t start_freq;
	uint16_t end_freq;
	uint16_t max_bw;
	uint8_t eirp_power;
	int8_t psd_power;
	uint16_t flags;
};
#endif

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

#if defined(CONFIG_BAND_6GHZ) && defined(COMPILE_REGDB_6G)
#define REG_MAX_PSD (0x7F) /* 127=63.5 dBm/MHz */

/**
 * struct sub_6g_regdomain
 * @min_bw: Minimum bandwidth in MHz
 * @max_bw: Maximum bandwidth in MHz
 * @num_reg_rules: number of regulatory rules
 * @reg_rules_id: regulatory rule index
 */
struct sub_6g_regdomain   {
	uint16_t min_bw;
	uint16_t max_bw;
	uint8_t num_reg_rules;
	uint8_t sixg_reg_rule_id[MAX_REG_RULES];
};
#endif

/**
 * struct country_code_to_reg_domain
 * @country_code: country code
 * @reg_dmn_pair_id: reg domainpair id
 * @reg_6ghz_super_dmn_id: 6GHz super domain id
 * @alpha2: iso-3166 alpha2
 * @max_bw_2g: maximum 2g bandwidth in MHz
 * @max_bw_5g: maximum 5g bandwidth in MHz
 * @max_bw_6g: maximum 6g bandwidth in MHz
 * @phymode_bitmap: phymodes not supported
 */
struct country_code_to_reg_domain   {
	uint16_t country_code;
	uint16_t reg_dmn_pair_id;
#if defined(CONFIG_BAND_6GHZ) && defined(COMPILE_REGDB_6G)
	uint16_t reg_6ghz_super_dmn_id;
#endif
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
	uint16_t max_bw_2g;
	uint16_t max_bw_5g;
#if defined(CONFIG_BAND_6GHZ) && defined(COMPILE_REGDB_6G)
	uint16_t max_bw_6g;
#endif
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

#if defined(CONFIG_BAND_6GHZ)
/**
 * enum reg_super_domain_6g - 6G Super Domain enumeration
 * @FCC1_6G_01: Super domain FCC1_6G_01 for US
 * @ETSI1_6G_02: Super domain ETSI1_6G_02 for EU
 * @ETSI2_6G_03: Super domain ETSI2_6G_03 for UK
 * @APL1_6G_04: Super domain APL1_6G_04 for Korea
 * @FCC1_6G_05: Super domain FCC1_6G_05 for Chile
 * @APL2_6G_06: Super domain APL2_6G_06 for Guatemala
 * @FCC1_6G_07: Super domain FCC1_6G_07 for Brazil
 * @APL3_6G_08: Super domain APL3_6G_08 for UAE
 * @FCC1_6G_09: Super domain FCC1_6G_09 for US AFC Testing
 * @FCC2_6G_10: Super domain FCC1_6G_10 for Canada LPI &
		SP(VLP to be added later)
 * @APL4_6G_11: Super domain APL3_6G_11 for Costa Rica LPI and VLP
 * @APL5_6G_12: Super domain APL3_6G_12 for CHILE LPI and VLP
 */
enum reg_super_domain_6g {
	FCC1_6G_01 = 0x01,
	ETSI1_6G_02 = 0x02,
	ETSI2_6G_03 = 0x03,
	APL1_6G_04 = 0x04,
	FCC1_6G_05 = 0x05,
	APL2_6G_06 = 0x06,
	FCC1_6G_07 = 0x07,
	APL3_6G_08 = 0x08,
	FCC1_6G_09 = 0x09,
	FCC2_6G_10 = 0x10,
	APL4_6G_11 = 0x11,
	APL5_6G_12 = 0x12,
};

#if defined(COMPILE_REGDB_6G)
/**
 * struct sixghz_super_to_subdomains
 * @reg_6ghz_super_dmn_id: 6G super domain id.
 * @reg_domain_6g_id_ap_lpi: 6G domain id for LPI AP.
 * @reg_domain_6g_id_ap_sp: 6G domain id for SP AP.
 * @reg_domain_6g_id_ap_vlp: 6G domain id for VLP AP.
 * @reg_domain_6g_id_client_lpi: 6G domain id for clients of the LPI AP.
 * @reg_domain_6g_id_client_sp: 6G domain id for clients of the SP AP.
 * @reg_domain_6g_id_client_vlp: 6G domain id for clients of the VLP AP.
 */
struct sixghz_super_to_subdomains {
	uint16_t reg_6ghz_super_dmn_id;
	uint8_t reg_domain_6g_id_ap_lpi;
	uint8_t reg_domain_6g_id_ap_sp;
	uint8_t reg_domain_6g_id_ap_vlp;
	uint8_t reg_domain_6g_id_client_lpi[REG_MAX_CLIENT_TYPE];
	uint8_t reg_domain_6g_id_client_sp[REG_MAX_CLIENT_TYPE];
	uint8_t reg_domain_6g_id_client_vlp[REG_MAX_CLIENT_TYPE];
};
#endif
#endif

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
