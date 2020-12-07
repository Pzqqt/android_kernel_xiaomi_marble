/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
				       u16 start_addr, u8 *reg_val)
{
	u16 i = 0;

	for (i = 0; i < COMP_MAX_SETTING; i++)
		snd_soc_component_write(component,
					start_addr + i * 4,
					reg_val[i]);

	return 0;
}
EXPORT_SYMBOL(lpass_cdc_update_compander_setting);
