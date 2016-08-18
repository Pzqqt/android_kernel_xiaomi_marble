/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/*===========================================================================

			s a p F s m . C

   OVERVIEW:

   This software unit holds the implementation of the WLAN SAP Finite
   State Machine modules

   DEPENDENCIES:

   Are listed for each API below.
   ===========================================================================*/

/*----------------------------------------------------------------------------
 * Include Files
 * -------------------------------------------------------------------------*/
#include "sap_internal.h"
/* Pick up the SME API definitions */
#include "sme_api.h"
/* Pick up the PMC API definitions */
#include "cds_utils.h"
#include "cds_ieee80211_common_i.h"
#include "cds_reg_service.h"
#include "qdf_util.h"
#include "cds_concurrency.h"

/*----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Type Declarations
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Global Data Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *  External declarations for global context
 * -------------------------------------------------------------------------*/
#ifdef FEATURE_WLAN_CH_AVOID
extern sapSafeChannelType safe_channels[];
#endif /* FEATURE_WLAN_CH_AVOID */

/*----------------------------------------------------------------------------
 * Static Variable Definitions
 * -------------------------------------------------------------------------*/

#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
/*
 * TODO: At present SAP Channel leakage matrix for ch 144
 * is not available from system's team. So to play it safe
 * and avoid crash if channel 144 is request, in following
 * matix channel 144 is added such that it will cause code
 * to avoid selecting channel 144.
 *
 * THESE ENTRIES SHOULD BE REPLACED WITH CORRECT VALUES AS
 * PROVIDED BY SYSTEM'S TEAM.
 */

/* channel tx leakage table - ht80 */
tSapChanMatrixInfo ht80_chan[] = {
	{52,
	 {{36, 148}, {40, 199},
	  {44, 193}, {48, 197},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, 153},
	  {60, 137}, {64, 134},
	  {100, 358}, {104, 350},
	  {108, 404}, {112, 344},
	  {116, 424}, {120, 429},
	  {124, 437}, {128, 435},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },


	{56,
	 {{36, 171}, {40, 178},
	  {44, 171}, {48, 178},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, 280},
	  {100, 351}, {104, 376},
	  {108, 362}, {112, 362},
	  {116, 403}, {120, 397},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{60,
	 {{36, 156}, {40, 146},
	  {44, SAP_TX_LEAKAGE_MIN}, {48, SAP_TX_LEAKAGE_MIN},
	  {52, 180}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, 376}, {104, 360},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, SAP_TX_LEAKAGE_MAX},
	  {116, 395}, {120, 399},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{64,
	 {{36, 217}, {40, 221},
	  {44, SAP_TX_LEAKAGE_MIN}, {48, SAP_TX_LEAKAGE_MIN},
	  {52, 176}, {56, 176},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, 384}, {104, 390},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, SAP_TX_LEAKAGE_MAX},
	  {116, 375}, {120, 374},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{100,
	 {{36, 357}, {40, 326},
	  {44, 321}, {48, 326},
	  {52, 378}, {56, 396},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, 196}, {112, 116},
	  {116, 166}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{104,
	 {{36, 325}, {40, 325},
	  {44, 305}, {48, 352},
	  {52, 411}, {56, 411},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, 460},
	  {116, 198}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{108,
	 {{36, 304}, {40, 332},
	  {44, 310}, {48, 335},
	  {52, 431}, {56, 391},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 280}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, 185}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{112,
	 {{36, 327}, {40, 335},
	  {44, 331}, {48, 345},
	  {52, 367}, {56, 401},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 131}, {104, 132},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, 189}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{116,
	 {{36, 384}, {40, 372},
	  {44, 389}, {48, 396},
	  {52, 348}, {56, 336},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 172}, {104, 169},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{120,
	 {{36, 395}, {40, 419},
	  {44, 439}, {48, 407},
	  {52, 321}, {56, 334},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 134}, {104, 186},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, 159},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{124,
	 {{36, 469}, {40, 433},
	  {44, 434}, {48, 435},
	  {52, 332}, {56, 345},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 146}, {104, 177},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, 350}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, 138},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{128,
	 {{36, 408}, {40, 434},
	  {44, 449}, {48, 444},
	  {52, 341}, {56, 374},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 205}, {104, 208},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, 142}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{132,
	 {{36, SAP_TX_LEAKAGE_MAX}, {40, SAP_TX_LEAKAGE_MAX},
	  {44, SAP_TX_LEAKAGE_MAX}, {48, SAP_TX_LEAKAGE_MAX},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{136,
	 {{36, SAP_TX_LEAKAGE_MAX}, {40, SAP_TX_LEAKAGE_MAX},
	  {44, SAP_TX_LEAKAGE_MAX}, {48, SAP_TX_LEAKAGE_MAX},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{140,
	 {{36, SAP_TX_LEAKAGE_MAX}, {40, SAP_TX_LEAKAGE_MAX},
	  {44, SAP_TX_LEAKAGE_MAX}, {48, SAP_TX_LEAKAGE_MAX},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },
};

/* channel tx leakage table - ht40 */
tSapChanMatrixInfo ht40_chan[] = {
	{52,
	 {{36, SAP_TX_LEAKAGE_AUTO_MIN}, {40, SAP_TX_LEAKAGE_AUTO_MIN},
	  {44, 230}, {48, 230},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, SAP_TX_LEAKAGE_AUTO_MIN}, {64, SAP_TX_LEAKAGE_AUTO_MIN},
	  {100, 625}, {104, 323},
	  {108, 646}, {112, 646},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{56,
	 {{36, SAP_TX_LEAKAGE_AUTO_MIN}, {40, SAP_TX_LEAKAGE_AUTO_MIN},
	  {44, SAP_TX_LEAKAGE_AUTO_MIN}, {48, SAP_TX_LEAKAGE_AUTO_MIN},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, 611}, {104, 611},
	  {108, 617}, {112, 617},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{60,
	 {{36, SAP_TX_LEAKAGE_AUTO_MIN}, {40, SAP_TX_LEAKAGE_AUTO_MIN},
	  {44, SAP_TX_LEAKAGE_AUTO_MIN}, {48, SAP_TX_LEAKAGE_AUTO_MIN},
	  {52, 190}, {56, 190},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, 608}, {104, 608},
	  {108, 623}, {112, 623},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{64,
	 {{36, SAP_TX_LEAKAGE_AUTO_MIN}, {40, SAP_TX_LEAKAGE_AUTO_MIN},
	  {44, SAP_TX_LEAKAGE_AUTO_MIN}, {48, SAP_TX_LEAKAGE_AUTO_MIN},
	  {52, 295}, {56, 295},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, 594}, {104, 594},
	  {108, 625}, {112, 625},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{100,
	 {{36, 618}, {40, 618},
	  {44, 604}, {48, 604},
	  {52, 596}, {56, 596},
	  {60, 584}, {64, 584},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, 299}, {112, 299},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, SAP_TX_LEAKAGE_AUTO_MIN},
	  {124, SAP_TX_LEAKAGE_AUTO_MIN}, {128, SAP_TX_LEAKAGE_AUTO_MIN},
	  {132, 538}, {136, 538},
	  {140, 598},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{104,
	 {{36, 636}, {40, 636},
	  {44, 601}, {48, 601},
	  {52, 616}, {56, 616},
	  {60, 584}, {64, 584},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, SAP_TX_LEAKAGE_AUTO_MIN},
	  {124, SAP_TX_LEAKAGE_AUTO_MIN}, {128, SAP_TX_LEAKAGE_AUTO_MIN},
	  {132, 553}, {136, 553},
	  {140, 568},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{108,
	 {{36, 600}, {40, 600},
	  {44, 627}, {48, 627},
	  {52, 611}, {56, 611},
	  {60, 611}, {64, 611},
	  {100, 214}, {104, 214},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, SAP_TX_LEAKAGE_AUTO_MIN},
	  {124, SAP_TX_LEAKAGE_AUTO_MIN}, {128, SAP_TX_LEAKAGE_AUTO_MIN},
	  {132, SAP_TX_LEAKAGE_AUTO_MIN}, {136, SAP_TX_LEAKAGE_AUTO_MIN},
	  {140, 534},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{112,
	 {{36, 645}, {40, 645},
	  {44, 641}, {48, 641},
	  {52, 618}, {56, 618},
	  {60, 612}, {64, 612},
	  {100, 293}, {104, 293},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_AUTO_MIN}, {128, SAP_TX_LEAKAGE_AUTO_MIN},
	  {132, SAP_TX_LEAKAGE_AUTO_MIN}, {136, SAP_TX_LEAKAGE_AUTO_MIN},
	  {140, 521},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{116,
	 {{36, 661}, {40, 661},
	  {44, 624}, {48, 624},
	  {52, 634}, {56, 634},
	  {60, 611}, {64, 611},
	  {100, SAP_TX_LEAKAGE_AUTO_MIN}, {104, SAP_TX_LEAKAGE_AUTO_MIN},
	  {108, 217}, {112, 217},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_AUTO_MIN}, {128, SAP_TX_LEAKAGE_AUTO_MIN},
	  {132, SAP_TX_LEAKAGE_AUTO_MIN}, {136, SAP_TX_LEAKAGE_AUTO_MIN},
	  {140, SAP_TX_LEAKAGE_AUTO_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{120,
	 {{36, 667}, {40, 667},
	  {44, 645}, {48, 645},
	  {52, 633}, {56, 633},
	  {60, 619}, {64, 619},
	  {100, SAP_TX_LEAKAGE_AUTO_MIN}, {104, SAP_TX_LEAKAGE_AUTO_MIN},
	  {108, 291}, {112, 291},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_AUTO_MIN}, {136, SAP_TX_LEAKAGE_AUTO_MIN},
	  {140, SAP_TX_LEAKAGE_AUTO_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{124,
	 {{36, 676}, {40, 676},
	  {44, 668}, {48, 668},
	  {52, 595}, {56, 595},
	  {60, 622}, {64, 622},
	  {100, SAP_TX_LEAKAGE_AUTO_MIN}, {104, SAP_TX_LEAKAGE_AUTO_MIN},
	  {108, SAP_TX_LEAKAGE_AUTO_MIN}, {112, SAP_TX_LEAKAGE_AUTO_MIN},
	  {116, 225}, {120, 225},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_AUTO_MIN}, {136, SAP_TX_LEAKAGE_AUTO_MIN},
	  {140, SAP_TX_LEAKAGE_AUTO_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{128,
	 {{36, 678}, {40, 678},
	  {44, 664}, {48, 664},
	  {52, 651}, {56, 651},
	  {60, 643}, {64, 643},
	  {100, SAP_TX_LEAKAGE_AUTO_MIN}, {104, SAP_TX_LEAKAGE_AUTO_MIN},
	  {108, SAP_TX_LEAKAGE_AUTO_MIN}, {112, SAP_TX_LEAKAGE_AUTO_MIN},
	  {116, 293}, {120, 293},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_AUTO_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{132,
	 {{36, 689}, {40, 689},
	  {44, 669}, {48, 669},
	  {52, 662}, {56, 662},
	  {60, 609}, {64, 609},
	  {100, 538}, {104, 538},
	  {108, SAP_TX_LEAKAGE_AUTO_MIN}, {112, SAP_TX_LEAKAGE_AUTO_MIN},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, SAP_TX_LEAKAGE_AUTO_MIN},
	  {124, 247}, {128, 247},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{136,
	 {{36, 703}, {40, 703},
	  {44, 688}, {48, SAP_TX_LEAKAGE_MIN},
	  {52, 671}, {56, 671},
	  {60, 658}, {64, 658},
	  {100, 504}, {104, 504},
	  {108, SAP_TX_LEAKAGE_AUTO_MIN}, {112, SAP_TX_LEAKAGE_AUTO_MIN},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, SAP_TX_LEAKAGE_AUTO_MIN},
	  {124, 289}, {128, 289},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{140,
	 {{36, 695}, {40, 695},
	  {44, 684}, {48, 684},
	  {52, 664}, {56, 664},
	  {60, 658}, {64, 658},
	  {100, 601}, {104, 601},
	  {108, 545}, {112, 545},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, SAP_TX_LEAKAGE_AUTO_MIN},
	  {124, SAP_TX_LEAKAGE_AUTO_MIN}, {128, SAP_TX_LEAKAGE_AUTO_MIN},
	  {132, 262}, {136, 262},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

};

/* channel tx leakage table - ht20 */
tSapChanMatrixInfo ht20_chan[] = {
	{52,
	 {{36, SAP_TX_LEAKAGE_AUTO_MIN}, {40, 286},
	  {44, 225}, {48, 121},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, 300}, {64, SAP_TX_LEAKAGE_AUTO_MIN},
	  {100, 637}, {104, SAP_TX_LEAKAGE_MAX},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, SAP_TX_LEAKAGE_MAX},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{56,
	 {{36, 468}, {40, SAP_TX_LEAKAGE_AUTO_MIN},
	  {44, SAP_TX_LEAKAGE_AUTO_MIN}, {48, 206},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, SAP_TX_LEAKAGE_MAX}, {104, SAP_TX_LEAKAGE_MAX},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, SAP_TX_LEAKAGE_MAX},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{60,
	 {{36, 507}, {40, 440},
	  {44, SAP_TX_LEAKAGE_AUTO_MIN}, {48, 313},
	  {52, SAP_TX_LEAKAGE_MIN}, {56, SAP_TX_LEAKAGE_MIN},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, SAP_TX_LEAKAGE_MAX}, {104, SAP_TX_LEAKAGE_MAX},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, SAP_TX_LEAKAGE_MAX},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{64,
	 {{36, 516}, {40, 520},
	  {44, 506}, {48, SAP_TX_LEAKAGE_AUTO_MIN},
	  {52, 301}, {56, 258},
	  {60, SAP_TX_LEAKAGE_MIN}, {64, SAP_TX_LEAKAGE_MIN},
	  {100, 620}, {104, 617},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, SAP_TX_LEAKAGE_MAX},
	  {116, SAP_TX_LEAKAGE_MAX}, {120, SAP_TX_LEAKAGE_MAX},
	  {124, SAP_TX_LEAKAGE_MAX}, {128, SAP_TX_LEAKAGE_MAX},
	  {132, SAP_TX_LEAKAGE_MAX}, {136, SAP_TX_LEAKAGE_MAX},
	  {140, SAP_TX_LEAKAGE_MAX},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{100,
	 {{36, 616}, {40, 601},
	  {44, 604}, {48, 589},
	  {52, 612}, {56, 592},
	  {60, 590}, {64, 582},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, 131},
	  {108, SAP_TX_LEAKAGE_AUTO_MIN}, {112, SAP_TX_LEAKAGE_AUTO_MIN},
	  {116, SAP_TX_LEAKAGE_AUTO_MIN}, {120, 522},
	  {124, 571}, {128, 589},
	  {132, 593}, {136, 598},
	  {140, 594},
	  {144, SAP_TX_LEAKAGE_MIN},
	  } },

	{104,
	 {{36, 622}, {40, 624},
	  {44, 618}, {48, 610},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, SAP_TX_LEAKAGE_MIN}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, 463},
	  {116, 483}, {120, 503},
	  {124, 523}, {128, 565},
	  {132, 570}, {136, 588},
	  {140, 585},
	  {144, SAP_TX_LEAKAGE_MIN},
	  } },

	{108,
	 {{36, 620}, {40, 638},
	  {44, 611}, {48, 614},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 477}, {104, SAP_TX_LEAKAGE_MIN},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, 477}, {120, 497},
	  {124, 517}, {128, 537},
	  {132, 557}, {136, 577},
	  {140, 603},
	  {144, SAP_TX_LEAKAGE_MIN},
	  } },

	{112,
	 {{36, 636}, {40, 623},
	  {44, 638}, {48, 628},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, 606},
	  {100, 501}, {104, 481},
	  {108, SAP_TX_LEAKAGE_MIN}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, 481},
	  {124, 501}, {128, 421},
	  {132, 541}, {136, 561},
	  {140, 583},
	  {144, SAP_TX_LEAKAGE_MIN},
	  } },

	{116,
	 {{36, 646}, {40, 648},
	  {44, 633}, {48, 634},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, 615}, {64, 594},
	  {100, 575}, {104, 554},
	  {108, 534}, {112, SAP_TX_LEAKAGE_MIN},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, 534}, {136, 554},
	  {140, 574},
	  {144, SAP_TX_LEAKAGE_MIN},
	  } },

	{120,
	 {{36, 643}, {40, 649},
	  {44, 654}, {48, 629},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, 621},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 565}, {104, 545},
	  {108, 525}, {112, 505},
	  {116, SAP_TX_LEAKAGE_MIN}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, 505},
	  {132, 525}, {136, 545},
	  {140, 565},
	  {144, SAP_TX_LEAKAGE_MIN},
	  } },

	{124,
	 {{36, 638}, {40, 657},
	  {44, 663}, {48, 649},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 581}, {104, 561},
	  {108, 541}, {112, 521},
	  {116, 499}, {120, SAP_TX_LEAKAGE_MIN},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, 499}, {136, 519},
	  {140, 539},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{128,
	 {{36, 651}, {40, 651},
	  {44, 674}, {48, 640},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, 603}, {104, 560},
	  {108, 540}, {112, 520},
	  {116, 499}, {120, 479},
	  {124, SAP_TX_LEAKAGE_MIN}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, 479},
	  {140, 499},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{132,
	 {{36, 643}, {40, 668},
	  {44, 651}, {48, 657},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, SAP_TX_LEAKAGE_MAX}, {104, 602},
	  {108, 578}, {112, 570},
	  {116, 550}, {120, 530},
	  {124, 510}, {128, SAP_TX_LEAKAGE_MIN},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, 490},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{136,
	 {{36, 654}, {40, 667},
	  {44, 666}, {48, 642},
	  {52, SAP_TX_LEAKAGE_MAX}, {56, SAP_TX_LEAKAGE_MAX},
	  {60, SAP_TX_LEAKAGE_MAX}, {64, SAP_TX_LEAKAGE_MAX},
	  {100, SAP_TX_LEAKAGE_MAX}, {104, SAP_TX_LEAKAGE_MAX},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, 596},
	  {116, 555}, {120, 535},
	  {124, 515}, {128, 495},
	  {132, SAP_TX_LEAKAGE_MIN}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },

	{140,
	 {{36, 679}, {40, 673},
	  {44, 667}, {48, 656},
	  {52, 634}, {56, 663},
	  {60, 662}, {64, 660},
	  {100, SAP_TX_LEAKAGE_MAX}, {104, SAP_TX_LEAKAGE_MAX},
	  {108, SAP_TX_LEAKAGE_MAX}, {112, 590},
	  {116, 573}, {120, 553},
	  {124, 533}, {128, 513},
	  {132, 490}, {136, SAP_TX_LEAKAGE_MIN},
	  {140, SAP_TX_LEAKAGE_MIN},
	  {144, SAP_TX_LEAKAGE_MIN}
	  } },
};
#endif /* WLAN_ENABLE_CHNL_MATRIX_RESTRICTION */

/*----------------------------------------------------------------------------
 * Static Function Declarations and Definitions
 * -------------------------------------------------------------------------*/
#ifdef SOFTAP_CHANNEL_RANGE
static QDF_STATUS sap_get_channel_list(ptSapContext sapContext,
				    uint8_t **channelList,
				    uint8_t *numberOfChannels);
#endif

/*==========================================================================
   FUNCTION    sap_get_5ghz_channel_list

   DESCRIPTION
    Function for initializing list of  2.4/5 Ghz [NON-DFS/DFS] available
    channels in the current regulatory domain.

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext: SAP Context

   RETURN VALUE
    NA

   SIDE EFFECTS
   ============================================================================*/
static QDF_STATUS sap_get_5ghz_channel_list(ptSapContext sapContext);

/*==========================================================================
   FUNCTION    sapStopDfsCacTimer

   DESCRIPTION
    Function to sttop the DFS CAC timer when SAP is stopped
   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext: SAP Context
   RETURN VALUE
    DFS Timer start status
   SIDE EFFECTS
   ============================================================================*/

static int sap_stop_dfs_cac_timer(ptSapContext sapContext);

/*==========================================================================
   FUNCTION    sapStartDfsCacTimer

   DESCRIPTION
    Function to start the DFS CAC timer when SAP is started on DFS Channel
   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext: SAP Context
   RETURN VALUE
    DFS Timer start status
   SIDE EFFECTS
   ============================================================================*/

int sap_start_dfs_cac_timer(ptSapContext sapContext);

/** sap_hdd_event_to_string() - convert hdd event to string
 * @event: eSapHddEvent event type
 *
 * This function converts eSapHddEvent into string
 *
 * Return: string for the @event.
 */
static uint8_t *sap_hdd_event_to_string(eSapHddEvent event)
{
	switch (event) {
	CASE_RETURN_STRING(eSAP_START_BSS_EVENT);
	CASE_RETURN_STRING(eSAP_STOP_BSS_EVENT);
	CASE_RETURN_STRING(eSAP_STA_ASSOC_IND);
	CASE_RETURN_STRING(eSAP_STA_ASSOC_EVENT);
	CASE_RETURN_STRING(eSAP_STA_REASSOC_EVENT);
	CASE_RETURN_STRING(eSAP_STA_DISASSOC_EVENT);
	CASE_RETURN_STRING(eSAP_STA_SET_KEY_EVENT);
	CASE_RETURN_STRING(eSAP_STA_MIC_FAILURE_EVENT);
	CASE_RETURN_STRING(eSAP_ASSOC_STA_CALLBACK_EVENT);
	CASE_RETURN_STRING(eSAP_GET_WPSPBC_SESSION_EVENT);
	CASE_RETURN_STRING(eSAP_WPS_PBC_PROBE_REQ_EVENT);
	CASE_RETURN_STRING(eSAP_REMAIN_CHAN_READY);
	CASE_RETURN_STRING(eSAP_SEND_ACTION_CNF);
	CASE_RETURN_STRING(eSAP_DISCONNECT_ALL_P2P_CLIENT);
	CASE_RETURN_STRING(eSAP_MAC_TRIG_STOP_BSS_EVENT);
	CASE_RETURN_STRING(eSAP_UNKNOWN_STA_JOIN);
	CASE_RETURN_STRING(eSAP_MAX_ASSOC_EXCEEDED);
	CASE_RETURN_STRING(eSAP_CHANNEL_CHANGE_EVENT);
	CASE_RETURN_STRING(eSAP_DFS_CAC_START);
	CASE_RETURN_STRING(eSAP_DFS_CAC_END);
	CASE_RETURN_STRING(eSAP_DFS_PRE_CAC_END);
	CASE_RETURN_STRING(eSAP_DFS_RADAR_DETECT);
	CASE_RETURN_STRING(eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC);
	CASE_RETURN_STRING(eSAP_DFS_NOL_GET);
	CASE_RETURN_STRING(eSAP_DFS_NOL_SET);
	CASE_RETURN_STRING(eSAP_DFS_NO_AVAILABLE_CHANNEL);
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	CASE_RETURN_STRING(eSAP_ACS_SCAN_SUCCESS_EVENT);
#endif
	CASE_RETURN_STRING(eSAP_ACS_CHANNEL_SELECTED);
	default:
		return "eSAP_HDD_EVENT_UNKNOWN";
	}
}

/*----------------------------------------------------------------------------
 * Externalized Function Definitions
 * -------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * Function Declarations and Documentation
 * -------------------------------------------------------------------------*/

/*==========================================================================
   FUNCTION    sap_event_init

   DESCRIPTION
    Function for initializing sWLAN_SAPEvent structure

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapEvent    : State machine event

   RETURN VALUE

    None

   SIDE EFFECTS
   ============================================================================*/
static inline void sap_event_init(ptWLAN_SAPEvent sapEvent)
{
	sapEvent->event = eSAP_MAC_SCAN_COMPLETE;
	sapEvent->params = 0;
	sapEvent->u1 = 0;
	sapEvent->u2 = 0;
}

#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
/*
 * sap_find_target_channel_in_channel_matrix() - finds the leakage matrix
 * @sapContext: Pointer to vos global context structure
 * @ch_width: target channel width
 * @NOL_channel: the NOL channel whose leakage matrix is required
 * @pTarget_chnl_mtrx: pointer to target channel matrix returned.
 *
 * This function gives the leakage matrix for given NOL channel and ch_width
 *
 * Return: TRUE or FALSE
 */
bool
sap_find_target_channel_in_channel_matrix(ptSapContext sapContext,
					  enum phy_ch_width ch_width,
					  uint8_t NOL_channel,
					  tSapTxLeakInfo **pTarget_chnl_mtrx)
{
	tSapTxLeakInfo *target_chan_matrix = NULL;
	tSapChanMatrixInfo *pchan_matrix = NULL;
	uint32_t nchan_matrix;
	int i = 0;

	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		/* HT20 */
		pchan_matrix = ht20_chan;
		nchan_matrix = QDF_ARRAY_SIZE(ht20_chan);
		break;
	case CH_WIDTH_40MHZ:
		/* HT40 */
		pchan_matrix = ht40_chan;
		nchan_matrix = QDF_ARRAY_SIZE(ht40_chan);
		break;
	case CH_WIDTH_80MHZ:
		/* HT80 */
		pchan_matrix = ht80_chan;
		nchan_matrix = QDF_ARRAY_SIZE(ht80_chan);
		break;
	default:
		/* handle exception and fall back to HT20 table */
		pchan_matrix = ht20_chan;
		nchan_matrix = QDF_ARRAY_SIZE(ht20_chan);
		break;
	}

	for (i = 0; i < nchan_matrix; i++) {
		/* find the SAP channel to map the leakage matrix */
		if (NOL_channel == pchan_matrix[i].channel) {
			target_chan_matrix = pchan_matrix[i].chan_matrix;
			break;
		}
	}

	if (NULL == target_chan_matrix) {
		return false;
	} else {
		*pTarget_chnl_mtrx = target_chan_matrix;
		return true;
	}
}

/**
 * sap_mark_leaking_ch() - to mark channel leaking in to nol
 * @sap_ctx: pointer to SAP context
 * @ch_width: channel width
 * @nol: nol info
 * @temp_ch_lst_sz: the target channel list
 * @temp_ch_lst: the target channel list
 *
 * This function removes the channels from temp channel list that
 * (if selected as target channel) will cause leakage in one of
 * the NOL channels
 *
 * Return: QDF_STATUS
 */

QDF_STATUS
sap_mark_leaking_ch(ptSapContext sap_ctx,
		enum phy_ch_width ch_width,
		tSapDfsNolInfo *nol,
		uint8_t temp_ch_lst_sz,
		uint8_t *temp_ch_lst)
{
	tSapTxLeakInfo *target_chan_matrix = NULL;
	uint32_t         num_channel = (CHAN_ENUM_144 - CHAN_ENUM_36) + 1;
	uint32_t         i = 0;
	uint32_t         j = 0;
	uint32_t         k = 0;
	uint8_t          dfs_nol_channel;


	/* traverse target_chan_matrix and */
	for (i = 0; i < NUM_5GHZ_CHANNELS ; i++) {
		dfs_nol_channel = nol[i].dfs_channel_number;
		if (nol[i].radar_status_flag == eSAP_DFS_CHANNEL_USABLE ||
		    nol[i].radar_status_flag == eSAP_DFS_CHANNEL_AVAILABLE) {
			/* not present in NOL */
			continue;
		}
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			FL("sapdfs: processing NOL channel: %d"),
			dfs_nol_channel);
		if (false == sap_find_target_channel_in_channel_matrix(
					sap_ctx, ch_width, dfs_nol_channel,
					&target_chan_matrix)) {
			/*
			 * should never happen, we should always find a table
			 * here, if we don't, need a fix here!
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("Couldn't find target channel matrix!"));
			QDF_ASSERT(0);
			return QDF_STATUS_E_FAILURE;
		}
		/*
		 * following is based on assumption that both temp_ch_lst
		 * and target channel matrix are in increasing order of
		 * ch_id
		 */
		for (j = 0, k = 0; j < temp_ch_lst_sz && k < num_channel;) {
			if (temp_ch_lst[j] == 0) {
				j++;
				continue;
			}
			if (target_chan_matrix[k].leak_chan != temp_ch_lst[j]) {
				k++;
				continue;
			}
			/*
			 * check leakage from candidate channel
			 * to NOL channel
			 */
			if (target_chan_matrix[k].leak_lvl <=
					SAP_TX_LEAKAGE_THRES) {
				/*
				 * candidate channel will have
				 * bad leakage in NOL channel,
				 * remove from temp list
				 */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_INFO_LOW,
					FL("sapdfs: channel: %d will have bad leakage due to channel: %d\n"),
					dfs_nol_channel, temp_ch_lst[j]);
				temp_ch_lst[j] = 0;
				break;
			}
			j++;
			k++;
		}
	} /* end of loop that selects each NOL */
	return QDF_STATUS_SUCCESS;
}
#endif /* end of WLAN_ENABLE_CHNL_MATRIX_RESTRICTION */

/*
 * This function adds availabe channel to bitmap
 *
 * PARAMETERS
 * IN
 * pBitmap: bitmap to populate
 * channel: channel to set in bitmap
 */
static void sap_set_bitmap(chan_bonding_bitmap *pBitmap, uint8_t channel)
{
	int i = 0;
	int start_channel = 0;
	for (i = 0; i < MAX_80MHZ_BANDS; i++) {
		start_channel = pBitmap->chanBondingSet[i].startChannel;
		if (channel >= start_channel && channel <= start_channel + 12) {
			pBitmap->chanBondingSet[i].channelMap |=
				1 << ((channel - start_channel) / 4);
			return;
		}
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		  FL("Channel=%d is not in the bitmap"), channel);
}

/**
 * sap_populate_available_channels() - To populate available channel
 * @bitmap: bitmap to populate
 * @ch_width: channel width
 * @avail_chnl: available channel list to populate
 *
 * This function reads the bitmap and populates available channel
 * list according to channel bonding mode. This will be called for
 * 80 MHz and 40 Mhz only. For 20 MHz no need for bitmap hence list
 * is directly created while parsing the main list
 *
 * Return: number of channels found
 */
static uint8_t sap_populate_available_channels(chan_bonding_bitmap *bitmap,
		enum phy_ch_width ch_width,
		uint8_t *avail_chnl)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint8_t start_channel = 0;

	switch (ch_width) {
	/* VHT80 */
	case CH_WIDTH_160MHZ:
	case CH_WIDTH_80P80MHZ:
	case CH_WIDTH_80MHZ:
		for (i = 0; i < MAX_80MHZ_BANDS; i++) {
			start_channel = bitmap->chanBondingSet[i].startChannel;
			if (bitmap->chanBondingSet[i].channelMap ==
				SAP_80MHZ_MASK) {
				avail_chnl[chnl_count++] = start_channel;
				avail_chnl[chnl_count++] = start_channel + 4;
				avail_chnl[chnl_count++] = start_channel + 8;
				avail_chnl[chnl_count++] = start_channel + 12;
			}
		}
		break;
	/* HT40 */
	case CH_WIDTH_40MHZ:
		for (i = 0; i < MAX_80MHZ_BANDS; i++) {
			start_channel = bitmap->chanBondingSet[i].startChannel;
			if ((bitmap->chanBondingSet[i].channelMap &
				SAP_40MHZ_MASK_L) == SAP_40MHZ_MASK_L) {
				avail_chnl[chnl_count++] = start_channel;
				avail_chnl[chnl_count++] = start_channel + 4;
			}
			if ((bitmap->chanBondingSet[i].channelMap &
				SAP_40MHZ_MASK_H) == SAP_40MHZ_MASK_H) {
				avail_chnl[chnl_count++] = start_channel + 8;
				avail_chnl[chnl_count++] = start_channel + 12;
			}
		}
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid case."));
		break;
	}

	return chnl_count;
}

/*
 * FUNCTION  sap_dfs_is_w53_invalid
 *
 * DESCRIPTION Checks if the passed channel is W53 and returns if
 *             SAP W53 opearation is allowed.
 *
 * DEPENDENCIES PARAMETERS
 * IN hHAL : HAL pointer
 * channelID: Channel Number to be verified
 *
 * RETURN VALUE  : bool
 *                 true: If W53 operation is disabled
 *                 false: If W53 operation is enabled
 *
 * SIDE EFFECTS
 */
bool sap_dfs_is_w53_invalid(tHalHandle hHal, uint8_t channelID)
{
	tpAniSirGlobal pMac;

	pMac = PMAC_STRUCT(hHal);
	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid pMac"));
		return false;
	}

	/*
	 * Check for JAPAN W53 Channel operation capability
	 */
	if (true == pMac->sap.SapDfsInfo.is_dfs_w53_disabled &&
	    true == IS_CHAN_JAPAN_W53(channelID)) {
		return true;
	}

	return false;
}

/*
 * FUNCTION  sap_dfs_is_channel_in_preferred_location
 *
 * DESCRIPTION Checks if the passed channel is in accordance with preferred
 *          Channel location settings.
 *
 * DEPENDENCIES PARAMETERS
 * IN hHAL : HAL pointer
 * channelID: Channel Number to be verified
 *
 * RETURN VALUE  :bool
 *        true:If Channel location is same as the preferred location
 *        false:If Channel location is not same as the preferred location
 *
 * SIDE EFFECTS
 */
bool sap_dfs_is_channel_in_preferred_location(tHalHandle hHal, uint8_t channelID)
{
	tpAniSirGlobal pMac;

	pMac = PMAC_STRUCT(hHal);
	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid pMac"));
		return true;
	}
	if ((SAP_CHAN_PREFERRED_INDOOR ==
	     pMac->sap.SapDfsInfo.sap_operating_chan_preferred_location) &&
	    (true == IS_CHAN_JAPAN_OUTDOOR(channelID))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL
				  ("CHAN=%d is Outdoor so invalid,preferred Indoor only"),
			  channelID);
		return false;
	} else if ((SAP_CHAN_PREFERRED_OUTDOOR ==
		    pMac->sap.SapDfsInfo.sap_operating_chan_preferred_location)
		   && (true == IS_CHAN_JAPAN_INDOOR(channelID))) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL
				  ("CHAN=%d is Indoor so invalid,preferred Outdoor only"),
			  channelID);
		return false;
	}

	return true;
}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
/**
 * sap_check_in_avoid_ch_list() - checks if given channel present is channel
 * avoidance list
 *
 * @sap_ctx:        sap context.
 * @channel:        channel to be checked in sap_ctx's avoid ch list
 *
 * sap_ctx contains sap_avoid_ch_info strcut containing the list of channels on
 * which MDM device's AP with MCC was detected. This function checks if given
 * channel is present in that list.
 *
 * Return: true, if channel was present, false othersie.
 */
bool sap_check_in_avoid_ch_list(ptSapContext sap_ctx, uint8_t channel)
{
	uint8_t i = 0;
	struct sap_avoid_channels_info *ie_info =
		&sap_ctx->sap_detected_avoid_ch_ie;
	for (i = 0; i < sizeof(ie_info->channels); i++)
		if (ie_info->channels[i] == channel)
			return true;
	return false;
}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

/**
 * sap_apply_rules() - validates channels in sap_ctx channel list
 * @sap_ctx: sap context pointer
 *
 * This function takes the channel list in sap_ctx and marks invalid channel
 * based on following rules:
 * -> invalid due to different reg domain dfs channel list
 * -> already present in NOL
 * -> outside ACS range or not
 * -> not usable due to another SAP operating MCC mode in same channel
 * -> preferred location is indoors or outdoors
 *
 * Return: number of valid channels after applying all the rules
 */
static uint8_t sap_apply_rules(ptSapContext sap_ctx)
{
	uint8_t num_valid_ch, i = 0, ch_id;
	tAll5GChannelList *sap_all_ch = &sap_ctx->SapAllChnlList;
	bool is_ch_nol = false;
	bool is_out_of_range = false;
	tpAniSirGlobal mac_ctx;
	tHalHandle hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
	uint8_t preferred_location;
	enum dfs_region dfs_region;

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("hal pointer NULL"));
		return 0;
	}

	mac_ctx = PMAC_STRUCT(hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("mac_ctx pointer NULL"));
		return 0;
	}

	preferred_location =
		mac_ctx->sap.SapDfsInfo.sap_operating_chan_preferred_location;
	cds_get_dfs_region(&dfs_region);
	/* loop to check ACS range or NOL channels */
	num_valid_ch = sap_all_ch->numChannel;
	for (i = 0; i < sap_all_ch->numChannel; i++) {
		ch_id = sap_all_ch->channelList[i].channel;

		/*
		 * IN MKK DFS REGION CHECK IF THE FOLLOWING TWO
		 * TWO RULES APPLY AND FILTER THE AVAILABLE CHANNELS
		 * ACCORDINGLY.
		 *
		 * 1. If we are operating in Japan regulatory domain
		 * Check if Japan W53 Channel operation is NOT
		 * allowed and if its not allowed then mark all the
		 * W53 channels as Invalid.
		 *
		 * 2. If we are operating in Japan regulatory domain
		 * Check if channel switch between Indoor/Outdoor
		 * is allowed. If it is not allowed then limit
		 * the avaiable channels to Indoor or Outdoor
		 * channels only based up on the SAP Channel location
		 * indicated by "sap_operating_channel_location" param.
		 */
		if (DFS_MKK_REGION == dfs_region) {
			/*
			 * Check for JAPAN W53 Channel operation capability
			 */
			if (true == sap_dfs_is_w53_invalid(hal, ch_id)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  FL("index:%d, Channel=%d Invalid,Japan W53 Disabled"),
					  i, ch_id);
				sap_all_ch->channelList[i].valid = false;
				num_valid_ch--;
				continue;
			}

			/*
			 * If SAP's preferred channel location is Indoor
			 * then set all the outdoor channels in the domain
			 * to invalid.If the preferred channel location is
			 * outdoor then set all the Indoor channels in the
			 * domain to Invalid.
			 */
			if (false ==
			    sap_dfs_is_channel_in_preferred_location(hal,
								ch_id)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  FL("CHAN=%d is invalid,preferred Channel Location %d Only"),
					  ch_id, preferred_location);
				sap_all_ch->channelList[i].valid = false;
				num_valid_ch--;
				continue;
			}
		}

		if (cds_get_channel_state(ch_id) == CHANNEL_STATE_DFS) {
			is_ch_nol = sap_dfs_is_channel_in_nol_list(sap_ctx,
					ch_id, PHY_SINGLE_CHANNEL_CENTERED);
			if (true == is_ch_nol) {
				/*
				 * Mark this channel invalid since it is still
				 * in DFS Non-Occupancy-Period which is 30 mins.
				 */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  FL("index: %d, Channel = %d Present in NOL LIST"),
					  i, ch_id);
				sap_all_ch->channelList[i].valid = false;
				num_valid_ch--;
				continue;
			}
		}

#ifdef FEATURE_AP_MCC_CH_AVOIDANCE
		/* avoid ch on which another MDM AP in MCC mode is detected */
		if (mac_ctx->sap.sap_channel_avoidance
		    && sap_ctx->sap_detected_avoid_ch_ie.present) {
			if (sap_check_in_avoid_ch_list(sap_ctx, ch_id)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  FL("index: %d, Channel = %d, avoided due to presence of another AP+AP MCC device in same channel."),
					  i, ch_id);
				sap_all_ch->channelList[i].valid = false;
			}
		}
#endif /* FEATURE_AP_MCC_CH_AVOIDANCE */

		/* check if the channel is within ACS channel range */
		is_out_of_range = sap_acs_channel_check(sap_ctx, ch_id);
		if (true == is_out_of_range) {
			/*
			 * mark this channel invalid since it is out of ACS
			 * channel range
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
				  FL("index: %d, Channel = %d out of ACS channel range"),
				  i, ch_id);
			sap_all_ch->channelList[i].valid = false;
			num_valid_ch--;
			continue;
		}
	} /* end of check for NOL or ACS channels */
	return num_valid_ch;
}

/**
 * select_rand_from_lst() - selects random channel from given list
 * @mac_ctx: mac context pointer
 * @ch_lst: channel list
 * @num_ch: number of channels
 *
 * Return: new target channel randomly selected
 */
static uint8_t select_rand_from_lst(tpAniSirGlobal mac_ctx, uint8_t *ch_lst,
				    uint8_t num_ch)
{
	uint32_t rand_byte = 0;
	uint8_t i, target_channel, non_dfs_num_ch = 0, dfs_num_ch = 0;
	uint8_t dfs_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
	uint8_t non_dfs_ch[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
	if (num_ch) {
		for (i = 0; i < num_ch; i++) {
			if (CDS_IS_DFS_CH(ch_lst[i]))
				dfs_ch[dfs_num_ch++] = ch_lst[i];
			else
				non_dfs_ch[non_dfs_num_ch++] = ch_lst[i];
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL("No target channel found"));
		return 0;
	}

	cds_rand_get_bytes(0, (uint8_t *)&rand_byte, 1);

	/* Give preference to non-DFS channel */
	if (!mac_ctx->f_prefer_non_dfs_on_radar) {
		i = (rand_byte + qdf_mc_timer_get_system_ticks()) % num_ch;
		target_channel = ch_lst[i];
	} else if (non_dfs_num_ch) {
		i = (rand_byte + qdf_mc_timer_get_system_ticks()) %
							non_dfs_num_ch;
		target_channel = non_dfs_ch[i];
	} else {
		i = (rand_byte + qdf_mc_timer_get_system_ticks()) % dfs_num_ch;
		target_channel = dfs_ch[i];
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  FL("sapdfs: New Channel width = %d"),
		  mac_ctx->sap.SapDfsInfo.new_chanWidth);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  FL("sapdfs: target_channel = %d"), target_channel);

	return target_channel;
}

/**
 * sap_find_ch_wh_fallback() - find given channel width from given list of
 * channels
 * @mac_ctx: mac context pointer
 * @ch_wd: channel width to find
 * @ch_lst: list of channels
 * @num_ch: number of channels
 *
 * This function tries to find given channel width and returns new target ch.
 * If not found updates ch_wd to next lower channel width.
 *
 * Return: new target channel if successful, 0 otherwise
 */
static uint8_t sap_find_ch_wh_fallback(tpAniSirGlobal mac_ctx,
				       enum phy_ch_width *ch_wd,
				       uint8_t *ch_lst,
				       uint8_t num_ch)
{
	bool flag = false;
	uint32_t rand_byte = 0;
	chan_bonding_bitmap ch_map = { { {0} } };
	uint8_t count = 0, i, index = 0, final_cnt = 0, target_channel = 0;
	uint8_t primary_seg_start_ch = 0, sec_seg_ch = 0, new_160_start_ch = 0;
	uint8_t final_lst[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};

	/* initialize ch_map for all 80 MHz bands: we have 6 80MHz bands */
	ch_map.chanBondingSet[0].startChannel = 36;
	ch_map.chanBondingSet[1].startChannel = 52;
	ch_map.chanBondingSet[2].startChannel = 100;
	ch_map.chanBondingSet[3].startChannel = 116;
	ch_map.chanBondingSet[4].startChannel = 132;
	ch_map.chanBondingSet[5].startChannel = 149;

	/* now loop through leakage free list */
	for (i = 0; i < num_ch; i++) {
		/* add tmp ch to bitmap */
		if (ch_lst[i] == 0)
			continue;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			  FL("sapdfs: Channel=%d added to bitmap"),
			  ch_lst[i]);
		sap_set_bitmap(&ch_map, ch_lst[i]);
	}

	/* populate available channel list from bitmap */
	final_cnt = sap_populate_available_channels(&ch_map, *ch_wd, final_lst);
	/* If no valid ch bonding found, fallback */
	if (final_cnt == 0) {
		if ((*ch_wd == CH_WIDTH_160MHZ) ||
			(*ch_wd == CH_WIDTH_80P80MHZ) ||
			(*ch_wd == CH_WIDTH_80MHZ)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("sapdfs:Changing chanWidth from [%d] to 40Mhz"),
				  *ch_wd);
			*ch_wd = CH_WIDTH_40MHZ;
		} else if (*ch_wd == CH_WIDTH_40MHZ) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("sapdfs:No 40MHz cb found, falling to 20MHz"));
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("sapdfs:Changing chanWidth from [%d] to [%d]"),
				  *ch_wd, CH_WIDTH_20MHZ);
			*ch_wd = CH_WIDTH_20MHZ;
		}
		return 0;
	}

	/* ch count should be > 8 to switch new channel in 160Mhz band */
	if (((*ch_wd == CH_WIDTH_160MHZ) || (*ch_wd == CH_WIDTH_80P80MHZ)) &&
			(final_cnt < SIR_DFS_MAX_20M_SUB_CH)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
			FL("sapdfs:Changing chanWidth from [%d] to [%d]"),
			*ch_wd, CH_WIDTH_80MHZ);
		*ch_wd = CH_WIDTH_80MHZ;
		return 0;
	}
	if (*ch_wd == CH_WIDTH_160MHZ) {
		/*
		 * NA supports only 2 blocks for 160Mhz bandwidth i.e 36-64 &
		 * 100-128 and all the channels in these blocks are continuous
		 * and seperated by 4Mhz.
		 */
		for (i = 1; ((i < final_cnt)); i++) {
			if ((final_lst[i] - final_lst[i-1]) == 4)
				count++;
			else
				count = 0;
			if (count == SIR_DFS_MAX_20M_SUB_CH - 1) {
				flag = true;
				new_160_start_ch = final_lst[i-7];
				break;
			}
		}
	} else if (*ch_wd == CH_WIDTH_80P80MHZ) {
		flag = true;
	}
	if ((flag == false) && (*ch_wd > CH_WIDTH_80MHZ)) {
		*ch_wd = CH_WIDTH_80MHZ;
		return 0;
	}

	if (*ch_wd == CH_WIDTH_160MHZ) {
		cds_rand_get_bytes(0, (uint8_t *)&rand_byte, 1);
		rand_byte = (rand_byte + qdf_mc_timer_get_system_ticks())
				% SIR_DFS_MAX_20M_SUB_CH;
		target_channel = new_160_start_ch + (rand_byte * 4);
	} else if (*ch_wd == CH_WIDTH_80P80MHZ) {
		cds_rand_get_bytes(0, (uint8_t *)&rand_byte, 1);
		index = (rand_byte + qdf_mc_timer_get_system_ticks()) %
			final_cnt;
		target_channel = final_lst[index];
		index -= (index % 4);
		primary_seg_start_ch = final_lst[index];

		/* reset channels associate with primary 80Mhz */
		for (i = 0; i < 4; i++)
			final_lst[i + index] = 0;
		/* select and calculate center freq for secondary segement */
		for (i = 0; i < final_cnt / 4; i++) {
			if (final_lst[i * 4] &&
			    (abs(primary_seg_start_ch - final_lst[i * 4]) >
				(SIR_DFS_MAX_20M_SUB_CH * 2))) {
				sec_seg_ch = final_lst[i * 4] +
				SIR_80MHZ_START_CENTER_CH_DIFF;
				break;
			}
		}
		if (!sec_seg_ch && (final_cnt == SIR_DFS_MAX_20M_SUB_CH))
			*ch_wd = CH_WIDTH_160MHZ;
		else if (!sec_seg_ch)
			*ch_wd = CH_WIDTH_80MHZ;

		mac_ctx->sap.SapDfsInfo.new_ch_params.center_freq_seg1
								= sec_seg_ch;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			FL("sapdfs: New Center Freq Seg1 = %d"), sec_seg_ch);
	} else {
		target_channel = select_rand_from_lst(mac_ctx, final_lst,
						      final_cnt);
	}
	return target_channel;
}

/**
 * sap_random_channel_sel() - This function randomly pick up an available
 * channel
 * @sap_ctx:        sap context.
 *
 * This function first eliminates invalid channel, then selects random channel
 * using following algorithm:
 * PASS: 1 - invalidate ch in sap_ctx->all_ch_lst as per rules
 * PASS: 2 - now mark channels that will leak into NOL
 * PASS: 3 - from leakage_adjusted_lst, either select random channel (for 20MHz)
 *           or find given channel width possible. if not found fallback to
 *           lower channel width and try again. once given channel width found
 *           use random channel from give possibilities.
 * Return: channel number picked
 */
static uint8_t sap_random_channel_sel(ptSapContext sap_ctx)
{
	uint8_t j = 0, i = 0, final_cnt = 0, target_channel = 0;
	/* count and ch list after applying all rules */
	uint8_t rule_adjusted_cnt, *rule_adjusted_lst;
	/* ch list after invalidating channels leaking into NOL */
	uint8_t *leakage_adjusted_lst;
	/* final list of channel from which random channel will be selected */
	uint8_t final_lst[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
	tAll5GChannelList *all_ch = &sap_ctx->SapAllChnlList;
	tHalHandle hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
	tpAniSirGlobal mac_ctx;
	/* channel width for current iteration */
	enum phy_ch_width ch_wd;
#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
	tSapDfsNolInfo *nol;
#endif

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid hal"));
		return 0;
	}

	mac_ctx = PMAC_STRUCT(hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid mac_ctx"));
		return 0;
	}

	/*
	 * Retrieve the original one and store it.
	 * use the stored original value when you call this function next time
	 * so fall back mechanism always starts with original ini value.
	 */
	if (mac_ctx->sap.SapDfsInfo.orig_chanWidth == 0) {
		ch_wd = sap_ctx->ch_width_orig;
		mac_ctx->sap.SapDfsInfo.orig_chanWidth = ch_wd;
	} else {
		ch_wd = mac_ctx->sap.SapDfsInfo.orig_chanWidth;
	}

	if (sap_get_5ghz_channel_list(sap_ctx)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL("Getting 5Ghz channel list failed"));
		return 0;
	}

	/* PASS: 1 - invalidate ch in sap_ctx->all_ch_lst as per rules */
	rule_adjusted_cnt = sap_apply_rules(sap_ctx);
	if (0 == rule_adjusted_cnt) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("No channels left after applying rules."));
		return 0;
	}

	/* this is list we get after applying all rules */
	rule_adjusted_lst = qdf_mem_malloc(rule_adjusted_cnt);

	/* list adjusted after leakage has been marked */
	leakage_adjusted_lst = qdf_mem_malloc(rule_adjusted_cnt);
	if (rule_adjusted_lst == NULL || leakage_adjusted_lst == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("sapdfs: memory alloc failed"));
		qdf_mem_free(rule_adjusted_lst);
		qdf_mem_free(leakage_adjusted_lst);
		return 0;
	}

	/* copy valid ch from sap_ctx->all_ch_lst into rule_adjusted_lst */
	for (i = 0, j = 0; i < all_ch->numChannel; i++) {
		if (!all_ch->channelList[i].valid)
			continue;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL("sapdfs: Adding Channel = %d to temp List"),
			  all_ch->channelList[i].channel);
		rule_adjusted_lst[j++] = all_ch->channelList[i].channel;
	}

	/*
	 * do - while loop for fallback mechanism. at each channel width this
	 * will try to find channel bonding, if not fall back to lower ch width
	 */
	do {
		/* save rules adjusted lst */
		qdf_mem_copy(leakage_adjusted_lst, rule_adjusted_lst,
			     rule_adjusted_cnt);
#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
		nol = mac_ctx->sap.SapDfsInfo.sapDfsChannelNolList;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("sapdfs: Processing tmp ch list against NOL."));
		/* PASS: 2 - now mark channels that will leak into NOL */
		if (sap_mark_leaking_ch(sap_ctx, ch_wd, nol, rule_adjusted_cnt,
				leakage_adjusted_lst) != QDF_STATUS_SUCCESS) {
			qdf_mem_free(rule_adjusted_lst);
			qdf_mem_free(leakage_adjusted_lst);
			return 0;
		}
#endif
		if (ch_wd == CH_WIDTH_20MHZ) {
			/*
			 * PASS: 3 - from leakage_adjusted_lst, prepare valid
			 * ch list and use random number from that
			 */
			for (i = 0; i < rule_adjusted_cnt; i++) {
				if (leakage_adjusted_lst[i] == 0)
					continue;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_DEBUG,
					  FL("sapdfs: Channel=%d added to available list"),
					  leakage_adjusted_lst[i]);
				final_lst[final_cnt] = leakage_adjusted_lst[i];
				final_cnt++;
			}
			target_channel = select_rand_from_lst(mac_ctx,
							final_lst, final_cnt);
			break;
		}

		/*
		 * PASS: 3 - following function will check from valid channels
		 * left if given ch_wd can be supported. if not reduce ch_wd
		 * (fallback), then continue with reduced ch_wd
		 */
		target_channel = sap_find_ch_wh_fallback(mac_ctx, &ch_wd,
							   leakage_adjusted_lst,
							   rule_adjusted_cnt);
		/*
		 * if target channel is 0, no channel bonding was found
		 * ch_wd would have been updated for fallback
		 */
		if (0 == target_channel)
			continue;
		else
			break;
	} while (true);

	if (target_channel)
		mac_ctx->sap.SapDfsInfo.new_chanWidth = ch_wd;

	qdf_mem_free(rule_adjusted_lst);
	qdf_mem_free(leakage_adjusted_lst);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		FL("sapdfs: New Channel width = %d"),
		mac_ctx->sap.SapDfsInfo.new_chanWidth);
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		FL("sapdfs: target_channel = %d"),
		target_channel);
	return target_channel;
}

bool sap_acs_channel_check(ptSapContext sapContext, uint8_t channelNumber)
{
	int i = 0;
	if (!sapContext->acs_cfg->acs_mode)
		return false;

	if ((channelNumber >= sapContext->acs_cfg->start_ch) &&
		(channelNumber <= sapContext->acs_cfg->end_ch)) {
		if (!sapContext->acs_cfg->ch_list) {
			return false;
		} else {
			for (i = 0; i < sapContext->acs_cfg->ch_list_count; i++)
				if (channelNumber ==
						sapContext->acs_cfg->ch_list[i])
					return false;
		}
	}
	return true;
}

/**
 * sap_mark_dfs_channels() - to mark dfs channel
 * @sapContext: pointer sap context
 * @channels: list of channels
 * @numChannels: number of channels
 * @time: time
 *
 * Mark the channels in NOL with time and eSAP_DFS_CHANNEL_UNAVAILABLE
 *
 * Return: none
 */
void sap_mark_dfs_channels(ptSapContext sapContext, uint8_t *channels,
			   uint8_t numChannels, uint64_t time)
{
	int i, j;
	tSapDfsNolInfo *psapDfsChannelNolList = NULL;
	uint8_t nRegDomainDfsChannels;
	tHalHandle hHal;
	tpAniSirGlobal pMac;

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == channels)
		return;
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid hHal"));
		return;
	}
	pMac = PMAC_STRUCT(hHal);
	if (NULL == pMac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid pMac"));
		return;
	}

	/*
	 * Mark the current channel on which Radar is found
	 * in the NOL list as eSAP_DFS_CHANNEL_UNAVAILABLE.
	 */
	psapDfsChannelNolList = pMac->sap.SapDfsInfo.sapDfsChannelNolList;
	nRegDomainDfsChannels =
		pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels;

	for (i = 0; i < numChannels; i++) {
		for (j = 0; j <= nRegDomainDfsChannels; j++) {
			if (!(psapDfsChannelNolList[j].dfs_channel_number ==
					channels[i]))
				continue;
			/*
			 * If channel is already in NOL, don't update it again.
			 * This is useful when marking bonding channels which
			 * are already unavailable.
			 */
			if (psapDfsChannelNolList[j].radar_status_flag ==
					eSAP_DFS_CHANNEL_UNAVAILABLE) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
						QDF_TRACE_LEVEL_INFO_HIGH,
						FL("Channel=%d already in NOL"),
						channels[i]);
				continue;
			}
			/*
			 * Capture the Radar Found timestamp on the
			 * Current Channel in ms.
			 */
			psapDfsChannelNolList[j].radar_found_timestamp = time;
			/* Mark the Channel to be unavailble for next 30 mins */
			psapDfsChannelNolList[j].radar_status_flag =
				eSAP_DFS_CHANNEL_UNAVAILABLE;

			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO_HIGH,
				FL("Channel=%d Added to NOL LIST"),
				channels[i]);
		}
	}
}

/*
 * This Function is to get bonding channels from primary channel.
 *
 */
uint8_t sap_get_bonding_channels(ptSapContext sapContext, uint8_t channel,
				 uint8_t *channels, uint8_t size,
				 ePhyChanBondState chanBondState)
{
	tHalHandle hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	tpAniSirGlobal pMac;
	uint8_t numChannel;

	if (channels == NULL)
		return 0;

	if (size < MAX_BONDED_CHANNELS)
		return 0;

	if (NULL != hHal) {
		pMac = PMAC_STRUCT(hHal);
	} else
		return 0;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
		  FL("cbmode: %d, channel: %d"), chanBondState, channel);

	switch (chanBondState) {
	case PHY_SINGLE_CHANNEL_CENTERED:
		numChannel = 1;
		channels[0] = channel;
		break;
	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		numChannel = 2;
		channels[0] = channel - 4;
		channels[1] = channel;
		break;
	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		numChannel = 2;
		channels[0] = channel;
		channels[1] = channel + 4;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_LOW:
		numChannel = 4;
		channels[0] = channel;
		channels[1] = channel + 4;
		channels[2] = channel + 8;
		channels[3] = channel + 12;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_LOW:
		numChannel = 4;
		channels[0] = channel - 4;
		channels[1] = channel;
		channels[2] = channel + 4;
		channels[3] = channel + 8;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_LOW_40MHZ_HIGH:
		numChannel = 4;
		channels[0] = channel - 8;
		channels[1] = channel - 4;
		channels[2] = channel;
		channels[3] = channel + 4;
		break;
	case PHY_QUADRUPLE_CHANNEL_20MHZ_HIGH_40MHZ_HIGH:
		numChannel = 4;
		channels[0] = channel - 12;
		channels[1] = channel - 8;
		channels[2] = channel - 4;
		channels[3] = channel;
		break;
	default:
		numChannel = 1;
		channels[0] = channel;
		break;
	}

	return numChannel;
}

/**
 * sap_dfs_check_if_channel_avaialable() - Check if a channel is out of NOL
 * @nol: Pointer to the Non-Occupancy List.
 *
 * This function Checks if a given channel is available or
 * usable or unavailable based on the time lapse since the
 * last radar time stamp.
 *
 * Return: true if channel available or usable, false if unavailable.
 */
static bool sap_dfs_check_if_channel_avaialable(tSapDfsNolInfo *nol)
{
	uint64_t time_since_last_radar, time_when_radar_found, current_time = 0;
	uint64_t max_jiffies;

	if ((nol->radar_status_flag == eSAP_DFS_CHANNEL_USABLE) ||
	    (nol->radar_status_flag == eSAP_DFS_CHANNEL_AVAILABLE)) {
		/*
		 * Allow SAP operation on this channel
		 * either the DFS channel has not been used
		 * for SAP operation or it is available for
		 * SAP operation since it is past
		 * Non-Occupancy-Period so, return false.
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL("Chan=%d not in NOL,CHAN AVAILABLE"),
			  nol->dfs_channel_number);
		return true;
	} else if (nol->radar_status_flag == eSAP_DFS_CHANNEL_UNAVAILABLE) {
		/*
		 * If a DFS Channel is UNAVAILABLE then
		 * check to see if it is past
		 * Non-occupancy-period
		 * of 30 minutes. If it is past 30 mins then
		 * mark the channel as AVAILABLE and return
		 * false as the channel is not anymore in
		 * NON-Occupancy-Period.
		 */
		time_when_radar_found = nol->radar_found_timestamp;
		current_time = cds_get_monotonic_boottime();
		if (current_time < time_when_radar_found) {
			/* cds_get_monotonic_boottime() can overflow.
			 * Jiffies is initialized such that 32 bit jiffies
			 * value wrap 5 minutes after boot so jiffies wrap bugs
			 * show up earlier
			 */
			max_jiffies = (uint64_t)UINT_MAX * 1000;
			time_since_last_radar = (max_jiffies -
				time_when_radar_found) + (current_time);
		} else {
			time_since_last_radar = current_time -
							time_when_radar_found;
		}
		if (time_since_last_radar >= SAP_DFS_NON_OCCUPANCY_PERIOD) {
			nol->radar_status_flag = eSAP_DFS_CHANNEL_AVAILABLE;
			nol->radar_found_timestamp = 0;

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
				  FL("Chan=%d not in NOL, Channel AVAILABLE"),
				  nol->dfs_channel_number);
			return true;
		} else {
			/*
			 * Channel is not still available for
			 * SAP operation so return true; As the
			 * Channel is still in
			 * Non-occupancy-Period.
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
				  FL("Chan=%d in NOL, Channel UNAVAILBLE"),
				  nol->dfs_channel_number);
			return false;
		}
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid Radar Status Flag"));
	}
	return true;
}

/**
 * sap_dfs_is_channel_in_nol_list() - given bonded channel is available
 * @sap_context: Handle to SAP context.
 * @channel_number: Channel on which availability should be checked.
 * @chan_bondState: The channel bonding mode of the passed channel.
 *
 * This function Checks if a given bonded channel is available or
 * usable for DFS operation.
 *
 * Return: false if channel is available, true if channel is in NOL.
 */
bool
sap_dfs_is_channel_in_nol_list(ptSapContext sap_context,
			       uint8_t channel_number,
			       ePhyChanBondState chan_bondState)
{
	int i, j;
	tHalHandle h_hal = CDS_GET_HAL_CB(sap_context->p_cds_gctx);
	tpAniSirGlobal mac_ctx;
	uint8_t channels[MAX_BONDED_CHANNELS];
	uint8_t num_channels;
	tSapDfsNolInfo *nol;
	tSapDfsInfo *dfs_info;
	bool channel_available;

	if (NULL == h_hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("invalid h_hal"));
		return false;
	} else {
		mac_ctx = PMAC_STRUCT(h_hal);
	}

	dfs_info = &mac_ctx->sap.SapDfsInfo;
	if ((dfs_info->numCurrentRegDomainDfsChannels == 0) ||
	    (dfs_info->numCurrentRegDomainDfsChannels >
	     NUM_5GHZ_CHANNELS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
			  FL("invalid dfs channel count %d"),
			  dfs_info->numCurrentRegDomainDfsChannels);
		return false;
	}

	/* get the bonded channels */
	num_channels = sap_get_bonding_channels(sap_context, channel_number,
				channels, MAX_BONDED_CHANNELS, chan_bondState);

	/* check for NOL, first on will break the loop */
	for (j = 0; j < num_channels; j++) {
		for (i = 0; i < dfs_info->numCurrentRegDomainDfsChannels; i++) {
			nol = &dfs_info->sapDfsChannelNolList[i];
			if (nol->dfs_channel_number != channels[j])
				continue;

			channel_available =
				sap_dfs_check_if_channel_avaialable(nol);

			if (channel_available == false)
				break;

		} /* loop for dfs channels */

		if (i < dfs_info->numCurrentRegDomainDfsChannels)
			break;

	} /* loop for bonded channels */

	/*
	 * if any of the channel is not available, mark all available channels
	 * as unavailable with same time stamp.
	 */
	if (j < num_channels &&
	    i < dfs_info->numCurrentRegDomainDfsChannels) {
		if (num_channels > MAX_BONDED_CHANNELS) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("num_channel>MAX_BONDED_CHANNEL, reset"));
			num_channels = MAX_BONDED_CHANNELS;
		}
		nol = &dfs_info->sapDfsChannelNolList[i];
		sap_mark_dfs_channels(sap_context, channels, num_channels,
				nol->radar_found_timestamp);

		/* set DFS-NOL back to keep it update-to-date in CNSS */
		sap_signal_hdd_event(sap_context, NULL, eSAP_DFS_NOL_SET,
				  (void *) eSAP_STATUS_SUCCESS);

		return true;
	}

	return false;
}

/**
 * sap_goto_channel_sel - Function for initiating scan request for SME
 * @sap_context: Sap Context value.
 * @sap_event: State machine event
 * @sap_do_acs_pre_start_bss: true, if ACS scan is issued pre start BSS
 *                            false, if ACS scan is issued post start BSS.
 * @check_for_connection_update: true, check and wait for connection update
 *                               false, do not perform connection update
 *
 * Initiates sme scan for ACS to pick a channel.
 *
 * Return: The QDF_STATUS code associated with performing the operation.
 */
QDF_STATUS sap_goto_channel_sel(ptSapContext sap_context,
	ptWLAN_SAPEvent sap_event,
	bool sap_do_acs_pre_start_bss,
	bool check_for_connection_update)
{

	/* Initiate a SCAN request */
	QDF_STATUS qdf_ret_status;
	/* To be initialised if scan is required */
	tCsrScanRequest scan_request;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx;

#ifdef SOFTAP_CHANNEL_RANGE
	uint8_t *channel_list = NULL;
	uint8_t num_of_channels = 0;
#endif
	tHalHandle h_hal;
	uint8_t con_ch;

	h_hal = cds_get_context(QDF_MODULE_ID_SME);
	if (NULL == h_hal) {
		/* we have a serious problem */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  FL("invalid h_hal"));
		return QDF_STATUS_E_FAULT;
	}

	mac_ctx = PMAC_STRUCT(h_hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	if (cds_concurrent_beaconing_sessions_running()) {
		con_ch =
			sme_get_concurrent_operation_channel(h_hal);
#ifdef FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE
		if (con_ch && sap_context->channel == AUTO_CHANNEL_SELECT) {
			sap_context->dfs_ch_disable = true;
		} else if (con_ch && sap_context->channel != con_ch &&
			   CDS_IS_DFS_CH(sap_context->channel)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("MCC DFS not supported in AP_AP Mode"));
			return QDF_STATUS_E_ABORTED;
		}
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		if (sap_context->cc_switch_mode !=
						QDF_MCC_TO_SCC_SWITCH_DISABLE) {
			con_ch = sme_check_concurrent_channel_overlap(h_hal,
					sap_context->channel,
					sap_context->csr_roamProfile.phyMode,
					sap_context->cc_switch_mode);
			if (con_ch) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_ERROR,
					"%s: Override ch %d to %d due to CC Intf",
					__func__, sap_context->channel, con_ch);
				sap_context->channel = con_ch;
				cds_set_channel_params(sap_context->channel, 0,
						&sap_context->ch_params);
			}
		}
#endif
	}

	if (cds_get_concurrency_mode() == (QDF_STA_MASK | QDF_SAP_MASK)) {
#ifdef FEATURE_WLAN_STA_AP_MODE_DFS_DISABLE
		if (sap_context->channel == AUTO_CHANNEL_SELECT)
			sap_context->dfs_ch_disable = true;
		else if (CDS_IS_DFS_CH(sap_context->channel)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
				  FL("DFS not supported in STA_AP Mode"));
			return QDF_STATUS_E_ABORTED;
		}
#endif
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
		if (sap_context->cc_switch_mode !=
						QDF_MCC_TO_SCC_SWITCH_DISABLE) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				FL("check for overlap: chan:%d mode:%d"),
				sap_context->channel,
				sap_context->csr_roamProfile.phyMode);
			con_ch = sme_check_concurrent_channel_overlap(h_hal,
					sap_context->channel,
					sap_context->csr_roamProfile.phyMode,
					sap_context->cc_switch_mode);
			if (con_ch && !CDS_IS_DFS_CH(con_ch)) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_ERROR,
					"%s: Override ch %d to %d due to CC Intf",
					__func__, sap_context->channel, con_ch);
				sap_context->channel = con_ch;
				cds_set_channel_params(sap_context->channel, 0,
						&sap_context->ch_params);
			}
		}
#endif
	}

	if (sap_context->channel == AUTO_CHANNEL_SELECT) {
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("%s skip_acs_status = %d "), __func__,
			  sap_context->acs_cfg->skip_scan_status);
		if (sap_context->acs_cfg->skip_scan_status !=
						eSAP_SKIP_ACS_SCAN) {
#endif
		qdf_mem_zero(&scan_request, sizeof(scan_request));

		/*
		 * Set scanType to Active scan. FW takes care of using passive
		 * scan for DFS and active for non DFS channels.
		 */
		scan_request.scanType = eSIR_ACTIVE_SCAN;

		/* Set min and max channel time to zero */
		scan_request.minChnTime = 0;
		scan_request.maxChnTime = 0;

		/* Set BSSType to default type */
		scan_request.BSSType = eCSR_BSS_TYPE_ANY;

#ifndef SOFTAP_CHANNEL_RANGE
		/*Scan all the channels */
		scan_request.ChannelInfo.num_of_channels = 0;

		scan_request.ChannelInfo.ChannelList = NULL;

		scan_request.requestType = eCSR_SCAN_REQUEST_FULL_SCAN;
		/* eCSR_SCAN_REQUEST_11D_SCAN; */

#else

		sap_get_channel_list(sap_context, &channel_list,
				  &num_of_channels);
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		if (num_of_channels != 0) {
#endif
		/*Scan the channels in the list */
		scan_request.ChannelInfo.numOfChannels =
			num_of_channels;

		scan_request.ChannelInfo.ChannelList =
			channel_list;

		scan_request.requestType =
			eCSR_SCAN_SOFTAP_CHANNEL_RANGE;

		sap_context->channelList = channel_list;

#endif
		/* Set requestType to Full scan */

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("calling sme_scan_request"));
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		if (sap_context->acs_cfg->skip_scan_status ==
			eSAP_DO_NEW_ACS_SCAN)
#endif
			sme_scan_flush_result(h_hal);
		if (true == sap_do_acs_pre_start_bss) {
			/*
			 * when ID == 0 11D scan/active scan with callback,
			 * min-maxChntime set in csrScanRequest()?
			 * csrScanCompleteCallback callback
			 * pContext scan_request_id filled up
			 */
			qdf_ret_status = sme_scan_request(h_hal,
				sap_context->sessionId,
				&scan_request,
				&wlansap_pre_start_bss_acs_scan_callback,
				sap_context);
		} else {
			/*
			 * when ID == 0 11D scan/active scan with callback,
			 * min-maxChntime set in csrScanRequest()?
			 * csrScanCompleteCallback callback,
			 * pContext scan_request_id filled up
			 */
			qdf_ret_status = sme_scan_request(h_hal,
				sap_context->sessionId,
				&scan_request,
				&wlansap_scan_callback,
				sap_context);
		}
		if (QDF_STATUS_SUCCESS != qdf_ret_status) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  FL("sme_scan_request  fail %d!!!"),
				  qdf_ret_status);
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  FL("SAP Configuring default channel, Ch=%d"),
				  sap_context->channel);
			/* In case of error, switch to default channel */
			sap_context->channel = SAP_DEFAULT_24GHZ_CHANNEL;

#ifdef SOFTAP_CHANNEL_RANGE
			if (sap_context->channelList != NULL) {
				sap_context->channel =
					sap_context->channelList[0];
				qdf_mem_free(sap_context->
					channelList);
				sap_context->channelList = NULL;
			}
#endif
			if (true == sap_do_acs_pre_start_bss) {
				/*
				* In case of ACS req before start Bss,
				* return failure so that the calling
				* fucntion can use the default channel.
				*/
				return QDF_STATUS_E_FAILURE;
			} else {
				/* Fill in the event structure */
				sap_event_init(sap_event);
				/* Handle event */
				qdf_status = sap_fsm(sap_context, sap_event);
			}
		} else {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				 FL("return sme_ScanReq, scanID=%d, Ch=%d"),
				 scan_request.scan_id, sap_context->channel);
		}
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		}
	} else {
		sap_context->acs_cfg->skip_scan_status = eSAP_SKIP_ACS_SCAN;
	}

	if (sap_context->acs_cfg->skip_scan_status == eSAP_SKIP_ACS_SCAN) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("## %s SKIPPED ACS SCAN"), __func__);
		wlansap_scan_callback(h_hal, sap_context,
			sap_context->sessionId, 0, eCSR_SCAN_SUCCESS);
	}
#endif
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("for configured channel, Ch= %d"),
			  sap_context->channel);

		if (check_for_connection_update) {
			/* This wait happens in the hostapd context. The event
			 * is set in the MC thread context.
			 */
			qdf_status = cds_update_and_wait_for_connection_update(
					sap_context->sessionId,
					sap_context->channel,
					SIR_UPDATE_REASON_START_AP);
			if (QDF_IS_STATUS_ERROR(qdf_status))
				return qdf_status;
		}

		if (sap_do_acs_pre_start_bss == true) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
				FL("ACS end due to Ch override. Sel Ch = %d"),
							sap_context->channel);
			sap_context->acs_cfg->pri_ch = sap_context->channel;
			sap_context->acs_cfg->ch_width =
						 sap_context->ch_width_orig;
			sap_config_acs_result(h_hal, sap_context, 0);
			return QDF_STATUS_E_CANCELED;
		} else {
			/*
			 * Fill in the event structure
			 * Eventhough scan was not done,
			 * means a user set channel was chosen
			 */
			sap_event_init(sap_event);
			/* Handle event */
			qdf_status = sap_fsm(sap_context, sap_event);
		}
	}

	/*
	 * If scan failed, get default channel and advance state
	 * machine as success with default channel
	 *
	 * Have to wait for the call back to be called to get the
	 * channel cannot advance state machine here as said above
	 */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("before exiting sap_goto_channel_sel channel=%d"),
		  sap_context->channel);

	return QDF_STATUS_SUCCESS;
}

/**
 * sap_open_session() - Opens a SAP session
 * @hHal: Hal handle
 * @sapContext:  Sap Context value
 * @session_id: Pointer to the session id
 *
 * Function for opening SME and SAP sessions when system is in SoftAP role
 *
 * Return: QDF_STATUS
 */

#define SAP_OPEN_SESSION_TIMEOUT 2000
QDF_STATUS sap_open_session(tHalHandle hHal, ptSapContext sapContext,
			    uint32_t *session_id)
{
	uint32_t type, subType;
	QDF_STATUS qdf_ret_status;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (sapContext->csr_roamProfile.csrPersona == QDF_P2P_GO_MODE)
		status = cds_get_vdev_types(QDF_P2P_GO_MODE, &type, &subType);
	else
		status = cds_get_vdev_types(QDF_SAP_MODE, &type, &subType);

	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  "failed to get vdev type");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_event_reset(&sapContext->sap_session_opened_evt);
	/* Open SME Session for Softap */
	qdf_ret_status = sme_open_session(hHal,
					  &wlansap_roam_callback,
					  sapContext,
					  sapContext->self_mac_addr,
					  &sapContext->sessionId, type, subType);

	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Error: In %s calling sme_roam_connect status = %d",
			  __func__, qdf_ret_status);

		return QDF_STATUS_E_FAILURE;
	}

	status = qdf_wait_single_event(&sapContext->sap_session_opened_evt,
				       SAP_OPEN_SESSION_TIMEOUT);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		cds_err("wait for sap open session event timed out");
		return QDF_STATUS_E_FAILURE;
	}

	pMac->sap.sapCtxList[sapContext->sessionId].sessionID =
		sapContext->sessionId;
	pMac->sap.sapCtxList[sapContext->sessionId].pSapContext = sapContext;
	pMac->sap.sapCtxList[sapContext->sessionId].sapPersona =
		sapContext->csr_roamProfile.csrPersona;
	*session_id = sapContext->sessionId;
	sapContext->isSapSessionOpen = eSAP_TRUE;
	sapContext->is_pre_cac_on = false;
	sapContext->pre_cac_complete = false;
	sapContext->chan_before_pre_cac = 0;
	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION    sapGotoStarting

   DESCRIPTION
    Function for initiating start bss request for SME

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    bssType     : Type of bss to start, INRA AP
    status      : Return the SAP status here

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation

    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS sap_goto_starting(ptSapContext sapContext, ptWLAN_SAPEvent sapEvent,
			     eCsrRoamBssType bssType)
{
	/* tHalHandle */
	tHalHandle hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	QDF_STATUS qdf_ret_status;

	/*- - - - - - - - TODO:once configs from hdd available - - - - - - - - -*/
	char key_material[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3,
		4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, };
	sapContext->key_type = 0x05;
	sapContext->key_length = 32;
	/* Need a key size define */
	qdf_mem_copy(sapContext->key_material, key_material,
		     sizeof(key_material));

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH, "In %s",
		  __func__);

	if (NULL == hHal) {
		/* we have a serious problem */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
			  "In %s, invalid hHal", __func__);
		return QDF_STATUS_E_FAULT;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO, "%s: session: %d",
		  __func__, sapContext->sessionId);

	qdf_ret_status = sme_roam_connect(hHal, sapContext->sessionId,
					  &sapContext->csr_roamProfile,
					  &sapContext->csr_roamId);
	if (QDF_STATUS_SUCCESS != qdf_ret_status)
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to issue sme_roam_connect", __func__);

	return qdf_ret_status;
} /* sapGotoStarting */

/*==========================================================================
   FUNCTION    sapGotoDisconnecting

   DESCRIPTION
    Processing of SAP FSM Disconnecting state

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext  : Sap Context value
    status      : Return the SAP status here

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation

    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS sap_goto_disconnecting(ptSapContext sapContext)
{
	QDF_STATUS qdf_ret_status;
	tHalHandle hHal;

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		/* we have a serious problem */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s, invalid hHal", __func__);
		return QDF_STATUS_E_FAULT;
	}

	sap_free_roam_profile(&sapContext->csr_roamProfile);
	qdf_ret_status = sme_roam_stop_bss(hHal, sapContext->sessionId);
	if (QDF_STATUS_SUCCESS != qdf_ret_status) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Error: In %s calling sme_roam_stop_bss status = %d",
			  __func__, qdf_ret_status);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS sap_roam_session_close_callback(void *pContext)
{
	ptSapContext sapContext = (ptSapContext) pContext;
	return sap_signal_hdd_event(sapContext, NULL,
				 eSAP_STOP_BSS_EVENT,
				 (void *) eSAP_STATUS_SUCCESS);
}

/*==========================================================================
   FUNCTION    sapGotoDisconnected

   DESCRIPTION
    Function for setting the SAP FSM to Disconnection state

   DEPENDENCIES
    NA.

   PARAMETERS

    IN
    sapContext  : Sap Context value
    sapEvent    : State machine event
    status      : Return the SAP status here

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation

    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS sap_goto_disconnected(ptSapContext sapContext)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tWLAN_SAPEvent sapEvent;
	/* Processing has to be coded */
	/* Clean up stations from TL etc as AP BSS is shut down then set event */
	sapEvent.event = eSAP_MAC_READY_FOR_CONNECTIONS;        /* hardcoded */
	sapEvent.params = 0;
	sapEvent.u1 = 0;
	sapEvent.u2 = 0;
	/* Handle event */
	qdf_status = sap_fsm(sapContext, &sapEvent);

	return qdf_status;
}

/**
 * sap_signal_hdd_event() - send event notification
 * @sap_ctx: Sap Context
 * @csr_roaminfo: Pointer to CSR roam information
 * @sap_hddevent: SAP HDD event
 * @context: to pass the element for future support
 *
 * Function for HDD to send the event notification using callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sap_signal_hdd_event(ptSapContext sap_ctx,
		tCsrRoamInfo *csr_roaminfo, eSapHddEvent sap_hddevent,
		void *context)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tSap_Event sap_ap_event;       /* This now encodes ALL event types */
	tHalHandle hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
	tpAniSirGlobal mac_ctx;
	tSirSmeChanInfo *chaninfo;
	tSap_StationAssocIndication *assoc_ind;
	tSap_StartBssCompleteEvent *bss_complete;
	struct sap_ch_selected_s *acs_selected;
	tSap_StationAssocReassocCompleteEvent *reassoc_complete;
	tSap_StationDisassocCompleteEvent *disassoc_comp;
	tSap_StationSetKeyCompleteEvent *key_complete;
	tSap_StationMICFailureEvent *mic_failure;

	/* Format the Start BSS Complete event to return... */
	if (NULL == sap_ctx->pfnSapEventCallback) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}
	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid hal"));
		return QDF_STATUS_E_FAILURE;
	}
	mac_ctx = PMAC_STRUCT(hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  FL("SAP event callback event = %s"),
		  sap_hdd_event_to_string(sap_hddevent));

	switch (sap_hddevent) {
	case eSAP_STA_ASSOC_IND:
		/*  TODO - Indicate the assoc request indication to OS */
		sap_ap_event.sapHddEventCode = eSAP_STA_ASSOC_IND;
		assoc_ind = &sap_ap_event.sapevt.sapAssocIndication;

		qdf_copy_macaddr(&assoc_ind->staMac, &csr_roaminfo->peerMac);
		assoc_ind->staId = csr_roaminfo->staId;
		assoc_ind->status = 0;
		/* Required for indicating the frames to upper layer */
		assoc_ind->beaconLength = csr_roaminfo->beaconLength;
		assoc_ind->beaconPtr = csr_roaminfo->beaconPtr;
		assoc_ind->assocReqLength = csr_roaminfo->assocReqLength;
		assoc_ind->assocReqPtr = csr_roaminfo->assocReqPtr;
		assoc_ind->fWmmEnabled = csr_roaminfo->wmmEnabledSta;
		if (csr_roaminfo->u.pConnectedProfile != NULL) {
			assoc_ind->negotiatedAuthType =
				csr_roaminfo->u.pConnectedProfile->AuthType;
			assoc_ind->negotiatedUCEncryptionType =
			    csr_roaminfo->u.pConnectedProfile->EncryptionType;
			assoc_ind->negotiatedMCEncryptionType =
			    csr_roaminfo->u.pConnectedProfile->mcEncryptionType;
			assoc_ind->fAuthRequired = csr_roaminfo->fAuthRequired;
		}
		break;
	case eSAP_START_BSS_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_START_BSS_EVENT;
		bss_complete = &sap_ap_event.sapevt.sapStartBssCompleteEvent;

		bss_complete->status = (eSapStatus) context;
		if (csr_roaminfo != NULL)
			bss_complete->staId = csr_roaminfo->staId;
		else
			bss_complete->staId = 0;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("(eSAP_START_BSS_EVENT): staId = %d"),
			  bss_complete->staId);

		bss_complete->operatingChannel = (uint8_t) sap_ctx->channel;
		bss_complete->sessionId = sap_ctx->sessionId;
		break;
	case eSAP_DFS_CAC_START:
	case eSAP_DFS_CAC_INTERRUPTED:
	case eSAP_DFS_CAC_END:
	case eSAP_DFS_PRE_CAC_END:
	case eSAP_DFS_RADAR_DETECT:
	case eSAP_DFS_RADAR_DETECT_DURING_PRE_CAC:
	case eSAP_DFS_NO_AVAILABLE_CHANNEL:
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
	case eSAP_ACS_SCAN_SUCCESS_EVENT:
#endif
		sap_ap_event.sapHddEventCode = sap_hddevent;
		sap_ap_event.sapevt.sapStopBssCompleteEvent.status =
			(eSapStatus) context;
		break;

	case eSAP_ACS_CHANNEL_SELECTED:
		sap_ap_event.sapHddEventCode = sap_hddevent;
		acs_selected = &sap_ap_event.sapevt.sap_ch_selected;
		if (eSAP_STATUS_SUCCESS == (eSapStatus)context) {
			acs_selected->pri_ch = sap_ctx->acs_cfg->pri_ch;
			acs_selected->ht_sec_ch = sap_ctx->acs_cfg->ht_sec_ch;
			acs_selected->ch_width = sap_ctx->acs_cfg->ch_width;
			acs_selected->vht_seg0_center_ch =
				sap_ctx->acs_cfg->vht_seg0_center_ch;
			acs_selected->vht_seg1_center_ch =
				sap_ctx->acs_cfg->vht_seg1_center_ch;
		} else if (eSAP_STATUS_FAILURE == (eSapStatus)context) {
			acs_selected->pri_ch = 0;
		}
		break;

	case eSAP_STOP_BSS_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_STOP_BSS_EVENT;
		sap_ap_event.sapevt.sapStopBssCompleteEvent.status =
			(eSapStatus) context;
		break;

	case eSAP_STA_ASSOC_EVENT:
	case eSAP_STA_REASSOC_EVENT:
		reassoc_complete =
		    &sap_ap_event.sapevt.sapStationAssocReassocCompleteEvent;

		if (csr_roaminfo->fReassocReq)
			sap_ap_event.sapHddEventCode = eSAP_STA_REASSOC_EVENT;
		else
			sap_ap_event.sapHddEventCode = eSAP_STA_ASSOC_EVENT;

		qdf_copy_macaddr(&reassoc_complete->staMac,
				 &csr_roaminfo->peerMac);
		reassoc_complete->staId = csr_roaminfo->staId;
		reassoc_complete->statusCode = csr_roaminfo->statusCode;
		reassoc_complete->iesLen = csr_roaminfo->rsnIELen;
		qdf_mem_copy(reassoc_complete->ies, csr_roaminfo->prsnIE,
			     csr_roaminfo->rsnIELen);

#ifdef FEATURE_WLAN_WAPI
		if (csr_roaminfo->wapiIELen) {
			uint8_t len = reassoc_complete->iesLen;
			reassoc_complete->iesLen += csr_roaminfo->wapiIELen;
			qdf_mem_copy(&reassoc_complete->ies[len],
				     csr_roaminfo->pwapiIE,
				     csr_roaminfo->wapiIELen);
		}
#endif
		if (csr_roaminfo->addIELen) {
			uint8_t len = reassoc_complete->iesLen;
			reassoc_complete->iesLen += csr_roaminfo->addIELen;
			qdf_mem_copy(&reassoc_complete->ies[len],
				     csr_roaminfo->paddIE,
				     csr_roaminfo->addIELen);
		}

		/* also fill up the channel info from the csr_roamInfo */
		chaninfo = &reassoc_complete->chan_info;

		chaninfo->chan_id = csr_roaminfo->chan_info.chan_id;
		chaninfo->mhz = csr_roaminfo->chan_info.mhz;
		chaninfo->info = csr_roaminfo->chan_info.info;
		chaninfo->band_center_freq1 =
			csr_roaminfo->chan_info.band_center_freq1;
		chaninfo->band_center_freq2 =
			csr_roaminfo->chan_info.band_center_freq2;
		chaninfo->reg_info_1 =
			csr_roaminfo->chan_info.reg_info_1;
		chaninfo->reg_info_2 =
			csr_roaminfo->chan_info.reg_info_2;
		chaninfo->nss = csr_roaminfo->chan_info.nss;
		chaninfo->rate_flags = csr_roaminfo->chan_info.rate_flags;

		reassoc_complete->wmmEnabled = csr_roaminfo->wmmEnabledSta;
		reassoc_complete->status = (eSapStatus) context;
		reassoc_complete->timingMeasCap = csr_roaminfo->timingMeasCap;
		break;

	case eSAP_STA_DISASSOC_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_STA_DISASSOC_EVENT;
		disassoc_comp =
			&sap_ap_event.sapevt.sapStationDisassocCompleteEvent;

		qdf_copy_macaddr(&disassoc_comp->staMac,
				 &csr_roaminfo->peerMac);
		disassoc_comp->staId = csr_roaminfo->staId;
		if (csr_roaminfo->reasonCode == eCSR_ROAM_RESULT_FORCED)
			disassoc_comp->reason = eSAP_USR_INITATED_DISASSOC;
		else
			disassoc_comp->reason = eSAP_MAC_INITATED_DISASSOC;

		disassoc_comp->statusCode = csr_roaminfo->statusCode;
		disassoc_comp->status = (eSapStatus) context;
		break;

	case eSAP_STA_SET_KEY_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_STA_SET_KEY_EVENT;
		key_complete =
			&sap_ap_event.sapevt.sapStationSetKeyCompleteEvent;
		key_complete->status = (eSapStatus) context;
		qdf_copy_macaddr(&key_complete->peerMacAddr,
				 &csr_roaminfo->peerMac);
		break;

	case eSAP_STA_MIC_FAILURE_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_STA_MIC_FAILURE_EVENT;
		mic_failure = &sap_ap_event.sapevt.sapStationMICFailureEvent;

		qdf_mem_copy(&mic_failure->srcMacAddr,
			     csr_roaminfo->u.pMICFailureInfo->srcMacAddr,
			     sizeof(tSirMacAddr));
		qdf_mem_copy(&mic_failure->staMac.bytes,
			     csr_roaminfo->u.pMICFailureInfo->taMacAddr,
			     sizeof(tSirMacAddr));
		qdf_mem_copy(&mic_failure->dstMacAddr.bytes,
			     csr_roaminfo->u.pMICFailureInfo->dstMacAddr,
			     sizeof(tSirMacAddr));
		mic_failure->multicast =
			csr_roaminfo->u.pMICFailureInfo->multicast;
		mic_failure->IV1 = csr_roaminfo->u.pMICFailureInfo->IV1;
		mic_failure->keyId = csr_roaminfo->u.pMICFailureInfo->keyId;
		qdf_mem_copy(mic_failure->TSC,
			     csr_roaminfo->u.pMICFailureInfo->TSC,
			     SIR_CIPHER_SEQ_CTR_SIZE);
		break;

	case eSAP_ASSOC_STA_CALLBACK_EVENT:
		break;

	case eSAP_WPS_PBC_PROBE_REQ_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_WPS_PBC_PROBE_REQ_EVENT;

		qdf_mem_copy(&sap_ap_event.sapevt.sapPBCProbeReqEvent.
			     WPSPBCProbeReq, csr_roaminfo->u.pWPSPBCProbeReq,
			     sizeof(tSirWPSPBCProbeReq));
		break;

	case eSAP_REMAIN_CHAN_READY:
		sap_ap_event.sapHddEventCode = eSAP_REMAIN_CHAN_READY;
		sap_ap_event.sapevt.sap_roc_ind.scan_id =
				sap_ctx->roc_ind_scan_id;
		break;
	case eSAP_SEND_ACTION_CNF:
		sap_ap_event.sapHddEventCode = eSAP_SEND_ACTION_CNF;
		sap_ap_event.sapevt.sapActionCnf.actionSendSuccess =
			(eSapStatus) context;
		break;

	case eSAP_DISCONNECT_ALL_P2P_CLIENT:
		sap_ap_event.sapHddEventCode = eSAP_DISCONNECT_ALL_P2P_CLIENT;
		sap_ap_event.sapevt.sapActionCnf.actionSendSuccess =
			(eSapStatus) context;
		break;

	case eSAP_MAC_TRIG_STOP_BSS_EVENT:
		sap_ap_event.sapHddEventCode = eSAP_MAC_TRIG_STOP_BSS_EVENT;
		sap_ap_event.sapevt.sapActionCnf.actionSendSuccess =
			(eSapStatus) context;
		break;

	case eSAP_UNKNOWN_STA_JOIN:
		sap_ap_event.sapHddEventCode = eSAP_UNKNOWN_STA_JOIN;
		qdf_mem_copy((void *) sap_ap_event.sapevt.sapUnknownSTAJoin.
			     macaddr.bytes, (void *) context,
			     QDF_MAC_ADDR_SIZE);
		break;

	case eSAP_MAX_ASSOC_EXCEEDED:
		sap_ap_event.sapHddEventCode = eSAP_MAX_ASSOC_EXCEEDED;
		qdf_copy_macaddr(&sap_ap_event.sapevt.
				 sapMaxAssocExceeded.macaddr,
				 &csr_roaminfo->peerMac);
		break;

	case eSAP_CHANNEL_CHANGE_EVENT:
		/*
		 * Reconfig ACS result info. For DFS AP-AP Mode Sec AP ACS
		 * follows pri AP
		 */
		sap_ctx->acs_cfg->pri_ch = sap_ctx->channel;
		sap_ctx->acs_cfg->ch_width =
				sap_ctx->csr_roamProfile.ch_params.ch_width;
		sap_config_acs_result(hal, sap_ctx,
			sap_ctx->csr_roamProfile.ch_params.sec_ch_offset);

		sap_ap_event.sapHddEventCode = eSAP_CHANNEL_CHANGE_EVENT;

		acs_selected = &sap_ap_event.sapevt.sap_ch_selected;
		acs_selected->pri_ch = sap_ctx->acs_cfg->pri_ch;
		acs_selected->ht_sec_ch =
			sap_ctx->csr_roamProfile.ch_params.sec_ch_offset;
		acs_selected->ch_width =
			sap_ctx->csr_roamProfile.ch_params.ch_width;
		acs_selected->vht_seg0_center_ch =
			sap_ctx->csr_roamProfile.ch_params.center_freq_seg0;
		acs_selected->vht_seg1_center_ch =
			sap_ctx->csr_roamProfile.ch_params.center_freq_seg1;
		break;

	case eSAP_DFS_NOL_GET:
		sap_ap_event.sapHddEventCode = eSAP_DFS_NOL_GET;
		sap_ap_event.sapevt.sapDfsNolInfo.sDfsList =
			NUM_5GHZ_CHANNELS * sizeof(tSapDfsNolInfo);
		sap_ap_event.sapevt.sapDfsNolInfo.pDfsList = (void *)
			(&mac_ctx->sap.SapDfsInfo.sapDfsChannelNolList[0]);
		break;

	case eSAP_DFS_NOL_SET:
		sap_ap_event.sapHddEventCode = eSAP_DFS_NOL_SET;
		sap_ap_event.sapevt.sapDfsNolInfo.sDfsList =
			mac_ctx->sap.SapDfsInfo.numCurrentRegDomainDfsChannels *
			sizeof(tSapDfsNolInfo);
		sap_ap_event.sapevt.sapDfsNolInfo.pDfsList = (void *)
			(&mac_ctx->sap.SapDfsInfo.sapDfsChannelNolList[0]);
		break;
	case eSAP_ECSA_CHANGE_CHAN_IND:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				"In %s, SAP event callback event = %s",
				__func__, "eSAP_ECSA_CHANGE_CHAN_IND");
		sap_ap_event.sapHddEventCode = eSAP_ECSA_CHANGE_CHAN_IND;
		sap_ap_event.sapevt.sap_chan_cng_ind.new_chan =
					   csr_roaminfo->target_channel;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("SAP Unknown callback event = %d"),
			  sap_hddevent);
		break;
	}
	qdf_status = (*sap_ctx->pfnSapEventCallback)
			(&sap_ap_event, sap_ctx->pUsrContext);

	return qdf_status;

}

/*==========================================================================
   FUNCTION  sap_find_valid_concurrent_session

   DESCRIPTION
    This function will return sapcontext of any valid sap session.

   PARAMETERS

    IN
    hHal        : HAL pointer

   RETURN VALUE
    ptSapContext : valid sap context

   SIDE EFFECTS
    NA
   ============================================================================*/
ptSapContext sap_find_valid_concurrent_session(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t intf = 0;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona)) &&
		    pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			return pMac->sap.sapCtxList[intf].pSapContext;
		}
	}

	return NULL;
}

/**
 * sap_find_cac_wait_session() - Get context of a SAP session in CAC wait state
 * @handle: Global MAC handle
 *
 * Finds and gets the context of a SAP session in CAC wait state.
 *
 * Return: Valid SAP context on success, else NULL
 */
ptSapContext sap_find_cac_wait_session(tHalHandle handle)
{
	tpAniSirGlobal mac = PMAC_STRUCT(handle);
	uint8_t i = 0;
	ptSapContext sapContext;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			"%s", __func__);

	for (i = 0; i < SAP_MAX_NUM_SESSION; i++) {
		sapContext = (ptSapContext) mac->sap.sapCtxList[i].pSapContext;
		if (((QDF_SAP_MODE == mac->sap.sapCtxList[i].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == mac->sap.sapCtxList[i].sapPersona)) &&
		    (sapContext) &&
		    (sapContext->sapsMachine == eSAP_DFS_CAC_WAIT)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				"%s: found SAP in cac wait state", __func__);
			return sapContext;
		}
		if (sapContext) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
					"sapdfs: mode:%d intf:%d state:%d",
					mac->sap.sapCtxList[i].sapPersona, i,
					sapContext->sapsMachine);
		}
	}

	return NULL;
}

/*==========================================================================
   FUNCTION   sap_close_session

   DESCRIPTION
    This function will close all the sme sessions as well as zero-out the
    sap global structure

   PARAMETERS

    IN
    hHal        : HAL pointer
    sapContext  : Sap Context value
    callback    : Roam Session close callback
    valid       : Sap context is valid or no

   RETURN VALUE
    The QDF_STATUS code associated with performing the operation
    QDF_STATUS_SUCCESS: Success

   SIDE EFFECTS
    NA
   ============================================================================*/
QDF_STATUS sap_close_session(tHalHandle hHal,
			     ptSapContext sapContext,
			     csr_roamSessionCloseCallback callback, bool valid)
{
	QDF_STATUS qdf_status;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	if (false == valid) {
		qdf_status = sme_close_session(hHal,
					       sapContext->sessionId,
					       callback, NULL);
	} else {
		qdf_status = sme_close_session(hHal,
					       sapContext->sessionId,
					       callback, sapContext);
	}

	sapContext->isCacStartNotified = false;
	sapContext->isCacEndNotified = false;
	pMac->sap.sapCtxList[sapContext->sessionId].pSapContext = NULL;
	sapContext->isSapSessionOpen = false;
	sapContext->pre_cac_complete = false;
	sapContext->is_pre_cac_on = false;
	sapContext->chan_before_pre_cac = 0;

	if (NULL == sap_find_valid_concurrent_session(hHal)) {
		/* If timer is running then stop the timer and destory it */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  "sapdfs: no session are valid, so clearing dfs global structure");
		/*
		 * CAC timer will be initiated and started only when SAP starts
		 * on DFS channel and it will be stopped and destroyed
		 * immediately once the radar detected or timedout. So
		 * as per design CAC timer should be destroyed after stop
		 */
		if (pMac->sap.SapDfsInfo.is_dfs_cac_timer_running) {
			qdf_mc_timer_stop(&pMac->sap.SapDfsInfo.
					  sap_dfs_cac_timer);
			pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = 0;
			qdf_mc_timer_destroy(
				&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);
		}
		pMac->sap.SapDfsInfo.cac_state = eSAP_DFS_DO_NOT_SKIP_CAC;
		sap_cac_reset_notify(hHal);
		qdf_mem_zero(&pMac->sap, sizeof(pMac->sap));
	}

	return qdf_status;
}

/*==========================================================================
   FUNCTION  sap_cac_reset_notify

   DESCRIPTION Function will be called up on stop bss indication to clean up
   DFS global structure.

   DEPENDENCIES PARAMETERS
     IN hHAL : HAL pointer

   RETURN VALUE  : void.

   SIDE EFFECTS
   ============================================================================*/
void sap_cac_reset_notify(tHalHandle hHal)
{
	uint8_t intf = 0;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		ptSapContext pSapContext =
			(ptSapContext) pMac->sap.sapCtxList[intf].pSapContext;
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			pSapContext->isCacStartNotified = false;
			pSapContext->isCacEndNotified = false;
		}
	}
}

/*==========================================================================
   FUNCTION  sap_cac_start_notify

   DESCRIPTION Function will be called to notify eSAP_DFS_CAC_START event
   to HDD

   DEPENDENCIES PARAMETERS
     IN hHAL : HAL pointer

   RETURN VALUE  : QDF_STATUS.

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS sap_cac_start_notify(tHalHandle hHal)
{
	uint8_t intf = 0;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		ptSapContext pSapContext =
			(ptSapContext) pMac->sap.sapCtxList[intf].pSapContext;
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL &&
		    (false == pSapContext->isCacStartNotified)) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: Signaling eSAP_DFS_CAC_START to HDD for sapctx[%p]",
				  pSapContext);

			qdf_status = sap_signal_hdd_event(pSapContext, NULL,
							  eSAP_DFS_CAC_START,
							  (void *)
							  eSAP_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "In %s, failed setting isCacStartNotified on interface[%d]",
					  __func__, intf);
				return qdf_status;
			}
			pSapContext->isCacStartNotified = true;
		}
	}
	return qdf_status;
}

/**
 * wlansap_update_pre_cac_end() - Update pre cac end to upper layer
 * @sap_context: SAP context
 * @mac: Global MAC structure
 * @intf: Interface number
 *
 * Notifies pre cac end to upper layer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlansap_update_pre_cac_end(ptSapContext sap_context,
		tpAniSirGlobal mac, uint8_t intf)
{
	QDF_STATUS qdf_status;

	sap_context->isCacEndNotified = true;
	mac->sap.SapDfsInfo.sap_radar_found_status = false;
	sap_context->sapsMachine = eSAP_STARTED;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"In %s, pre cac end notify on %d: from state %s => %s",
			__func__, intf, "eSAP_DFS_CAC_WAIT",
			"eSAP_STARTED");

	qdf_status = sap_signal_hdd_event(sap_context,
			NULL, eSAP_DFS_PRE_CAC_END,
			(void *)eSAP_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(qdf_status)) {
		QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_ERROR,
				"In %s, pre cac notify failed on intf %d",
				__func__, intf);
		return qdf_status;
	}

	return QDF_STATUS_SUCCESS;
}

/*==========================================================================
   FUNCTION  sap_cac_end_notify

   DESCRIPTION Function will be called to notify eSAP_DFS_CAC_END event
   to HDD

   DEPENDENCIES PARAMETERS
     IN hHAL : HAL pointer

   RETURN VALUE  : QDF_STATUS.

   SIDE EFFECTS
   ============================================================================*/
QDF_STATUS sap_cac_end_notify(tHalHandle hHal, tCsrRoamInfo *roamInfo)
{
	uint8_t intf;
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	/*
	 * eSAP_DFS_CHANNEL_CAC_END:
	 * CAC Period elapsed and there was no radar
	 * found so, SAP can continue beaconing.
	 * sap_radar_found_status is set to 0
	 */
	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		ptSapContext pSapContext =
			(ptSapContext) pMac->sap.sapCtxList[intf].pSapContext;
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL &&
		    (false == pSapContext->isCacEndNotified) &&
		    (pSapContext->sapsMachine == eSAP_DFS_CAC_WAIT)) {
			pSapContext = pMac->sap.sapCtxList[intf].pSapContext;

			/* If this is an end notification of a pre cac, the
			 * SAP must not start beaconing and must delete the
			 * temporary interface created for pre cac and switch
			 * the original SAP to the pre CAC channel.
			 */
			if (pSapContext->is_pre_cac_on) {
				qdf_status = wlansap_update_pre_cac_end(
						pSapContext, pMac, intf);
				if (QDF_IS_STATUS_ERROR(qdf_status))
					return qdf_status;
				/* pre CAC is not allowed with any concurrency.
				 * So, we can break from here.
				 */
				break;
			}

			qdf_status = sap_signal_hdd_event(pSapContext, NULL,
							  eSAP_DFS_CAC_END,
							  (void *)
							  eSAP_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "In %s, failed setting isCacEndNotified on interface[%d]",
					  __func__, intf);
				return qdf_status;
			}
			pSapContext->isCacEndNotified = true;
			pMac->sap.SapDfsInfo.sap_radar_found_status = false;
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: Start beacon request on sapctx[%p]",
				  pSapContext);

			/* Start beaconing on the new channel */
			wlansap_start_beacon_req(pSapContext);

			/* Transition from eSAP_STARTING to eSAP_STARTED
			 * (both without substates)
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				  "sapdfs: channel[%d] from state %s => %s",
				  pSapContext->channel, "eSAP_STARTING",
				  "eSAP_STARTED");

			pSapContext->sapsMachine = eSAP_STARTED;

			/*Action code for transition */
			qdf_status = sap_signal_hdd_event(pSapContext, roamInfo,
							  eSAP_START_BSS_EVENT,
							  (void *)
							  eSAP_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  "In %s, failed setting isCacEndNotified on interface[%d]",
					  __func__, intf);
				return qdf_status;
			}

			/* Transition from eSAP_STARTING to eSAP_STARTED
			 * (both without substates)
			 */
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "In %s, from state %s => %s",
				  __func__, "eSAP_DFS_CAC_WAIT",
				  "eSAP_STARTED");
		}
	}
	/*
	 * All APs are done with CAC timer, all APs should start beaconing.
	 * Lets assume AP1 and AP2 started beaconing on DFS channel, Now lets
	 * say AP1 goes down and comes back on same DFS channel. In this case
	 * AP1 shouldn't start CAC timer and start beacon immediately beacause
	 * AP2 is already beaconing on this channel. This case will be handled
	 * by checking against eSAP_DFS_SKIP_CAC while starting the timer.
	 */
	pMac->sap.SapDfsInfo.cac_state = eSAP_DFS_SKIP_CAC;
	return qdf_status;
}

/**
 * sap_fsm_state_disconnected() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_DISCONNECTED"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_disconnected(ptSapContext sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_HDD_START_INFRA_BSS) {
		/*
		 * Transition from eSAP_DISCONNECTED to eSAP_CH_SELECT
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("new from state %s => %s: session:%d"),
			  "eSAP_DISCONNECTED", "eSAP_CH_SELECT",
			  sap_ctx->sessionId);

		if (sap_ctx->isSapSessionOpen == eSAP_FALSE) {
			uint32_t type, subtype;
			if (sap_ctx->csr_roamProfile.csrPersona ==
			    QDF_P2P_GO_MODE)
				qdf_status = cds_get_vdev_types(QDF_P2P_GO_MODE,
							&type, &subtype);
			else
				qdf_status = cds_get_vdev_types(QDF_SAP_MODE,
								&type,
								&subtype);

			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
						QDF_TRACE_LEVEL_FATAL,
						"failed to get vdev type");
				return QDF_STATUS_E_FAILURE;
			}
			/* Open SME Session for scan */
			qdf_status = sme_open_session(hal, NULL,
					sap_ctx, sap_ctx->self_mac_addr,
					&sap_ctx->sessionId, type, subtype);
			if (QDF_STATUS_SUCCESS != qdf_status) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					 QDF_TRACE_LEVEL_ERROR,
					 FL("Error: calling sme_open_session"));
				return QDF_STATUS_E_FAILURE;
			}

			sap_ctx->isSapSessionOpen = eSAP_TRUE;
		}

		/* init dfs channel nol */
		sap_init_dfs_channel_nol_list(sap_ctx);

		/* Set SAP device role */
		sap_ctx->sapsMachine = eSAP_CH_SELECT;

		/*
		 * Perform sme_ScanRequest. This scan request is post start bss
		 * request so, set the third to false.
		 */
		qdf_status = sap_goto_channel_sel(sap_ctx, sap_event, false,
						true);

		/*
		 * Transition from eSAP_DISCONNECTED to eSAP_CH_SELECT
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_DISCONNECTED", "eSAP_CH_SELECT");
	} else if (msg == eSAP_DFS_CHANNEL_CAC_START) {
		/*
		 * No need of state check here, caller is expected to perform
		 * the checks before sending the event
		 */
		sap_ctx->sapsMachine = eSAP_DFS_CAC_WAIT;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			FL("from state eSAP_DISCONNECTED => SAP_DFS_CAC_WAIT"));
		if (mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running != true) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			    FL("sapdfs: starting dfs cac timer on sapctx[%p]"),
			    sap_ctx);
			sap_start_dfs_cac_timer(sap_ctx);
		}

		qdf_status = sap_cac_start_notify(hal);
	} else if (msg == eSAP_CHANNEL_SELECTION_RETRY) {
		/* Set SAP device role */
		sap_ctx->sapsMachine = eSAP_CH_SELECT;

		/*
		 * Perform sme_ScanRequest. This scan request is post start bss
		 * request so, set the third to false.
		 */
		qdf_status = sap_goto_channel_sel(sap_ctx, sap_event, false,
					false);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, event msg %d"),
			  "eSAP_DISCONNECTED", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_ch_select() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_CH_SELECT"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_ch_select(ptSapContext sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint32_t cbmode;
	bool b_leak_chan = false;
#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
	uint8_t temp_chan;
	tSapDfsNolInfo *p_nol;
#endif

	if (msg == eSAP_MAC_SCAN_COMPLETE) {
		/* get the bonding mode */
		if (sap_ctx->channel <= 14)
			cbmode = sme_get_cb_phy_state_from_cb_ini_value(
					sme_get_channel_bonding_mode24_g(hal));
		else
			cbmode = sme_get_cb_phy_state_from_cb_ini_value(
					sme_get_channel_bonding_mode5_g(hal));

#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
		temp_chan = sap_ctx->channel;
		p_nol = mac_ctx->sap.SapDfsInfo.sapDfsChannelNolList;

		sap_mark_leaking_ch(sap_ctx,
			cbmode, p_nol, 1, &temp_chan);

		/*
		 * if selelcted channel has leakage to channels
		 * in NOL, the temp_chan will be reset
		 */
		b_leak_chan = (temp_chan != sap_ctx->channel);
#endif
		/*
		 * check if channel is in DFS_NOL or if the channel
		 * has leakage to the channels in NOL
		 */
		if (sap_dfs_is_channel_in_nol_list(sap_ctx, sap_ctx->channel,
			cbmode) || b_leak_chan) {
			uint8_t ch;

			/* find a new available channel */
			ch = sap_random_channel_sel(sap_ctx);
			if (ch == 0) {
				/* No available channel found */
				QDF_TRACE(QDF_MODULE_ID_SAP,
					QDF_TRACE_LEVEL_ERROR,
					FL("No available channel found!!!"));
				sap_signal_hdd_event(sap_ctx, NULL,
					eSAP_DFS_NO_AVAILABLE_CHANNEL,
					(void *)eSAP_STATUS_SUCCESS);
				return QDF_STATUS_E_FAULT;
			}

			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  FL("channel %d is in NOL, StartBss on new channel %d"),
				  sap_ctx->channel, ch);

			sap_ctx->channel = ch;
			cds_set_channel_params(sap_ctx->channel,
				sap_ctx->secondary_ch, &sap_ctx->ch_params);
		}
		if (sap_ctx->channel > 14 &&
		    (sap_ctx->csr_roamProfile.phyMode == eCSR_DOT11_MODE_11g ||
		     sap_ctx->csr_roamProfile.phyMode ==
						eCSR_DOT11_MODE_11g_ONLY))
			sap_ctx->csr_roamProfile.phyMode = eCSR_DOT11_MODE_11a;

		/*
		 * when AP2 is started while AP1 is performing ACS, we may not
		 * have the AP1 channel yet.So here after the completion of AP2
		 * ACS check if AP1 ACS resulting channel is DFS and if yes
		 * override AP2 ACS scan result with AP1 DFS channel
		 */
		if (cds_concurrent_beaconing_sessions_running()) {
			uint16_t con_ch;

			con_ch = sme_get_concurrent_operation_channel(hal);
			if (con_ch && CDS_IS_DFS_CH(con_ch))
				sap_ctx->channel = con_ch;
		}

		/*
		 * Transition from eSAP_CH_SELECT to eSAP_STARTING
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_CH_SELECT", "eSAP_STARTING");
		/* Channel selected. Now can sap_goto_starting */
		sap_ctx->sapsMachine = eSAP_STARTING;
		/* Specify the channel */
		sap_ctx->csr_roamProfile.ChannelInfo.numOfChannels =
						1;
		sap_ctx->csr_roamProfile.ChannelInfo.ChannelList =
			&sap_ctx->csr_roamProfile.operationChannel;
		sap_ctx->csr_roamProfile.operationChannel =
			(uint8_t) sap_ctx->channel;
		sap_ctx->csr_roamProfile.ch_params.ch_width =
					sap_ctx->ch_params.ch_width;
		sap_ctx->csr_roamProfile.ch_params.center_freq_seg0 =
				sap_ctx->ch_params.center_freq_seg0;
		sap_ctx->csr_roamProfile.ch_params.center_freq_seg1 =
				sap_ctx->ch_params.center_freq_seg1;
		sap_ctx->csr_roamProfile.ch_params.sec_ch_offset =
				sap_ctx->ch_params.sec_ch_offset;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		    FL("notify hostapd about channel selection: %d"),
		    sap_ctx->channel);
		sap_signal_hdd_event(sap_ctx, NULL,
					eSAP_CHANNEL_CHANGE_EVENT,
					(void *) eSAP_STATUS_SUCCESS);
		qdf_status = sap_goto_starting(sap_ctx, sap_event,
					  eCSR_BSS_TYPE_INFRA_AP);
	} else if (msg == eSAP_CHANNEL_SELECTION_FAILED) {
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		FL("Cannot start BSS, ACS Fail"));
	} else if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		sap_signal_hdd_event(sap_ctx, NULL, eSAP_START_BSS_EVENT,
					(void *)eSAP_STATUS_FAILURE);
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			"%s: BSS stopped when Ch select in Progress", __func__);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_CH_SELECT", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_dfs_cac_wait() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_DFS_CAC_WAIT"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_dfs_cac_wait(ptSapContext sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	tCsrRoamInfo *roam_info = (tCsrRoamInfo *) (sap_event->params);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_DFS_CHANNEL_CAC_START) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_CH_SELECT", "eSAP_DFS_CAC_WAIT");
		if (mac_ctx->sap.SapDfsInfo.is_dfs_cac_timer_running != true)
			sap_start_dfs_cac_timer(sap_ctx);
		qdf_status = sap_cac_start_notify(hal);
	} else if (msg == eSAP_DFS_CHANNEL_CAC_RADAR_FOUND) {
		uint8_t intf;
		/*
		 * Radar found while performing channel availability
		 * check, need to switch the channel again
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  "ENTERTRED CAC WAIT STATE-->eSAP_DISCONNECTING\n");
		if (mac_ctx->sap.SapDfsInfo.target_channel) {
			cds_set_channel_params(
				mac_ctx->sap.SapDfsInfo.target_channel, 0,
				&sap_ctx->ch_params);
		}

		for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
			ptSapContext t_sap_ctx;
			if (((QDF_SAP_MODE ==
				 mac_ctx->sap.sapCtxList[intf].sapPersona) ||
			     (QDF_P2P_GO_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona)) &&
			    mac_ctx->sap.sapCtxList[intf].pSapContext != NULL) {
				t_sap_ctx =
				    mac_ctx->sap.sapCtxList[intf].pSapContext;
				/* SAP to be moved to DISCONNECTING state */
				sap_ctx->sapsMachine = eSAP_DISCONNECTING;
				/*
				 * eSAP_DFS_CHANNEL_CAC_RADAR_FOUND:
				 * A Radar is found on current DFS Channel
				 * while in CAC WAIT period So, do a channel
				 * switch to randomly selected  target channel.
				 * Send the Channel change message to SME/PE.
				 * sap_radar_found_status is set to 1
				 */
				sap_signal_hdd_event(t_sap_ctx, NULL,
					eSAP_DFS_RADAR_DETECT,
					(void *)eSAP_STATUS_SUCCESS);

				wlansap_channel_change_request(
					(void *)t_sap_ctx,
					mac_ctx->sap.SapDfsInfo.target_channel);
			}
		}
	} else if (msg == eSAP_DFS_CHANNEL_CAC_END) {
		qdf_status = sap_cac_end_notify(hal, roam_info);
	} else if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		/* Transition from eSAP_DFS_CAC_WAIT to eSAP_DISCONNECTING */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_DFS_CAC_WAIT", "eSAP_DISCONNECTING");

		/*
		 * Stop the CAC timer only in following conditions
		 * single AP: if there is a single AP then stop the timer
		 * mulitple APs: incase of multiple APs, make sure that
		 *               all APs are down.
		 */
		if (NULL == sap_find_valid_concurrent_session(hal)) {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				  QDF_TRACE_LEVEL_INFO_MED,
				  FL("sapdfs: no sessions are valid, stopping timer"));
			sap_stop_dfs_cac_timer(sap_ctx);
		}

		sap_ctx->sapsMachine = eSAP_DISCONNECTING;
		qdf_status = sap_goto_disconnecting(sap_ctx);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_DFS_CAC_WAIT", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_starting() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 * @hal: HAL handle
 *
 * This function is called for state transition from "eSAP_STARTING"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_starting(ptSapContext sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	tCsrRoamInfo *roam_info = (tCsrRoamInfo *) (sap_event->params);
	tSapDfsInfo *sap_dfs_info;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint8_t is_dfs = false;

	if (msg == eSAP_MAC_START_BSS_SUCCESS) {
		/*
		 * Transition from eSAP_STARTING to eSAP_STARTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state channel = %d %s => %s ch_width %d"),
			  sap_ctx->channel, "eSAP_STARTING", "eSAP_STARTED",
			  sap_ctx->ch_params.ch_width);
		sap_ctx->sapsMachine = eSAP_STARTED;

		/* Action code for transition */
		qdf_status = sap_signal_hdd_event(sap_ctx, roam_info,
				eSAP_START_BSS_EVENT,
				(void *) eSAP_STATUS_SUCCESS);

		/*
		 * The upper layers have been informed that AP is up and
		 * running, however, the AP is still not beaconing, until
		 * CAC is done if the operating channel is DFS
		 */
		if (sap_ctx->ch_params.ch_width == CH_WIDTH_160MHZ) {
			is_dfs = true;
		} else if (sap_ctx->ch_params.ch_width == CH_WIDTH_80P80MHZ) {
			if (cds_get_channel_state(sap_ctx->channel) ==
			    CHANNEL_STATE_DFS ||
			    cds_get_channel_state(sap_ctx->
				ch_params.center_freq_seg1 -
				SIR_80MHZ_START_CENTER_CH_DIFF) ==
					CHANNEL_STATE_DFS)
				is_dfs = true;
		} else {
			if (cds_get_channel_state(sap_ctx->channel) ==
							CHANNEL_STATE_DFS)
				is_dfs = true;
		}

		if (is_dfs) {
			sap_dfs_info = &mac_ctx->sap.SapDfsInfo;
			if ((false == sap_dfs_info->ignore_cac) &&
			    (eSAP_DFS_DO_NOT_SKIP_CAC ==
					sap_dfs_info->cac_state) &&
			    !sap_ctx->pre_cac_complete) {
				/* Move the device in CAC_WAIT_STATE */
				sap_ctx->sapsMachine = eSAP_DFS_CAC_WAIT;

				/*
				 * Need to stop the OS transmit queues,
				 * so that no traffic can flow down the stack
				 */

				/* Start CAC wait timer */
				if (sap_dfs_info->is_dfs_cac_timer_running !=
									true)
					sap_start_dfs_cac_timer(sap_ctx);
				qdf_status = sap_cac_start_notify(hal);

			} else {
				wlansap_start_beacon_req(sap_ctx);
			}
		}
	} else if (msg == eSAP_MAC_START_FAILS) {
		/*
		 * Transition from STARTING to DISCONNECTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("from state %s => %s"),
			  "eSAP_STARTING", "eSAP_DISCONNECTED");

		/*Action code for transition */
		qdf_status = sap_signal_hdd_event(sap_ctx, NULL,
				eSAP_START_BSS_EVENT,
				(void *) eSAP_STATUS_FAILURE);
		qdf_status = sap_goto_disconnected(sap_ctx);
		/* Advance outer statevar */
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
	} else if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		/*
		 * Transition from eSAP_STARTING to eSAP_DISCONNECTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_STARTING", "eSAP_DISCONNECTED");

		/* Advance outer statevar */
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;
		qdf_status = sap_signal_hdd_event(sap_ctx, NULL,
				eSAP_START_BSS_EVENT,
				(void *) eSAP_STATUS_FAILURE);
		qdf_status = sap_goto_disconnected(sap_ctx);
		/* Close the SME session */

		if (eSAP_TRUE == sap_ctx->isSapSessionOpen) {
			if (QDF_STATUS_SUCCESS == sap_close_session(hal,
						sap_ctx, NULL, false))
				sap_ctx->isSapSessionOpen = eSAP_FALSE;
		}
	} else if (msg == eSAP_OPERATING_CHANNEL_CHANGED) {
		/* The operating channel has changed, update hostapd */
		sap_ctx->channel =
			(uint8_t) mac_ctx->sap.SapDfsInfo.target_channel;

		sap_ctx->sapsMachine = eSAP_STARTED;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_STARTING", "eSAP_STARTED");

		/* Indicate change in the state to upper layers */
		qdf_status = sap_signal_hdd_event(sap_ctx, roam_info,
				  eSAP_START_BSS_EVENT,
				  (void *)eSAP_STATUS_SUCCESS);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_STARTING", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_started() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 *
 * This function is called for state transition from "eSAP_STARTED"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_started(ptSapContext sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_HDD_STOP_INFRA_BSS) {
		/*
		 * Transition from eSAP_STARTED to eSAP_DISCONNECTING
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_STARTED", "eSAP_DISCONNECTING");
		sap_ctx->sapsMachine = eSAP_DISCONNECTING;
		qdf_status = sap_goto_disconnecting(sap_ctx);
	} else if (eSAP_DFS_CHNL_SWITCH_ANNOUNCEMENT_START == msg) {
		uint8_t intf;
		/*
		 * Radar is seen on the current operating channel
		 * send CSA IE for all associated stations
		 * Request for CSA IE transmission
		 */
		for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
			ptSapContext temp_sap_ctx;
			if (((QDF_SAP_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona) ||
			    (QDF_P2P_GO_MODE ==
				mac_ctx->sap.sapCtxList[intf].sapPersona)) &&
			    mac_ctx->sap.sapCtxList[intf].pSapContext != NULL) {
				temp_sap_ctx =
				    mac_ctx->sap.sapCtxList[intf].pSapContext;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_MED,
					  FL("sapdfs: Sending CSAIE for sapctx[%p]"),
					  temp_sap_ctx);

				qdf_status = wlansap_dfs_send_csa_ie_request(
						(void *) temp_sap_ctx);
			}
		}
	} else if (eSAP_CHANNEL_SWITCH_ANNOUNCEMENT_START == msg) {
		enum tQDF_ADAPTER_MODE persona;

		if (!sap_ctx) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
					FL("Invalid sap_ctx"));
			return qdf_status;
		}

		persona = mac_ctx->sap.sapCtxList[sap_ctx->sessionId].
								sapPersona;

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
				FL("app trigger chan switch: mode:%d vdev:%d"),
				persona, sap_ctx->sessionId);

		if ((QDF_SAP_MODE == persona) || (QDF_P2P_GO_MODE == persona))
			qdf_status = wlansap_dfs_send_csa_ie_request(
					(void *) sap_ctx);
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_STARTED", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm_state_disconnecting() - utility function called from sap fsm
 * @sap_ctx: SAP context
 * @sap_event: SAP event buffer
 * @mac_ctx: global MAC context
 *
 * This function is called for state transition from "eSAP_DISCONNECTING"
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_fsm_state_disconnecting(ptSapContext sap_ctx,
			ptWLAN_SAPEvent sap_event, tpAniSirGlobal mac_ctx,
			tHalHandle hal)
{
	uint32_t msg = sap_event->event;
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;

	if (msg == eSAP_MAC_READY_FOR_CONNECTIONS) {
		/*
		 * Transition from eSAP_DISCONNECTING to eSAP_DISCONNECTED
		 * (both without substates)
		 */
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  FL("from state %s => %s"),
			  "eSAP_DISCONNECTING", "eSAP_DISCONNECTED");
		sap_ctx->sapsMachine = eSAP_DISCONNECTED;

		/* Close the SME session */
		if (eSAP_TRUE == sap_ctx->isSapSessionOpen) {
			sap_ctx->isSapSessionOpen = eSAP_FALSE;
			qdf_status = sap_close_session(hal, sap_ctx,
					sap_roam_session_close_callback, true);
			if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
				qdf_status = sap_signal_hdd_event(sap_ctx, NULL,
						eSAP_STOP_BSS_EVENT,
						(void *)eSAP_STATUS_SUCCESS);
			}
		}
	} else if (msg == eWNI_SME_CHANNEL_CHANGE_REQ) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			  FL("sapdfs: Send channel change request on sapctx[%p]"),
			  sap_ctx);
		/*
		 * Most likely, radar has been detected and SAP wants to
		 * change the channel
		 */
		qdf_status = wlansap_channel_change_request((void *) sap_ctx,
				mac_ctx->sap.SapDfsInfo.target_channel);

		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("Sending DFS eWNI_SME_CHANNEL_CHANGE_REQ"));
	} else {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("in state %s, invalid event msg %d"),
			  "eSAP_DISCONNECTING", msg);
	}

	return qdf_status;
}

/**
 * sap_fsm() - SAP statem machine entry function
 * @sap_ctx: SAP context
 * @sap_event: SAP event
 *
 * SAP statem machine entry function
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sap_fsm(ptSapContext sap_ctx, ptWLAN_SAPEvent sap_event)
{
	/*
	 * Retrieve the phy link state machine structure
	 * from the sap_ctx value
	 * state var that keeps track of state machine
	 */
	eSapFsmStates_t state_var = sap_ctx->sapsMachine;
	uint32_t msg = sap_event->event; /* State machine input event message */
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	tHalHandle hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
	tpAniSirGlobal mac_ctx;

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid hal"));
		return QDF_STATUS_E_FAILURE;
	}

	mac_ctx = PMAC_STRUCT(hal);
	if (NULL == mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Invalid MAC context"));
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
		  FL("sap_ctx=%p, state_var=%d, msg=0x%x"),
		  sap_ctx, state_var, msg);

	switch (state_var) {
	case eSAP_DISCONNECTED:
		qdf_status = sap_fsm_state_disconnected(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_CH_SELECT:
		qdf_status = sap_fsm_state_ch_select(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_DFS_CAC_WAIT:
		qdf_status = sap_fsm_state_dfs_cac_wait(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_STARTING:
		qdf_status = sap_fsm_state_starting(sap_ctx, sap_event,
				mac_ctx, hal);
		break;

	case eSAP_STARTED:
		qdf_status = sap_fsm_state_started(sap_ctx, sap_event,
				mac_ctx);
		break;

	case eSAP_DISCONNECTING:
		qdf_status = sap_fsm_state_disconnecting(sap_ctx, sap_event,
				mac_ctx, hal);
		break;
	}
	return qdf_status;
}

eSapStatus
sapconvert_to_csr_profile(tsap_Config_t *pconfig_params, eCsrRoamBssType bssType,
			  tCsrRoamProfile *profile)
{
	/* Create Roam profile for SoftAP to connect */
	profile->BSSType = eCSR_BSS_TYPE_INFRA_AP;
	profile->SSIDs.numOfSSIDs = 1;
	profile->csrPersona = pconfig_params->persona;
	profile->disableDFSChSwitch = pconfig_params->disableDFSChSwitch;

	qdf_mem_zero(profile->SSIDs.SSIDList[0].SSID.ssId,
		     sizeof(profile->SSIDs.SSIDList[0].SSID.ssId));

	/* Flag to not broadcast the SSID information */
	profile->SSIDs.SSIDList[0].ssidHidden =
		pconfig_params->SSIDinfo.ssidHidden;

	profile->SSIDs.SSIDList[0].SSID.length =
		pconfig_params->SSIDinfo.ssid.length;
	qdf_mem_copy(&profile->SSIDs.SSIDList[0].SSID.ssId,
		     pconfig_params->SSIDinfo.ssid.ssId,
		     sizeof(pconfig_params->SSIDinfo.ssid.ssId));

	profile->negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	if (pconfig_params->authType == eSAP_OPEN_SYSTEM) {
		profile->negotiatedAuthType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
	} else if (pconfig_params->authType == eSAP_SHARED_KEY) {
		profile->negotiatedAuthType = eCSR_AUTH_TYPE_SHARED_KEY;
	} else {
		profile->negotiatedAuthType = eCSR_AUTH_TYPE_AUTOSWITCH;
	}

	profile->AuthType.numEntries = 1;
	profile->AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	/* Always set the Encryption Type */
	profile->EncryptionType.numEntries = 1;
	profile->EncryptionType.encryptionType[0] =
		pconfig_params->RSNEncryptType;

	profile->mcEncryptionType.numEntries = 1;
	profile->mcEncryptionType.encryptionType[0] =
		pconfig_params->mcRSNEncryptType;

	if (pconfig_params->privacy & eSAP_SHARED_KEY) {
		profile->AuthType.authType[0] = eCSR_AUTH_TYPE_SHARED_KEY;
	}

	profile->privacy = pconfig_params->privacy;
	profile->fwdWPSPBCProbeReq = pconfig_params->fwdWPSPBCProbeReq;

	if (pconfig_params->authType == eSAP_SHARED_KEY) {
		profile->csr80211AuthType = eSIR_SHARED_KEY;
	} else if (pconfig_params->authType == eSAP_OPEN_SYSTEM) {
		profile->csr80211AuthType = eSIR_OPEN_SYSTEM;
	} else {
		profile->csr80211AuthType = eSIR_AUTO_SWITCH;
	}

	/* Initialize we are not going to use it */
	profile->pWPAReqIE = NULL;
	profile->nWPAReqIELength = 0;

	/* set the RSN/WPA IE */
	profile->pRSNReqIE = NULL;
	profile->nRSNReqIELength = pconfig_params->RSNWPAReqIELength;
	if (pconfig_params->RSNWPAReqIELength) {
		profile->pRSNReqIE =
			qdf_mem_malloc(pconfig_params->RSNWPAReqIELength);
		if (NULL == profile->pRSNReqIE) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
				  " %s Fail to alloc memory", __func__);
			return eSAP_STATUS_FAILURE;
		}
		qdf_mem_copy(profile->pRSNReqIE, pconfig_params->RSNWPAReqIE,
			     pconfig_params->RSNWPAReqIELength);
		profile->nRSNReqIELength = pconfig_params->RSNWPAReqIELength;
	}
	/* Turn off CB mode */
	profile->CBMode = eCSR_CB_OFF;

	/* set the phyMode to accept anything */
	/* Best means everything because it covers all the things we support */
	/* eCSR_DOT11_MODE_BEST */
	profile->phyMode = pconfig_params->SapHw_mode;

	/* Configure beaconInterval */
	profile->beaconInterval = (uint16_t) pconfig_params->beacon_int;

	/* set DTIM period */
	profile->dtimPeriod = pconfig_params->dtim_period;

	/* set Uapsd enable bit */
	profile->ApUapsdEnable = pconfig_params->UapsdEnable;

	/* Enable protection parameters */
	profile->protEnabled = pconfig_params->protEnabled;
	profile->obssProtEnabled = pconfig_params->obssProtEnabled;
	profile->cfg_protection = pconfig_params->ht_capab;

	/* country code */
	if (pconfig_params->countryCode[0])
		qdf_mem_copy(profile->countryCode, pconfig_params->countryCode,
			     WNI_CFG_COUNTRY_CODE_LEN);
	profile->ieee80211d = pconfig_params->ieee80211d;
	/* wps config info */
	profile->wps_state = pconfig_params->wps_state;

#ifdef WLAN_FEATURE_11W
	/* MFP capable/required */
	profile->MFPCapable = pconfig_params->mfpCapable ? 1 : 0;
	profile->MFPRequired = pconfig_params->mfpRequired ? 1 : 0;
#endif

	if (pconfig_params->probeRespIEsBufferLen > 0 &&
	    pconfig_params->pProbeRespIEsBuffer != NULL) {
		profile->addIeParams.probeRespDataLen =
			pconfig_params->probeRespIEsBufferLen;
		profile->addIeParams.probeRespData_buff =
			pconfig_params->pProbeRespIEsBuffer;
	} else {
		profile->addIeParams.probeRespDataLen = 0;
		profile->addIeParams.probeRespData_buff = NULL;
	}
	/*assoc resp IE */
	if (pconfig_params->assocRespIEsLen > 0 &&
	    pconfig_params->pAssocRespIEsBuffer != NULL) {
		profile->addIeParams.assocRespDataLen =
			pconfig_params->assocRespIEsLen;
		profile->addIeParams.assocRespData_buff =
			pconfig_params->pAssocRespIEsBuffer;
	} else {
		profile->addIeParams.assocRespDataLen = 0;
		profile->addIeParams.assocRespData_buff = NULL;
	}

	if (pconfig_params->probeRespBcnIEsLen > 0 &&
	    pconfig_params->pProbeRespBcnIEsBuffer != NULL) {
		profile->addIeParams.probeRespBCNDataLen =
			pconfig_params->probeRespBcnIEsLen;
		profile->addIeParams.probeRespBCNData_buff =
			pconfig_params->pProbeRespBcnIEsBuffer;
	} else {
		profile->addIeParams.probeRespBCNDataLen = 0;
		profile->addIeParams.probeRespBCNData_buff = NULL;
	}
	profile->sap_dot11mc = pconfig_params->sap_dot11mc;

	return eSAP_STATUS_SUCCESS;     /* Success. */
}

void sap_free_roam_profile(tCsrRoamProfile *profile)
{
	if (profile->pRSNReqIE) {
		qdf_mem_free(profile->pRSNReqIE);
		profile->pRSNReqIE = NULL;
	}
}

void sap_sort_mac_list(struct qdf_mac_addr *macList, uint8_t size)
{
	uint8_t outer, inner;
	struct qdf_mac_addr temp;
	int32_t nRes = -1;

	if ((NULL == macList) || (size > MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("either buffer is NULL or size = %d is more"), size);
		return;
	}

	for (outer = 0; outer < size; outer++) {
		for (inner = 0; inner < size - 1; inner++) {
			nRes =
				qdf_mem_cmp((macList + inner)->bytes,
						 (macList + inner + 1)->bytes,
						 QDF_MAC_ADDR_SIZE);
			if (nRes > 0) {
				qdf_mem_copy(temp.bytes,
					     (macList + inner + 1)->bytes,
					     QDF_MAC_ADDR_SIZE);
				qdf_mem_copy((macList + inner + 1)->bytes,
					     (macList + inner)->bytes,
					     QDF_MAC_ADDR_SIZE);
				qdf_mem_copy((macList + inner)->bytes,
					     temp.bytes, QDF_MAC_ADDR_SIZE);
			}
		}
	}
}

eSapBool
sap_search_mac_list(struct qdf_mac_addr *macList,
		    uint8_t num_mac, uint8_t *peerMac,
		    uint8_t *index)
{
	int32_t nRes = -1;
	int8_t nStart = 0, nEnd, nMiddle;
	nEnd = num_mac - 1;

	if ((NULL == macList) || (num_mac > MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		    FL("either buffer is NULL or size = %d is more."), num_mac);
		return eSAP_FALSE;
	}

	while (nStart <= nEnd) {
		nMiddle = (nStart + nEnd) / 2;
		nRes =
			qdf_mem_cmp(&macList[nMiddle], peerMac,
					 QDF_MAC_ADDR_SIZE);

		if (0 == nRes) {
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
				  "search SUCC");
			/* "index equals NULL" means the caller does not need the */
			/* index value of the peerMac being searched */
			if (index != NULL) {
				*index = (uint8_t) nMiddle;
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_HIGH, "index %d",
					  *index);
			}
			return eSAP_TRUE;
		}
		if (nRes < 0)
			nStart = nMiddle + 1;
		else
			nEnd = nMiddle - 1;
	}

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "search not succ");
	return eSAP_FALSE;
}

void sap_add_mac_to_acl(struct qdf_mac_addr *macList,
			uint8_t *size, uint8_t *peerMac)
{
	int32_t nRes = -1;
	int i;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "add acl entered");

	if (NULL == macList || *size == 0 || *size > MAX_ACL_MAC_ADDRESS) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("either buffer is NULL or size = %d is incorrect."),
			*size);
		return;
	}

	for (i = ((*size) - 1); i >= 0; i--) {
		nRes =
			qdf_mem_cmp(&macList[i], peerMac, QDF_MAC_ADDR_SIZE);
		if (nRes > 0) {
			/* Move alphabetically greater mac addresses one index down to allow for insertion
			   of new mac in sorted order */
			qdf_mem_copy((macList + i + 1)->bytes,
				     (macList + i)->bytes, QDF_MAC_ADDR_SIZE);
		} else {
			break;
		}
	}
	/* This should also take care of if the element is the first to be added in the list */
	qdf_mem_copy((macList + i + 1)->bytes, peerMac, QDF_MAC_ADDR_SIZE);
	/* increment the list size */
	(*size)++;
}

void sap_remove_mac_from_acl(struct qdf_mac_addr *macList,
			     uint8_t *size, uint8_t index)
{
	int i;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "remove acl entered");
	/*
	 * Return if the list passed is empty. Ideally this should never happen
	 * since this funcn is always called after sap_search_mac_list to get
	 * the index of the mac addr to be removed and this will only get
	 * called if the search is successful. Still no harm in having the check
	 */
	if ((macList == NULL) || (*size == 0) ||
					(*size > MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("either buffer is NULL or size %d is incorrect."),
			*size);
		return;
	}
	for (i = index; i < ((*size) - 1); i++) {
		/* Move mac addresses starting from "index" passed one index up to delete the void
		   created by deletion of a mac address in ACL */
		qdf_mem_copy((macList + i)->bytes, (macList + i + 1)->bytes,
			     QDF_MAC_ADDR_SIZE);
	}
	/* The last space should be made empty since all mac addesses moved one step up */
	qdf_mem_zero((macList + (*size) - 1)->bytes, QDF_MAC_ADDR_SIZE);
	/* reduce the list size by 1 */
	(*size)--;
}

void sap_print_acl(struct qdf_mac_addr *macList, uint8_t size)
{
	int i;
	uint8_t *macArray;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "print acl entered");

	if ((NULL == macList) || (size == 0) || (size >= MAX_ACL_MAC_ADDRESS)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, either buffer is NULL or size %d is incorrect.",
			  __func__, size);
		return;
	}

	for (i = 0; i < size; i++) {
		macArray = (macList + i)->bytes;
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "** ACL entry %i - " MAC_ADDRESS_STR, i,
			  MAC_ADDR_ARRAY(macArray));
	}
	return;
}

QDF_STATUS sap_is_peer_mac_allowed(ptSapContext sapContext, uint8_t *peerMac)
{
	if (eSAP_ALLOW_ALL == sapContext->eSapMacAddrAclMode)
		return QDF_STATUS_SUCCESS;

	if (sap_search_mac_list
		    (sapContext->acceptMacList, sapContext->nAcceptMac, peerMac, NULL))
		return QDF_STATUS_SUCCESS;

	if (sap_search_mac_list
		    (sapContext->denyMacList, sapContext->nDenyMac, peerMac, NULL)) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Peer " MAC_ADDRESS_STR " in deny list",
			  __func__, MAC_ADDR_ARRAY(peerMac));
		return QDF_STATUS_E_FAILURE;
	}
	/* A new station CAN associate, unless in deny list. Less stringent mode */
	if (eSAP_ACCEPT_UNLESS_DENIED == sapContext->eSapMacAddrAclMode)
		return QDF_STATUS_SUCCESS;

	/* A new station CANNOT associate, unless in accept list. More stringent mode */
	if (eSAP_DENY_UNLESS_ACCEPTED == sapContext->eSapMacAddrAclMode) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Peer " MAC_ADDRESS_STR
			  " denied, Mac filter mode is eSAP_DENY_UNLESS_ACCEPTED",
			  __func__, MAC_ADDR_ARRAY(peerMac));
		return QDF_STATUS_E_FAILURE;
	}

	/* The new STA is neither in accept list nor in deny list. In this case, deny the association
	 * but send a wifi event notification indicating the mac address being denied
	 */
	if (eSAP_SUPPORT_ACCEPT_AND_DENY == sapContext->eSapMacAddrAclMode) {
		sap_signal_hdd_event(sapContext, NULL, eSAP_UNKNOWN_STA_JOIN,
				     (void *) peerMac);
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "In %s, Peer " MAC_ADDRESS_STR
			  " denied, Mac filter mode is eSAP_SUPPORT_ACCEPT_AND_DENY",
			  __func__, MAC_ADDR_ARRAY(peerMac));
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

#ifdef SOFTAP_CHANNEL_RANGE
/**
 * sap_get_channel_list() - get the list of channels
 * @sap_ctx: sap context
 * @ch_list: pointer to channel list array
 * @num_ch: pointer to number of channels.
 *
 * This function populates the list of channels for scanning.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sap_get_channel_list(ptSapContext sap_ctx,
				       uint8_t **ch_list,
				       uint8_t *num_ch)
{
	uint8_t loop_count;
	uint8_t *list;
	uint8_t ch_count;
	uint8_t start_ch_num, band_start_ch;
	uint8_t end_ch_num, band_end_ch;
	uint32_t en_lte_coex;
	tHalHandle hal = CDS_GET_HAL_CB(sap_ctx->p_cds_gctx);
#ifdef FEATURE_WLAN_CH_AVOID
	uint8_t i;
#endif
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (NULL == hal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			FL("Invalid HAL pointer from p_cds_gctx"));
		*num_ch = 0;
		*ch_list = NULL;
		return QDF_STATUS_E_FAULT;
	}

	start_ch_num = sap_ctx->acs_cfg->start_ch;
	end_ch_num = sap_ctx->acs_cfg->end_ch;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			FL("startChannel %d, EndChannel %d, HW:%d"),
			start_ch_num, end_ch_num,
			sap_ctx->acs_cfg->hw_mode);

	wlansap_extend_to_acs_range(&start_ch_num, &end_ch_num,
					    &band_start_ch, &band_end_ch);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("expanded startChannel %d,EndChannel %d"),
			  start_ch_num, end_ch_num);

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			  FL("band_start_ch %d, band_end_ch %d"),
			  band_start_ch, band_end_ch);

	sme_cfg_get_int(hal, WNI_CFG_ENABLE_LTE_COEX, &en_lte_coex);

	/* Check if LTE coex is enabled and 2.4GHz is selected */
	if (en_lte_coex && (band_start_ch == CHAN_ENUM_1) &&
	    (band_end_ch == CHAN_ENUM_14)) {
		/* Set 2.4GHz upper limit to channel 9 for LTE COEX */
		band_end_ch = CHAN_ENUM_9;
	}

	/* Allocate the max number of channel supported */
	list = (uint8_t *) qdf_mem_malloc(NUM_5GHZ_CHANNELS +
						NUM_24GHZ_CHANNELS);
	if (NULL == list) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  FL("Unable to allocate channel list"));
		*num_ch = 0;
		*ch_list = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	/* Search for the Active channels in the given range */
	ch_count = 0;
	for (loop_count = band_start_ch; loop_count <= band_end_ch;
	     loop_count++) {
		/* go to next channel if rf_channel is out of range */
		if ((start_ch_num > CDS_CHANNEL_NUM(loop_count)) ||
		    (end_ch_num < CDS_CHANNEL_NUM(loop_count)))
			continue;
		/*
		 * go to next channel if none of these condition pass
		 * - DFS scan enabled and chan not in CHANNEL_STATE_DISABLE
		 * - DFS scan disable but chan in CHANNEL_STATE_ENABLE
		 */
		if (!(((eSAP_TRUE == mac_ctx->scan.fEnableDFSChnlScan) &&
		      CDS_CHANNEL_STATE(loop_count)) ||
		    ((eSAP_FALSE == mac_ctx->scan.fEnableDFSChnlScan) &&
		     (CHANNEL_STATE_ENABLE ==
		      CDS_CHANNEL_STATE(loop_count)))))
			continue;

#ifdef FEATURE_WLAN_CH_AVOID
		for (i = 0; i < NUM_CHANNELS; i++) {
			if ((safe_channels[i].channelNumber ==
			     CDS_CHANNEL_NUM(loop_count))) {
				/* Check if channel is safe */
				if (true == safe_channels[i].isSafe) {
#endif
#ifdef FEATURE_WLAN_AP_AP_ACS_OPTIMIZE
		uint8_t ch;
		ch = CDS_CHANNEL_NUM(loop_count);
		if ((sap_ctx->acs_cfg->skip_scan_status ==
			eSAP_DO_PAR_ACS_SCAN)) {
		    if ((ch >= sap_ctx->acs_cfg->skip_scan_range1_stch &&
			 ch <= sap_ctx->acs_cfg->skip_scan_range1_endch) ||
			(ch >= sap_ctx->acs_cfg->skip_scan_range2_stch &&
			 ch <= sap_ctx->acs_cfg->skip_scan_range2_endch)) {
			list[ch_count] =
				CDS_CHANNEL_NUM(loop_count);
			ch_count++;
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO,
				FL("%d %d added to ACS ch range"),
				ch_count, ch);
		    } else {
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO_HIGH,
				FL("%d %d skipped from ACS ch range"),
				ch_count, ch);
		    }
		} else {
			list[ch_count] =
				CDS_CHANNEL_NUM(loop_count);
			ch_count++;
			QDF_TRACE(QDF_MODULE_ID_SAP,
				QDF_TRACE_LEVEL_INFO,
				FL("%d %d added to ACS ch range"),
				ch_count, ch);
		}
#else
		list[ch_count] = CDS_CHANNEL_NUM(loop_count);
		ch_count++;
#endif
#ifdef FEATURE_WLAN_CH_AVOID
				}
				break;
			}
		}
#endif
	}
	if (0 == ch_count) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
		    FL("No active channels present for the current region"));
		/*
		 * LTE COEX: channel range outside the restricted 2.4GHz
		 * band limits
		 */
		if (en_lte_coex && (start_ch_num > band_end_ch))
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_FATAL,
				FL("SAP can't be started as due to LTE COEX"));
	}

	/* return the channel list and number of channels to scan */
	*num_ch = ch_count;
	if (ch_count != 0) {
		*ch_list = list;
	} else {
		*ch_list = NULL;
		qdf_mem_free(list);
	}

	for (loop_count = 0; loop_count < ch_count; loop_count++) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_DEBUG,
			FL("channel number: %d"), list[loop_count]);
	}
	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * Function for initializing list of  2.4/5 Ghz [NON-DFS/DFS]
 * available channels in the current regulatory domain.
 */
static QDF_STATUS sap_get_5ghz_channel_list(ptSapContext sapContext)
{
	uint8_t count = 0;
	int i;
	struct sir_pcl_list pcl;
	QDF_STATUS status;
	enum channel_state ch_state;
	pcl.pcl_len = 0;
	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid sapContext pointer on sap_get_channel_list");
		return QDF_STATUS_E_FAULT;
	}

	if (sapContext->SapAllChnlList.channelList) {
		qdf_mem_free(sapContext->SapAllChnlList.channelList);
		sapContext->SapAllChnlList.channelList = NULL;
	}

	sapContext->SapAllChnlList.channelList =
		(tChannelInfo *) qdf_mem_malloc(WNI_CFG_VALID_CHANNEL_LIST_LEN *
						sizeof(tChannelInfo));
	if (NULL == sapContext->SapAllChnlList.channelList) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  " Memory Allocation failed sap_get_channel_list");
		return QDF_STATUS_E_NOMEM;
	}

	status = cds_get_pcl_for_existing_conn(CDS_SAP_MODE,
			pcl.pcl_list, &pcl.pcl_len,
			pcl.weight_list, QDF_ARRAY_SIZE(pcl.weight_list));
	if (status != QDF_STATUS_SUCCESS) {
		cds_err("Get PCL failed");
		return status;
	}
	for (i = 0; i <= pcl.pcl_len; i++) {
		if (CDS_IS_CHANNEL_5GHZ(pcl.pcl_list[i])) {
			ch_state = cds_get_channel_state(pcl.pcl_list[i]);
			if (!(ch_state == CHANNEL_STATE_ENABLE ||
				ch_state == CHANNEL_STATE_DFS))
				continue;
			sapContext->SapAllChnlList.channelList[count].channel =
				pcl.pcl_list[i];
			QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
				  "%s[%d] CHANNEL = %d", __func__, __LINE__,
				  pcl.pcl_list[i]);
			sapContext->SapAllChnlList.channelList[count].valid =
				true;
			count++;
		}
	}

	sapContext->SapAllChnlList.numChannel = count;
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  "%s[%d] NUMBER OF CHANNELS count = %d"
		  "sapContext->SapAllChnlList.numChannel = %d",
		  __func__, __LINE__, count,
		  sapContext->SapAllChnlList.numChannel);
	return QDF_STATUS_SUCCESS;
}

/*
 * This function randomly selects the channel to switch after the detection
 * of radar
 * param sapContext - sap context
 * dfs_event - Dfs information from DFS
 * return - channel to which AP wishes to switch
 */
uint8_t sap_indicate_radar(ptSapContext sapContext,
			   tSirSmeDfsEventInd *dfs_event)
{
	uint8_t target_channel = 0;
	tHalHandle hHal;
	tpAniSirGlobal pMac;

	if (NULL == sapContext || NULL == dfs_event) {
		/* Invalid sap context of dfs event passed */
		return 0;
	}
	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return 0;
	}
	pMac = PMAC_STRUCT(hHal);

	if (!dfs_event->dfs_radar_status) {
		/*dfs status does not indicate a radar on the channel-- False Alarm */
		return 0;
	}

	/*
	 * SAP needs to generate Channel Switch IE
	 * if the radar is found in the STARTED state
	 */
	if (eSAP_STARTED == sapContext->sapsMachine)
		pMac->sap.SapDfsInfo.csaIERequired = eSAP_TRUE;

	if (sapContext->csr_roamProfile.disableDFSChSwitch) {
		return sapContext->channel;
	}

	/* set the Radar Found flag in SapDfsInfo */
	pMac->sap.SapDfsInfo.sap_radar_found_status = true;

	if (dfs_event->chan_list.nchannels > SIR_DFS_MAX_20M_SUB_CH) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
			  FL("nchannels >SIR_DFS_MAX_20M_SUB_CH so resetting"));
		dfs_event->chan_list.nchannels = SIR_DFS_MAX_20M_SUB_CH;
	}

	sap_mark_dfs_channels(sapContext, dfs_event->chan_list.channels,
			      dfs_event->chan_list.nchannels,
			      cds_get_monotonic_boottime());

	if (sapContext->chan_before_pre_cac) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO,
			FL("sapdfs: set chan before pre cac %d as target chan"),
			sapContext->chan_before_pre_cac);
		return sapContext->chan_before_pre_cac;
	}

	/*
	 * (1) skip static turbo channel as it will require STA to be in
	 * static turbo to work.
	 * (2) skip channel which's marked with radar detction
	 * (3) WAR: we allow user to config not to use any DFS channel
	 * (4) When we pick a channel, skip excluded 11D channels
	 * (5) Create the available channel list with the above rules
	 */

	target_channel = sap_random_channel_sel(sapContext);
	if (0 == target_channel) {
		sap_signal_hdd_event(sapContext, NULL,
				     eSAP_DFS_NO_AVAILABLE_CHANNEL,
				     (void *) eSAP_STATUS_SUCCESS);
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_WARN,
		  FL("sapdfs: New selected target channel is [%d]"),
		  target_channel);
	return target_channel;
}

/*
 * CAC timer callback function.
 * Post eSAP_DFS_CHANNEL_CAC_END event to sap_fsm().
 */
void sap_dfs_cac_timer_callback(void *data)
{
	ptSapContext sapContext;
	tWLAN_SAPEvent sapEvent;
	tHalHandle hHal = (tHalHandle) data;
	tpAniSirGlobal pMac;

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return;
	}
	pMac = PMAC_STRUCT(hHal);
	sapContext = sap_find_cac_wait_session(hHal);
	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			"%s: no SAP contexts in wait state", __func__);
		return;
	}

	/*
	 * SAP may not be in CAC wait state, when the timer runs out.
	 * if following flag is set, then timer is in initialized state,
	 * destroy timer here.
	 */
	if (pMac->sap.SapDfsInfo.is_dfs_cac_timer_running == true) {
		qdf_mc_timer_destroy(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);
		pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = false;
	}

	/*
	 * CAC Complete, post eSAP_DFS_CHANNEL_CAC_END to sap_fsm
	 */
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
			"sapdfs: Sending eSAP_DFS_CHANNEL_CAC_END for target_channel = %d on sapctx[%p]",
			sapContext->channel, sapContext);

	sapEvent.event = eSAP_DFS_CHANNEL_CAC_END;
	sapEvent.params = 0;
	sapEvent.u1 = 0;
	sapEvent.u2 = 0;

	sap_fsm(sapContext, &sapEvent);
}

/*
 * Function to stop the DFS CAC Timer
 */
static int sap_stop_dfs_cac_timer(ptSapContext sapContext)
{
	tHalHandle hHal;
	tpAniSirGlobal pMac;
	if (sapContext == NULL)
		return 0;

	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);
	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return 0;
	}
	pMac = PMAC_STRUCT(hHal);

	if (QDF_TIMER_STATE_RUNNING !=
	    qdf_mc_timer_get_current_state(&pMac->sap.SapDfsInfo.
					   sap_dfs_cac_timer)) {
		return 0;
	}

	qdf_mc_timer_stop(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);
	pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = 0;
	qdf_mc_timer_destroy(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);

	return 0;
}


/**
 * sap_is_channel_bonding_etsi_weather_channel() - check weather chan bonding.
 * @sap_context:                                   SAP context
 *
 * Check if the current SAP operating channel is bonded to weather radar
 * channel in ETSI domain.
 *
 * Return: True if bonded to weather channel in ETSI
 */
static bool
sap_is_channel_bonding_etsi_weather_channel(ptSapContext sap_context)
{
	if (IS_CH_BONDING_WITH_WEATHER_CH(sap_context->channel) &&
	  (sap_context->ch_params.ch_width != CH_WIDTH_20MHZ))
		return true;

	return false;
}

/*
 * Function to start the DFS CAC Timer
 * when SAP is started on a DFS channel
 */
int sap_start_dfs_cac_timer(ptSapContext sapContext)
{
	QDF_STATUS status;
	uint32_t cacTimeOut;
	tHalHandle hHal = NULL;
	tpAniSirGlobal pMac = NULL;
	enum dfs_region dfs_region;

	if (sapContext == NULL) {
		return 0;
	}
	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return 0;
	}
	pMac = PMAC_STRUCT(hHal);

	if (pMac->sap.SapDfsInfo.ignore_cac) {
		/*
		 * If User has set to ignore the CAC
		 * so, continue without CAC Timer.
		 */
		return 2;
	}
	cacTimeOut = DEFAULT_CAC_TIMEOUT;

	cds_get_dfs_region(&dfs_region);

	if ((dfs_region == DFS_ETSI_REGION)
	    && ((IS_ETSI_WEATHER_CH(sapContext->channel)) ||
		(sap_is_channel_bonding_etsi_weather_channel(sapContext)))) {
		cacTimeOut = ETSI_WEATHER_CH_CAC_TIMEOUT;
	}
	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_MED,
		  "sapdfs: SAP_DFS_CHANNEL_CAC_START on CH - %d, CAC TIMEOUT - %d sec",
		  sapContext->channel, cacTimeOut / 1000);

	qdf_mc_timer_init(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer,
			  QDF_TIMER_TYPE_SW,
			  sap_dfs_cac_timer_callback, (void *) hHal);

	/*Start the CAC timer */
	status =
		qdf_mc_timer_start(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer,
				   cacTimeOut);
	if (status == QDF_STATUS_SUCCESS) {
		pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = true;
		return 1;
	} else {
		pMac->sap.SapDfsInfo.is_dfs_cac_timer_running = false;
		qdf_mc_timer_destroy(&pMac->sap.SapDfsInfo.sap_dfs_cac_timer);
		return 0;
	}
}

/*
 * This function initializes the NOL list
 * parameters required to track the radar
 * found DFS channels in the current Reg. Domain .
 */
QDF_STATUS sap_init_dfs_channel_nol_list(ptSapContext sapContext)
{
	uint8_t count = 0;
	int i;
	bool bFound = false;
	tHalHandle hHal;
	tpAniSirGlobal pMac;

	if (NULL == sapContext) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "Invalid sapContext pointer on sap_init_dfs_channel_nol_list");
		return QDF_STATUS_E_FAULT;
	}
	hHal = CDS_GET_HAL_CB(sapContext->p_cds_gctx);

	if (NULL == hHal) {
		QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_ERROR,
			  "In %s invalid hHal", __func__);
		return QDF_STATUS_E_FAULT;
	}
	pMac = PMAC_STRUCT(hHal);

	/* to indicate hdd to get cnss dfs nol */
	if (QDF_STATUS_SUCCESS == sap_signal_hdd_event(sapContext, NULL,
						       eSAP_DFS_NOL_GET,
						       (void *)
						       eSAP_STATUS_SUCCESS)) {
		bFound = true;
	}

	for (i = CHAN_ENUM_36; i <= CHAN_ENUM_165; i++) {
		if (CDS_CHANNEL_STATE(i) == CHANNEL_STATE_DFS) {
			/* if dfs nol is not found, initialize it */
			if (!bFound) {
				pMac->sap.SapDfsInfo.sapDfsChannelNolList[count]
				.dfs_channel_number =
					CDS_CHANNEL_NUM(i);

				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_INFO_LOW,
					  "%s: CHANNEL = %d", __func__,
					  pMac->sap.SapDfsInfo.
					  sapDfsChannelNolList[count].
					  dfs_channel_number);

				pMac->sap.SapDfsInfo.sapDfsChannelNolList[count]
				.radar_status_flag =
					eSAP_DFS_CHANNEL_USABLE;
				pMac->sap.SapDfsInfo.sapDfsChannelNolList[count]
				.radar_found_timestamp = 0;
			}
			count++;
		}
	}

	pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels = count;

	QDF_TRACE(QDF_MODULE_ID_SAP, QDF_TRACE_LEVEL_INFO_LOW,
		  "%s[%d] NUMBER OF DFS CHANNELS = %d",
		  __func__, __LINE__,
		  pMac->sap.SapDfsInfo.numCurrentRegDomainDfsChannels);

	return QDF_STATUS_SUCCESS;
}

/*
 * This function will calculate how many interfaces
 * have sap persona and returns total number of sap persona.
 */
uint8_t sap_get_total_number_sap_intf(tHalHandle hHal)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	uint8_t intf = 0;
	uint8_t intf_count = 0;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			intf_count++;
		}
	}
	return intf_count;
}

/*
 * This function will find the concurrent sap context apart from
 * passed sap context and return its channel change ready status
 */
bool is_concurrent_sap_ready_for_channel_change(tHalHandle hHal,
						ptSapContext sapContext)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	ptSapContext pSapContext;
	uint8_t intf = 0;

	for (intf = 0; intf < SAP_MAX_NUM_SESSION; intf++) {
		if (((QDF_SAP_MODE == pMac->sap.sapCtxList[intf].sapPersona)
		    ||
		    (QDF_P2P_GO_MODE == pMac->sap.sapCtxList[intf].sapPersona))
		    && pMac->sap.sapCtxList[intf].pSapContext != NULL) {
			pSapContext =
				(ptSapContext) pMac->sap.sapCtxList[intf].
				pSapContext;
			if (pSapContext == sapContext) {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL("sapCtx matched [%p]"),
					  sapContext);
				continue;
			} else {
				QDF_TRACE(QDF_MODULE_ID_SAP,
					  QDF_TRACE_LEVEL_ERROR,
					  FL
						  ("concurrent sapCtx[%p] didn't matche with [%p]"),
					  pSapContext, sapContext);
				return pSapContext->is_sap_ready_for_chnl_chng;
			}
		}
	}
	return false;
}
