/*
 * Copyright (c) 2011,2013-2016 The Linux Foundation. All rights reserved.
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
 */

#include "qdf_types.h"
#include "wma.h"
#include "cds_regdomain.h"

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

static const struct reg_dmn_pair g_reg_dmn_pairs[] = {
	{NO_ENUMRD, FCC8, FCCA, CTRY_DEFAULT},
	{NULL1_WORLD, NULL1, WORLD, CTRY_DEFAULT},
	{FCC1_FCCA, FCC1, FCCA, CTRY_DEFAULT},
	{FCC1_WORLD, FCC1, WORLD, CTRY_DEFAULT},
	{FCC2_WORLD, FCC2, WORLD, CTRY_DEFAULT},
	{FCC2_ETSIC, FCC2, ETSIC, CTRY_DEFAULT},
	{FCC3_FCCA, FCC3, FCCA, CTRY_DEFAULT},
	{FCC3_WORLD, FCC3, WORLD, CTRY_DEFAULT},
	{FCC3_ETSIC, FCC3, ETSIC, CTRY_DEFAULT},
	{FCC4_FCCA, FCC4, FCCA, CTRY_DEFAULT},
	{FCC5_FCCA, FCC5, FCCA, CTRY_DEFAULT},
	{FCC6_FCCA, FCC6, FCCA, CTRY_DEFAULT},
	{FCC7_FCCA, FCC7, FCCA, CTRY_DEFAULT},
	{FCC8_FCCA, FCC8, FCCA, CTRY_DEFAULT},
	{FCC6_WORLD, FCC6, WORLD, CTRY_DEFAULT},
	{FCC9_FCCA, FCC9, FCCA, CTRY_DEFAULT},
	{FCC10_FCCA, FCC10, FCCA, CTRY_DEFAULT},
	{FCC11_WORLD, FCC11, WORLD, CTRY_DEFAULT},
	{ETSI1_WORLD, ETSI1, WORLD, CTRY_DEFAULT},
	{ETSI3_WORLD, ETSI3, WORLD, CTRY_DEFAULT},
	{ETSI4_WORLD, ETSI4, WORLD, CTRY_DEFAULT},
	{ETSI7_WORLD, ETSI4, WORLD, CTRY_DEFAULT},
	{ETSI8_WORLD, ETSI8, WORLD, CTRY_DEFAULT},
	{ETSI9_WORLD, ETSI9, WORLD, CTRY_DEFAULT},
	{APL4_WORLD, APL4, WORLD, CTRY_DEFAULT},
	{APL2_WORLD, APL2, WORLD, CTRY_DEFAULT},
	{APL2_FCCA, APL2, FCCA, CTRY_DEFAULT},
	{APL1_WORLD, APL1, WORLD, CTRY_DEFAULT},
	{APL1_ETSIC, APL1, ETSIC, CTRY_DEFAULT},
	{APL6_WORLD, APL6, WORLD, CTRY_DEFAULT},
	{APL7_FCCA, APL7, FCCA, CTRY_DEFAULT},
	{APL8_WORLD, APL8, WORLD, CTRY_DEFAULT},
	{APL9_WORLD, APL9, WORLD, CTRY_DEFAULT},
	{APL10_WORLD, APL10, WORLD, CTRY_DEFAULT},
	{APL12_WORLD, APL12, WORLD, CTRY_DEFAULT},
	{APL13_WORLD, APL13, WORLD, CTRY_DEFAULT},
	{WOR0_WORLD, WOR0_WORLD, WOR0_WORLD, CTRY_DEFAULT},
	{WOR1_WORLD, WOR1_WORLD, WOR1_WORLD, CTRY_DEFAULT},
	{WOR2_WORLD, WOR2_WORLD, WOR2_WORLD, CTRY_DEFAULT},
	{WOR3_WORLD, WOR3_WORLD, WOR3_WORLD, CTRY_DEFAULT},
	{WOR4_FCCA, WOR4_FCCA, WOR4_FCCA, CTRY_DEFAULT},
	{WOR5_ETSIC, WOR5_ETSIC, WOR5_ETSIC, CTRY_DEFAULT},
	{WOR01_WORLD, WOR01_WORLD, WOR01_WORLD, CTRY_DEFAULT},
	{WOR02_WORLD, WOR02_WORLD, WOR02_WORLD, CTRY_DEFAULT},
	{EU1_WORLD, EU1_WORLD, EU1_WORLD, CTRY_DEFAULT},
	{WOR9_WORLD, WOR9_WORLD, WOR9_WORLD, CTRY_DEFAULT},
	{WORA_WORLD, WORA_WORLD, WORA_WORLD, CTRY_DEFAULT},
	{WORB_WORLD, WORB_WORLD, WORB_WORLD, CTRY_DEFAULT},
	{WORC_WORLD, WORC_WORLD, WORC_WORLD, CTRY_DEFAULT},
	{MKK5_MKKA2, MKK5, MKKA, CTRY_JAPAN14},
};

static const struct country_code_to_reg_dmn g_all_countries[] = {
	{CTRY_AFGHANISTAN, ETSI1_WORLD, "AF", "AFGHANISTAN"},
	{CTRY_ALBANIA, ETSI1_WORLD, "AL", "ALBANIA"},
	{CTRY_ALGERIA, APL13_WORLD, "DZ", "ALGERIA"},
	{CTRY_AMERICAN_SAMOA, FCC3_FCCA, "AS", "AMERICAN SAMOA"},
	{CTRY_ANGUILLA, ETSI1_WORLD, "AI", "ANGUILLA"},
	{CTRY_ARGENTINA, FCC6_WORLD, "AR", "ARGENTINA"},
	{CTRY_ARMENIA, ETSI4_WORLD, "AM", "ARMENIA"},
	{CTRY_ARUBA, ETSI1_WORLD, "AW", "ARUBA"},
	{CTRY_AUSTRALIA, FCC6_WORLD, "AU", "AUSTRALIA"},
	{CTRY_AUSTRIA, ETSI1_WORLD, "AT", "AUSTRIA"},
	{CTRY_AZERBAIJAN, ETSI4_WORLD, "AZ", "AZERBAIJAN"},
	{CTRY_BAHAMAS, FCC3_WORLD, "BS", "BAHAMAS"},
	{CTRY_BAHRAIN, APL6_WORLD, "BH", "BAHRAIN"},
	{CTRY_BANGLADESH, APL1_WORLD, "BD", "BANGLADESH"},
	{CTRY_BARBADOS, FCC2_WORLD, "BB", "BARBADOS"},
	{CTRY_BELARUS, ETSI1_WORLD, "BY", "BELARUS"},
	{CTRY_BELGIUM, ETSI1_WORLD, "BE", "BELGIUM"},
	{CTRY_BELIZE, APL1_ETSIC, "BZ", "BELIZE"},
	{CTRY_BERMUDA, FCC3_FCCA, "BM", "BERMUDA"},
	{CTRY_BHUTAN, ETSI1_WORLD, "BT", "BHUTAN"},
	{CTRY_BOLIVIA, APL8_WORLD, "BO", "BOLIVIA"},
	{CTRY_BOSNIA_HERZ, ETSI1_WORLD, "BA", "BOSNIA AND HERZEGOVINA"},
	{CTRY_BRAZIL, FCC3_ETSIC, "BR", "BRAZIL"},
	{CTRY_BRUNEI_DARUSSALAM, APL6_WORLD, "BN", "BRUNEI DARUSSALAM"},
	{CTRY_BULGARIA, ETSI1_WORLD, "BG", "BULGARIA"},
	{CTRY_BURKINA_FASO, FCC3_WORLD, "BF", "BURKINA-FASO"},
	{CTRY_CAMBODIA, ETSI1_WORLD, "KH", "CAMBODIA"},
	{CTRY_CANADA, FCC6_FCCA, "CA", "CANADA"},
	{CTRY_CAYMAN_ISLANDS, FCC3_WORLD, "KY", "CAYMAN ISLANDS"},
	{CTRY_CENTRAL_AFRICA_REPUBLIC, FCC3_WORLD, "CF", "AFRICA REPUBLIC"},
	{CTRY_CHAD, ETSI1_WORLD, "TD", "CHAD"},
	{CTRY_CHILE, APL6_WORLD, "CL", "CHILE"},
	{CTRY_CHINA, FCC2_WORLD, "CN", "CHINA"},
	{CTRY_CHRISTMAS_ISLAND, FCC3_WORLD, "CX", "CHRISTMAS ISLAND"},
	{CTRY_COLOMBIA, FCC3_WORLD, "CO", "COLOMBIA"},
	{CTRY_COSTA_RICA, FCC3_WORLD, "CR", "COSTA RICA"},
	{CTRY_COTE_DIVOIRE, FCC3_WORLD, "CI", "COTE DIVOIRE"},
	{CTRY_CROATIA, ETSI1_WORLD, "HR", "CROATIA"},
	{CTRY_CYPRUS, ETSI1_WORLD, "CY", "CYPRUS"},
	{CTRY_CZECH, ETSI1_WORLD, "CZ", "CZECH REPUBLIC"},
	{CTRY_DENMARK, ETSI1_WORLD, "DK", "DENMARK"},
	{CTRY_DOMINICA, FCC1_FCCA, "DM", "DOMINICA"},
	{CTRY_DOMINICAN_REPUBLIC, FCC1_FCCA, "DO", "DOMINICAN REPUBLIC"},
	{CTRY_ECUADOR, FCC3_WORLD, "EC", "ECUADOR"},
	{CTRY_EGYPT, ETSI3_WORLD, "EG", "EGYPT"},
	{CTRY_EL_SALVADOR, FCC1_WORLD, "SV", "EL SALVADOR"},
	{CTRY_ESTONIA, ETSI1_WORLD, "EE", "ESTONIA"},
	{CTRY_ETHIOPIA, ETSI1_WORLD, "ET", "ETHIOPIA"},
	{CTRY_FINLAND, ETSI1_WORLD, "FI", "FINLAND"},
	{CTRY_FRANCE, ETSI1_WORLD, "FR", "FRANCE"},
	{CTRY_FRENCH_GUIANA, ETSI1_WORLD, "GF", "FRENCH GUIANA"},
	{CTRY_FRENCH_POLYNESIA, ETSI1_WORLD, "PF", "FRENCH POLYNESIA"},
	{CTRY_GEORGIA, ETSI4_WORLD, "GE", "GEORGIA"},
	{CTRY_GERMANY, ETSI1_WORLD, "DE", "GERMANY"},
	{CTRY_GHANA, FCC3_WORLD, "GH", "GHANA"},
	{CTRY_GREECE, ETSI1_WORLD, "GR", "GREECE"},
	{CTRY_GREENLAND, ETSI1_WORLD, "GL", "GREENLAND"},
	{CTRY_GRENADA, FCC3_FCCA, "GD", "GRENADA"},
	{CTRY_GUADELOUPE, ETSI1_WORLD, "GP", "GUADELOUPE"},
	{CTRY_GUAM, FCC3_FCCA, "GU", "GUAM"},
	{CTRY_GUATEMALA, ETSI1_WORLD, "GT", "GUATEMALA"},
	{CTRY_GUYANA, APL1_ETSIC, "GY", "GUYANA"},
	{CTRY_HAITI, FCC3_FCCA, "HT", "HAITI"},
	{CTRY_HONDURAS, FCC3_WORLD, "HN", "HONDURAS"},
	{CTRY_HONG_KONG, FCC3_WORLD, "HK", "HONG KONG"},
	{CTRY_HUNGARY, ETSI1_WORLD, "HU", "HUNGARY"},
	{CTRY_ICELAND, ETSI1_WORLD, "IS", "ICELAND"},
	{CTRY_INDIA, APL6_WORLD, "IN", "INDIA"},
	{CTRY_INDONESIA, APL2_ETSIC, "ID", "INDONESIA"},
	{CTRY_IRAN, APL1_WORLD, "IR", "IRAN"},
	{CTRY_IRELAND, ETSI1_WORLD, "IE", "IRELAND"},
	{CTRY_ISRAEL, ETSI3_WORLD, "IL", "ISRAEL"},
	{CTRY_ITALY, ETSI1_WORLD, "IT", "ITALY"},
	{CTRY_JAMAICA, FCC3_WORLD, "JM", "JAMAICA"},
	{CTRY_JORDAN, APL4_WORLD, "JO", "JORDAN"},
	{CTRY_KAZAKHSTAN, NULL1_WORLD, "KZ", "KAZAKHSTAN"},
	{CTRY_KENYA, APL12_WORLD, "KE", "KENYA"},
	{CTRY_KOREA_NORTH, APL9_WORLD, "KP", "NORTH KOREA"},
	{CTRY_KOREA_ROC, APL9_WORLD, "KR", "KOREA REPUBLIC"},
	{CTRY_KUWAIT, ETSI3_WORLD, "KW", "KUWAIT"},
	{CTRY_LATVIA, ETSI1_WORLD, "LV", "LATVIA"},
	{CTRY_LEBANON, FCC3_WORLD, "LB", "LEBANON"},
	{CTRY_LESOTHO, ETSI1_WORLD, "LS", "LESOTHO"},
	{CTRY_LIECHTENSTEIN, ETSI1_WORLD, "LI", "LIECHTENSTEIN"},
	{CTRY_LITHUANIA, ETSI1_WORLD, "LT", "LITHUANIA"},
	{CTRY_LUXEMBOURG, ETSI1_WORLD, "LU", "LUXEMBOURG"},
	{CTRY_MACAU, FCC3_WORLD, "MO", "MACAU SAR"},
	{CTRY_MACEDONIA, ETSI1_WORLD, "MK", "MACEDONIA, FYRO"},
	{CTRY_MALAWI, ETSI1_WORLD, "MW", "MALAWI"},
	{CTRY_MALAYSIA, FCC11_WORLD, "MY", "MALAYSIA"},
	{CTRY_MALDIVES, APL6_WORLD, "MV", "MALDIVES"},
	{CTRY_MALTA, ETSI1_WORLD, "MT", "MALTA"},
	{CTRY_MARSHALL_ISLANDS, FCC3_FCCA, "MH", "MARSHALL ISLANDS"},
	{CTRY_MARTINIQUE, ETSI1_WORLD, "MQ", "MARTINIQUE"},
	{CTRY_MAURITANIA, ETSI1_WORLD, "MR", "MAURITANA"},
	{CTRY_MAURITIUS, FCC3_WORLD, "MU", "MAURITIUS"},
	{CTRY_MAYOTTE, ETSI1_WORLD, "YT", "MAYOTTE"},
	{CTRY_MEXICO, FCC3_ETSIC, "MX", "MEXICO"},
	{CTRY_MICRONESIA, FCC3_FCCA, "FM", "MICRONESIA"},
	{CTRY_MOLDOVA, ETSI1_WORLD, "MD", "MOLDOVA"},
	{CTRY_MONACO, ETSI1_WORLD, "MC", "MONACO"},
	{CTRY_MONGOLIA, FCC3_WORLD, "MN", "MONGOLIA"},
	{CTRY_MONTENEGRO, ETSI1_WORLD, "ME", "MONTENEGRO"},
	{CTRY_MOROCCO, ETSI3_WORLD, "MA", "MOROCCO"},
	{CTRY_NAMIBIA, APL10_WORLD, "NA", "NAMIBIA"},
	{CTRY_NEPAL, APL6_WORLD, "NP", "NEPAL"},
	{CTRY_NETHERLANDS, ETSI1_WORLD, "NL", "NETHERLANDS"},
	{CTRY_NETHERLANDS_ANTILLES, ETSI1_WORLD, "AN", "NETHERLANDS ANTILLES"},
	{CTRY_NEW_ZEALAND, FCC3_ETSIC, "NZ", "NEW ZEALAND"},
	{CTRY_NIGERIA, APL8_WORLD, "NG", "NIGERIA"},
	{CTRY_NORTHERN_MARIANA_ISLANDS, FCC3_FCCA, "MP", "MARIANA ISLANDS"},
	{CTRY_NICARAGUA, FCC3_FCCA, "NI", "NICARAGUA"},
	{CTRY_NORWAY, ETSI1_WORLD, "NO", "NORWAY"},
	{CTRY_OMAN, ETSI1_WORLD, "OM", "OMAN"},
	{CTRY_PAKISTAN, APL1_ETSIC, "PK", "PAKISTAN"},
	{CTRY_PALAU, FCC3_FCCA, "PW", "PALAU"},
	{CTRY_PANAMA, FCC1_FCCA, "PA", "PANAMA"},
	{CTRY_PAPUA_NEW_GUINEA, FCC3_WORLD, "PG", "PAPUA NEW GUINEA"},
	{CTRY_PARAGUAY, FCC3_WORLD, "PY", "PARAGUAY"},
	{CTRY_PERU, FCC3_WORLD, "PE", "PERU"},
	{CTRY_PHILIPPINES, FCC3_WORLD, "PH", "PHILIPPINES"},
	{CTRY_POLAND, ETSI1_WORLD, "PL", "POLAND"},
	{CTRY_PORTUGAL, ETSI1_WORLD, "PT", "PORTUGAL"},
	{CTRY_PUERTO_RICO, FCC3_FCCA, "PR", "PUERTO RICO"},
	{CTRY_QATAR, APL1_WORLD, "QA", "QATAR"},
	{CTRY_REUNION, ETSI1_WORLD, "RE", "REUNION"},
	{CTRY_ROMANIA, ETSI1_WORLD, "RO", "ROMANIA"},
	{CTRY_RUSSIA, ETSI8_WORLD, "RU", "RUSSIA"},
	{CTRY_RWANDA, FCC3_WORLD, "RW", "RWANDA"},
	{CTRY_SAINT_BARTHELEMY, ETSI1_WORLD, "BL", "SAINT BARTHELEMY"},
	{CTRY_SAINT_KITTS_AND_NEVIS, APL10_WORLD, "KN", "SAINT KITTS"},
	{CTRY_SAINT_LUCIA, APL10_WORLD, "LC", "SAINT LUCIA"},
	{CTRY_SAINT_MARTIN, ETSI1_WORLD, "MF", "SAINT MARTIN"},
	{CTRY_SAINT_PIERRE_AND_MIQUELON, ETSI1_WORLD, "PM", "SAINT PIERRE"},
	{CTRY_SAINT_VINCENT_AND_THE_GRENADIENS, ETSI1_WORLD, "VC", "VINCENT"},
	{CTRY_SAMOA, ETSI1_WORLD, "WS", "SAMOA"},
	{CTRY_SAUDI_ARABIA, ETSI1_WORLD, "SA", "SAUDI ARABIA"},
	{CTRY_SENEGAL, FCC3_WORLD, "SN", "SENEGAL"},
	{CTRY_SERBIA, ETSI1_WORLD, "RS", "REPUBLIC OF SERBIA"},
	{CTRY_SINGAPORE, FCC3_WORLD, "SG", "SINGAPORE"},
	{CTRY_SLOVAKIA, ETSI1_WORLD, "SK", "SLOVAKIA"},
	{CTRY_SLOVENIA, ETSI1_WORLD, "SI", "SLOVENIA"},
	{CTRY_SOUTH_AFRICA, FCC3_WORLD, "ZA", "SOUTH AFRICA"},
	{CTRY_SPAIN, ETSI1_WORLD, "ES", "SPAIN"},
	{CTRY_SURINAME, ETSI1_WORLD, "SR", "SURINAME"},
	{CTRY_SRI_LANKA, FCC3_WORLD, "LK", "SRI LANKA"},
	{CTRY_SWEDEN, ETSI1_WORLD, "SE", "SWEDEN"},
	{CTRY_SWITZERLAND, ETSI1_WORLD, "CH", "SWITZERLAND"},
	{CTRY_SYRIA, NULL1_WORLD, "SY", "SYRIAN ARAB REPUBLIC"},
	{CTRY_TAIWAN, FCC3_FCCA, "TW", "TAIWAN"},
	{CTRY_TANZANIA, APL1_WORLD, "TZ", "TANZANIA"},
	{CTRY_THAILAND, FCC3_WORLD, "TH", "THAILAND"},
	{CTRY_TOGO, ETSI1_WORLD, "TG", "TOGO"},
	{CTRY_TRINIDAD_Y_TOBAGO, FCC3_WORLD, "TT", "TRINIDAD AND TOBAGO"},
	{CTRY_TUNISIA, ETSI3_WORLD, "TN", "TUNISIA"},
	{CTRY_TURKEY, ETSI1_WORLD, "TR", "TURKEY"},
	{CTRY_TURKS_AND_CAICOS, FCC3_WORLD, "TC" "TURKS AND CAICOS"},
	{CTRY_UGANDA, FCC3_WORLD, "UG", "UGANDA"},
	{CTRY_UKRAINE, ETSI9_WORLD, "UA", "UKRAINE"},
	{CTRY_UAE, FCC3_WORLD, "AE", "UNITED ARAB EMIRATES"},
	{CTRY_UNITED_KINGDOM, ETSI1_WORLD, "GB", "UNITED KINGDOM"},
	{CTRY_UNITED_STATES, FCC3_FCCA, "US", "UNITED STATES"},
	{CTRY_UNITED_STATES_PS, FCC4_FCCA, "PS", "US PUBLIC SAFETY"},
	{CTRY_URUGUAY, FCC2_WORLD, "UY", "URUGUAY"},
	{CTRY_UZBEKISTAN, ETSI3_WORLD, "UZ", "UZBEKISTAN"},
	{CTRY_VANUATU, FCC3_WORLD, "VU", "VANUATU"},
	{CTRY_VENEZUELA, FCC2_ETSIC, "VE", "VENEZUELA"},
	{CTRY_VIET_NAM, FCC3_WORLD, "VN", "VIETNAM"},
	{CTRY_VIRGIN_ISLANDS, FCC3_FCCA, "VI", "VIRGIN ISLANDS"},
	{CTRY_WALLIS_AND_FUTUNA, ETSI1_WORLD, "WF" "WALLIS"},
	{CTRY_YEMEN, NULL1_WORLD, "YE", "YEMEN"},
	{CTRY_ZIMBABWE, ETSI1_WORLD, "ZW", "ZIMBABWE"},
	{CTRY_JAPAN14, MKK5_MKKA2, "JP", "JAPAN"},
	{CTRY_XA, MKK5_MKKA2, "XA", "JAPAN PASSIVE"}
};

static const struct reg_dmn g_reg_dmns[] = {
	{FCC1, FCC},
	{FCC2, FCC},
	{FCC3, FCC},
	{FCC4, FCC},
	{FCC5, FCC},
	{FCC6, FCC},
	{FCC7, FCC},
	{FCC8, FCC},
	{FCC9, FCC},
	{FCC10, FCC},
	{FCC11, FCC},
	{ETSI1, ETSI},
	{ETSI2, ETSI},
	{ETSI3, ETSI},
	{ETSI4, ETSI},
	{ETSI5, ETSI},
	{ETSI6, ETSI},
	{ETSI8, ETSI},
	{ETSI9, ETSI},
	{ETSI10, ETSI},
	{ETSI11, ETSI},
	{APL1, ETSI},
	{APL2, ETSI},
	{APL3, ETSI},
	{APL4, ETSI},
	{APL5, ETSI},
	{APL6, ETSI},
	{APL7, ETSI},
	{APL8, ETSI},
	{APL9, ETSI},
	{APL10, ETSI},
	{APL11, ETSI},
	{APL12, ETSI},
	{APL13, ETSI},
	{APL14, FCC},
	{NULL1, NO_CTL},
	{MKK3, MKK},
	{MKK4, MKK},
	{MKK5, MKK},
	{MKK9, MKK},
	{MKK10, MKK},
	{MKK11, MKK},
	{WORLD, ETSI},
	{FCCA, FCC},
	{MKKA, MKK},
	{MKKC, MKK},
	{ETSIC, ETSI},
	{WOR0_WORLD, NO_CTL},
	{WOR1_WORLD, NO_CTL},
	{WOR2_WORLD, NO_CTL},
	{WOR3_WORLD, NO_CTL},
	{WOR4_FCCA, NO_CTL},
	{WOR5_ETSIC, NO_CTL},
	{WOR01_WORLD, NO_CTL},
	{WOR02_WORLD, NO_CTL},
	{EU1_WORLD, NO_CTL},
	{WOR9_WORLD, NO_CTL},
	{WORA_WORLD, NO_CTL},
	{WORB_WORLD, NO_CTL},
	{WORC_WORLD, NO_CTL},
};


struct reg_dmn_tables g_reg_dmn_tbl = {
	g_reg_dmn_pairs,
	g_all_countries,
	g_reg_dmns,
	QDF_ARRAY_SIZE(g_reg_dmn_pairs),
	QDF_ARRAY_SIZE(g_all_countries),
	QDF_ARRAY_SIZE(g_reg_dmns),
};

/**
 * get_bdf_reg_dmn() - get regulatory domain from BDF
 * @reg_dmn: BDF regulatory domain
 *
 * Return: regulatory domain
 */
static uint16_t get_bdf_reg_dmn(uint16_t reg_dmn)
{
	return reg_dmn & ~WORLDWIDE_ROAMING_FLAG;
}

/**
 * is_reg_dmn_valid() - is regulatory domain valid
 * @reg_dmn: regulatory domain
 *
 * Return: true or false
 */
static bool is_reg_dmn_valid(uint16_t reg_dmn)
{
	int32_t i;

	if (reg_dmn & COUNTRY_ERD_FLAG) {
		uint16_t cc = reg_dmn & ~COUNTRY_ERD_FLAG;
		for (i = 0; i < g_reg_dmn_tbl.all_countries_cnt; i++)
			if (g_reg_dmn_tbl.all_countries[i].country_code == cc)
				return true;
	} else {
		for (i = 0; i < g_reg_dmn_tbl.reg_dmn_pairs_cnt; i++)
			if (g_reg_dmn_tbl.reg_dmn_pairs[i].reg_dmn_pair
			    == reg_dmn)
				return true;
	}

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "invalid regulatory domain/country code 0x%x", reg_dmn);

	return false;
}

/**
 * find_country() - find country data
 * @country_code: country code
 *
 * Return: country code data pointer
 */
static const struct country_code_to_reg_dmn *find_country(uint16_t country_code)
{
	int32_t i;

	for (i = 0; i < g_reg_dmn_tbl.all_countries_cnt; i++) {
		if (g_reg_dmn_tbl.all_countries[i].country_code == country_code)
			return &g_reg_dmn_tbl.all_countries[i];
	}

	return NULL;
}

/**
 * cds_get_country_from_alpha2() - get country from alpha2
 * @alpha2: country code alpha2
 *
 * Return: country code
 */
int32_t cds_get_country_from_alpha2(uint8_t *alpha2)
{
	int32_t i;

	for (i = 0; i < g_reg_dmn_tbl.all_countries_cnt; i++) {
		if (g_reg_dmn_tbl.all_countries[i].alpha2[0] == alpha2[0] &&
		    g_reg_dmn_tbl.all_countries[i].alpha2[1] == alpha2[1])
			return g_reg_dmn_tbl.all_countries[i].country_code;
	}

	return CTRY_DEFAULT;
}

/**
 * reg_dmn_get_default_country() - get default country for regulatory domain
 * @reg_dmn: regulatory domain
 *
 * Return: default country
 */
static uint16_t reg_dmn_get_default_country(uint16_t reg_dmn)
{
	int32_t i;
	const struct country_code_to_reg_dmn *country = NULL;
	uint16_t cc = reg_dmn & ~COUNTRY_ERD_FLAG;

	if (reg_dmn & COUNTRY_ERD_FLAG) {
		country = find_country(cc);
		if (country)
			return cc;
	}

	for (i = 0; i < g_reg_dmn_tbl.reg_dmn_pairs_cnt; i++) {
		if (g_reg_dmn_tbl.reg_dmn_pairs[i].reg_dmn_pair == reg_dmn) {
			if (g_reg_dmn_tbl.reg_dmn_pairs[i].single_cc != 0)
				return g_reg_dmn_tbl.reg_dmn_pairs[i].single_cc;
			else
				i = g_reg_dmn_tbl.reg_dmn_pairs_cnt;
		}
	}

	return CTRY_DEFAULT;
}

/**
 * get_reg_dmn_pair() - get regulatory domain pair pointer
 * @reg_dmn: regulatory domain
 *
 * Return: pointer to regulatory domain pair data
 */
static const struct reg_dmn_pair *get_reg_dmn_pair(uint16_t reg_dmn)
{
	int32_t i;

	for (i = 0; i < g_reg_dmn_tbl.reg_dmn_pairs_cnt; i++) {
		if (g_reg_dmn_tbl.reg_dmn_pairs[i].reg_dmn_pair == reg_dmn)
			return &g_reg_dmn_tbl.reg_dmn_pairs[i];
	}

	return NULL;
}

/**
 * get_reg_dmn() - get regulatory domain pointer
 * @reg_dmn: regulatory domain
 *
 * Return: pointer to regulatory domain data
 */
static const struct reg_dmn *get_reg_dmn(uint16_t reg_dmn)
{
	int32_t i;

	for (i = 0; i < g_reg_dmn_tbl.reg_dmns_cnt; i++) {
		if (g_reg_dmn_tbl.reg_dmns[i].reg_dmn == reg_dmn)
			return &g_reg_dmn_tbl.reg_dmns[i];
	}

	return NULL;
}

/**
 * get_country_from_rd() - get country from regulatory domain
 * @reg_dmn: regulatory domain
 *
 * Return: country code enum
 */
static const struct country_code_to_reg_dmn *get_country_from_rd(
	uint16_t reg_dmn)
{
	int32_t i;

	for (i = 0; i < g_reg_dmn_tbl.all_countries_cnt; i++) {
		if (g_reg_dmn_tbl.all_countries[i].reg_dmn_pair == reg_dmn)
			return &g_reg_dmn_tbl.all_countries[i];
	}

	return NULL;
}

/**
 * reg_dmn_sanitize() - sanitize regulatory domain
 * @reg: regulatory data structure
 *
 * Return: none
 */
static void reg_dmn_sanitize(struct regulatory *reg)
{
	if (reg->reg_domain != COUNTRY_ERD_FLAG)
		return;

	reg->reg_domain = WOR0_WORLD;
}

/**
 * cds_fill_some_regulatory_info() - fill regulatory information
 * @reg: regulatory data structure
 *
 * Return: error code
 */
int32_t cds_fill_some_regulatory_info(struct regulatory *reg)
{
	uint16_t country_code;
	uint16_t reg_dmn, rd;
	const struct country_code_to_reg_dmn *country = NULL;

	reg_dmn_sanitize(reg);
	rd = reg->reg_domain;

	if (!is_reg_dmn_valid(rd))
		return -EINVAL;

	reg_dmn = get_bdf_reg_dmn(rd);

	country_code = reg_dmn_get_default_country(reg_dmn);
	if (country_code == CTRY_DEFAULT && reg_dmn == CTRY_DEFAULT)
		country_code = CTRY_UNITED_STATES;

	if (country_code != CTRY_DEFAULT) {
		country = find_country(country_code);
		if (!country) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "not a valid country code");
			return -EINVAL;
		}

		reg_dmn = country->reg_dmn_pair;
	}

	reg->regpair = get_reg_dmn_pair(reg_dmn);
	if (!reg->regpair) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "no regpair is found, can not proceeed");
		return -EINVAL;
	}

	reg->country_code = country_code;

	if (!country)
		country = get_country_from_rd(reg_dmn);

	if (country) {
		reg->alpha2[0] = country->alpha2[0];
		reg->alpha2[1] = country->alpha2[1];
	} else {
		reg->alpha2[0] = '0';
		reg->alpha2[1] = '0';
	}

	return 0;
}

/**
 * get_reg_dmn_for_country() - get regulatory domain for country
 * @alpha2: country alpha2
 *
 * Return: regulatory domain
 */
int32_t get_reg_dmn_for_country(uint8_t *alpha2)
{
	uint8_t i;

	for (i = 0; i < g_reg_dmn_tbl.all_countries_cnt; i++) {
		if ((g_reg_dmn_tbl.all_countries[i].alpha2[0] == alpha2[0]) &&
		    (g_reg_dmn_tbl.all_countries[i].alpha2[1] == alpha2[1]))
			return g_reg_dmn_tbl.all_countries[i].reg_dmn_pair;
	}

	return -1;
}

/**
 * cds_fill_and_send_ctl_to_fw() - fill and send ctl to firmware
 * @reg: the regulatory handle
 *
 * Return: none
 */
void cds_fill_and_send_ctl_to_fw(struct regulatory *reg)
{
	const struct reg_dmn *reg_dmn_2g = NULL;
	const struct reg_dmn *reg_dmn_5g = NULL;
	int8_t ctl_2g, ctl_5g;
	const struct reg_dmn_pair *regpair;
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "unable to get WMA handle");
		return;
	}

	regpair = reg->regpair;
	reg_dmn_2g = get_reg_dmn(regpair->reg_dmn_2ghz);
	if (!reg_dmn_2g) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "failed to get regdmn 2G");
		return;
	}

	reg_dmn_5g = get_reg_dmn(regpair->reg_dmn_5ghz);
	if (!reg_dmn_5g) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "failed to get regdmn 5G");
		return;
	}

	ctl_2g = reg_dmn_2g->conformance_test_limit;
	ctl_5g = reg_dmn_5g->conformance_test_limit;


	reg->ctl_5g = ctl_5g;
	reg->ctl_2g = ctl_2g;

	wma_send_regdomain_info_to_fw(reg->reg_domain, regpair->reg_dmn_2ghz,
				      regpair->reg_dmn_5ghz, ctl_2g, ctl_5g);
}

/**
 * cds_set_wma_dfs_region() - to set the dfs region to wma
 * @reg: the regulatory handle
 *
 * Return: none
 */
void cds_set_wma_dfs_region(uint8_t dfs_region)
{
	tp_wma_handle wma = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "unable to get WMA handle");
		return;
	}

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "dfs_region: %d", dfs_region);

	wma_set_dfs_region(wma, dfs_region);
}

/**
 * cds_get_reg_dmn_5g() - get the 5G reg-domain
 * @reg_dmn: the complete reg domain
 *
 * Return: 5 G reg domain
 */
uint16_t cds_get_reg_dmn_5g(uint32_t reg_dmn)
{
	uint16_t i;

	for (i = 0; i < g_reg_dmn_tbl.reg_dmn_pairs_cnt; i++) {
		if (g_reg_dmn_tbl.reg_dmn_pairs[i].reg_dmn_pair == reg_dmn)
			return g_reg_dmn_tbl.reg_dmn_pairs[i].reg_dmn_5ghz;
	}

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		  "invalid regulatory domain/country code 0x%x",
		  reg_dmn);

	return 0;
}

/**
 * cds_reg_dmn_get_chanwidth_from_opclass() - return chan width based on opclass
 * @country: country name
 * @channel: operating channel
 * @opclass: operating class
 *
 * Given a value of country, channel and opclass this API will return value of
 * channel width.
 *
 * Return: channel width
 *
 */
uint16_t cds_reg_dmn_get_chanwidth_from_opclass(uint8_t *country,
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
			  (i < MAX_CHANNELS_PER_OPERATING_CLASS &&
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


/**
 * cds_reg_dmn_get_opclass_from_channel() - get operating class from channel
 * @country: the complete reg domain
 * @channel: channel number
 * @offset: the value of offset
 *
 * Return: operating class
 */
uint16_t cds_reg_dmn_get_opclass_from_channel(uint8_t *country, uint8_t channel,
					      uint8_t offset)
{
	const struct reg_dmn_op_class_map_t *class = NULL;
	uint16_t i = 0;

	if (!qdf_mem_cmp(country, "US", 2)) {
		class = us_op_class;
	} else if (!qdf_mem_cmp(country, "EU", 2)) {
		class = euro_op_class;
	} else if (!qdf_mem_cmp(country, "JP", 2)) {
		class = japan_op_class;
	} else {
		class = global_op_class;
	}

	while (class->op_class) {
		if ((offset == class->offset) || (offset == BWALL)) {
			for (i = 0;
			     (i < MAX_CHANNELS_PER_OPERATING_CLASS &&
			      class->channels[i]); i++) {
				if (channel == class->channels[i])
					return class->op_class;
			}
		}
		class++;
	}

	return 0;
}

/**
 * cds_reg_dmn_set_curr_opclasses() - set the current operating class
 * @num_classes: number of classes
 * @class: operating class
 *
 * Return: error code
 */
uint16_t cds_reg_dmn_set_curr_opclasses(uint8_t num_classes, uint8_t *class)
{
	uint8_t i;

	if (CDS_MAX_SUPP_OPER_CLASSES < num_classes) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "invalid num classes %d", num_classes);
		return -1;
	}

	for (i = 0; i < num_classes; i++) {
		reg_dmn_curr_supp_opp_classes.classes[i] = class[i];
	}

	reg_dmn_curr_supp_opp_classes.num_classes = num_classes;

	return 0;
}

/**
 * cds_reg_dmn_get_curr_opclasses() - get the current operating class
 * @num_classes: number of classes
 * @class: operating class
 *
 * Return: error code
 */
uint16_t cds_reg_dmn_get_curr_opclasses(uint8_t *num_classes, uint8_t *class)
{
	uint8_t i;

	if (!num_classes || !class) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "either num_classes or class is null");
		return -1;
	}

	for (i = 0; i < reg_dmn_curr_supp_opp_classes.num_classes; i++)
		class[i] = reg_dmn_curr_supp_opp_classes.classes[i];

	*num_classes = reg_dmn_curr_supp_opp_classes.num_classes;

	return 0;
}
