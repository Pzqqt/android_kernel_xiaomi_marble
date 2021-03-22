/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */
#include "lpass-cdc-comp.h"

int lpass_cdc_load_compander_coeff(struct snd_soc_component *component,
				   u16 lsb_reg, u16 msb_reg,
				   struct comp_coeff_val *comp_coeff_table,
				   u16 arr_size)
{
	int i = 0;

	/* Load Compander Coeff */
	for (i = 0; i < arr_size; i++) {
		snd_soc_component_write(component, lsb_reg,
				comp_coeff_table[i].lsb);
		snd_soc_component_write(component, msb_reg,
				comp_coeff_table[i].msb);
	}

	return 0;
}
EXPORT_SYMBOL(lpass_cdc_load_compander_coeff);

int lpass_cdc_update_compander_setting(struct snd_soc_component *component,
				u16 start_addr,
				struct lpass_cdc_comp_setting *comp_setting)
{
	int zone2_rms, zone3_rms, zone4_rms, zone5_rms, zone6_rms;
	int path_gain;
	int max_attn;
	int zone1_rms = 6;
	int upper_gain_int = comp_setting->upper_gain_int;
	int lower_gain_int = comp_setting->lower_gain_int;
	int ana_addr_map = comp_setting->ana_addr_map;
	int upper_gain_dig_int = upper_gain_int - lower_gain_int;

	/* skip comp_ctl8, comp_ctl9 default settings is fine */

	/* apply zone settings */
	snd_soc_component_write(component,
				start_addr + 8,
				zone1_rms);

	if (upper_gain_dig_int >= 24)
		zone2_rms = 18;
	else if (upper_gain_dig_int >= 18)
		zone2_rms = 12;
	else
		zone2_rms = upper_gain_dig_int;
	snd_soc_component_write(component,
				start_addr + 0xC,
				zone2_rms);

	if (upper_gain_dig_int >= 66)
		zone3_rms = 33;
	else if (upper_gain_dig_int >= 36)
		zone3_rms = 30;
	else if (upper_gain_dig_int >= 30)
		zone3_rms = 24;
	else
		zone3_rms = upper_gain_dig_int;
	snd_soc_component_write(component,
				start_addr + 0x10,
				zone3_rms);

	if (upper_gain_dig_int >= 66)
		zone4_rms = 48;
	else if (upper_gain_dig_int >= 48)
		zone4_rms = 42;
	else if (upper_gain_dig_int >= 42)
		zone4_rms = 36;
	else
		zone4_rms = upper_gain_dig_int;
	snd_soc_component_write(component,
				start_addr + 0x14,
				zone4_rms);

	if (upper_gain_dig_int >= 69)
		zone5_rms = 63;
	else if (upper_gain_dig_int >= 66)
		zone5_rms = 60;
	else if (upper_gain_dig_int >= 60)
		zone5_rms = 54;
	else if (upper_gain_dig_int >= 54)
		zone5_rms = 48;
	else
		zone5_rms = upper_gain_dig_int;
	snd_soc_component_write(component,
				start_addr + 0x18,
				zone5_rms);

	zone6_rms = upper_gain_dig_int;
	snd_soc_component_write(component,
				start_addr + 0x1C,
				zone6_rms);

	if (lower_gain_int < 0)
		max_attn = 256 + lower_gain_int;
	else
		max_attn = lower_gain_int;
	snd_soc_component_write(component,
				start_addr + 0x20,
				max_attn);

	path_gain = zone6_rms - abs(lower_gain_int);
	snd_soc_component_write(component,
				start_addr + 0x24,
				path_gain);

	snd_soc_component_write(component,
				start_addr + 0x28,
				ana_addr_map);

	return 0;
}
EXPORT_SYMBOL(lpass_cdc_update_compander_setting);
