/*
 * Copyright (c) 2011, 2014-2016 The Linux Foundation. All rights reserved.
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

/*
 * Notifications and licenses are retained for attribution purposes only.
 */
/*
 * Copyright (c) 2002-2006 Sam Leffler, Errno Consulting
 * Copyright (c) 2005-2006 Atheros Communications, Inc.
 * Copyright (c) 2010, Atheros Communications Inc.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the following conditions are met:
 * 1. The materials contained herein are unmodified and are used
 *    unmodified.
 * 2. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following NO
 *    ''WARRANTY'' disclaimer below (''Disclaimer''), without
 *    modification.
 * 3. Redistributions in binary form must reproduce at minimum a
 *    disclaimer similar to the Disclaimer below and any redistribution
 *    must be conditioned upon including a substantially similar
 *    Disclaimer requirement for further binary redistribution.
 * 4. Neither the names of the above-listed copyright holders nor the
 *    names of any contributors may be used to endorse or promote
 *    product derived from this software without specific prior written
 *    permission.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT,
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 * FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES.
 *
 * This module contains the regulatory domain private structure definitions .
 *
 */

#ifndef REGULATORY_H
#define REGULATORY_H

#define	CTRY_DEBUG              0x1ff
#define	CTRY_DEFAULT            0
#define COUNTRY_ERD_FLAG        0x8000
#define WORLDWIDE_ROAMING_FLAG  0x4000
#define DEF_REGDMN              FCC3_FCCA

/* These defines should match the table from ah_internal.h */
enum HAL_DFS_DOMAIN {
	DFS_UNINIT_DOMAIN = 0,  /* Uninitialized dfs domain */
	DFS_FCC_DOMAIN = 1,     /* FCC3 dfs domain */
	DFS_ETSI_DOMAIN = 2,    /* ETSI dfs domain */
	DFS_MKK4_DOMAIN = 3     /* Japan dfs domain */
};


/*
 * THE following table is the mapping of regdomain pairs specified by
 * an 8 bit regdomain value to the individual unitary reg domains
 */

typedef struct reg_dmn_pair_mapping {
	uint16_t regDmnEnum;    /* 16 bit reg domain pair */
	uint16_t regDmn5GHz;    /* 5GHz reg domain */
	uint16_t regDmn2GHz;    /* 2GHz reg domain */
	uint16_t singleCC;      /* Country code of single country if
	                           a one-on-one mapping exists */
} REG_DMN_PAIR_MAPPING;

typedef struct {
	uint16_t countryCode;
	uint16_t regDmnEnum;
	const char *isoName;
	const char *name;
} COUNTRY_CODE_TO_ENUM_RD;

typedef struct reg_domain {
	uint16_t regDmnEnum;    /* value from EnumRd table */
	uint8_t conformance_test_limit;
} REG_DOMAIN;

/* Multi-Device RegDomain Support */
typedef struct ath_hal_reg_dmn_tables {
	/* regDomainPairs: Map of 8-bit regdomain values to unitary reg domain */
	const REG_DMN_PAIR_MAPPING *regDomainPairs;
	/* allCountries: Master list of freq. bands (flags, settings) */
	const COUNTRY_CODE_TO_ENUM_RD *allCountries;
	/* regDomains: Array of supported reg domains */
	const REG_DOMAIN *regDomains;

	uint16_t regDomainPairsCt;      /* Num reg domain pair entries */
	uint16_t allCountriesCt;        /* Num country entries */
	uint16_t regDomainsCt;  /* Num reg domain entries */
} HAL_REG_DMN_TABLES;

enum CountryCode {
	CTRY_AFGHANISTAN = 4,
	CTRY_ALBANIA = 8,
	CTRY_ALGERIA = 12,
	CTRY_AMERICAN_SAMOA = 16,
	CTRY_ANGUILLA = 660,
	CTRY_ARGENTINA = 32,
	CTRY_ARGENTINA_AP = 5003,
	CTRY_ARMENIA = 51,
	CTRY_ARUBA = 533,
	CTRY_AUSTRALIA = 36,
	CTRY_AUSTRALIA_AP = 5000,
	CTRY_AUSTRIA = 40,
	CTRY_AZERBAIJAN = 31,
	CTRY_BAHAMAS = 44,
	CTRY_BAHRAIN = 48,
	CTRY_BANGLADESH = 50,
	CTRY_BARBADOS = 52,
	CTRY_BELARUS = 112,
	CTRY_BELGIUM = 56,
	CTRY_BELIZE = 84,
	CTRY_BERMUDA = 60,
	CTRY_BHUTAN = 64,
	CTRY_BOLIVIA = 68,
	CTRY_BOSNIA_HERZ = 70,
	CTRY_BRAZIL = 76,
	CTRY_BRUNEI_DARUSSALAM = 96,
	CTRY_BULGARIA = 100,
	CTRY_BURKINA_FASO = 854,
	CTRY_CAMBODIA = 116,
	CTRY_CANADA = 124,
	CTRY_CANADA_AP = 5001,
	CTRY_CAYMAN_ISLANDS = 136,
	CTRY_CENTRAL_AFRICA_REPUBLIC = 140,
	CTRY_CHAD = 148,
	CTRY_CHILE = 152,
	CTRY_CHINA = 156,
	CTRY_CHRISTMAS_ISLAND = 162,
	CTRY_COLOMBIA = 170,
	CTRY_COSTA_RICA = 188,
	CTRY_COTE_DIVOIRE = 384,
	CTRY_CROATIA = 191,
	CTRY_CYPRUS = 196,
	CTRY_CZECH = 203,
	CTRY_DENMARK = 208,
	CTRY_DOMINICA = 212,
	CTRY_DOMINICAN_REPUBLIC = 214,
	CTRY_ECUADOR = 218,
	CTRY_EGYPT = 818,
	CTRY_EL_SALVADOR = 222,
	CTRY_ESTONIA = 233,
	CTRY_ETHIOPIA = 231,
	CTRY_FINLAND = 246,
	CTRY_FRANCE = 250,
	CTRY_FRENCH_GUIANA = 254,
	CTRY_FRENCH_POLYNESIA = 258,
	CTRY_GEORGIA = 268,
	CTRY_GERMANY = 276,
	CTRY_GHANA = 288,
	CTRY_GREECE = 300,
	CTRY_GREENLAND = 304,
	CTRY_GRENADA = 308,
	CTRY_GUADELOUPE = 312,
	CTRY_GUAM = 316,
	CTRY_GUATEMALA = 320,
	CTRY_GUYANA = 328,
	CTRY_HAITI = 332,
	CTRY_HONDURAS = 340,
	CTRY_HONG_KONG = 344,
	CTRY_HUNGARY = 348,
	CTRY_ICELAND = 352,
	CTRY_INDIA = 356,
	CTRY_INDONESIA = 360,
	CTRY_IRAN = 364,
	CTRY_IRELAND = 372,
	CTRY_ISRAEL = 376,
	CTRY_ITALY = 380,
	CTRY_JAMAICA = 388,
	CTRY_JORDAN = 400,
	CTRY_KAZAKHSTAN = 398,
	CTRY_KENYA = 404,
	CTRY_KOREA_NORTH = 408,
	CTRY_KOREA_ROC = 410,
	CTRY_KOREA_ROC_AP = 412,
	CTRY_KUWAIT = 414,
	CTRY_LATVIA = 428,
	CTRY_LEBANON = 422,
	CTRY_LESOTHO = 426,
	CTRY_LIBYA = 434,
	CTRY_LIECHTENSTEIN = 438,
	CTRY_LITHUANIA = 440,
	CTRY_LUXEMBOURG = 442,
	CTRY_MACAU = 446,
	CTRY_MACEDONIA = 807,
	CTRY_MALAWI = 454,
	CTRY_MALAYSIA = 458,
	CTRY_MALDIVES = 462,
	CTRY_MALTA = 470,
	CTRY_MARSHALL_ISLANDS = 584,
	CTRY_MARTINIQUE = 474,
	CTRY_MAURITANIA = 478,
	CTRY_MAURITIUS = 480,
	CTRY_MAYOTTE = 175,
	CTRY_MEXICO = 484,
	CTRY_MICRONESIA = 583,
	CTRY_MOLDOVA = 498,
	CTRY_MONACO = 492,
	CTRY_MONGOLIA = 496,
	CTRY_MONTENEGRO = 499,
	CTRY_MOROCCO = 504,
	CTRY_NEPAL = 524,
	CTRY_NETHERLANDS = 528,
	CTRY_NETHERLANDS_ANTILLES = 530,
	CTRY_NEW_ZEALAND = 554,
	CTRY_NIGERIA = 566,
	CTRY_NORTHERN_MARIANA_ISLANDS = 580,
	CTRY_NICARAGUA = 558,
	CTRY_NORWAY = 578,
	CTRY_OMAN = 512,
	CTRY_PAKISTAN = 586,
	CTRY_PALAU = 585,
	CTRY_PANAMA = 591,
	CTRY_PAPUA_NEW_GUINEA = 598,
	CTRY_PARAGUAY = 600,
	CTRY_PERU = 604,
	CTRY_PHILIPPINES = 608,
	CTRY_POLAND = 616,
	CTRY_PORTUGAL = 620,
	CTRY_PUERTO_RICO = 630,
	CTRY_QATAR = 634,
	CTRY_REUNION = 638,
	CTRY_ROMANIA = 642,
	CTRY_RUSSIA = 643,
	CTRY_RWANDA = 646,
	CTRY_SAINT_BARTHELEMY = 652,
	CTRY_SAINT_KITTS_AND_NEVIS = 659,
	CTRY_SAINT_LUCIA = 662,
	CTRY_SAINT_MARTIN = 663,
	CTRY_SAINT_PIERRE_AND_MIQUELON = 666,
	CTRY_SAINT_VINCENT_AND_THE_GRENADIENS = 670,
	CTRY_SAMOA = 882,
	CTRY_SAUDI_ARABIA = 682,
	CTRY_SENEGAL = 686,
	CTRY_SERBIA = 688,
	CTRY_SINGAPORE = 702,
	CTRY_SLOVAKIA = 703,
	CTRY_SLOVENIA = 705,
	CTRY_SOUTH_AFRICA = 710,
	CTRY_SPAIN = 724,
	CTRY_SURINAME = 740,
	CTRY_SRI_LANKA = 144,
	CTRY_SWEDEN = 752,
	CTRY_SWITZERLAND = 756,
	CTRY_SYRIA = 760,
	CTRY_TAIWAN = 158,
	CTRY_TANZANIA = 834,
	CTRY_THAILAND = 764,
	CTRY_TOGO = 768,
	CTRY_TRINIDAD_Y_TOBAGO = 780,
	CTRY_TUNISIA = 788,
	CTRY_TURKEY = 792,
	CTRY_TURKS_AND_CAICOS = 796,
	CTRY_UGANDA = 800,
	CTRY_UKRAINE = 804,
	CTRY_UAE = 784,
	CTRY_UNITED_KINGDOM = 826,
	CTRY_UNITED_STATES = 840,
	CTRY_UNITED_STATES_AP = 841,
	CTRY_UNITED_STATES_AP2 = 843,
	CTRY_UNITED_STATES_PS = 842,
	CTRY_URUGUAY = 858,
	CTRY_UZBEKISTAN = 860,
	CTRY_VANUATU = 548,
	CTRY_VENEZUELA = 862,
	CTRY_VIET_NAM = 704,
	CTRY_VIRGIN_ISLANDS = 850,
	CTRY_WALLIS_AND_FUTUNA = 876,
	CTRY_YEMEN = 887,
	CTRY_ZIMBABWE = 716,
	CTRY_JAPAN7 = 4007,
	CTRY_JAPAN8 = 4008,
	CTRY_JAPAN9 = 4009,
	CTRY_JAPAN10 = 4010,
	CTRY_JAPAN11 = 4011,
	CTRY_JAPAN12 = 4012,
	CTRY_JAPAN13 = 4013,
	CTRY_JAPAN14 = 4014,
	CTRY_JAPAN15 = 4015,
	CTRY_JAPAN25 = 4025,
	CTRY_JAPAN26 = 4026,
	CTRY_JAPAN27 = 4027,
	CTRY_JAPAN28 = 4028,
	CTRY_JAPAN29 = 4029,
	CTRY_JAPAN34 = 4034,
	CTRY_JAPAN35 = 4035,
	CTRY_JAPAN36 = 4036,
	CTRY_JAPAN37 = 4037,
	CTRY_JAPAN38 = 4038,
	CTRY_JAPAN39 = 4039,
	CTRY_JAPAN40 = 4040,
	CTRY_JAPAN41 = 4041,
	CTRY_JAPAN42 = 4042,
	CTRY_JAPAN43 = 4043,
	CTRY_JAPAN44 = 4044,
	CTRY_JAPAN45 = 4045,
	CTRY_JAPAN46 = 4046,
	CTRY_JAPAN47 = 4047,
	CTRY_JAPAN48 = 4048,
	CTRY_JAPAN49 = 4049,
	CTRY_JAPAN55 = 4055,
	CTRY_JAPAN56 = 4056,

};


/**
 * enum ch_width - channel width
 * @CH_WIDTH_20MHZ: channel width 20 MHz
 * @CH_WIDTH_40MHZ: channel width 40 MHz
 * @CH_WIDTH_80MHZ: channel width 80MHz
 * @CH_WIDTH_160MHZ: channel width 160 MHz
 * @CH_WIDTH_80P80MHZ: channel width 160MHz(80+80)
 */
enum ch_width {
	CH_WIDTH_20MHZ = 0,
	CH_WIDTH_40MHZ = 1,
	CH_WIDTH_80MHZ = 2,
	CH_WIDTH_160MHZ = 3,
	CH_WIDTH_80P80MHZ = 4,
	CH_WIDTH_MAX
};

/**
 * struct ch_params_s
 *
 * @ch_width: channel width
 * @sec_ch_offset: secondary channel offset
 * @center_freq_seg0: center freq for segment 0
 * @center_freq_seg1: center freq for segment 1
 */
struct ch_params_s {
	enum ch_width ch_width;
	uint8_t sec_ch_offset;
	uint8_t center_freq_seg0;
	uint8_t center_freq_seg1;
};


int32_t cds_fill_some_regulatory_info(struct regulatory *reg);
void cds_fill_and_send_ctl_to_fw(struct regulatory *reg);
int32_t cds_get_country_from_alpha2(uint8_t *alpha2);
void cds_fill_send_ctl_info_to_fw(struct regulatory *reg);
void cds_set_wma_dfs_region(uint8_t dfs_region);
void cds_set_ch_params(uint8_t ch, uint32_t phy_mode,
		       struct ch_params_s *ch_params);

#endif /* REGULATORY_H */
