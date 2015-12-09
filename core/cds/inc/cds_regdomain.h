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

enum {
	CTRY_DEBUG = 0x1ff,     /* debug country code */
	CTRY_DEFAULT = 0        /* default country code */
};

#define BMLEN 2                 /* Use 2 64 bit uint for channel bitmask */

/*
 * The following table is the master list for all different freqeuncy
 * bands with the complete matrix of all possible flags and settings
 * for each band if it is used in ANY reg domain.
 */

#define DEF_REGDMN              FCC3_FCCA
#define    DEF_DMN_5            FCC1
#define    DEF_DMN_2            FCCA
#define    COUNTRY_ERD_FLAG     0x8000
#define WORLDWIDE_ROAMING_FLAG  0x4000
#define    SUPER_DOMAIN_MASK    0x0fff
#define    COUNTRY_CODE_MASK    0x3fff
#define CF_INTERFERENCE         (CHANNEL_CW_INT | CHANNEL_RADAR_INT)

/*
 * The following describe the bit masks for different passive scan
 * capability/requirements per regdomain.
 */
#define NO_PSCAN    0x0ULL
#define PSCAN_FCC   0x0000000000000001ULL
#define PSCAN_FCC_T 0x0000000000000002ULL
#define PSCAN_ETSI  0x0000000000000004ULL
#define PSCAN_MKK1  0x0000000000000008ULL
#define PSCAN_MKK2  0x0000000000000010ULL
#define PSCAN_MKKA  0x0000000000000020ULL
#define PSCAN_MKKA_G    0x0000000000000040ULL
#define PSCAN_ETSIA 0x0000000000000080ULL
#define PSCAN_ETSIB 0x0000000000000100ULL
#define PSCAN_ETSIC 0x0000000000000200ULL
#define PSCAN_WWR   0x0000000000000400ULL
#define PSCAN_MKKA1 0x0000000000000800ULL
#define PSCAN_MKKA1_G   0x0000000000001000ULL
#define PSCAN_MKKA2 0x0000000000002000ULL
#define PSCAN_MKKA2_G   0x0000000000004000ULL
#define PSCAN_MKK3  0x0000000000008000ULL
#define PSCAN_EXT_CHAN  0x0000000000010000ULL
#define PSCAN_DEFER 0x7FFFFFFFFFFFFFFFULL
#define IS_ECM_CHAN 0x8000000000000000ULL

/* define in ah_eeprom.h */
#define SD_NO_CTL       0xf0
#define NO_CTL          0xff
#define CTL_MODE_M      0x0f
#define CTL_11A         0
#define CTL_11B         1
#define CTL_11G         2
#define CTL_TURBO       3
#define CTL_108G        4
#define CTL_2GHT20      5
#define CTL_5GHT20      6
#define CTL_2GHT40      7
#define CTL_5GHT40      8
#define CTL_5GVHT80     9

#ifndef ATH_NO_5G_SUPPORT
#define REGDMN_MODE_11A_TURBO    REGDMN_MODE_108A
#define CHAN_11A_BMZERO BMZERO,
#define CHAN_11A_BM(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l) \
	BM(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l),
#else
/* remove 11a channel info if 11a is not supported */
#define CHAN_11A_BMZERO
#define CHAN_11A_BM(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l)
#endif
#ifndef ATH_REMOVE_2G_TURBO_RD_TABLE
#define REGDMN_MODE_11G_TURBO    REGDMN_MODE_108G
#define CHAN_TURBO_G_BMZERO BMZERO,
#define CHAN_TURBO_G_BM(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l)	\
	BM(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l),
#else
/* remove turbo-g channel info if turbo-g is not supported */
#define CHAN_TURBO_G(a, b)
#define CHAN_TURBO_G_BMZERO
#define CHAN_TURBO_G_BM(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j, _k, _l)
#endif

#define BMLEN 2                 /* Use 2 64 bit uint for channel bitmask
	                           NB: Must agree with macro below (BM) */
#define BMZERO {(uint64_t) 0, (uint64_t) 0} /* BMLEN zeros */

#ifndef SUPPRESS_SHIFT_WARNING
#define SUPPRESS_SHIFT_WARNING
#endif

/* Suppress MS warning "C4293: 'operator' : shift count negative or too big,
 * undefined behavior"
 * This is safe below because the the operand is properly range-checked, but
 * the compiler can't reason that out before it spits the warning.
 * Using suppress, so the warning can still be enabled globally to catch other
 * incorrect uses.
 */
#define BM(_fa, _fb, _fc, _fd, _fe, _ff, _fg, _fh, _fi, _fj, _fk, _fl) \
	SUPPRESS_SHIFT_WARNING \
	{((((_fa >= 0) && (_fa < 64)) ? (((uint64_t) 1) << _fa) : (uint64_t) 0) | \
	  (((_fb >= 0) && (_fb < 64)) ? (((uint64_t) 1) << _fb) : (uint64_t) 0) | \
	  (((_fc >= 0) && (_fc < 64)) ? (((uint64_t) 1) << _fc) : (uint64_t) 0) | \
	  (((_fd >= 0) && (_fd < 64)) ? (((uint64_t) 1) << _fd) : (uint64_t) 0) | \
	  (((_fe >= 0) && (_fe < 64)) ? (((uint64_t) 1) << _fe) : (uint64_t) 0) | \
	  (((_ff >= 0) && (_ff < 64)) ? (((uint64_t) 1) << _ff) : (uint64_t) 0) | \
	  (((_fg >= 0) && (_fg < 64)) ? (((uint64_t) 1) << _fg) : (uint64_t) 0) | \
	  (((_fh >= 0) && (_fh < 64)) ? (((uint64_t) 1) << _fh) : (uint64_t) 0) | \
	  (((_fi >= 0) && (_fi < 64)) ? (((uint64_t) 1) << _fi) : (uint64_t) 0) | \
	  (((_fj >= 0) && (_fj < 64)) ? (((uint64_t) 1) << _fj) : (uint64_t) 0) | \
	  (((_fk >= 0) && (_fk < 64)) ? (((uint64_t) 1) << _fk) : (uint64_t) 0) | \
	  (((_fl >= 0) && (_fl < 64)) ? (((uint64_t) 1) << _fl) : (uint64_t) 0) ) \
	 ,(((((_fa > 63) && (_fa < 128)) ? (((uint64_t) 1) << (_fa - 64)) : (uint64_t) 0) | \
	    (((_fb > 63) && (_fb < 128)) ? (((uint64_t) 1) << (_fb - 64)) : (uint64_t) 0) | \
	    (((_fc > 63) && (_fc < 128)) ? (((uint64_t) 1) << (_fc - 64)) : (uint64_t) 0) | \
	    (((_fd > 63) && (_fd < 128)) ? (((uint64_t) 1) << (_fd - 64)) : (uint64_t) 0) | \
	    (((_fe > 63) && (_fe < 128)) ? (((uint64_t) 1) << (_fe - 64)) : (uint64_t) 0) | \
	    (((_ff > 63) && (_ff < 128)) ? (((uint64_t) 1) << (_ff - 64)) : (uint64_t) 0) | \
	    (((_fg > 63) && (_fg < 128)) ? (((uint64_t) 1) << (_fg - 64)) : (uint64_t) 0) | \
	    (((_fh > 63) && (_fh < 128)) ? (((uint64_t) 1) << (_fh - 64)) : (uint64_t) 0) | \
	    (((_fi > 63) && (_fi < 128)) ? (((uint64_t) 1) << (_fi - 64)) : (uint64_t) 0) | \
	    (((_fj > 63) && (_fj < 128)) ? (((uint64_t) 1) << (_fj - 64)) : (uint64_t) 0) | \
	    (((_fk > 63) && (_fk < 128)) ? (((uint64_t) 1) << (_fk - 64)) : (uint64_t) 0) | \
	    (((_fl > 63) && (_fl < 128)) ? (((uint64_t) 1) << (_fl - 64)) : (uint64_t) 0)))}

/*
 * THE following table is the mapping of regdomain pairs specified by
 * an 8 bit regdomain value to the individual unitary reg domains
 */

typedef struct reg_dmn_pair_mapping {
	uint16_t regDmnEnum;    /* 16 bit reg domain pair */
	uint16_t regDmn5GHz;    /* 5GHz reg domain */
	uint16_t regDmn2GHz;    /* 2GHz reg domain */
	uint32_t flags5GHz;     /* Requirements flags (AdHoc
	                           disallow, noise floor cal needed,
	                           etc) */
	uint32_t flags2GHz;     /* Requirements flags (AdHoc
	                           disallow, noise floor cal needed,
	                           etc) */
	uint64_t pscanMask;     /* Passive Scan flags which
	                           can override unitary domain
	                           passive scan flags.  This
	                           value is used as a mask on
	                           the unitary flags */
	uint16_t singleCC;      /* Country code of single country if
	                           a one-on-one mapping exists */
} REG_DMN_PAIR_MAPPING;

typedef struct {
	uint16_t countryCode;
	uint16_t regDmnEnum;
	const char *isoName;
	const char *name;
	uint16_t allow11g : 1, allow11aTurbo : 1, allow11gTurbo : 1, allow11ng20 : 1, /* HT-20 allowed in 2GHz? */
		 allow11ng40 : 1, /* HT-40 allowed in 2GHz? */
		 allow11na20 : 1, /* HT-20 allowed in 5GHz? */
		 allow11na40 : 1, /* HT-40 VHT-40 allowed in 5GHz? */
		 allow11na80 : 1; /* VHT-80 allowed in 5GHz */
	uint16_t outdoorChanStart;
} COUNTRY_CODE_TO_ENUM_RD;

typedef struct RegDmnFreqBand {
	uint16_t lowChannel;    /* Low channel center in MHz */
	uint16_t highChannel;   /* High Channel center in MHz */
	uint8_t powerDfs;       /* Max power (dBm) for channel
	                           range when using DFS */
	uint8_t antennaMax;     /* Max allowed antenna gain */
	uint8_t channelBW;      /* Bandwidth of the channel */
	uint8_t channelSep;     /* Channel separation within
	                           the band */
	uint64_t useDfs;        /* Use DFS in the RegDomain
	                           if corresponding bit is set */
	uint64_t usePassScan;   /* Use Passive Scan in the RegDomain
	                           if corresponding bit is set */
	uint8_t regClassId;     /* Regulatory class id */
} REG_DMN_FREQ_BAND;

typedef struct reg_domain {
	uint16_t regDmnEnum;    /* value from EnumRd table */
	uint8_t conformance_test_limit;
	uint64_t dfsMask;       /* DFS bitmask for 5Ghz tables */
	uint64_t pscan;         /* Bitmask for passive scan */
	uint32_t flags;         /* Requirement flags (AdHoc disallow, noise
	                           floor cal needed, etc) */
	uint64_t chan11a[BMLEN];        /* 128 bit bitmask for channel/band selection */
	uint64_t chan11a_turbo[BMLEN];  /* 128 bit bitmask for channel/band select */
	uint64_t chan11a_dyn_turbo[BMLEN];      /* 128 bit mask for chan/band select */

	uint64_t chan11b[BMLEN];        /* 128 bit bitmask for channel/band selection */
	uint64_t chan11g[BMLEN];        /* 128 bit bitmask for channel/band selection */
	uint64_t chan11g_turbo[BMLEN];
} REG_DOMAIN;

struct cmode {
	uint32_t mode;
	uint32_t flags;
};

#define    YES    true
#define    NO    false

/* mapping of old skus to new skus for Japan */
typedef struct {
	uint16_t domain;
	uint16_t newdomain_pre53;       /* pre eeprom version 5.3 */
	uint16_t newdomain_post53;      /* post eeprom version 5.3 */
} JAPAN_SKUMAP;

/* mapping of countrycode to new skus for Japan */
typedef struct {
	uint16_t ccode;
	uint16_t newdomain_pre53;       /* pre eeprom version 5.3 */
	uint16_t newdomain_post53;      /* post eeprom version 5.3 */
} JAPAN_COUNTRYMAP;

/* check rd flags in eeprom for japan */
typedef struct {
	uint16_t freqbandbit;
	uint32_t eepromflagtocheck;
} JAPAN_BANDCHECK;

/* Common mode power table for 5Ghz */
typedef struct {
	uint16_t lchan;
	uint16_t hchan;
	uint8_t pwrlvl;
} COMMON_MODE_POWER;

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
void cds_fill_send_ctl_info_to_fw(struct regulatory *reg, uint32_t modesAvail,
				  uint32_t modeSelect);
void cds_set_wma_dfs_region(uint8_t dfs_region);
void cds_set_ch_params(uint8_t ch, uint32_t phy_mode,
		       struct ch_params_s *ch_params);

#endif /* REGULATORY_H */
