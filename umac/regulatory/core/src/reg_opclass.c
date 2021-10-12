/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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

/**
 * Given a global opclass number create the corresponding  array token.
 * Examples:
 *     'CFISARR(132)' expands to  'opcls_132_cfis_arr'
 *     'CFISARR(133)' expands to  'opcls_133_cfis_arr'
 */
#define CFISARR(_g_opcls)  opcls_ ## _g_opcls ## _cfis_arr

/**
 * Given a global opclass number create the corresponding list token.
 * Examples:
 *     'CFISLST(132)' expands to  'opcls_132_cfis_lst'
 *     'CFISLST(133)' expands to  'opcls_133_cfis_lst'
 */
#define CFISLST(_g_opcls)  opcls_ ## _g_opcls ## _cfis_lst

/* The type of the opclass list objects */
#define CFISLST_TYPE static const struct c_freq_lst

/* The number of elements of the array */
#define NELEMS QDF_ARRAY_SIZE

/**
 * Given a global opclass number create the corresponding cfis list and assign
 * the corresponding cfis array and size of the cfis array
 * Examples:
 *     'CREATE_CFIS_LST(132);'
 *     expands to
 *     '
 *     static const struct c_freq_lst opcls_132_cfis_lst =
 *                   {QDF_ARRAY_SIZE(opcls_132_cfis_arr), opcls_132_cfis_arr};
 *     '
 *
 *     'CREATE_CFIS_LST(133);'
 *     expands to
 *     '
 *     static const struct c_freq_lst opcls_133_cfis_lst =
 *                   {QDF_ARRAY_SIZE(opcls_133_cfis_arr), opcls_133_cfis_arr};
 *     '
 */
#define CREATE_CFIS_LST(_gopcls) \
CFISLST_TYPE CFISLST(_gopcls) = {NELEMS(CFISARR(_gopcls)), CFISARR(_gopcls)}

/* The NULL pointer to a cfis list object */
#define NULL_CFIS_LST NULL

/* CFIs for global opclass 131: (start Freq=5925 BW=20MHz) */
static const uint8_t opcls_131_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	  1, 5, 9, 13, 17, 21, 25, 29, 33,
	  37, 41, 45, 49, 53, 57, 61, 65, 69,
	  73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125,
	  129, 133, 137, 141, 145, 149, 153,
	  157, 161, 165, 169, 173, 177, 181,
	  185, 189, 193, 197, 201, 205, 209,
	  213, 217, 221, 225, 229, 233,
#endif
};

/* CFIs for global opclass 132: (start Freq=5925 BW=40MHz) */
static const uint8_t opcls_132_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	3, 11, 19, 27, 35, 43, 51, 59, 67, 75,
	83, 91, 99, 107, 115, 123, 131, 139, 147, 155,
	163, 171, 179, 187, 195, 203, 211, 219, 227,
#endif
};

/* CFIs for global opclass 133: (start Freq=5925 BW=80MHz) */
static const uint8_t opcls_133_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183,
	  199, 215,
#endif
};

/* CFIs for global opclass 134: (start Freq=5950 BW=160MHz) */
static const uint8_t opcls_134_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	15, 47, 79, 111, 143, 175, 207,
#endif
};

/* CFIs for global opclass 135: (start Freq=5950 BW=80MHz+80MHz) */
static const uint8_t opcls_135_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183,
	199, 215,
#endif
};

/* CFIs for global opclass 136: (start Freq=5925 BW=20MHz) */
static const uint8_t opcls_136_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	2,
#endif
};

/* CFIs for global opclass 137: (start Freq=5950 BW=320MHz) */
#ifdef WLAN_FEATURE_11BE
static const uint8_t opcls_137_cfis_arr[] = {
#ifdef CONFIG_AFC_SUPPORT
	31, 63, 95, 127, 159, 191,
#endif
};
#endif

/* Create the CFIS static constant lists */
CREATE_CFIS_LST(131);
CREATE_CFIS_LST(132);
CREATE_CFIS_LST(133);
CREATE_CFIS_LST(134);
CREATE_CFIS_LST(135);
CREATE_CFIS_LST(136);
#ifdef WLAN_FEATURE_11BE
CREATE_CFIS_LST(137);
#endif

static const struct reg_dmn_op_class_map_t global_op_class[] = {
	{81, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{82, 25, BW20, BIT(BEHAV_NONE), 2414,
	 {14},
	 NULL_CFIS_LST },
	{83, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9},
	 NULL_CFIS_LST },
	{84, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{115, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48},
	 NULL_CFIS_LST },
	{116, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44},
	 NULL_CFIS_LST },
	{117, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48},
	 NULL_CFIS_LST },
	{118, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64},
	 NULL_CFIS_LST },
	{119, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60},
	 NULL_CFIS_LST },
	{120, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64},
	 NULL_CFIS_LST },
	{121, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144},
	 NULL_CFIS_LST },
	{122, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132, 140},
	 NULL_CFIS_LST },
	{123, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136, 144},
	 NULL_CFIS_LST },
	{125, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165, 169, 173, 177},
	 NULL_CFIS_LST },
	{126, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {149, 157, 165, 173},
	 NULL_CFIS_LST },
	{127, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {153, 161, 169, 177},
	 NULL_CFIS_LST },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64,
	  100, 104, 108, 112, 116, 120, 124, 128,
	  132, 136, 140, 144, 149, 153, 157, 161,
	  165, 169, 173, 177},
	  NULL_CFIS_LST },
	{129, 160, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64,
	  100, 104, 108, 112, 116, 120, 124, 128,
	  149, 153, 157, 161, 165, 169, 173, 177},
	 NULL_CFIS_LST },
	{130, 80, BW80, BIT(BEHAV_BW80_PLUS), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64,
	  100, 104, 108, 112, 116, 120, 124, 128,
	  132, 136, 140, 144, 149, 153, 157, 161,
	  165, 169, 173, 177},
	 NULL_CFIS_LST },

#ifdef CONFIG_BAND_6GHZ
	{131, 20, BW20, BIT(BEHAV_NONE), 5950,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33,
	  37, 41, 45, 49, 53, 57, 61, 65, 69,
	  73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125,
	  129, 133, 137, 141, 145, 149, 153,
	  157, 161, 165, 169, 173, 177, 181,
	  185, 189, 193, 197, 201, 205, 209,
	  213, 217, 221, 225, 229, 233},
	&CFISLST(131)},

	{132, 40, BW40_LOW_PRIMARY, BIT(BEHAV_NONE), 5950,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49,
	  53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125, 129, 133, 137,
	  141, 145, 149, 153, 157, 161, 165, 169, 173, 177,
	  181, 185, 189, 193, 197, 201, 205, 209, 213, 217,
	  221, 225, 229, 233},
	&CFISLST(132)},

	{133, 80, BW80, BIT(BEHAV_NONE), 5950,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49,
	  53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97,
	  101, 105, 109, 113, 117, 121, 125, 129, 133, 137,
	  141, 145, 149, 153, 157, 161, 165, 169, 173,
	  177, 181, 185, 189, 193, 197, 201, 205, 209, 213,
	  217, 221, 225, 229, 233},
	&CFISLST(133)},

	{134, 160, BW80, BIT(BEHAV_NONE), 5950,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45,
	  49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89,
	  93, 97, 101, 105, 109, 113, 117, 121, 125,
	  129, 133, 137, 141, 145, 149, 153, 157, 161,
	  165, 169, 173, 177, 181, 185, 189, 193, 197,
	     201, 205, 209, 213, 217, 221, 225, 229, 233},
	&CFISLST(134)},

	{135, 80, BW80, BIT(BEHAV_BW80_PLUS), 5950,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41,
	  45, 49, 53, 57, 61, 65, 69, 73, 77, 81,
	  85, 89, 93, 97, 101, 105, 109, 113, 117,
	  121, 125, 129, 133, 137, 141, 145, 149,
	  153, 157, 161, 165, 169, 173, 177, 181,
	  185, 189, 193, 197, 201, 205, 209, 213,
	  217, 221, 225, 229, 233},
	&CFISLST(135)},

	{136, 20, BW20, BIT(BEHAV_NONE), 5925,
	 {2},
	&CFISLST(136)},
#ifdef WLAN_FEATURE_11BE
	{137, 320, BW20, BIT(BEHAV_NONE), 5950,
	 {1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41,
	  45, 49, 53, 57, 61, 65, 69, 73, 77, 81,
	  85, 89, 93, 97, 101, 105, 109, 113, 117,
	  121, 125, 129, 133, 137, 141, 145, 149,
	  153, 157, 161, 165, 169, 173, 177, 181,
	  185, 189, 193, 197, 201, 205, 209, 213,
	  217, 221, 225, 229, 233},
	&CFISLST(137)},
#endif
#endif
	{0, 0, 0, 0, 0, {0},
	NULL_CFIS_LST },
};

static const struct reg_dmn_op_class_map_t us_op_class[] = {
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48},
	 NULL_CFIS_LST },
	{2, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64},
	 NULL_CFIS_LST },
	{4, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144},
	 NULL_CFIS_LST },
	{5, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165},
	 NULL_CFIS_LST },
	{12, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
	 NULL_CFIS_LST },
	{22, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44},
	 NULL_CFIS_LST },
	{23, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60},
	 NULL_CFIS_LST },
	{24, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132, 140},
	 NULL_CFIS_LST },
	{26, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {149, 157},
	 NULL_CFIS_LST },
	{27, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48},
	 NULL_CFIS_LST },
	{28, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64},
	 NULL_CFIS_LST },
	{29, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136, 144},
	 NULL_CFIS_LST },
	{30, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {153, 161},
	 NULL_CFIS_LST },
	{31, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {153, 161},
	 NULL_CFIS_LST },
	{32, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7},
	 NULL_CFIS_LST },
	{33, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11},
	 NULL_CFIS_LST },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100,
	  104, 108, 112, 116, 120, 124, 128, 132,
	  136, 140, 144, 149, 153, 157, 161},
	 NULL_CFIS_LST },
	{129, 160, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100,
	  104, 108, 112, 116, 120, 124, 128},
	 NULL_CFIS_LST },
	{130, 80, BW80, BIT(BEHAV_BW80_PLUS), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100,
	  104, 108, 112, 116, 120, 124, 128, 132,
	  136, 140, 144, 149, 153, 157, 161},
	 NULL_CFIS_LST },
	{0, 0, 0, 0, 0, {0},
	 NULL_CFIS_LST },
};

static const struct reg_dmn_op_class_map_t euro_op_class[] = {
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48},
	 NULL_CFIS_LST },
	{2, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64},
	 NULL_CFIS_LST },
	{3, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120,
	  124, 128, 132, 136, 140},
	 NULL_CFIS_LST },
	{4, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{5, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44},
	 NULL_CFIS_LST },
	{6, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60},
	 NULL_CFIS_LST },
	{7, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132},
	 NULL_CFIS_LST },
	{8, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48},
	 NULL_CFIS_LST },
	{9, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64},
	 NULL_CFIS_LST },
	{10, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136},
	 NULL_CFIS_LST },
	{11, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9},
	 NULL_CFIS_LST },
	{12, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{17, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165, 169},
	 NULL_CFIS_LST },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120,
	  124, 128},
	 NULL_CFIS_LST },
	{129, 160, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100,
	  104, 108, 112, 116, 120, 124, 128},
	 NULL_CFIS_LST },
	{130, 80, BW80, BIT(BEHAV_BW80_PLUS), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120,
	  124, 128},
	 NULL_CFIS_LST },
	{0, 0, 0, 0, 0, {0},
	 NULL_CFIS_LST },
};

static const struct reg_dmn_op_class_map_t japan_op_class[] = {
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48},
	 NULL_CFIS_LST },
	{30, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{31, 25, BW20, BIT(BEHAV_NONE), 2414,
	 {14},
	 NULL_CFIS_LST },
	{32, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64},
	 NULL_CFIS_LST },
	{34, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140},
	 NULL_CFIS_LST },
	{36, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44},
	 NULL_CFIS_LST },
	{37, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60},
	 NULL_CFIS_LST },
	{39, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {100, 108, 116, 124, 132},
	 NULL_CFIS_LST },
	{41, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48},
	 NULL_CFIS_LST },
	{42, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {56, 64},
	 NULL_CFIS_LST },
	{44, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {104, 112, 120, 128, 136},
	 NULL_CFIS_LST },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120,
	  124, 128, 132, 136, 140, 144},
	 NULL_CFIS_LST },
	{129, 160, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100,
	  104, 108, 112, 116, 120, 124, 128},
	 NULL_CFIS_LST },
	{130, 80, BW80, BIT(BEHAV_BW80_PLUS), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120,
	  124, 128, 132, 136, 140, 144},
	 NULL_CFIS_LST },
	{0, 0, 0, 0, 0, {0},
	 NULL_CFIS_LST },
};

static const struct reg_dmn_op_class_map_t china_op_class[] = {
	{7, 25, BW20, BIT(BEHAV_NONE), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{8, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 2407,
	 {1, 2, 3, 4, 5, 6, 7, 8, 9},
	 NULL_CFIS_LST },
	{9, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 2407,
	 {5, 6, 7, 8, 9, 10, 11, 12, 13},
	 NULL_CFIS_LST },
	{1, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48},
	 NULL_CFIS_LST },
	{4, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {36, 44},
	 NULL_CFIS_LST },
	{117, 40, BW40_HIGH_PRIMARY, BIT(BEHAV_BW40_HIGH_PRIMARY), 5000,
	 {40, 48},
	 NULL_CFIS_LST },
	{2, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {52, 56, 60, 64},
	 NULL_CFIS_LST },
	{5, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {52, 60},
	 NULL_CFIS_LST },
	{3, 20, BW20, BIT(BEHAV_NONE), 5000,
	 {149, 153, 157, 161, 165},
	 NULL_CFIS_LST },
	{6, 40, BW40_LOW_PRIMARY, BIT(BEHAV_BW40_LOW_PRIMARY), 5000,
	 {149, 157},
	 NULL_CFIS_LST },
	{128, 80, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161},
	 NULL_CFIS_LST },
	{129, 160, BW80, BIT(BEHAV_NONE), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64,},
	 NULL_CFIS_LST },
	{130, 80, BW80, BIT(BEHAV_BW80_PLUS), 5000,
	 {36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161},
	 NULL_CFIS_LST },
	{0, 0, 0, 0, 0, {0},
	 NULL_CFIS_LST },
};
#ifdef HOST_OPCLASS
/**
 * reg_get_class_from_country()- Get Class from country
 * @country- Country
 *
 * Return: class.
 */
static const struct reg_dmn_op_class_map_t
*reg_get_class_from_country(const uint8_t *country)
{
	const struct reg_dmn_op_class_map_t *class = NULL;

	reg_debug_rl("Country %c%c 0x%x", country[0], country[1], country[2]);

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

	case OP_CLASS_CHINA:
		class = china_op_class;
		break;
	default:
		if (!qdf_mem_cmp(country, "US", 2))
			class = us_op_class;
		else if (!qdf_mem_cmp(country, "EU", 2))
			class = euro_op_class;
		else if (!qdf_mem_cmp(country, "JP", 2))
			class = japan_op_class;
		else if (!qdf_mem_cmp(country, "CN", 2))
			class = china_op_class;
		else
			class = global_op_class;
	}
	return class;
}

#ifdef CONFIG_AFC_SUPPORT
static bool reg_is_range_valid(struct freq_range *range)
{
	return (range->right > range->left);
}

/**
 * reg_is_subrange() - Check if range_first is a subrange of range_second
 * @range_first: Pointer to first range
 * @range_second: Pointer to first range
 *
 * Return: True if the range_first is a subrange range_second, else false
 */
static bool reg_is_subrange(struct freq_range *range_first,
			    struct freq_range *range_second)
{
	bool is_subrange;
	bool is_valid;

	is_valid = reg_is_range_valid(range_first) &&
		   reg_is_range_valid(range_second);

	if (!is_valid)
		return false;

	is_subrange = (range_first->left >= range_second->left) &&
		      (range_first->right <= range_second->right);

	return is_subrange;
}

/**
 * reg_is_cfi_freq_in_ranges() - Check if the given 'cfi' in the any of the
 * frequency ranges
 * @cfi_freq: The center frequency index frequency
 * @bw: bandwidth of the band with center freq cfi_freq
 * @p_frange_lst: Pointer to frequency range list (AFC)
 *
 * return: True if the cfi is in the ranges, else false
 */
static bool reg_is_cfi_freq_in_ranges(qdf_freq_t cfi_freq,
				      uint16_t bw,
				      struct wlan_afc_frange_list *p_frange_lst)
{
	uint32_t num_ranges;
	struct wlan_afc_freq_range_obj *p_range_objs;
	uint8_t i;
	bool is_cfi_supported = false;

	num_ranges = p_frange_lst->num_ranges;
	p_range_objs = &p_frange_lst->range_objs[0];
	for (i = 0; i <  num_ranges; i++) {
		qdf_freq_t cfi_band_left;
		qdf_freq_t cfi_band_right;
		struct freq_range range_cfi;
		struct freq_range range_chip;

		cfi_band_left = cfi_freq - bw / 2;
		cfi_band_right = cfi_freq + bw / 2;

		range_cfi = reg_init_freq_range(cfi_band_left,
						cfi_band_right);
		range_chip = reg_init_freq_range(p_range_objs->lowfreq,
						 p_range_objs->highfreq);
		is_cfi_supported = reg_is_subrange(&range_cfi, &range_chip);

		if (is_cfi_supported)
			return true;

		p_range_objs++;
	}

	return is_cfi_supported;
}

void reg_dmn_free_6g_opclasses_and_channels(struct wlan_objmgr_pdev *pdev,
					    uint8_t num_opclasses,
					    uint8_t *opclass_lst,
					    uint8_t *chansize_lst,
					    uint8_t *channel_lists[])
{
	/*
	 * All the elements of channel_lists were allocated as a single
	 * allocation with 'channel_lists[0]' holding the first location of the
	 * allocation. Therefore, freeing only 'channel_lists[0]' is enough.
	 * Freeing any other 'channel_lists[i]' will result in error of freeing
	 * unallocated memory.

	 */
	if (channel_lists)
		qdf_mem_free(channel_lists[0]);

	/*
	 * opclass_lst, chansize_lst and channel_lists were allocated as a
	 * single allocation with 'opclass_lst' holding the first location of
	 * allocation. Therefore, freeing only 'opclass_lst' is enough.
	 * Freeing chansize_lst, channel_lists will result in error of freeing
	 * unallocated memory.
	 */
	qdf_mem_free(opclass_lst);
}

/**
 * reg_dmn_get_num_6g_opclasses() - Calculate the number of opclasses in the
 * 6GHz band.
 * Return: The number of opclasses
 */
static uint8_t reg_dmn_get_num_6g_opclasses(struct wlan_objmgr_pdev *pdev)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;
	uint8_t count;

	op_class_tbl = global_op_class;

	count = 0;
	while (op_class_tbl && op_class_tbl->op_class) {
		const struct c_freq_lst *p_lst;

		p_lst = op_class_tbl->p_cfi_lst_obj;
		if (p_lst &&
		    reg_is_6ghz_op_class(pdev, op_class_tbl->op_class))
			count++;

		op_class_tbl++;
	}

	return count;
}

/**
 * reg_dmn_fill_6g_opcls_chan_lists() - Copy the channel lists for 6g opclasses
 * to the output argument list ('channel_lists')
 * @pdev: Pointer to pdev.
 * @p_frange_lst: Pointer to frequencey range list (AFC)
 * @chansize_lst: Array of sizes of channel lists
 * @channel_lists: The array list pointers where the channel lists are to be
 *                 copied.
 *
 * Return: Void
 */
static void reg_dmn_fill_6g_opcls_chan_lists(struct wlan_objmgr_pdev *pdev,
					     struct wlan_afc_frange_list *p_frange_lst,
					     uint8_t chansize_lst[],
					     uint8_t *channel_lists[])
{
	uint8_t i = 0;
	const struct reg_dmn_op_class_map_t *op_class_tbl;

	op_class_tbl = global_op_class;

	while (op_class_tbl && op_class_tbl->op_class) {
		const struct c_freq_lst *p_lst;

		p_lst = op_class_tbl->p_cfi_lst_obj;
		if (p_lst &&
		    reg_is_6ghz_op_class(pdev, op_class_tbl->op_class)) {
			uint8_t j;
			uint8_t cfi_idx = 0;
			uint8_t *dst;

			dst = channel_lists[i];
			for (j = 0; j < p_lst->num_cfis; j++) {
				uint8_t cfi;
				qdf_freq_t cfi_freq;
				qdf_freq_t start_freq = op_class_tbl->start_freq;
				uint16_t bw = op_class_tbl->chan_spacing;

				cfi = p_lst->p_cfis_arr[j];
				cfi_freq = start_freq +
					FREQ_TO_CHAN_SCALE * cfi;

				if (reg_is_cfi_freq_in_ranges(cfi_freq,
							      bw,
							      p_frange_lst)) {
					dst[cfi_idx++] = cfi;
				}
			}
			i++;
		}
		op_class_tbl++;
	}
}

static bool reg_is_val_within_range(qdf_freq_t val,
				    qdf_freq_t left,
				    qdf_freq_t right)
{
	bool is_within = false;

	if (val >= left && val <= right)
		is_within = true;

	return is_within;
}

QDF_STATUS reg_dmn_get_6g_opclasses_and_channels(struct wlan_objmgr_pdev *pdev,
						 struct wlan_afc_frange_list *p_frange_lst,
						 uint8_t *num_opclasses,
						 uint8_t **opclass_lst,
						 uint8_t **chansize_lst,
						 uint8_t **channel_lists[])
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;
	uint8_t *l_opcls_lst;
	uint8_t *l_chansize_lst;
	uint8_t count;
	uint8_t i;
	uint8_t **arr_chan_lists;
	uint16_t total_alloc_size;
	uint16_t opcls_lst_size;
	uint16_t chansize_lst_size;
	uint16_t arr_chan_lists_size;
	uint8_t *p_total_alloc1;
	uint8_t *p_total_alloc2;
	uint8_t *p_temp_alloc;

	*opclass_lst = NULL;
	*chansize_lst =  NULL;
	*channel_lists = NULL;

	op_class_tbl = global_op_class;

	*num_opclasses = reg_dmn_get_num_6g_opclasses(pdev);
	opcls_lst_size = *num_opclasses * sizeof(uint8_t);
	chansize_lst_size = *num_opclasses * sizeof(uint8_t);
	arr_chan_lists_size = *num_opclasses * sizeof(uint8_t *);

	total_alloc_size = 0;
	total_alloc_size += opcls_lst_size
		+ chansize_lst_size
		+ arr_chan_lists_size;

	p_total_alloc1 = qdf_mem_malloc(total_alloc_size);
	if (!p_total_alloc1) {
		reg_err("out-of-memory");
		return QDF_STATUS_E_NOMEM;
	}

	 /* Assign memory locations to each pointers */
	p_temp_alloc = p_total_alloc1;

	l_opcls_lst = p_temp_alloc;
	p_temp_alloc += opcls_lst_size;

	l_chansize_lst = p_temp_alloc;
	p_temp_alloc += chansize_lst_size;

	arr_chan_lists = (uint8_t **)p_temp_alloc;

	/* Fill arrays with opclasses and chanlist sizes */
	count = 0;
	while (op_class_tbl && op_class_tbl->op_class) {
		const struct c_freq_lst *p_lst;

		p_lst = op_class_tbl->p_cfi_lst_obj;
		if (p_lst &&
		    reg_is_6ghz_op_class(pdev, op_class_tbl->op_class)) {
			uint8_t n_supp_cfis = 0;
			uint8_t j;

			l_opcls_lst[count] = op_class_tbl->op_class;
			for (j = 0; j < p_lst->num_cfis; j++) {
				uint8_t cfi;
				qdf_freq_t cfi_freq;
				qdf_freq_t start_freq = op_class_tbl->start_freq;
				uint16_t bw = op_class_tbl->chan_spacing;

				cfi = p_lst->p_cfis_arr[j];
				cfi_freq = start_freq +
					FREQ_TO_CHAN_SCALE * cfi;
				if (reg_is_cfi_freq_in_ranges(cfi_freq,
							      bw,
							      p_frange_lst)) {
					n_supp_cfis++;
				}
			}
			l_chansize_lst[count] = n_supp_cfis;
			count++;
		}
		op_class_tbl++;
	}

	/* Calculate total alloction size for the array */
	total_alloc_size = 0;
	for (i = 0; i < *num_opclasses; i++)
		total_alloc_size += l_chansize_lst[i] * sizeof(uint8_t *);

	p_total_alloc2 = qdf_mem_malloc(total_alloc_size);
	if (!p_total_alloc2) {
		reg_err("out-of-memory");
		qdf_mem_free(p_total_alloc1);
		return QDF_STATUS_E_NOMEM;
	}

	/* Assign memory locations to each list pointers */
	p_temp_alloc = p_total_alloc2;
	for (i = 0; i < *num_opclasses; i++) {
		if (!l_chansize_lst[i])
			arr_chan_lists[i] = NULL;
		else
			arr_chan_lists[i] = p_temp_alloc;

		p_temp_alloc += l_chansize_lst[i] * sizeof(uint8_t *);
	}

	/* Fill the array with channl lists */
	reg_dmn_fill_6g_opcls_chan_lists(pdev, p_frange_lst, l_chansize_lst, arr_chan_lists);

	*opclass_lst = l_opcls_lst;
	*chansize_lst = l_chansize_lst;
	*channel_lists = arr_chan_lists;

	return QDF_STATUS_SUCCESS;
}
#endif /* CONFIG_AFC_SUPPORT */

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

uint16_t reg_dmn_get_chanwidth_from_opclass_auto(uint8_t *country,
						 uint8_t channel,
						 uint8_t opclass)
{
	uint16_t ret;
	uint8_t global_country[REG_ALPHA2_LEN + 1];

	ret = reg_dmn_get_chanwidth_from_opclass(country, channel, opclass);

	if (!ret) {
		global_country[2] = OP_CLASS_GLOBAL;
		ret = reg_dmn_get_chanwidth_from_opclass(global_country,
							 channel, opclass);
	}

	return ret;
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

uint8_t reg_dmn_get_opclass_from_freq_width(uint8_t *country,
					    qdf_freq_t freq,
					    uint16_t ch_width,
					    uint16_t behav_limit)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl = NULL;
	uint16_t i = 0;

	op_class_tbl = reg_get_class_from_country(country);

	while (op_class_tbl && op_class_tbl->op_class) {
		if (op_class_tbl->chan_spacing == ch_width) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     op_class_tbl->channels[i]); i++) {
				if ((op_class_tbl->start_freq +
				     (FREQ_TO_CHAN_SCALE *
				      op_class_tbl->channels[i]) == freq) &&
				    (behav_limit & op_class_tbl->behav_limit)) {
					return op_class_tbl->op_class;
				}
			}
		}
		op_class_tbl++;
	}

	return 0;
}

static void
reg_get_band_cap_from_chan_set(const struct reg_dmn_op_class_map_t
			       *op_class_tbl,
			       uint8_t *supported_band)
{
	qdf_freq_t chan_freq = op_class_tbl->start_freq +
						(op_class_tbl->channels[0] *
						 FREQ_TO_CHAN_SCALE);

	if (reg_is_24ghz_ch_freq(chan_freq))
		*supported_band |= BIT(REG_BAND_2G);
	else if (reg_is_5ghz_ch_freq(chan_freq))
		*supported_band |= BIT(REG_BAND_5G);
	else if (reg_is_6ghz_chan_freq(chan_freq))
		*supported_band |= BIT(REG_BAND_6G);
	else
		reg_err_rl("Unknown band");
}

uint8_t reg_get_band_cap_from_op_class(const uint8_t *country,
				       uint8_t num_of_opclass,
				       const uint8_t *opclass)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;
	uint8_t supported_band = 0, opclassidx;

	op_class_tbl = reg_get_class_from_country(country);

	while (op_class_tbl && op_class_tbl->op_class) {
		for (opclassidx = 0; opclassidx < num_of_opclass;
		     opclassidx++) {
			if (op_class_tbl->op_class == opclass[opclassidx]) {
				reg_get_band_cap_from_chan_set(op_class_tbl,
							       &supported_band);
			}
		}
		op_class_tbl++;
	}

	if (!supported_band)
		reg_err_rl("None of the operating classes is found");

	return supported_band;
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
/**
 * reg_find_opclass_absent_in_ctry_opclss_tables() - Check Global Opclass table
 * when Opclass is not present in specific country.
 * @pdev - Pointer to pdev
 * @freq - Destination Frequency
 * @chan_width- Channel Width
 * @global_tbl_lookup - Global Table Lookup
 * @behav_limit - Behav Limit
 * @op_class - Pointer to Opclass
 * @chan_num - Pointer to Channel
 *
 * Return: Void
 */
static void
reg_find_opclass_absent_in_ctry_opclss_tables(struct wlan_objmgr_pdev *pdev,
					      qdf_freq_t freq,
					      uint16_t chan_width,
					      bool global_tbl_lookup,
					      uint16_t behav_limit,
					      uint8_t *op_class,
					      uint8_t *chan_num)
{
	if (!global_tbl_lookup && !*op_class) {
		global_tbl_lookup = true;
		reg_freq_width_to_chan_op_class(pdev, freq,
						chan_width,
						global_tbl_lookup,
						behav_limit,
						op_class,
						chan_num);
	}
}

void reg_freq_width_to_chan_op_class_auto(struct wlan_objmgr_pdev *pdev,
					  qdf_freq_t freq,
					  uint16_t chan_width,
					  bool global_tbl_lookup,
					  uint16_t behav_limit,
					  uint8_t *op_class,
					  uint8_t *chan_num)
{
	if (reg_freq_to_band(freq) == REG_BAND_6G) {
		global_tbl_lookup = true;
		if (chan_width == BW_40_MHZ)
			behav_limit = BIT(BEHAV_NONE);
	} else if (reg_is_5dot9_ghz_freq(pdev, freq)) {
		global_tbl_lookup = true;
	} else {
		global_tbl_lookup = false;
	}

	*op_class = 0;
	reg_freq_width_to_chan_op_class(pdev, freq,
					chan_width,
					global_tbl_lookup,
					behav_limit,
					op_class,
					chan_num);
	reg_find_opclass_absent_in_ctry_opclss_tables(pdev, freq,
						      chan_width,
						      global_tbl_lookup,
						      behav_limit,
						      op_class,
						      chan_num);
}

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
		reg_err_rl("Invalid chan enum %d", chan_enum);
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
			op_class_tbl = china_op_class;
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

	reg_err_rl("no op class for frequency %d", freq);
}

void reg_freq_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
			       qdf_freq_t freq,
			       bool global_tbl_lookup,
			       uint16_t behav_limit,
			       uint8_t *op_class,
			       uint8_t *chan_num)
{
	enum channel_enum chan_enum;
	struct regulatory_channel *cur_chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct ch_params chan_params;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err_rl("NULL pdev reg obj");
		return;
	}

	cur_chan_list = pdev_priv_obj->cur_chan_list;

	chan_enum = reg_get_chan_enum_for_freq(freq);

	if (chan_enum == INVALID_CHANNEL) {
		reg_err_rl("Invalid chan enum %d", chan_enum);
		return;
	}

	chan_params.ch_width = CH_WIDTH_MAX;
	reg_set_channel_params_for_freq(pdev, freq, 0, &chan_params);

	reg_freq_width_to_chan_op_class(pdev, freq,
					reg_get_bw_value(chan_params.ch_width),
					global_tbl_lookup,
					behav_limit,
					op_class,
					chan_num);
}

bool reg_is_freq_in_country_opclass(struct wlan_objmgr_pdev *pdev,
				    const uint8_t country[3],
				    uint8_t op_class,
				    qdf_freq_t chan_freq)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;
	uint8_t i;

	op_class_tbl = reg_get_class_from_country((uint8_t *)country);

	while (op_class_tbl && op_class_tbl->op_class) {
		if  (op_class_tbl->op_class == op_class) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     op_class_tbl->channels[i]); i++) {
				if (op_class_tbl->channels[i] *
				    FREQ_TO_CHAN_SCALE +
				    op_class_tbl->start_freq == chan_freq)
					return true;
			}
		}
		op_class_tbl++;
	}
	return false;
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
			op_class_tbl = china_op_class;
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
			op_class_tbl = china_op_class;
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
					return op_class_tbl->start_freq +
						(chan * FREQ_TO_CHAN_SCALE);
				}
			}
			reg_err_rl("Channel not found");
			return 0;
		}
		op_class_tbl++;
	}
	reg_err_rl("Invalid opclass");
	return 0;
}

qdf_freq_t reg_chan_opclass_to_freq_auto(uint8_t chan, uint8_t op_class,
					 bool global_tbl_lookup)
{
	if ((op_class >= MIN_6GHZ_OPER_CLASS) &&
	    (op_class <= MAX_6GHZ_OPER_CLASS)) {
		global_tbl_lookup = true;
	} else {
		qdf_freq_t freq = reg_chan_opclass_to_freq(chan,
				op_class,
				global_tbl_lookup);
		if (freq)
			return freq;
		global_tbl_lookup = true;
	}

	return reg_chan_opclass_to_freq(chan, op_class, global_tbl_lookup);
}

#ifdef HOST_OPCLASS_EXT
qdf_freq_t reg_country_chan_opclass_to_freq(struct wlan_objmgr_pdev *pdev,
					    const uint8_t country[3],
					    uint8_t chan, uint8_t op_class,
					    bool strict)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl, *op_class_tbl_org;
	uint16_t i;

	if (reg_is_6ghz_op_class(pdev, op_class))
		op_class_tbl_org = global_op_class;
	else
		op_class_tbl_org =
			reg_get_class_from_country((uint8_t *)country);
	op_class_tbl = op_class_tbl_org;
	while (op_class_tbl && op_class_tbl->op_class) {
		if  (op_class_tbl->op_class == op_class) {
			for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
				     op_class_tbl->channels[i]); i++) {
				if (op_class_tbl->channels[i] == chan)
					return op_class_tbl->start_freq +
						(chan * FREQ_TO_CHAN_SCALE);
			}
		}
		op_class_tbl++;
	}
	reg_debug_rl("Not found ch %d in op class %d ch list, strict %d",
		     chan, op_class, strict);
	if (strict)
		return 0;

	op_class_tbl = op_class_tbl_org;
	while (op_class_tbl && op_class_tbl->op_class) {
		for (i = 0; (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
			     op_class_tbl->channels[i]); i++) {
			if (op_class_tbl->channels[i] == chan)
				return op_class_tbl->start_freq +
					(chan * FREQ_TO_CHAN_SCALE);
		}
		op_class_tbl++;
	}
	reg_debug_rl("Got invalid freq 0 for ch %d", chan);

	return 0;
}
#endif

static void
reg_get_op_class_tbl_by_chan_map(const struct
				 reg_dmn_op_class_map_t **op_class_tbl)
{
	if (channel_map == channel_map_us)
		*op_class_tbl = us_op_class;
	else if (channel_map == channel_map_eu)
		*op_class_tbl = euro_op_class;
	else if (channel_map == channel_map_china)
		*op_class_tbl = china_op_class;
	else if (channel_map == channel_map_jp)
		*op_class_tbl = japan_op_class;
	else
		*op_class_tbl = global_op_class;
}

/**
 * reg_get_channel_cen - Calculate central channel in the channel set.
 *
 * @op_class_tbl - Pointer to op_class_tbl.
 * @idx - Pointer to channel index.
 * @num_channels - Number of channels.
 * @center_chan - Pointer to center channel number
 *
 * Return : void
 */
static void reg_get_channel_cen(const struct
				reg_dmn_op_class_map_t *op_class_tbl,
				uint8_t *idx,
				uint8_t num_channels,
				uint8_t *center_chan)
{
	uint8_t i;
	uint16_t new_chan = 0;

	for (i = *idx; i < (*idx + num_channels); i++)
		new_chan += op_class_tbl->channels[i];

	new_chan = new_chan / num_channels;
	*center_chan = new_chan;
	*idx = *idx + num_channels;
}

/**
 * reg_get_chan_or_chan_center - Calculate central channel in the channel set.
 *
 * @op_class_tbl - Pointer to op_class_tbl.
 * @idx - Pointer to channel index.
 *
 * Return : Center channel number
 */
static uint8_t reg_get_chan_or_chan_center(const struct
					   reg_dmn_op_class_map_t *op_class_tbl,
					   uint8_t *idx)
{
	uint8_t center_chan;

	if (((op_class_tbl->chan_spacing == BW_80_MHZ) &&
	     (op_class_tbl->behav_limit == BIT(BEHAV_NONE))) ||
	    ((op_class_tbl->chan_spacing == BW_80_MHZ) &&
	     (op_class_tbl->behav_limit == BIT(BEHAV_BW80_PLUS)))) {
		reg_get_channel_cen(op_class_tbl,
				    idx,
				    NUM_20_MHZ_CHAN_IN_80_MHZ_CHAN,
				    &center_chan);
	} else if (op_class_tbl->chan_spacing == BW_160_MHZ) {
		reg_get_channel_cen(op_class_tbl,
				    idx,
				    NUM_20_MHZ_CHAN_IN_160_MHZ_CHAN,
				    &center_chan);
	} else {
		center_chan = op_class_tbl->channels[*idx];
		*idx = *idx + 1;
	}

	return center_chan;
}

/**
 * reg_get_channels_from_opclassmap()- Get channels from the opclass map
 * @pdev: Pointer to pdev
 * @reg_ap_cap: Pointer to reg_ap_cap
 * @index: Pointer to index of reg_ap_cap
 * @op_class_tbl: Pointer to op_class_tbl
 * @is_opclass_operable: Set true if opclass is operable, else set false
 *
 * Populate channels from opclass map to reg_ap_cap as supported and
 * non-supported channels.
 *
 * Return: void.
 */
static void
reg_get_channels_from_opclassmap(
		struct wlan_objmgr_pdev *pdev,
		struct regdmn_ap_cap_opclass_t *reg_ap_cap,
		uint8_t index,
		const struct reg_dmn_op_class_map_t *op_class_tbl,
		bool *is_opclass_operable)
{
	uint8_t op_cls_chan;
	qdf_freq_t search_freq;
	bool is_freq_present;
	uint8_t chan_idx = 0, n_sup_chans = 0, n_unsup_chans = 0;

	while (op_class_tbl->channels[chan_idx]) {
		op_cls_chan = op_class_tbl->channels[chan_idx];
		search_freq = op_class_tbl->start_freq +
					(FREQ_TO_CHAN_SCALE * op_cls_chan);
		is_freq_present =
			reg_is_freq_present_in_cur_chan_list(pdev, search_freq);

		if (!is_freq_present) {
			reg_ap_cap[index].non_sup_chan_list[n_unsup_chans++] =
				reg_get_chan_or_chan_center(op_class_tbl,
							    &chan_idx);
			reg_ap_cap[index].num_non_supported_chan++;
		} else {
			reg_ap_cap[index].sup_chan_list[n_sup_chans++] =
				reg_get_chan_or_chan_center(op_class_tbl,
							    &chan_idx);
			reg_ap_cap[index].num_supported_chan++;
		}
	}

	if (reg_ap_cap[index].num_supported_chan >= 1)
		*is_opclass_operable = true;
}

QDF_STATUS reg_get_opclass_details(struct wlan_objmgr_pdev *pdev,
				   struct regdmn_ap_cap_opclass_t *reg_ap_cap,
				   uint8_t *n_opclasses,
				   uint8_t max_supp_op_class,
				   bool global_tbl_lookup)
{
	uint8_t max_reg_power = 0;
	const struct reg_dmn_op_class_map_t *op_class_tbl;
	uint8_t index = 0;

	if (global_tbl_lookup)
		op_class_tbl = global_op_class;
	else
		reg_get_op_class_tbl_by_chan_map(&op_class_tbl);

	max_reg_power = reg_get_max_tx_power(pdev);

	while (op_class_tbl->op_class && (index < max_supp_op_class)) {
		bool is_opclass_operable = false;

		qdf_mem_zero(reg_ap_cap[index].sup_chan_list,
			     REG_MAX_CHANNELS_PER_OPERATING_CLASS);
		reg_ap_cap[index].num_supported_chan = 0;
		qdf_mem_zero(reg_ap_cap[index].non_sup_chan_list,
			     REG_MAX_CHANNELS_PER_OPERATING_CLASS);
		reg_ap_cap[index].num_non_supported_chan = 0;
		reg_get_channels_from_opclassmap(pdev,
						 reg_ap_cap,
						 index,
						 op_class_tbl,
						 &is_opclass_operable);
		if (is_opclass_operable) {
			reg_ap_cap[index].op_class = op_class_tbl->op_class;
			reg_ap_cap[index].ch_width =
						op_class_tbl->chan_spacing;
			reg_ap_cap[index].start_freq =
						op_class_tbl->start_freq;
			reg_ap_cap[index].max_tx_pwr_dbm = max_reg_power;
			reg_ap_cap[index].behav_limit =
						op_class_tbl->behav_limit;
			index++;
		}

		op_class_tbl++;
	}

	*n_opclasses = index;

	return QDF_STATUS_SUCCESS;
}

bool reg_is_6ghz_op_class(struct wlan_objmgr_pdev *pdev, uint8_t op_class)
{
	return ((op_class >= MIN_6GHZ_OPER_CLASS) &&
		(op_class <= MAX_6GHZ_OPER_CLASS));
}

/**
 * reg_is_opclass_band_found - Check if the input opclass is 2G or 5G.
 *
 * @country - Pointer to country.
 * @op_class - Operating class.
 * @bandmask = Bitmask for band.
 *
 * Return : Return true if the input opclass' band (2Ghz or 5Ghz) matches one
 * of bandmask's band.
 */
static bool reg_is_opclass_band_found(const uint8_t *country,
				      uint8_t op_class,
				      uint8_t bandmask)
{
	const struct reg_dmn_op_class_map_t *op_class_tbl;

	op_class_tbl = reg_get_class_from_country((uint8_t *)country);

	while (op_class_tbl && op_class_tbl->op_class) {
		if (op_class_tbl->op_class == op_class) {
			qdf_freq_t freq = op_class_tbl->start_freq +
			(op_class_tbl->channels[0] * FREQ_TO_CHAN_SCALE);

			if ((bandmask & BIT(REG_BAND_5G)) &&
			    REG_IS_5GHZ_FREQ(freq))
				return true;

			if ((bandmask & BIT(REG_BAND_2G)) &&
			    REG_IS_24GHZ_CH_FREQ(freq))
				return true;

			return false;
		}

		op_class_tbl++;
	}

	reg_err_rl("Opclass %d is not found", op_class);

	return false;
}

bool reg_is_5ghz_op_class(const uint8_t *country, uint8_t op_class)
{
	return reg_is_opclass_band_found(country, op_class, BIT(REG_BAND_5G));
}

bool reg_is_2ghz_op_class(const uint8_t *country, uint8_t op_class)
{
	return reg_is_opclass_band_found(country, op_class, BIT(REG_BAND_2G));
}
#endif
