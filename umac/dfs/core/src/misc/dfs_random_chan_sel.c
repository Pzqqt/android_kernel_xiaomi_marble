/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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
#include <wlan_reg_services_api.h>
#include "../dfs_process_radar_found_ind.h"

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
	{52, 5260,
	 {{36, 5180, 148}, {40, 5200, 199},
	  {44, 5520, 193}, {48, 5240, 197},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, 153},
	  {60, 5300, 137}, {64, 5320, 134},
	  {100, 5500, 358}, {104, 5520, 350},
	  {108, 5540, 404}, {112, 5560, 344},
	  {116, 5580, 424}, {120, 5600, 429},
	  {124, 5620, 437}, {128, 5640, 435},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },


	{56, 5280,
	 {{36, 5180, 171}, {40, 5200, 178},
	  {44, 5220, 171}, {48, 5240, 178},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, 280},
	  {100, 5500, 351}, {104, 5520, 376},
	  {108, 5540, 362}, {112, 5560, 362},
	  {116, 5580, 403}, {120, 5600, 397},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{60,5300,
	 {{36, 5180, 156}, {40, 5200, 146},
	  {44, 5220, DFS_TX_LEAKAGE_MIN}, {48, 5240, DFS_TX_LEAKAGE_MIN},
	  {52, 5260, 180}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, 376}, {104, 5520, 360},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, DFS_TX_LEAKAGE_MAX},
	  {116, 5580, 395}, {120, 5600, 399},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{64, 5320,
	 {{36, 5180,  217}, {40, 5200, 221},
	  {44, 5220, DFS_TX_LEAKAGE_MIN}, {48, 5240, DFS_TX_LEAKAGE_MIN},
	  {52, 5260, 176}, {56, 5280, 176},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, 384}, {104, 5520, 390},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, DFS_TX_LEAKAGE_MAX},
	  {116, 5580, 375}, {120, 5600, 374},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{100, 5500,
	 {{36, 5180, 357}, {40, 5200, 326},
	  {44, 5220, 321}, {48, 5240, 326},
	  {52, 5260, 378}, {56, 5280, 396},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, 196}, {112, 5560, 116},
	  {116, 5580, 166}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{104, 5520,
	 {{36, 5180,  325}, {40, 5200, 325},
	  {44, 5220, 305}, {48, 5240, 352},
	  {52, 5260, 411}, {56, 5280, 411},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, 460},
	  {116, 5580, 198}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{108, 5540,
	 {{36,5180,  304}, {40, 5200, 332},
	  {44, 5220, 310}, {48, 5240, 335},
	  {52, 5260, 431}, {56, 5280, 391},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 280}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, 185}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{112,5560,
	 {{36, 5180, 327}, {40, 5200, 335},
	  {44, 5220, 331}, {48, 5240, 345},
	  {52, 5260, 367}, {56, 5280, 401},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 131}, {104, 5520, 132},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, 189}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{116, 5580,
	 {{36, 5180, 384}, {40, 5200, 372},
	  {44, 5220, 389}, {48, 5240, 396},
	  {52, 5260, 348}, {56, 5280, 336},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 172}, {104, 5520, 169},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{120, 5600,
	 {{36, 5180, 395}, {40, 5200, 419},
	  {44, 5220, 439}, {48, 5240, 407},
	  {52, 5260, 321}, {56, 5280, 334},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 134}, {104, 5520, 186},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, 159},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{124, 5620,
	 {{36, 5180, 469}, {40, 5200, 433},
	  {44, 5220, 434}, {48, 5240, 435},
	  {52, 5260, 332}, {56, 5280, 345},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 146}, {104, 5520, 177},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, 350}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, 138},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{128, 5640,
	 {{36, 5180, 408}, {40, 5200, 434},
	  {44, 5220, 449}, {48, 5240, 444},
	  {52, 5260, 341}, {56, 5280, 374},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 205}, {104, 5520, 208},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, 142}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{132, 5660,
	 {{36, 5180, DFS_TX_LEAKAGE_MAX}, {40, 5200, DFS_TX_LEAKAGE_MAX},
	  {44, 5220, DFS_TX_LEAKAGE_MAX}, {48, 5240, DFS_TX_LEAKAGE_MAX},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{136, 5680,
	 {{36, 5180, DFS_TX_LEAKAGE_MAX}, {40, 5200, DFS_TX_LEAKAGE_MAX},
	  {44, 5220, DFS_TX_LEAKAGE_MAX}, {48, 5240, DFS_TX_LEAKAGE_MAX},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{140, 5700,
	 {{36, 5180,  DFS_TX_LEAKAGE_MAX}, {40, 5200, DFS_TX_LEAKAGE_MAX},
	  {44, 5220, DFS_TX_LEAKAGE_MAX}, {48, 5240, DFS_TX_LEAKAGE_MAX},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{144, 5720,
	 {{36, 5180, DFS_TX_LEAKAGE_MAX}, {40, 5200, DFS_TX_LEAKAGE_MAX},
	  {44, 5220, DFS_TX_LEAKAGE_MAX}, {48, 5240, DFS_TX_LEAKAGE_MAX},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },
};

/* channel tx leakage table - ht40 */
struct dfs_matrix_tx_leak_info ht40_chan[] = {
	{52, 5260,
	 {{36, 5180, DFS_TX_LEAKAGE_AUTO_MIN}, {40, 5200, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, 5220, 230}, {48, 5240, 230},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, DFS_TX_LEAKAGE_AUTO_MIN}, {64, 5320, DFS_TX_LEAKAGE_AUTO_MIN},
	  {100, 5500, 625}, {104, 5520, 323},
	  {108, 5540, 646}, {112, 5560, 646},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{56, 5280,
	 {{36, 5180, DFS_TX_LEAKAGE_AUTO_MIN}, {40, 5200, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, 5220, DFS_TX_LEAKAGE_AUTO_MIN}, {48, 5240, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, 611}, {104, 5520, 611},
	  {108, 5540, 617}, {112, 5560, 617},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{60, 5300,
	 {{36, 5180, DFS_TX_LEAKAGE_AUTO_MIN}, {40, 5200, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, 5220, DFS_TX_LEAKAGE_AUTO_MIN}, {48, 5240, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 5260, 190}, {56, 5280, 190},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, 608}, {104, 5520, 608},
	  {108, 5540, 623}, {112, 5560, 623},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{64, 5320,
	 {{36, 5180, DFS_TX_LEAKAGE_AUTO_MIN}, {40, 5200, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, 5220, DFS_TX_LEAKAGE_AUTO_MIN}, {48, 5240, DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 5260, 295}, {56, 5280, 295},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, 594}, {104, 5520, 594},
	  {108, 5540, 625}, {112, 5560, 625},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{100, 5500,
	 {{36, 5180, 618}, {40, 5200, 618},
	  {44, 5220, 604}, {48, 5240, 604},
	  {52, 5260, 596}, {56, 5280, 596},
	  {60, 5300, 584}, {64, 5320, 584},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, 299}, {112, 5560, 299},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, 538}, {136,5680, 538},
	  {140, 5700, 598},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{104, 5520,
	 {{36, 5180, 636}, {40, 5200, 636},
	  {44, 5220, 601}, {48, 5240, 601},
	  {52, 5260, 616}, {56, 5280, 616},
	  {60, 5300, 584}, {64, 5320, 584},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, 553}, {136, 5680, 553},
	  {140, 5700, 568},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{108, 5540,
	 {{36, 5180, 600}, {40, 5200, 600},
	  {44, 5220, 627}, {48, 5240, 627},
	  {52, 5260, 611}, {56, 5280, 611},
	  {60, 5300, 611}, {64, 5320, 611},
	  {100, 5500, 214}, {104, 5520, 214},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_AUTO_MIN}, {136, 5680, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 5700, 534},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{112, 5560,
	 {{36, 5180, 645}, {40, 5200, 645},
	  {44, 5220, 641}, {48, 5240, 641},
	  {52, 5260, 618}, {56, 5280, 618},
	  {60, 5300, 612}, {64, 5320, 612},
	  {100, 5500, 293}, {104, 5520, 293},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_AUTO_MIN}, {136, 5680, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 5700, 521},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{116, 5580,
	 {{36, 5180, 661}, {40, 5200, 661},
	  {44, 5220, 624}, {48, 5240, 624},
	  {52, 5260, 634}, {56, 5280, 634},
	  {60, 5300, 611}, {64, 5320, 611},
	  {100, 5500, DFS_TX_LEAKAGE_AUTO_MIN}, {104, 5520, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, 5540, 217}, {112, 5560, 217},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_AUTO_MIN}, {136, 5680, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{120, 5600,
	 {{36, 5180, 667}, {40, 5200, 667},
	  {44, 5220, 645}, {48, 5240, 645},
	  {52, 5260, 633}, {56, 5280, 633},
	  {60, 5300, 619}, {64, 5320, 619},
	  {100, 5500, DFS_TX_LEAKAGE_AUTO_MIN}, {104, 5520, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, 5540, 291}, {112, 5560, 291},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_AUTO_MIN}, {136, 5680, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{124, 5620,
	 {{36, 5180,  676}, {40, 5200, 676},
	  {44, 5220, 668}, {48, 5240, 668},
	  {52, 5260, 595}, {56, 5280, 595},
	  {60, 5300, 622}, {64, 5320, 622},
	  {100, 5500, DFS_TX_LEAKAGE_AUTO_MIN}, {104, 5520, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_AUTO_MIN}, {112, 5560, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 5580, 225}, {120, 5600, 225},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_AUTO_MIN}, {136, 5680, DFS_TX_LEAKAGE_AUTO_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{128, 5640,
	 {{36, 5180, 678}, {40, 5200, 678},
	  {44, 5220, 664}, {48, 5240, 664},
	  {52, 5260, 651}, {56, 5280, 651},
	  {60, 5300, 643}, {64, 5320, 643},
	  {100, 5500, DFS_TX_LEAKAGE_AUTO_MIN}, {104, 5520, DFS_TX_LEAKAGE_AUTO_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_AUTO_MIN}, {112, 5560, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 5580, 293}, {120, 5600, 293},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_AUTO_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{132, 5660,
	 {{36, 5180, 689}, {40, 5200, 689},
	  {44, 5220, 669}, {48, 5240, 669},
	  {52, 5260, 662}, {56, 5280, 662},
	  {60, 5300, 609}, {64, 5320, 609},
	  {100, 5500, 538}, {104, 5520, 538},
	  {108, 5540, DFS_TX_LEAKAGE_AUTO_MIN}, {112, 5560, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, 247}, {128, 5640, 247},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{136, 5680,
	 {{36, 5180, 703}, {40, 5200, 703},
	  {44, 5220, 688}, {48, 5240, DFS_TX_LEAKAGE_MIN},
	  {52, 5260, 671}, {56, 5280, 671},
	  {60, 5300, 658}, {64, 5320, 658},
	  {100, 5500, 504}, {104, 5520, 504},
	  {108, 5540, DFS_TX_LEAKAGE_AUTO_MIN}, {112, 5560, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, 289}, {128, 5640, 289},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{140, 5700,
	 {{36, 5180, 695}, {40, 5200, 695},
	  {44, 5220, 684}, {48, 5240, 684},
	  {52, 5260, 664}, {56, 5280, 664},
	  {60, 5300, 658}, {64, 5320, 658},
	  {100, 5500, 601}, {104, 5520, 601},
	  {108, 5540, 545}, {112, 5560, 545},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, 262}, {136, 5680, 262},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{144, 5720,
	 {{36, 5180, 695}, {40, 5200, 695},
	  {44, 5220, 684}, {48, 5240, 684},
	  {52, 5260, 664}, {56, 5280, 664},
	  {60, 5300, 658}, {64, 5320, 658},
	  {100, 5500, 601}, {104, 5520, 601},
	  {108, 5540, 545}, {112, 5560, 545},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, DFS_TX_LEAKAGE_AUTO_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_AUTO_MIN}, {128, 5640, DFS_TX_LEAKAGE_AUTO_MIN},
	  {132, 5660, 262}, {136, 5680, 262},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },
};

/* channel tx leakage table - ht20 */
struct dfs_matrix_tx_leak_info ht20_chan[] = {
	{52, 5260,
	 {{36, 5180,DFS_TX_LEAKAGE_AUTO_MIN}, {40, 5200, 286},
	  {44, 5220, 225}, {48,5240, 121},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, 300}, {64, 5320, DFS_TX_LEAKAGE_AUTO_MIN},
	  {100, 5500, 637}, {104, 5520, DFS_TX_LEAKAGE_MAX},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, DFS_TX_LEAKAGE_MAX},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{56, 5280,
	 {{36, 5180, 468}, {40, 5200, DFS_TX_LEAKAGE_AUTO_MIN},
	  {44, 5220, DFS_TX_LEAKAGE_AUTO_MIN}, {48, 5240, 206},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, DFS_TX_LEAKAGE_MAX}, {104, 5520, DFS_TX_LEAKAGE_MAX},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, DFS_TX_LEAKAGE_MAX},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{60, 5300,
	 {{36, 5180, 507}, {40, 5200, 440},
	  {44, 5220, DFS_TX_LEAKAGE_AUTO_MIN}, {48,5240, 313},
	  {52, 5260, DFS_TX_LEAKAGE_MIN}, {56, 5280, DFS_TX_LEAKAGE_MIN},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, DFS_TX_LEAKAGE_MAX}, {104, 5520, DFS_TX_LEAKAGE_MAX},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, DFS_TX_LEAKAGE_MAX},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{64, 5320 ,
	 {{36, 5180, 516}, {40, 5200, 520},
	  {44, 5220, 506}, {48, 5240,DFS_TX_LEAKAGE_AUTO_MIN},
	  {52, 5260, 301}, {56, 5280, 258},
	  {60, 5300, DFS_TX_LEAKAGE_MIN}, {64, 5320, DFS_TX_LEAKAGE_MIN},
	  {100, 5500, 620}, {104, 5520, 617},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, DFS_TX_LEAKAGE_MAX},
	  {116, 5580, DFS_TX_LEAKAGE_MAX}, {120, 5600, DFS_TX_LEAKAGE_MAX},
	  {124, 5620, DFS_TX_LEAKAGE_MAX}, {128, 5640, DFS_TX_LEAKAGE_MAX},
	  {132, 5660, DFS_TX_LEAKAGE_MAX}, {136, 5680, DFS_TX_LEAKAGE_MAX},
	  {140, 5700, DFS_TX_LEAKAGE_MAX},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{100, 5500,
	 {{36, 5180, 616}, {40, 5200, 601},
	  {44, 5220, 604}, {48, 5240, 589},
	  {52, 5260, 612}, {56, 5280, 592},
	  {60, 5300, 590}, {64, 5320, 582},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, 131},
	  {108, 5540, DFS_TX_LEAKAGE_AUTO_MIN}, {112, 5560, DFS_TX_LEAKAGE_AUTO_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_AUTO_MIN}, {120, 5600, 522},
	  {124, 5620, 571}, {128, 5640, 589},
	  {132, 5660, 593}, {136, 5680, 598},
	  {140, 5700, 594},
	  {144, 5720, DFS_TX_LEAKAGE_MIN},
	  } },

	{104, 5520,
	 {{36, 5180, 622}, {40, 5200, 624},
	  {44, 5220, 618}, {48, 5240, 610},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, DFS_TX_LEAKAGE_MIN}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, 463},
	  {116, 5580, 483}, {120, 5600, 503},
	  {124, 5620, 523}, {128, 5640, 565},
	  {132, 5660, 570}, {136, 5680, 588},
	  {140, 5700, 585},
	  {144, 5720, DFS_TX_LEAKAGE_MIN},
	  } },

	{108, 5540,
	 {{36, 5180, 620}, {40, 5200, 638},
	  {44, 5220, 611}, {48, 5240, 614},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 477}, {104, 5520, DFS_TX_LEAKAGE_MIN},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, 477}, {120, 5600, 497},
	  {124, 5620, 517}, {128, 5640, 537},
	  {132, 5660, 557}, {136, 5680, 577},
	  {140, 5700, 603},
	  {144, 5720, DFS_TX_LEAKAGE_MIN},
	  } },

	{112, 5560,
	 {{36, 5180, 636}, {40, 5200, 623},
	  {44, 5220, 638}, {48, 5240, 628},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, 606},
	  {100, 5500, 501}, {104, 5520, 481},
	  {108, 5540, DFS_TX_LEAKAGE_MIN}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, 481},
	  {124, 5620, 501}, {128, 5640, 421},
	  {132, 5660, 541}, {136, 5680, 561},
	  {140, 5700, 583},
	  {144, 5720, DFS_TX_LEAKAGE_MIN},
	  } },

	{116, 5580,
	 {{36, 5180, 646}, {40, 5200, 648},
	  {44, 5220, 633}, {48, 5240, 634},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, 615}, {64, 5320, 594},
	  {100, 5500, 575}, {104, 5520, 554},
	  {108, 5540, 534}, {112, 5560, DFS_TX_LEAKAGE_MIN},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, 534}, {136, 5680, 554},
	  {140, 5700, 574},
	  {144, 5720, DFS_TX_LEAKAGE_MIN},
	  } },

	{120, 5600,
	 {{36, 5180, 643}, {40, 5200, 649},
	  {44, 5220, 654}, {48, 5240, 629},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, 621},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 565}, {104, 5520, 545},
	  {108, 5540, 525}, {112, 5560, 505},
	  {116, 5580, DFS_TX_LEAKAGE_MIN}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, 505},
	  {132, 5660, 525}, {136, 5680, 545},
	  {140, 5700, 565},
	  {144, 5720, DFS_TX_LEAKAGE_MIN},
	  } },

	{124, 5620,
	 {{36, 5180, 638}, {40, 5200, 657},
	  {44, 5220, 663}, {48, 5240, 649},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 581}, {104, 5520, 561},
	  {108, 5540, 541}, {112, 5560, 521},
	  {116, 5580, 499}, {120, 5600, DFS_TX_LEAKAGE_MIN},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, 499}, {136, 5680, 519},
	  {140, 5700, 539},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{128, 5640,
	 {{36, 5180, 651}, {40, 5200, 651},
	  {44, 5220, 674}, {48, 5240, 640},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, 603}, {104, 5520, 560},
	  {108, 5540, 540}, {112, 5560, 520},
	  {116, 5580, 499}, {120, 5600, 479},
	  {124, 5620, DFS_TX_LEAKAGE_MIN}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, 479},
	  {140, 5700, 499},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{132, 5660,
	 {{36, 5180, 643}, {40, 5200, 668},
	  {44, 5220, 651}, {48, 5240, 657},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, DFS_TX_LEAKAGE_MAX}, {104, 5520, 602},
	  {108, 5540, 578}, {112,5560, 570},
	  {116, 5580, 550}, {120, 5600, 530},
	  {124, 5620, 510}, {128, 5640, DFS_TX_LEAKAGE_MIN},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, 490},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{136,5680,
	 {{36, 5180, 654}, {40, 5200, 667},
	  {44, 5220, 666}, {48, 5240, 642},
	  {52, 5260, DFS_TX_LEAKAGE_MAX}, {56, 5280, DFS_TX_LEAKAGE_MAX},
	  {60, 5300, DFS_TX_LEAKAGE_MAX}, {64, 5320, DFS_TX_LEAKAGE_MAX},
	  {100, 5500, DFS_TX_LEAKAGE_MAX}, {104, 5520, DFS_TX_LEAKAGE_MAX},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, 596},
	  {116, 5580, 555}, {120, 5600, 535},
	  {124, 5620, 515}, {128, 5640, 495},
	  {132, 5660, DFS_TX_LEAKAGE_MIN}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{140,5700,
	 {{36, 5180, 679}, {40, 5200, 673},
	  {44, 5220, 667}, {48, 5240, 656},
	  {52, 5260, 634}, {56, 5280, 663},
	  {60, 5300, 662}, {64, 5320, 660},
	  {100, 5500, DFS_TX_LEAKAGE_MAX}, {104, 5520, DFS_TX_LEAKAGE_MAX},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, 590},
	  {116, 5580, 573}, {120, 5600, 553},
	  {124, 5620, 533}, {128, 5640, 513},
	  {132, 5660, 490}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
	  } },

	{144,5720,
	 {{36, 5180, 679}, {40, 5200, 673},
	  {44, 5220, 667}, {48, 5240, 656},
	  {52, 5260, 634}, {56, 5280, 663},
	  {60, 5300, 662}, {64, 5320, 660},
	  {100, 5500, DFS_TX_LEAKAGE_MAX}, {104, 5520, DFS_TX_LEAKAGE_MAX},
	  {108, 5540, DFS_TX_LEAKAGE_MAX}, {112, 5560, 590},
	  {116, 5580, 573}, {120, 5600, 553},
	  {124, 5620, 533}, {128, 5640, 513},
	  {132, 5660, 490}, {136, 5680, DFS_TX_LEAKAGE_MIN},
	  {140, 5700, DFS_TX_LEAKAGE_MIN},
	  {144, 5720, DFS_TX_LEAKAGE_MIN}
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
#ifdef CONFIG_CHAN_NUM_API
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
#endif

/*
 * dfs_find_target_channel_in_channel_matrix_for_freq() - finds the leakage
 * matrix.
 * @chan_width: target channel width
 * @nol_channel: the NOL channel frequency whose leakage matrix is required
 * @pTarget_chnl_mtrx: pointer to target channel matrix returned.
 *
 * This function gives the leakage matrix for given NOL channel and ch_width
 *
 * Return: TRUE or FALSE
 */
#ifdef CONFIG_CHAN_FREQ_API
static bool
dfs_find_target_channel_in_channel_matrix_for_freq(enum phy_ch_width chan_width,
						   uint16_t nol_freq,
						   struct dfs_tx_leak_info
						   **pTarget_chnl_mtrx)
{
	struct dfs_tx_leak_info *target_chan_matrix = NULL;
	struct dfs_matrix_tx_leak_info *pchan_matrix = NULL;
	uint32_t nchan_matrix;
	int i = 0;

	switch (chan_width) {
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
		if (nol_freq == pchan_matrix[i].channel_freq) {
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
#endif

#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS
dfs_mark_leaking_ch(struct wlan_dfs *dfs,
		enum phy_ch_width ch_width,
		uint8_t temp_ch_lst_sz,
		uint8_t *temp_ch_lst)
{
	struct dfs_tx_leak_info *target_chan_matrix = NULL;
	uint32_t         num_channel = (CHAN_ENUM_5720 - CHAN_ENUM_5180) + 1;
	uint32_t         j = 0;
	uint32_t         k = 0;
	uint8_t          dfs_nol_channel;
	struct dfs_nolelem *nol;

	nol = dfs->dfs_nol;
	while (nol) {
		dfs_nol_channel = wlan_reg_freq_to_chan(dfs->dfs_pdev_obj,
							nol->nol_freq);
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
#endif

#ifdef CONFIG_CHAN_FREQ_API

#ifdef CONFIG_BAND_6GHZ
#define END_CHAN_INDEX CHAN_ENUM_7115
#else
#define END_CHAN_INDEX CHAN_ENUM_5720
#endif

#define START_CHAN_INDEX CHAN_ENUM_5180
QDF_STATUS
dfs_mark_leaking_chan_for_freq(struct wlan_dfs *dfs,
			     enum phy_ch_width ch_width,
			     uint8_t temp_chan_lst_sz,
			     uint16_t *temp_freq_lst)
{
	struct dfs_tx_leak_info *target_chan_matrix = NULL;
	uint32_t         num_channel = (END_CHAN_INDEX - START_CHAN_INDEX) + 1;
	uint32_t         j = 0;
	uint32_t         k = 0;
	struct dfs_nolelem *nol;

	nol = dfs->dfs_nol;
	while (nol) {
		if (false == dfs_find_target_channel_in_channel_matrix_for_freq(
					ch_width, nol->nol_freq,
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
		 * following is based on assumption that both temp_freq_lst
		 * and target channel matrix are in increasing order of
		 * ch_id
		 */
		for (j = 0, k = 0; j < temp_chan_lst_sz && k < num_channel;) {
			if (temp_freq_lst[j] == 0) {
				j++;
				continue;
			}
			if (target_chan_matrix[k].leak_chan_freq !=
			    temp_freq_lst[j]) {
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
					nol->nol_freq, temp_freq_lst[j]);
				temp_freq_lst[j] = 0;
			}
			j++;
			k++;
		}
		nol = nol->nol_next;
	} /* end of loop that selects each NOL */

	return QDF_STATUS_SUCCESS;
}
#endif
#else
#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS
dfs_mark_leaking_ch(struct wlan_dfs *dfs,
		enum phy_ch_width ch_width,
		uint8_t temp_ch_lst_sz,
		uint8_t *temp_ch_lst)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS
dfs_mark_leaking_chan_for_freq(struct wlan_dfs *dfs,
			     enum phy_ch_width ch_width,
			     uint8_t temp_chan_lst_sz,
			     uint16_t *temp_freq_lst)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif

/**
 * dfs_populate_80mhz_available_channels()- Populate channels for 80MHz using
 *                                          bitmap
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @avail_freq_list: prepared channel list
 *
 * Prepare 80MHz channels from the bitmap.
 *
 * Return: channel count
 */
#ifdef CONFIG_CHAN_NUM_API
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
#endif

/*
 * dfs_populate_80mhz_available_channel_for_freq() - Populate 80MHZ channels
 * available for selection.
 * @dfs: Pointer to wlan_dfs.
 * @bitmap: Pointer to bonding channel bitmap.
 * @avail_freq_list: Pointer to frequency list of available channels.
 */
#ifdef CONFIG_CHAN_FREQ_API
static uint8_t dfs_populate_80mhz_available_channel_for_freq(
		struct wlan_dfs *dfs,
		struct chan_bonding_bitmap *bitmap,
		uint16_t *avail_freq_list)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint16_t start_chan_freq = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan_freq = bitmap->chan_bonding_set[i].start_chan_freq;
		if (bitmap->chan_bonding_set[i].chan_map ==
			DFS_80MHZ_MASK) {
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 0);
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 1);
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 2);
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 3);
		}
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
		 "channel count %d", chnl_count);

	return chnl_count;
}
#endif

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
#ifdef CONFIG_CHAN_NUM_API
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
#endif

#ifdef CONFIG_CHAN_FREQ_API
static uint8_t
dfs_populate_40mhz_available_channel_for_freq(struct wlan_dfs *dfs,
					      struct chan_bonding_bitmap *bmap,
					      uint16_t *avail_freq_list)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint16_t start_chan_freq = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan_freq = bmap->chan_bonding_set[i].start_chan_freq;
		if ((bmap->chan_bonding_set[i].chan_map &
			DFS_40MHZ_MASK_L) == DFS_40MHZ_MASK_L) {
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 0);
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 1);
		}
		if ((bmap->chan_bonding_set[i].chan_map &
			DFS_40MHZ_MASK_H) == DFS_40MHZ_MASK_H) {
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 2);
			avail_freq_list[chnl_count++] = start_chan_freq +
				(DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET * 3);
		}
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
		 "channel count %d", chnl_count);

	return chnl_count;
}
#endif

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
#ifdef CONFIG_CHAN_NUM_API
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
#endif

/**
 * dfs_populate_available_channel_for_freq()- Populate channels based on width
 * and bitmap.
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @chan_width: channel width
 * @avail_freq_list: prepared channel list
 *
 * Prepare channel list based on width and channel bitmap.
 *
 * Return: channel count
 */
#ifdef CONFIG_CHAN_FREQ_API
static uint8_t
dfs_populate_available_channel_for_freq(struct wlan_dfs *dfs,
					struct chan_bonding_bitmap *bitmap,
					uint8_t chan_width,
					uint16_t *freq_list)
{
	switch (chan_width) {
	case DFS_CH_WIDTH_160MHZ:
	case DFS_CH_WIDTH_80P80MHZ:
	case DFS_CH_WIDTH_80MHZ:
		return dfs_populate_80mhz_available_channel_for_freq(dfs,
								     bitmap,
								     freq_list);
	case DFS_CH_WIDTH_40MHZ:
		return dfs_populate_40mhz_available_channel_for_freq(dfs,
								     bitmap,
								     freq_list);
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			"Invalid chan_width %d", chan_width);
		break;
	}

	return 0;
}
#endif

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
#ifdef CONFIG_CHAN_NUM_API
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
#endif

/**
 * dfs_get_rand_from_lst_for_freq()- Get random channel from a given channel
 * list.
 * @dfs: Pointer to DFS structure.
 * @freq_lst: Frequency list
 * @num_chan: number of channels
 *
 * Get random channel from given channel list.
 *
 * Return: channel frequency.
 */

#ifdef CONFIG_CHAN_FREQ_API
static uint16_t dfs_get_rand_from_lst_for_freq(struct wlan_dfs *dfs,
					       uint16_t *freq_lst,
					       uint8_t num_chan)
{
	uint8_t i;
	uint32_t rand_byte = 0;

	if (!num_chan || !freq_lst) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"invalid param freq_lst %pK, num_chan = %d",
			freq_lst, num_chan);
		return 0;
	}

	get_random_bytes((uint8_t *)&rand_byte, 1);
	i = (rand_byte + qdf_mc_timer_get_system_ticks()) % num_chan;

	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
		 "random channel %d", freq_lst[i]);

	return freq_lst[i];
}
#endif

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
#ifdef CONFIG_CHAN_NUM_API
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
#endif

/**
 * dfs_random_channel_sel_set_bitmap()- Set channel bit in bitmap based
 * on given channel number
 * @dfs: Pointer to DFS structure.
 * @bitmap: bitmap
 * @chan_freq: channel frequency
 *
 * Set channel bit in bitmap based on given channel frequency.
 *
 * Return: None
 */
#ifdef CONFIG_CHAN_FREQ_API
#define FREQUENCY_BAND_LIMIT 60
static void
dfs_random_channel_sel_set_bitmap_for_freq(struct wlan_dfs *dfs,
					   struct chan_bonding_bitmap *bitmap,
					   uint16_t chan_freq)
{
	int i = 0;
	int start_chan_freq = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan_freq = bitmap->chan_bonding_set[i].start_chan_freq;
		if (chan_freq >= start_chan_freq &&
		    chan_freq <= start_chan_freq +
		    FREQUENCY_BAND_LIMIT) {
			bitmap->chan_bonding_set[i].chan_map |=
				(1 << ((chan_freq - start_chan_freq) /
				       DFS_80_NUM_SUB_CHANNEL_FREQ));
			return;
		}
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
		  "Frequency=%d is not in the bitmap", chan_freq);
}
#endif

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
#ifdef CONFIG_CHAN_NUM_API
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
	uint8_t final_lst[NUM_CHANNELS] = {0};

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
#endif

#ifdef CONFIG_BAND_6GHZ
/**
 * dfs_assign_6g_channels()- Assign the center frequency of the first 20 MHZ
 * channel in every 80MHz channel, present in the 6G band.
 * @ch_map: Pointer to ch_map.
 *
 * Return: Void
 */
static void dfs_assign_6g_channels(struct  chan_bonding_bitmap *ch_map)
{
	ch_map->chan_bonding_set[6].start_chan_freq = 5955;
	ch_map->chan_bonding_set[7].start_chan_freq = 6035;
	ch_map->chan_bonding_set[8].start_chan_freq = 6115;
	ch_map->chan_bonding_set[9].start_chan_freq = 6195;
	ch_map->chan_bonding_set[10].start_chan_freq = 6275;
	ch_map->chan_bonding_set[11].start_chan_freq = 6355;
	ch_map->chan_bonding_set[12].start_chan_freq = 6435;
	ch_map->chan_bonding_set[13].start_chan_freq = 6515;
	ch_map->chan_bonding_set[14].start_chan_freq = 6595;
	ch_map->chan_bonding_set[15].start_chan_freq = 6675;
	ch_map->chan_bonding_set[16].start_chan_freq = 6755;
	ch_map->chan_bonding_set[17].start_chan_freq = 6835;
	ch_map->chan_bonding_set[18].start_chan_freq = 6915;
	ch_map->chan_bonding_set[19].start_chan_freq = 6995;
}
#else
static inline void dfs_assign_6g_channels(struct  chan_bonding_bitmap *ch_map)
{
}
#endif

/**
 * dfs_find_ch_with_fallback_for_freq()- find random channel
 * @dfs: Pointer to DFS structure.
 * @chan_wd: channel width
 * @center_freq_seg1: center frequency of secondary segment.
 * @freq_lst: list of available frequency.
 * @num_chan: number of channels in the list.
 *
 * Find random channel based on given channel width and channel list,
 * fallback to lower width if requested channel width not available.
 *
 * Return: channel frequency.
 */
#ifdef CONFIG_CHAN_FREQ_API
static uint16_t dfs_find_ch_with_fallback_for_freq(struct wlan_dfs *dfs,
						   uint8_t *chan_wd,
						   qdf_freq_t *center_freq_seg1,
						   uint16_t *freq_lst,
						   uint32_t num_chan)
{
	bool flag = false;
	uint32_t rand_byte = 0;
	struct  chan_bonding_bitmap ch_map = { { {0} } };
	uint8_t count = 0, i, index = 0, final_cnt = 0;
	uint16_t target_channel = 0;
	uint16_t primary_seg_start_ch = 0, sec_seg_ch = 0, new_160_start_ch = 0;
	uint16_t final_lst[NUM_CHANNELS] = {0};

	/* initialize ch_map for all 80 MHz bands: we have 6 80MHz bands */
	ch_map.chan_bonding_set[0].start_chan_freq = 5180;
	ch_map.chan_bonding_set[1].start_chan_freq = 5260;
	ch_map.chan_bonding_set[2].start_chan_freq = 5500;
	ch_map.chan_bonding_set[3].start_chan_freq = 5580;
	ch_map.chan_bonding_set[4].start_chan_freq = 5660;
	ch_map.chan_bonding_set[5].start_chan_freq = 5745;

	dfs_assign_6g_channels(&ch_map);
	for (i = 0; i < num_chan; i++) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			  "channel = %d added to bitmap", freq_lst[i]);
		dfs_random_channel_sel_set_bitmap_for_freq(dfs, &ch_map,
							   freq_lst[i]);
	}

	/* populate available channel list from bitmap */
	final_cnt = dfs_populate_available_channel_for_freq(dfs, &ch_map,
							    *chan_wd, final_lst);

	/* If no valid ch bonding found, fallback */
	if (final_cnt == 0) {
		if ((*chan_wd == DFS_CH_WIDTH_160MHZ) ||
		    (*chan_wd == DFS_CH_WIDTH_80P80MHZ) ||
		    (*chan_wd == DFS_CH_WIDTH_80MHZ)) {
			dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				 "from [%d] to 40Mhz", *chan_wd);
			*chan_wd = DFS_CH_WIDTH_40MHZ;
		} else if (*chan_wd == DFS_CH_WIDTH_40MHZ) {
			dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				 "from 40Mhz to 20MHz");
			*chan_wd = DFS_CH_WIDTH_20MHZ;
		}
		return 0;
	}

	/* ch count should be > 8 to switch new channel in 160Mhz band */
	if (((*chan_wd == DFS_CH_WIDTH_160MHZ) ||
	     (*chan_wd == DFS_CH_WIDTH_80P80MHZ)) &&
	     (final_cnt < DFS_MAX_20M_SUB_CH)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			 "from [%d] to 80Mhz", *chan_wd);
		*chan_wd = DFS_CH_WIDTH_80MHZ;
		return 0;
	}

	if (*chan_wd == DFS_CH_WIDTH_160MHZ) {
		/*
		 * Only 2 blocks for 160Mhz bandwidth i.e 36-64 & 100-128
		 * and all the channels in these blocks are continuous
		 * and separated by 4Mhz.
		 */
		for (i = 1; ((i < final_cnt)); i++) {
			if ((final_lst[i] - final_lst[i - 1]) ==
			     DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET)
				count++;
			else
				count = 0;
			if (count == DFS_MAX_20M_SUB_CH - 1) {
				flag = true;
				new_160_start_ch = final_lst[i - count];
				break;
			}
		}
	} else if (*chan_wd == DFS_CH_WIDTH_80P80MHZ) {
		flag = true;
	}

	if ((flag == false) && (*chan_wd > DFS_CH_WIDTH_80MHZ)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			 "from [%d] to 80Mhz", *chan_wd);
		*chan_wd = DFS_CH_WIDTH_80MHZ;
		return 0;
	}

	if (*chan_wd == DFS_CH_WIDTH_160MHZ) {
		get_random_bytes((uint8_t *)&rand_byte, 1);
		rand_byte = (rand_byte + qdf_mc_timer_get_system_ticks())
			% DFS_MAX_20M_SUB_CH;
		target_channel = new_160_start_ch + (rand_byte *
				DFS_80_NUM_SUB_CHANNEL_FREQ);
	} else if (*chan_wd == DFS_CH_WIDTH_80P80MHZ) {
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
			     (DFS_80P80M_FREQ_DIFF * 2))) {
				sec_seg_ch = final_lst[i *
					DFS_80_NUM_SUB_CHANNEL] +
					DFS_80MHZ_START_CENTER_CH_FREQ_DIFF;
				break;
			}
		}

		if (!sec_seg_ch && (final_cnt == DFS_MAX_20M_SUB_CH))
			*chan_wd = DFS_CH_WIDTH_160MHZ;
		else if (!sec_seg_ch)
			*chan_wd = DFS_CH_WIDTH_80MHZ;

		*center_freq_seg1 = sec_seg_ch;
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			 "Center frequency seg1 = %d", sec_seg_ch);
	} else {
		target_channel = dfs_get_rand_from_lst_for_freq(dfs,
								final_lst,
								final_cnt);
	}
	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
		 "target channel = %d", target_channel);

	return target_channel;
}
#endif

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
 * return: none
 */
#ifdef CONFIG_CHAN_NUM_API
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
	uint16_t j;

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
			for (j = 0; j < acs_info->num_of_channel; j++) {
				if (acs_info->chan_freq_list[j] ==
							     chan->dfs_ch_freq){
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
#endif

/**
 * dfs_apply_rules_for_freq()- prepare channel list based on flags
 * @dfs: dfs handler
 * @flags: channel flags
 * @random_chan_freq_list: output channel list
 * @random_chan_cnt: output channel count
 * @chan_list: input channel list
 * @chan_cnt: input channel count
 * @dfs_region: dfs region
 * @acs_info: acs channel range information
 *
 * prepare channel list based on flags
 *
 * return: none
 */
#ifdef CONFIG_CHAN_FREQ_API
static void dfs_apply_rules_for_freq(struct wlan_dfs *dfs,
				     uint32_t flags,
				     uint16_t *random_chan_freq_list,
				     uint32_t *random_chan_cnt,
				     struct dfs_channel *chan_list,
				     uint32_t chan_cnt,
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
	bool flag_no_6g_freq;
	int i;
	bool found = false;
	uint16_t j;
	uint16_t freq_list[NUM_CHANNELS_160MHZ];
	uint8_t num_channels = 0;

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
	flag_no_6g_freq = flags & DFS_RANDOM_CH_FLAG_NO_6GHZ_CH;

	if (flags & DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH) {
		num_channels =
			dfs_get_bonding_channel_without_seg_info_for_freq
			(dfs->dfs_curchan, freq_list);
	}

	for (i = 0; i < chan_cnt; i++) {
		chan = &chan_list[i];
		found = false;

		if ((chan->dfs_ch_ieee == 0) ||
		    (chan->dfs_ch_ieee > MAX_CHANNEL_NUM)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "invalid channel %d", chan->dfs_ch_ieee);
			continue;
		}

		if (flags & DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH) {
			for (j = 0; j < num_channels; j++) {
				if (chan->dfs_ch_freq == freq_list[j]) {
					dfs_debug(dfs,
						  WLAN_DEBUG_DFS_RANDOM_CHAN,
						  "skip %d current operating channel",
						  chan->dfs_ch_freq);
					found = true;
					break;
				}
			}

			if (found)
				continue;
		}

		if (acs_info && acs_info->acs_mode) {
			for (j = 0; j < acs_info->num_of_channel; j++) {
				if (acs_info->chan_freq_list[j] ==
				    chan->dfs_ch_freq) {
					found = true;
					break;
				}
			}

			if (!found) {
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					  "skip ch freq %d not in acs range",
					  chan->dfs_ch_freq);
				continue;
			}
			found = false;
		}

		if (flag_no_2g_chan &&
		    chan->dfs_ch_freq <= DFS_MAX_24GHZ_CHANNEL_FREQ) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip 2.4 GHz channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_5g_chan &&
		    WLAN_REG_IS_5GHZ_CH_FREQ(chan->dfs_ch_freq)) {
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
		    DFS_IS_CHAN_JAPAN_INDOOR_FREQ(chan->dfs_ch_freq)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip indoor channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_upper_5g &&
		    DFS_IS_CHAN_JAPAN_OUTDOOR_FREQ(chan->dfs_ch_freq)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip outdoor channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_6g_freq &&
		    WLAN_REG_IS_6GHZ_CHAN_FREQ(chan->dfs_ch_freq)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip 6 GHz channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_dfs_chan &&
		    (chan->dfs_ch_flagext & WLAN_CHAN_DFS)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip dfs channel=%d", chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_japan_w53 &&
		    DFS_IS_CHAN_JAPAN_W53_FREQ(chan->dfs_ch_freq)) {
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

		random_chan_freq_list[*random_chan_cnt] = chan->dfs_ch_freq;
		*random_chan_cnt += 1;
	}
}
#endif

#ifdef CONFIG_CHAN_NUM_API
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
#endif

#ifdef CONFIG_CHAN_FREQ_API
uint16_t dfs_prepare_random_channel_for_freq(struct wlan_dfs *dfs,
					     struct dfs_channel *chan_list,
					     uint32_t chan_cnt,
					     uint32_t flags,
					     struct ch_params *chan_params,
					     uint8_t dfs_region,
					     struct dfs_acs_info *acs_info)
{
	int i = 0;
	uint8_t final_cnt = 0;
	uint16_t target_freq = 0;
	uint16_t *random_chan_freq_list = NULL;
	uint32_t random_chan_cnt = 0;
	uint16_t flag_no_weather = 0;
	uint16_t *leakage_adjusted_lst;
	uint16_t final_lst[NUM_CHANNELS] = {0};
	uint8_t *chan_wd = (uint8_t *)&chan_params->ch_width;

	if (!chan_list || !chan_cnt) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			 "Invalid params %pK, chan_cnt=%d",
			 chan_list, chan_cnt);
		return 0;
	}

	if (*chan_wd < DFS_CH_WIDTH_20MHZ || *chan_wd > DFS_CH_WIDTH_80P80MHZ) {
		dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
			 "Invalid chan_wd %d", *chan_wd);
		return 0;
	}

	random_chan_freq_list =
	    qdf_mem_malloc(chan_cnt * sizeof(*random_chan_freq_list));
	if (!random_chan_freq_list)
		return 0;

	dfs_apply_rules_for_freq(dfs, flags, random_chan_freq_list,
				 &random_chan_cnt, chan_list, chan_cnt,
				 dfs_region, acs_info);
	flag_no_weather = (dfs_region == DFS_ETSI_REGION_VAL) ?
		flags & DFS_RANDOM_CH_FLAG_NO_WEATHER_CH : 0;

	/* list adjusted after leakage has been marked */
	leakage_adjusted_lst = qdf_mem_malloc(random_chan_cnt *
					      sizeof(*leakage_adjusted_lst));
	if (!leakage_adjusted_lst) {
		qdf_mem_free(random_chan_freq_list);
		return 0;
	}

	do {
		int ret;

		qdf_mem_copy(leakage_adjusted_lst, random_chan_freq_list,
			     random_chan_cnt * sizeof(*leakage_adjusted_lst));
		ret = dfs_mark_leaking_chan_for_freq(dfs, *chan_wd,
						   random_chan_cnt,
						   leakage_adjusted_lst);
		if (QDF_IS_STATUS_ERROR(ret)) {
			qdf_mem_free(random_chan_freq_list);
			qdf_mem_free(leakage_adjusted_lst);
			return 0;
		}

		if (*chan_wd == DFS_CH_WIDTH_20MHZ) {
			/*
			 * PASS: 3 - from leakage_adjusted_lst, prepare valid
			 * ch list and use random number from that
			 */
			for (i = 0; i < random_chan_cnt; i++) {
				if (leakage_adjusted_lst[i] == 0)
					continue;
				dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
					  "Channel=%d added to available list",
					  leakage_adjusted_lst[i]);
				final_lst[final_cnt] = leakage_adjusted_lst[i];
				final_cnt++;
			}
			target_freq = dfs_get_rand_from_lst_for_freq(dfs,
								     final_lst,
								     final_cnt);
			break;
		}
		target_freq = dfs_find_ch_with_fallback_for_freq(
				dfs, chan_wd, &chan_params->mhz_freq_seg1,
				leakage_adjusted_lst, random_chan_cnt);

		/* Since notion of 80+80 is not present in the regulatory
		 * channel the function may return invalid 80+80 channels for
		 * some devices (e.g. Pine). Therefore, check if we need to
		 * correct it by checking the following condition.
		 */
		if ((*chan_wd == DFS_CH_WIDTH_80P80MHZ) &&
		    (flags & DFS_RANDOM_CH_FLAG_RESTRICTED_80P80_ENABLED) &&
		    target_freq) {
			wlan_reg_set_channel_params_for_freq(dfs->dfs_pdev_obj,
							     target_freq,
							     0, chan_params);
			if (!(CHAN_WITHIN_RESTRICTED_80P80(
						chan_params->mhz_freq_seg0,
						chan_params->mhz_freq_seg1))) {
				*chan_wd = DFS_CH_WIDTH_160MHZ;
				target_freq =
				    dfs_find_ch_with_fallback_for_freq(
					    dfs, chan_wd,
					    &chan_params->mhz_freq_seg1,
					    leakage_adjusted_lst,
					    random_chan_cnt);
			}
		}

		/*
		 * When flag_no_weather is set, avoid usage of Adjacent
		 * weather radar channel in HT40 mode as extension channel
		 * will be on 5600.
		 */
		if (flag_no_weather &&
		    (target_freq ==
		     DFS_ADJACENT_WEATHER_RADAR_CHANNEL_FREQ) &&
		    (*chan_wd == DFS_CH_WIDTH_40MHZ)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN,
				  "skip weather adjacent ch=%d\n",
				  target_freq);
			continue;
		}

		if (target_freq)
			break;
	} while (true);

	qdf_mem_free(random_chan_freq_list);
	qdf_mem_free(leakage_adjusted_lst);
	dfs_info(dfs, WLAN_DEBUG_DFS_RANDOM_CHAN, "target_freq = %d",
		 target_freq);

	return target_freq;
}
#endif
