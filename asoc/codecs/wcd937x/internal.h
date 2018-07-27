/* Copyright (c) 2018, The Linux Foundation. All rights reserved.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _WCD937X_INTERNAL_H
#define _WCD937X_INTERNAL_H

#include "../wcd-mbhc-v2.h"

#define WCD937X_MAX_MICBIAS 3

extern struct regmap_config wcd937x_regmap_config;

struct wcd937x_priv {
	struct device *dev;

	int variant;
	struct snd_soc_codec *codec;
	struct device_node *rst_np;
	struct regmap *regmap;

	struct swr_device *rx_swr_dev;
	struct swr_device *tx_swr_dev;

	s32 micb_ref[WCD937X_MAX_MICBIAS];
	s32 pullup_ref[WCD937X_MAX_MICBIAS];

	struct fw_info *fw_data;
	struct device_node *wcd_rst_np;

	s32 dmic_0_1_clk_cnt;
	s32 dmic_2_3_clk_cnt;
	s32 dmic_4_5_clk_cnt;
	/* mbhc module */
	struct wcd_mbhc mbhc;
	struct blocking_notifier_head notifier;
	struct mutex micb_lock;

	u32 hph_mode;

	u32 rx_clk_cnt;
};

struct wcd937x_pdata {
	struct device_node *rst_np;
	struct device_node *rx_slave;
	struct device_node *tx_slave;
};

#endif
