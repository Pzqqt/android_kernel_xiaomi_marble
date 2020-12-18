/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */
#ifndef LPASS_CDC_COMP_H
#define LPASS_CDC_COMP_H

#include <sound/soc.h>

struct comp_coeff_val {
	u8 lsb;
	u8 msb;
};

struct lpass_cdc_comp_setting {
	int upper_gain_int;
	int lower_gain_int;
	int ana_addr_map;
};

int lpass_cdc_load_compander_coeff(struct snd_soc_component *component,
				   u16 lsb_reg, u16 msb_reg,
				   struct comp_coeff_val *comp_coeff_table,
				   u16 arr_size);
int lpass_cdc_update_compander_setting(struct snd_soc_component *component,
				u16 start_addr,
				struct lpass_cdc_comp_setting *comp_setting);
#endif
