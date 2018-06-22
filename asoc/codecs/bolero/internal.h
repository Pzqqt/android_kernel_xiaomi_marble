/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _BOLERO_INTERNAL_H
#define _BOLERO_INTERNAL_H

#include "bolero-cdc-registers.h"

enum {
	REG_NO_ACCESS,
	RD_REG,
	WR_REG,
	RD_WR_REG
};

struct bolero_priv {
	struct device *dev;
	struct snd_soc_codec *codec;
	struct regmap *regmap;
	struct mutex io_lock;
	struct mutex clk_lock;
	bool va_without_decimation;
	bool macros_supported[MAX_MACRO];
	struct macro_ops macro_params[MAX_MACRO];
	struct snd_soc_dai_driver *bolero_dais;
	u16 num_dais;
	u16 num_macros_registered;
	u16 child_num;
	u16 current_mclk_mux_macro[MAX_MACRO];
	struct work_struct bolero_add_child_devices_work;
	u32 version;

	/* Entry for version info */
	struct snd_info_entry *entry;
	struct snd_info_entry *version_entry;

	int (*read_dev)(struct bolero_priv *priv,
			u16 macro_id, u16 reg, u8 *val);
	int (*write_dev)(struct bolero_priv *priv,
			 u16 macro_id, u16 reg, u8 val);
};

struct regmap *bolero_regmap_init(struct device *dev,
				  const struct regmap_config *config);
int bolero_get_macro_id(bool va_no_dec_flag, u16 reg);

extern const struct regmap_config bolero_regmap_config;
extern u8 *bolero_reg_access[MAX_MACRO];
extern u8 bolero_va_top_reg_access[BOLERO_CDC_VA_MACRO_TOP_MAX];
extern const u16 macro_id_base_offset[MAX_MACRO];

#endif
