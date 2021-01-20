// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_debug.h"

int msm_vidc_debug = VIDC_HIGH | VIDC_PKT | VIDC_ERR | VIDC_PRINTK |
	FW_ERROR | FW_FATAL | FW_FTRACE | FW_LOW | FW_MED | FW_HIGH |
	FW_PERF | FW_PRINTK;
EXPORT_SYMBOL(msm_vidc_debug);

bool msm_vidc_lossless_encode = !true;
EXPORT_SYMBOL(msm_vidc_lossless_encode);

bool msm_vidc_syscache_disable = !true;
EXPORT_SYMBOL(msm_vidc_syscache_disable);

int msm_vidc_clock_voting = !1;

const char *level_str(u32 level)
{
	if (level & VIDC_ERR)
		return "err ";
	else if (level & VIDC_HIGH)
		return "high";
	else if (level & VIDC_LOW)
		return "low ";
	else if (level & VIDC_PERF)
		return "perf";
	else if (level & VIDC_PKT)
		return "pkt ";
	else if (level & VIDC_BUS)
		return "bus ";
	else
		return "????";
}