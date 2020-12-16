// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_power_iris2.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_debug.h"

u64 msm_vidc_calc_freq_iris2(struct msm_vidc_inst *inst)
{
	u64 freq = 0;

	/* 240 Mhz for iris2 based video hw */
	freq = 240 * 1000 * 1000;
	s_vpr_h(inst->sid, "%s: freq %lu\n", __func__, freq);

	return freq;
}

u64 msm_vidc_calc_bw_iris2(struct msm_vidc_inst *inst)
{
	u64 freq = 0;

	/* 600 Mhz for iris2 based video hw */
	freq = 600 * 1000 * 1000;
	s_vpr_h(inst->sid, "%s: freq %lu\n", __func__, freq);

	return freq;
}
