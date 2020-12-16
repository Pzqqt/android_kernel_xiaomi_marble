// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_power.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "venus_hfi.h"

int msm_vidc_scale_power(struct msm_vidc_inst *inst)
{
	int rc = 0;
	u64 freq;
	struct msm_vidc_core* core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return -EINVAL;
	}
	core = inst->core;

	freq = call_session_op(core, calc_freq, inst);
	rc = venus_hfi_scale_clocks(inst, freq);
	if (rc)
		return rc;

	freq = call_session_op(core, calc_bw, inst);
	rc = venus_hfi_scale_buses(inst, freq);
	if (rc)
		return rc;

	return 0;
}
