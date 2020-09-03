/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __MMRM_DEBUG__
#define __MMRM_DEBUG__

#include <linux/debugfs.h>
#include <linux/printk.h>

#ifndef MMRM_DBG_LABEL
#define MMRM_DBG_LABEL "msm_mmrm: "
#endif

//#define d_mpr_h(__fmt, ...) pr_info(MMRM_DBG_LABEL __fmt, ##__VA_ARGS__)
#define d_mpr_h(__fmt, ...) pr_err(MMRM_DBG_LABEL __fmt, ##__VA_ARGS__)
#define d_mpr_e(__fmt, ...) pr_err(MMRM_DBG_LABEL __fmt, ##__VA_ARGS__)

#endif
