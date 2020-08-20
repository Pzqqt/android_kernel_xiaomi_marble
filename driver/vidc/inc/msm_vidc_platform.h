/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_PLATFORM_H_
#define _MSM_VIDC_PLATFORM_H_

#include <linux/platform_device.h>

#include "msm_vidc_internal.h"

struct msm_platform_core_capability {
	enum msm_vidc_core_capability_type type;
	u32 value;
};

struct msm_platform_inst_capability {
	enum msm_vidc_inst_capability_type cap;
	enum msm_vidc_domain_type domain;
	enum msm_vidc_codec_type codec;
	s32 min;
	s32 max;
	u32 step_or_mask;
	s32 value;
	enum msm_vidc_inst_capability_flags flags;
	u32 v4l2_id;
	u32 hfi_id;
	u8 parents[MAX_CAP_PARENTS];
	u8 children[MAX_CAP_CHILDREN];
	void (*adjust)(void *inst, s32 new_value);
	int (*set)(void *inst, struct v4l2_ctrl *ctrl);
};

struct msm_vidc_csc_coeff {
	u32 *vpe_csc_custom_matrix_coeff;
	u32 *vpe_csc_custom_bias_coeff;
	u32 *vpe_csc_custom_limit_coeff;
};

struct msm_vidc_efuse_data {
	u32 start_address;
	u32 size;
	u32 mask;
	u32 shift;
	enum efuse_purpose purpose;
};

struct msm_vidc_ubwc_config_data {
	struct {
		u32 max_channel_override : 1;
		u32 mal_length_override : 1;
		u32 hb_override : 1;
		u32 bank_swzl_level_override : 1;
		u32 bank_swz2_level_override : 1;
		u32 bank_swz3_level_override : 1;
		u32 bank_spreading_override : 1;
		u32 reserved : 27;
	} override_bit_info;

	u32 max_channels;
	u32 mal_length;
	u32 highest_bank_bit;
	u32 bank_swzl_level;
	u32 bank_swz2_level;
	u32 bank_swz3_level;
	u32 bank_spreading;
};

struct msm_vidc_platform_data {
	struct msm_platform_core_capability *core_data;
	u32 core_data_size;
	struct msm_platform_inst_capability *instance_data;
	u32 instance_data_size;
	struct allowed_clock_rates_table *allowed_clks_tbl;
	u32 allowed_clks_tbl_size;
	struct msm_vidc_csc_coeff csc_data;
	struct msm_vidc_ubwc_config_data *ubwc_config;
};

struct msm_vidc_platform {
	void *core;
	struct msm_vidc_platform_data data;
};

int msm_vidc_init_platform(struct platform_device *pdev);
int msm_vidc_deinit_platform(struct platform_device *pdev);

#endif // _MSM_VIDC_PLATFORM_H_
