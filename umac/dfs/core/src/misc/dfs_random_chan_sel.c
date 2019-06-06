/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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

#include "../dfs.h"
#include "../dfs_random_chan_sel.h"
#include <qdf_mc_timer.h>
#include <wlan_utility.h>

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
struct dfs_matrix_tx_leak_info ht80_chan[] = {
	{52,
	 {{36, 148}, {40, 199},
	  {44, 193}, {48, 197},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, 153},
	  {60, 137}, {64, 134},
	  {100, 358}, {104, 350},
	  {108, 404}, {112, 344},
	  {116, 424}, {120, 429},
	  {124, 437}, {128, 435},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },


	{56,
	 {{36, 171}, {40, 178},
	  {44, 171}, {48, 178},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, 280},
	  {100, 351}, {104, 376},
	  {108, 362}, {112, 362},
	  {116, 403}, {120, 397},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{60,
	 {{36, 156}, {40, 146},
	  {44, DFS_TX_LEAKAGE_MIN}, {48, DFS_TX_LEAKAGE_MIN},
	  {52, 180}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, 376}, {104, 360},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, DFS_TX_LEAKAGE_MAX},
	  {116, 395}, {120, 399},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{64,
	 {{36, 217}, {40, 221},
	  {44, DFS_TX_LEAKAGE_MIN}, {48, DFS_TX_LEAKAGE_MIN},
	  {52, 176}, {56, 176},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, 384}, {104, 390},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, DFS_TX_LEAKAGE_MAX},
	  {116, 375}, {120, 374},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{100,
	 {{36, 357}, {40, 326},
	  {44, 321}, {48, 326},
	  {52, 378}, {56, 396},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, 196}, {112, 116},
	  {116, 166}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{104,
	 {{36, 325}, {40, 325},
	  {44, 305}, {48, 352},
	  {52, 411}, {56, 411},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, 460},
	  {116, 198}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{108,
	 {{36, 304}, {40, 332},
	  {44, 310}, {48, 335},
	  {52, 431}, {56, 391},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 280}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, 185}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{112,
	 {{36, 327}, {40, 335},
	  {44, 331}, {48, 345},
	  {52, 367}, {56, 401},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 131}, {104, 132},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, 189}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{116,
	 {{36, 384}, {40, 372},
	  {44, 389}, {48, 396},
	  {52, 348}, {56, 336},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 172}, {104, 169},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{120,
	 {{36, 395}, {40, 419},
	  {44, 439}, {48, 407},
	  {52, 321}, {56, 334},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 134}, {104, 186},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, 159},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{124,
	 {{36, 469}, {40, 433},
	  {44, 434}, {48, 435},
	  {52, 332}, {56, 345},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 146}, {104, 177},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, 350}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, 138},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{128,
	 {{36, 408}, {40, 434},
	  {44, 449}, {48, 444},
	  {52, 341}, {56, 374},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 205}, {104, 208},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, 142}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{132,
	 {{36, DFS_TX_LEAKAGE_MAX}, {40, DFS_TX_LEAKAGE_MAX},
	  {44, DFS_TX_LEAKAGE_MAX}, {48, DFS_TX_LEAKAGE_MAX},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{136,
	 {{36, DFS_TX_LEAKAGE_MAX}, {40, DFS_TX_LEAKAGE_MAX},
	  {44, DFS_TX_LEAKAGE_MAX}, {48, DFS_TX_LEAKAGE_MAX},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{140,
	 {{36, DFS_TX_LEAKAGE_MAX}, {40, DFS_TX_LEAKAGE_MAX},
	  {44, DFS_TX_LEAKAGE_MAX}, {48, DFS_TX_LEAKAGE_MAX},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{144,
	 {{36, DFS_TX_LEAKAGE_MAX}, {40, DFS_TX_LEAKAGE_MAX},
	  {44, DFS_TX_LEAKAGE_MAX}, {48, DFS_TX_LEAKAGE_MAX},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },
};

/* channel tx leakage table - ht40 */
struct dfs_matrix_tx_leak_info ht40_chan[] = {
	{52,
	 {{36, DFS_TX_LEAKAGE_AUTO_MIN}, {40, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, 230}, {48, 230},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, DFS_TX_LEAKAGE_AUTO_MIN}, {64, DFS_TX_LEAKAGE_AUTO_MIN},
	  {100, 625}, {104, 323},
	  {108, 646}, {112, 646},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{56,
	 {{36, DFS_TX_LEAKAGE_AUTO_MIN}, {40, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, DFS_TX_LEAKAGE_AUTO_MIN}, {48, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, 611}, {104, 611},
	  {108, 617}, {112, 617},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{60,
	 {{36, DFS_TX_LEAKAGE_AUTO_MIN}, {40, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, DFS_TX_LEAKAGE_AUTO_MIN}, {48, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 190}, {56, 190},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, 608}, {104, 608},
	  {108, 623}, {112, 623},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{64,
	 {{36, DFS_TX_LEAKAGE_AUTO_MIN}, {40, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, DFS_TX_LEAKAGE_AUTO_MIN}, {48, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 295}, {56, 295},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, 594}, {104, 594},
	  {108, 625}, {112, 625},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{100,
	 {{36, 618}, {40, 618},
	  {44, 604}, {48, 604},
	  {52, 596}, {56, 596},
	  {60, 584}, {64, 584},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, 299}, {112, 299},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 538}, {136, 538},
	  {140, 598},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{104,
	 {{36, 636}, {40, 636},
	  {44, 601}, {48, 601},
	  {52, 616}, {56, 616},
	  {60, 584}, {64, 584},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 553}, {136, 553},
	  {140, 568},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{108,
	 {{36, 600}, {40, 600},
	  {44, 627}, {48, 627},
	  {52, 611}, {56, 611},
	  {60, 611}, {64, 611},
	  {100, 214}, {104, 214},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, DFS_TX_LEAKAGE_AUTO_MIN}, {136, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 534},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{112,
	 {{36, 645}, {40, 645},
	  {44, 641}, {48, 641},
	  {52, 618}, {56, 618},
	  {60, 612}, {64, 612},
	  {100, 293}, {104, 293},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, DFS_TX_LEAKAGE_AUTO_MIN}, {136, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 521},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{116,
	 {{36, 661}, {40, 661},
	  {44, 624}, {48, 624},
	  {52, 634}, {56, 634},
	  {60, 611}, {64, 611},
	  {100, DFS_TX_LEAKAGE_AUTO_MIN}, {104, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, 217}, {112, 217},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, DFS_TX_LEAKAGE_AUTO_MIN}, {136, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{120,
	 {{36, 667}, {40, 667},
	  {44, 645}, {48, 645},
	  {52, 633}, {56, 633},
	  {60, 619}, {64, 619},
	  {100, DFS_TX_LEAKAGE_AUTO_MIN}, {104, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, 291}, {112, 291},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_AUTO_MIN}, {136, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{124,
	 {{36, 676}, {40, 676},
	  {44, 668}, {48, 668},
	  {52, 595}, {56, 595},
	  {60, 622}, {64, 622},
	  {100, DFS_TX_LEAKAGE_AUTO_MIN}, {104, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, DFS_TX_LEAKAGE_AUTO_MIN}, {112, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 225}, {120, 225},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_AUTO_MIN}, {136, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{128,
	 {{36, 678}, {40, 678},
	  {44, 664}, {48, 664},
	  {52, 651}, {56, 651},
	  {60, 643}, {64, 643},
	  {100, DFS_TX_LEAKAGE_AUTO_MIN}, {104, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, DFS_TX_LEAKAGE_AUTO_MIN}, {112, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 293}, {120, 293},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{132,
	 {{36, 689}, {40, 689},
	  {44, 669}, {48, 669},
	  {52, 662}, {56, 662},
	  {60, 609}, {64, 609},
	  {100, 538}, {104, 538},
	  {108, DFS_TX_LEAKAGE_AUTO_MIN}, {112, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 247}, {128, 247},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{136,
	 {{36, 703}, {40, 703},
	  {44, 688}, {48, DFS_TX_LEAKAGE_MIN},
	  {52, 671}, {56, 671},
	  {60, 658}, {64, 658},
	  {100, 504}, {104, 504},
	  {108, DFS_TX_LEAKAGE_AUTO_MIN}, {112, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 289}, {128, 289},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{140,
	 {{36, 695}, {40, 695},
	  {44, 684}, {48, 684},
	  {52, 664}, {56, 664},
	  {60, 658}, {64, 658},
	  {100, 601}, {104, 601},
	  {108, 545}, {112, 545},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 262}, {136, 262},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{144,
	 {{36, 695}, {40, 695},
	  {44, 684}, {48, 684},
	  {52, 664}, {56, 664},
	  {60, 658}, {64, 658},
	  {100, 601}, {104, 601},
	  {108, 545}, {112, 545},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, DFS_TX_LEAKAGE_AUTO_MIN}, {128, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 262}, {136, 262},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },
};

/* channel tx leakage table - ht20 */
struct dfs_matrix_tx_leak_info ht20_chan[] = {
	{52,
	 {{36, DFS_TX_LEAKAGE_AUTO_MIN}, {40, 286},
	  {44, 225}, {48, 121},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, 300}, {64, DFS_TX_LEAKAGE_AUTO_MIN},
	  {100, 637}, {104, DFS_TX_LEAKAGE_MAX},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, DFS_TX_LEAKAGE_MAX},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{56,
	 {{36, 468}, {40, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, DFS_TX_LEAKAGE_AUTO_MIN}, {48, 206},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, DFS_TX_LEAKAGE_MAX}, {104, DFS_TX_LEAKAGE_MAX},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, DFS_TX_LEAKAGE_MAX},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{60,
	 {{36, 507}, {40, 440},
	  {44, DFS_TX_LEAKAGE_AUTO_MIN}, {48, 313},
	  {52, DFS_TX_LEAKAGE_MIN}, {56, DFS_TX_LEAKAGE_MIN},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, DFS_TX_LEAKAGE_MAX}, {104, DFS_TX_LEAKAGE_MAX},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, DFS_TX_LEAKAGE_MAX},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{64,
	 {{36, 516}, {40, 520},
	  {44, 506}, {48, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 301}, {56, 258},
	  {60, DFS_TX_LEAKAGE_MIN}, {64, DFS_TX_LEAKAGE_MIN},
	  {100, 620}, {104, 617},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, DFS_TX_LEAKAGE_MAX},
	  {116, DFS_TX_LEAKAGE_MAX}, {120, DFS_TX_LEAKAGE_MAX},
	  {124, DFS_TX_LEAKAGE_MAX}, {128, DFS_TX_LEAKAGE_MAX},
	  {132, DFS_TX_LEAKAGE_MAX}, {136, DFS_TX_LEAKAGE_MAX},
	  {140, DFS_TX_LEAKAGE_MAX},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{100,
	 {{36, 616}, {40, 601},
	  {44, 604}, {48, 589},
	  {52, 612}, {56, 592},
	  {60, 590}, {64, 582},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, 131},
	  {108, DFS_TX_LEAKAGE_AUTO_MIN}, {112, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 522},
	  {124, 571}, {128, 589},
	  {132, 593}, {136, 598},
	  {140, 594},
	  {144, DFS_TX_LEAKAGE_MIN},
	  } },

	{104,
	 {{36, 622}, {40, 624},
	  {44, 618}, {48, 610},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, DFS_TX_LEAKAGE_MIN}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, 463},
	  {116, 483}, {120, 503},
	  {124, 523}, {128, 565},
	  {132, 570}, {136, 588},
	  {140, 585},
	  {144, DFS_TX_LEAKAGE_MIN},
	  } },

	{108,
	 {{36, 620}, {40, 638},
	  {44, 611}, {48, 614},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 477}, {104, DFS_TX_LEAKAGE_MIN},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, 477}, {120, 497},
	  {124, 517}, {128, 537},
	  {132, 557}, {136, 577},
	  {140, 603},
	  {144, DFS_TX_LEAKAGE_MIN},
	  } },

	{112,
	 {{36, 636}, {40, 623},
	  {44, 638}, {48, 628},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, 606},
	  {100, 501}, {104, 481},
	  {108, DFS_TX_LEAKAGE_MIN}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, 481},
	  {124, 501}, {128, 421},
	  {132, 541}, {136, 561},
	  {140, 583},
	  {144, DFS_TX_LEAKAGE_MIN},
	  } },

	{116,
	 {{36, 646}, {40, 648},
	  {44, 633}, {48, 634},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, 615}, {64, 594},
	  {100, 575}, {104, 554},
	  {108, 534}, {112, DFS_TX_LEAKAGE_MIN},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, 534}, {136, 554},
	  {140, 574},
	  {144, DFS_TX_LEAKAGE_MIN},
	  } },

	{120,
	 {{36, 643}, {40, 649},
	  {44, 654}, {48, 629},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, 621},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 565}, {104, 545},
	  {108, 525}, {112, 505},
	  {116, DFS_TX_LEAKAGE_MIN}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, 505},
	  {132, 525}, {136, 545},
	  {140, 565},
	  {144, DFS_TX_LEAKAGE_MIN},
	  } },

	{124,
	 {{36, 638}, {40, 657},
	  {44, 663}, {48, 649},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 581}, {104, 561},
	  {108, 541}, {112, 521},
	  {116, 499}, {120, DFS_TX_LEAKAGE_MIN},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, 499}, {136, 519},
	  {140, 539},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{128,
	 {{36, 651}, {40, 651},
	  {44, 674}, {48, 640},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, 603}, {104, 560},
	  {108, 540}, {112, 520},
	  {116, 499}, {120, 479},
	  {124, DFS_TX_LEAKAGE_MIN}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, 479},
	  {140, 499},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{132,
	 {{36, 643}, {40, 668},
	  {44, 651}, {48, 657},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, DFS_TX_LEAKAGE_MAX}, {104, 602},
	  {108, 578}, {112, 570},
	  {116, 550}, {120, 530},
	  {124, 510}, {128, DFS_TX_LEAKAGE_MIN},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, 490},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{136,
	 {{36, 654}, {40, 667},
	  {44, 666}, {48, 642},
	  {52, DFS_TX_LEAKAGE_MAX}, {56, DFS_TX_LEAKAGE_MAX},
	  {60, DFS_TX_LEAKAGE_MAX}, {64, DFS_TX_LEAKAGE_MAX},
	  {100, DFS_TX_LEAKAGE_MAX}, {104, DFS_TX_LEAKAGE_MAX},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, 596},
	  {116, 555}, {120, 535},
	  {124, 515}, {128, 495},
	  {132, DFS_TX_LEAKAGE_MIN}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{140,
	 {{36, 679}, {40, 673},
	  {44, 667}, {48, 656},
	  {52, 634}, {56, 663},
	  {60, 662}, {64, 660},
	  {100, DFS_TX_LEAKAGE_MAX}, {104, DFS_TX_LEAKAGE_MAX},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, 590},
	  {116, 573}, {120, 553},
	  {124, 533}, {128, 513},
	  {132, 490}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },

	{144,
	 {{36, 679}, {40, 673},
	  {44, 667}, {48, 656},
	  {52, 634}, {56, 663},
	  {60, 662}, {64, 660},
	  {100, DFS_TX_LEAKAGE_MAX}, {104, DFS_TX_LEAKAGE_MAX},
	  {108, DFS_TX_LEAKAGE_MAX}, {112, 590},
	  {116, 573}, {120, 553},
	  {124, 533}, {128, 513},
	  {132, 490}, {136, DFS_TX_LEAKAGE_MIN},
	  {140, DFS_TX_LEAKAGE_MIN},
	  {144, DFS_TX_LEAKAGE_MIN}
	  } },
};

/*
 * dfs_find_target_channel_in_channel_matrix() - finds the leakage matrix
 * @ch_width: target channel width
 * @NOL_channel: the NOL channel whose leakage matrix is required
 * @pTarget_chnl_mtrx: pointer to target channel matrix returned.
 *
 * This function gives the leakage matrix for given NOL channel and ch_width
 *
 * Return: TRUE or FALSE
 */
static bool
dfs_find_target_channel_in_channel_matrix(enum phy_ch_width ch_width,
				uint8_t NOL_channel,
				struct dfs_tx_leak_info **pTarget_chnl_mtrx)
{
	struct dfs_tx_leak_info *target_chan_matrix = NULL;
	struct dfs_matrix_tx_leak_info *pchan_matrix = NULL;
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

	if (!target_chan_matrix) {
		return false;
	} else {
		*pTarget_chnl_mtrx = target_chan_matrix;
		return true;
	}
}

QDF_STATUS
dfs_mark_leaking_ch(struct wlan_dfs *dfs,
		enum phy_ch_width ch_width,
		uint8_t temp_ch_lst_sz,
		uint8_t *temp_ch_lst)
{
	struct dfs_tx_leak_info *target_chan_matrix = NULL;
	uint32_t         num_channel = (CHAN_ENUM_144 - CHAN_ENUM_36) + 1;
	uint32_t         j = 0;
	uint32_t         k = 0;
	uint8_t          dfs_nol_channel;
	struct dfs_nolelem *nol;

	nol = dfs->dfs_nol;
	while (nol) {
		dfs_nol_channel = wlan_freq_to_chan(nol->nol_freq);
		if (false == dfs_find_target_channel_in_channel_matrix(
					ch_width, dfs_nol_channel,
					&target_chan_matrix)) {
			/*
			 * should never happen, we should always find a table
			 * here, if we don't, need a fix here!
			 */
			dfs_err(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"Couldn't find target channel matrix!");
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
				dfs->tx_leakage_threshold) {
				/*
				 * candidate channel will have
				 * bad leakage in NOL channel,
				 * remove from temp list
				 */
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					"dfs: channel: %d will have bad leakage due to channel: %d\n",
					dfs_nol_channel, temp_ch_lst[j]);
				temp_ch_lst[j] = 0;
			}
			j++;
			k++;
		}
		nol = nol->nol_next;
	} /* end of loop that selects each NOL */

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS
dfs_mark_leaking_ch(struct wlan_dfs *dfs,
		enum phy_ch_width ch_width,
		uint8_t temp_ch_lst_sz,
		uint8_t *temp_ch_lst)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dfs_populate_80mhz_available_channels()- Populate channels for 80MHz using
 *                                          bitmap
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @avail_chnl: prepared channel list
 *
 * Prepare 80MHz channels from the bitmap.
 *
 * Return: channel count
 */
static uint8_t dfs_populate_80mhz_available_channels(
		struct wlan_dfs *dfs,
		struct chan_bonding_bitmap *bitmap,
		uint8_t *avail_chnl)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint8_t start_chan = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan = bitmap->chan_bonding_set[i].start_chan;
		if (bitmap->chan_bonding_set[i].chan_map ==
			DFS_80MHZ_MASK) {
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 0);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 1);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 2);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 3);
		}
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			"channel count %d", chnl_count);

	return chnl_count;
}

/**
 * dfs_populate_40mhz_available_channels()- Populate channels for 40MHz using
 *                                          bitmap
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @avail_chnl: prepared channel list
 *
 * Prepare 40MHz channels from the bitmap.
 *
 * Return: channel count
 */
static uint8_t dfs_populate_40mhz_available_channels(
		struct wlan_dfs *dfs,
		struct chan_bonding_bitmap *bitmap,
		uint8_t *avail_chnl)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint8_t start_chan = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan = bitmap->chan_bonding_set[i].start_chan;
		if ((bitmap->chan_bonding_set[i].chan_map &
			DFS_40MHZ_MASK_L) == DFS_40MHZ_MASK_L) {
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 0);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 1);
		}
		if ((bitmap->chan_bonding_set[i].chan_map &
			DFS_40MHZ_MASK_H) == DFS_40MHZ_MASK_H) {
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 2);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 3);
		}
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			"channel count %d", chnl_count);

	return chnl_count;
}

/**
 * dfs_populate_available_channels()- Populate channels based on width and
 *                                    bitmap
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @ch_width: channel width
 * @avail_chnl: prepared channel list
 *
 * Prepare channel list based on width and channel bitmap.
 *
 * Return: channel count
 */
static uint8_t dfs_populate_available_channels(
		struct wlan_dfs *dfs,
		struct chan_bonding_bitmap *bitmap,
		uint8_t ch_width,
		uint8_t *avail_chnl)
{
	switch (ch_width) {
	case DFS_CH_WIDTH_160MHZ:
	case DFS_CH_WIDTH_80P80MHZ:
	case DFS_CH_WIDTH_80MHZ:
		return dfs_populate_80mhz_available_channels(
			dfs, bitmap, avail_chnl);
	case DFS_CH_WIDTH_40MHZ:
		return dfs_populate_40mhz_available_channels(
			dfs, bitmap, avail_chnl);
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"Invalid ch_width %d", ch_width);
		break;
	}

	return 0;
}

/**
 * dfs_get_rand_from_lst()- Get random channel from a given channel list
 * @dfs: Pointer to DFS structure.
 * @ch_lst: channel list
 * @num_ch: number of channels
 *
 * Get random channel from given channel list.
 *
 * Return: channel number
 */
static uint8_t dfs_get_rand_from_lst(
		struct wlan_dfs *dfs,
		uint8_t *ch_lst,
		uint8_t num_ch)
{
	uint8_t i;
	uint32_t rand_byte = 0;

	if (!num_ch || !ch_lst) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"invalid param ch_lst %pK, num_ch = %d",
				ch_lst, num_ch);
		return 0;
	}

	get_random_bytes((uint8_t *)&rand_byte, 1);
	i = (rand_byte + qdf_mc_timer_get_system_ticks()) % num_ch;

	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			"random channel %d", ch_lst[i]);

	return ch_lst[i];
}

/**
 * dfs_random_channel_sel_set_bitmap()- Set channel bit in bitmap based
 * on given channel number
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @channel: channel number
 *
 * Set channel bit in bitmap based on given channel number.
 *
 * Return: None
 */
static void dfs_random_channel_sel_set_bitmap(
		struct wlan_dfs *dfs,
		struct chan_bonding_bitmap *bitmap,
		uint8_t channel)
{
	int i = 0;
	int start_chan = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan = bitmap->chan_bonding_set[i].start_chan;
		if (channel >= start_chan && channel <= start_chan + 12) {
			bitmap->chan_bonding_set[i].chan_map |=
			(1 << ((channel - start_chan) /
			DFS_80_NUM_SUB_CHANNEL));
			return;
		}
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			"Channel=%d is not in the bitmap", channel);
}

/**
 * dfs_find_ch_with_fallback()- find random channel
 * @dfs: Pointer to DFS structure.
 * @ch_wd: channel width
 * @center_freq_seg1: center frequency of secondary segment.
 * @ch_lst: list of available channels.
 * @num_ch: number of channels in the list.
 *
 * Find random channel based on given channel width and channel list,
 * fallback to lower width if requested channel width not available.
 *
 * Return: channel number
 */
static uint8_t dfs_find_ch_with_fallback(
		struct wlan_dfs *dfs,
		uint8_t *ch_wd,
		uint8_t *center_freq_seg1,
		uint8_t *ch_lst,
		uint32_t num_ch)
{
	bool flag = false;
	uint32_t rand_byte = 0;
	struct  chan_bonding_bitmap ch_map = { { {0} } };
	uint8_t count = 0, i, index = 0, final_cnt = 0, target_channel = 0;
	uint8_t primary_seg_start_ch = 0, sec_seg_ch = 0, new_160_start_ch = 0;
	uint8_t final_lst[DFS_MAX_NUM_CHAN] = {0};

	/* initialize ch_map for all 80 MHz bands: we have 6 80MHz bands */
	ch_map.chan_bonding_set[0].start_chan = 36;
	ch_map.chan_bonding_set[1].start_chan = 52;
	ch_map.chan_bonding_set[2].start_chan = 100;
	ch_map.chan_bonding_set[3].start_chan = 116;
	ch_map.chan_bonding_set[4].start_chan = 132;
	ch_map.chan_bonding_set[5].start_chan = 149;

	for (i = 0; i < num_ch; i++) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"channel = %d added to bitmap", ch_lst[i]);
		dfs_random_channel_sel_set_bitmap(dfs, &ch_map, ch_lst[i]);
	}

	/* populate available channel list from bitmap */
	final_cnt = dfs_populate_available_channels(dfs, &ch_map,
			*ch_wd, final_lst);

	/* If no valid ch bonding found, fallback */
	if (final_cnt == 0) {
		if ((*ch_wd == DFS_CH_WIDTH_160MHZ) ||
		    (*ch_wd == DFS_CH_WIDTH_80P80MHZ) ||
		    (*ch_wd == DFS_CH_WIDTH_80MHZ)) {
			dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					"from [%d] to 40Mhz", *ch_wd);
			*ch_wd = DFS_CH_WIDTH_40MHZ;
		} else if (*ch_wd == DFS_CH_WIDTH_40MHZ) {
			dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					"from 40Mhz to 20MHz");
			*ch_wd = DFS_CH_WIDTH_20MHZ;
		}
		return 0;
	}

	/* ch count should be > 8 to switch new channel in 160Mhz band */
	if (((*ch_wd == DFS_CH_WIDTH_160MHZ) ||
	     (*ch_wd == DFS_CH_WIDTH_80P80MHZ)) &&
	     (final_cnt < DFS_MAX_20M_SUB_CH)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"from [%d] to 80Mhz", *ch_wd);
		*ch_wd = DFS_CH_WIDTH_80MHZ;
		return 0;
	}

	if (*ch_wd == DFS_CH_WIDTH_160MHZ) {
		/*
		 * Only 2 blocks for 160Mhz bandwidth i.e 36-64 & 100-128
		 * and all the channels in these blocks are continuous
		 * and separated by 4Mhz.
		 */
		for (i = 1; ((i < final_cnt)); i++) {
			if ((final_lst[i] - final_lst[i-1]) ==
			     DFS_NEXT_5GHZ_CHANNEL)
				count++;
			else
				count = 0;
			if (count == DFS_MAX_20M_SUB_CH - 1) {
				flag = true;
				new_160_start_ch = final_lst[i - count];
				break;
			}
		}
	} else if (*ch_wd == DFS_CH_WIDTH_80P80MHZ) {
		flag = true;
	}

	if ((flag == false) && (*ch_wd > DFS_CH_WIDTH_80MHZ)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"from [%d] to 80Mhz", *ch_wd);
		*ch_wd = DFS_CH_WIDTH_80MHZ;
		return 0;
	}

	if (*ch_wd == DFS_CH_WIDTH_160MHZ) {
		get_random_bytes((uint8_t *)&rand_byte, 1);
		rand_byte = (rand_byte + qdf_mc_timer_get_system_ticks())
			% DFS_MAX_20M_SUB_CH;
		target_channel = new_160_start_ch + (rand_byte *
				DFS_80_NUM_SUB_CHANNEL);
	} else if (*ch_wd == DFS_CH_WIDTH_80P80MHZ) {
		get_random_bytes((uint8_t *)&rand_byte, 1);
		index = (rand_byte + qdf_mc_timer_get_system_ticks()) %
			final_cnt;
		target_channel = final_lst[index];
		index -= (index % DFS_80_NUM_SUB_CHANNEL);
		primary_seg_start_ch = final_lst[index];

		/* reset channels associate with primary 80Mhz */
		for (i = 0; i < DFS_80_NUM_SUB_CHANNEL; i++)
			final_lst[i + index] = 0;
		/* select and calculate center freq for secondary segment */
		for (i = 0; i < final_cnt / DFS_80_NUM_SUB_CHANNEL; i++) {
			if (final_lst[i * DFS_80_NUM_SUB_CHANNEL] &&
			    (abs(primary_seg_start_ch -
			     final_lst[i * DFS_80_NUM_SUB_CHANNEL]) >
			     (DFS_MAX_20M_SUB_CH * 2))) {
				sec_seg_ch =
					final_lst[i * DFS_80_NUM_SUB_CHANNEL] +
					DFS_80MHZ_START_CENTER_CH_DIFF;
				break;
			}
		}

		if (!sec_seg_ch && (final_cnt == DFS_MAX_20M_SUB_CH))
			*ch_wd = DFS_CH_WIDTH_160MHZ;
		else if (!sec_seg_ch)
			*ch_wd = DFS_CH_WIDTH_80MHZ;

		*center_freq_seg1 = sec_seg_ch;
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"Center frequency seg1 = %d", sec_seg_ch);
	} else {
		target_channel = dfs_get_rand_from_lst(dfs,
				final_lst, final_cnt);
	}
	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			"target channel = %d", target_channel);

	return target_channel;
}

/**
 * dfs_remove_cur_ch_from_list()- remove current operating channels
 * @ch_list: list of avilable channel list
 * @ch_cnt: number of channels.
 * @ch_wd: channel width.
 * @cur_chan: current channel.
 *
 * Remove current channels from list of available channels.
 *
 * Return: channel number
 */
static void dfs_remove_cur_ch_from_list(
	struct dfs_channel *ch_list,
	uint32_t *ch_cnt,
	uint8_t *ch_wd,
	struct dfs_channel *cur_chan)
{
	/* TODO */
	return;
}

bool dfs_is_freq_in_nol(struct wlan_dfs *dfs, uint32_t freq)
{
	struct dfs_nolelem *nol;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,  "null dfs");
		return false;
	}

	nol = dfs->dfs_nol;
	while (nol) {
		if (freq == nol->nol_freq) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					"%d is in nol", freq);
			return true;
		}
		nol = nol->nol_next;
	}

	return false;
}

/**
 * dfs_apply_rules()- prepare channel list based on flags
 * @dfs: dfs handler
 * @flags: channel flags
 * @random_chan_list: output channel list
 * @random_chan_cnt: output channel count
 * @ch_list: input channel list
 * @ch_cnt: input channel count
 * @dfs_region: dfs region
 * @acs_info: acs channel range information
 *
 * prepare channel list based on flags
 *
 * Return: None
 */
static void dfs_apply_rules(struct wlan_dfs *dfs,
	uint32_t flags,
	uint8_t *random_chan_list,
	uint32_t *random_chan_cnt,
	struct dfs_channel *ch_list,
	uint32_t ch_cnt,
	uint8_t dfs_region,
	struct dfs_acs_info *acs_info)
{
	struct dfs_channel *chan;
	bool flag_no_weather = 0;
	bool flag_no_lower_5g = 0;
	bool flag_no_upper_5g = 0;
	bool flag_no_dfs_chan = 0;
	bool flag_no_2g_chan  = 0;
	bool flag_no_5g_chan  = 0;
	bool flag_no_japan_w53 = 0;
	int i;
	bool found = false;

	dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN, "flags %d", flags);
	flag_no_weather = (dfs_region == DFS_ETSI_REGION_VAL) ?
		flags & DFS_RANDOM_CH_FLAG_NO_WEATHER_CH : 0;

	if (dfs_region == DFS_MKK_REGION_VAL) {
		flag_no_lower_5g = flags & DFS_RANDOM_CH_FLAG_NO_LOWER_5G_CH;
		flag_no_upper_5g = flags & DFS_RANDOM_CH_FLAG_NO_UPEER_5G_CH;
		flag_no_japan_w53 = flags & DFS_RANDOM_CH_FLAG_NO_JAPAN_W53_CH;
	}

	flag_no_dfs_chan = flags & DFS_RANDOM_CH_FLAG_NO_DFS_CH;
	flag_no_2g_chan  = flags & DFS_RANDOM_CH_FLAG_NO_2GHZ_CH;
	flag_no_5g_chan  = flags & DFS_RANDOM_CH_FLAG_NO_5GHZ_CH;

	for (i = 0; i < ch_cnt; i++) {
		chan = &ch_list[i];

		if ((chan->dfs_ch_ieee == 0) ||
				(chan->dfs_ch_ieee > MAX_CHANNEL_NUM)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "invalid channel %d", chan->dfs_ch_ieee);
			continue;
		}

		if (flags & DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH) {
			/* TODO : Skip all HT20 channels in the given mode */
			if (chan->dfs_ch_ieee ==
					dfs->dfs_curchan->dfs_ch_ieee) {
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					  "skip %d current operating channel",
					  chan->dfs_ch_ieee);
				continue;
			}
		}

		if (acs_info && acs_info->acs_mode) {
			for (i = 0; i < acs_info->num_of_channel; i++) {
				if (acs_info->channel_list[i] ==
				    chan->dfs_ch_ieee) {
					found = true;
					break;
				}
			}

			if (!found) {
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					  "skip ch %d not in acs range",
					  chan->dfs_ch_ieee);
				continue;
			}
			found = false;
		}

		if (flag_no_2g_chan &&
				chan->dfs_ch_ieee <= DFS_MAX_24GHZ_CHANNEL) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip 2.4 GHz channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_5g_chan &&
				chan->dfs_ch_ieee > DFS_MAX_24GHZ_CHANNEL) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip 5 GHz channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_weather) {
			if (DFS_IS_CHANNEL_WEATHER_RADAR(chan->dfs_ch_freq)) {
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					  "skip weather channel=%d",
					  chan->dfs_ch_ieee);
				continue;
			}
		}

		if (flag_no_lower_5g &&
		    DFS_IS_CHAN_JAPAN_INDOOR(chan->dfs_ch_ieee)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip indoor channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_upper_5g &&
		    DFS_IS_CHAN_JAPAN_OUTDOOR(chan->dfs_ch_ieee)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip outdoor channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_dfs_chan &&
		    (chan->dfs_ch_flagext & WLAN_CHAN_DFS)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip dfs channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_japan_w53 &&
		    DFS_IS_CHAN_JAPAN_W53(chan->dfs_ch_ieee)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip japan W53 channel=%d",
				  chan->dfs_ch_ieee);
			continue;
		}

		if (dfs_is_freq_in_nol(dfs, chan->dfs_ch_freq)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip nol channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		random_chan_list[*random_chan_cnt] = chan->dfs_ch_ieee;
		*random_chan_cnt += 1;
	}
}

uint8_t dfs_prepare_random_channel(struct wlan_dfs *dfs,
	struct dfs_channel *ch_list,
	uint32_t ch_cnt,
	uint32_t flags,
	uint8_t *ch_wd,
	struct dfs_channel *cur_chan,
	uint8_t dfs_region,
	struct dfs_acs_info *acs_info)
{
	int i = 0;
	uint8_t final_cnt = 0;
	uint8_t target_ch = 0;
	uint8_t *random_chan_list = NULL;
	uint32_t random_chan_cnt = 0;
	uint16_t flag_no_weather = 0;
	uint8_t *leakage_adjusted_lst;
	uint8_t final_lst[NUM_CHANNELS] = {0};

	if (!ch_list || !ch_cnt) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"Invalid params %pK, ch_cnt=%d",
				ch_list, ch_cnt);
		return 0;
	}

	if (*ch_wd < DFS_CH_WIDTH_20MHZ || *ch_wd > DFS_CH_WIDTH_80P80MHZ) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				"Invalid ch_wd %d", *ch_wd);
		return 0;
	}

	random_chan_list = qdf_mem_malloc(ch_cnt * sizeof(*random_chan_list));
	if (!random_chan_list)
		return 0;

	if (flags & DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH)
		dfs_remove_cur_ch_from_list(ch_list, &ch_cnt, ch_wd, cur_chan);

	dfs_apply_rules(dfs, flags, random_chan_list, &random_chan_cnt,
		    ch_list, ch_cnt, dfs_region, acs_info);

	flag_no_weather = (dfs_region == DFS_ETSI_REGION_VAL) ?
		flags & DFS_RANDOM_CH_FLAG_NO_WEATHER_CH : 0;

	/* list adjusted after leakage has been marked */
	leakage_adjusted_lst = qdf_mem_malloc(random_chan_cnt);
	if (!leakage_adjusted_lst) {
		qdf_mem_free(random_chan_list);
		return 0;
	}

	do {
		qdf_mem_copy(leakage_adjusted_lst, random_chan_list,
			     random_chan_cnt);
		if (QDF_IS_STATUS_ERROR(dfs_mark_leaking_ch(dfs, *ch_wd,
				random_chan_cnt,
				leakage_adjusted_lst))) {
			qdf_mem_free(random_chan_list);
			qdf_mem_free(leakage_adjusted_lst);
			return 0;
		}

		if (*ch_wd == DFS_CH_WIDTH_20MHZ) {
			/*
			 * PASS: 3 - from leakage_adjusted_lst, prepare valid
			 * ch list and use random number from that
			 */
			for (i = 0; i < random_chan_cnt; i++) {
				if (leakage_adjusted_lst[i] == 0)
					continue;
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					  "dfs: Channel=%d added to available list",
					  leakage_adjusted_lst[i]);
				final_lst[final_cnt] = leakage_adjusted_lst[i];
				final_cnt++;
			}
			target_ch = dfs_get_rand_from_lst(
				dfs, final_lst, final_cnt);
			break;
		}

		target_ch = dfs_find_ch_with_fallback(dfs, ch_wd,
				&cur_chan->dfs_ch_vhtop_ch_freq_seg2,
				leakage_adjusted_lst,
				random_chan_cnt);

		/*
		 * When flag_no_weather is set, avoid usage of Adjacent
		 * weather radar channel in HT40 mode as extension channel
		 * will be on 5600.
		 */
		if (flag_no_weather &&
				(target_ch ==
				 DFS_ADJACENT_WEATHER_RADAR_CHANNEL_NUM) &&
				(*ch_wd == DFS_CH_WIDTH_40MHZ)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					"skip weather adjacent ch=%d\n",
					target_ch);
			continue;
		}

		if (target_ch)
			break;
	} while (true);

	qdf_mem_free(random_chan_list);
	qdf_mem_free(leakage_adjusted_lst);
	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN, "target_ch = %d", target_ch);

	return target_ch;
}
