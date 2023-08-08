/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _MSM_VIDC_PLATFORM_H_
#define _MSM_VIDC_PLATFORM_H_

#include <linux/platform_device.h>

#include "msm_vidc_internal.h"
#include <media/v4l2-ctrls.h>

struct msm_vidc_core;

#define DDR_TYPE_LPDDR4 0x6
#define DDR_TYPE_LPDDR4X 0x7
#define DDR_TYPE_LPDDR5 0x8
#define DDR_TYPE_LPDDR5X 0x9

#define UBWC_CONFIG(mc, ml, hbb, bs1, bs2, bs3, bsp) \
{	                                                 \
	.max_channels = mc,                              \
	.mal_length = ml,                                \
	.highest_bank_bit = hbb,                         \
	.bank_swzl_level = bs1,                          \
	.bank_swz2_level = bs2,                          \
	.bank_swz3_level = bs3,                          \
	.bank_spreading = bsp,                           \
}

#define EFUSE_ENTRY(sa, s, m, sh, p) \
{	                                 \
	.start_address = sa,             \
	.size = s,                       \
	.mask = m,                       \
	.shift = sh,                     \
	.purpose = p                     \
}

extern u32 vpe_csc_custom_matrix_coeff[MAX_MATRIX_COEFFS];
extern u32 vpe_csc_custom_bias_coeff[MAX_BIAS_COEFFS];
extern u32 vpe_csc_custom_limit_coeff[MAX_LIMIT_COEFFS];

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
	u32 v4l2_id;
	u32 hfi_id;
	enum msm_vidc_inst_capability_flags flags;
	enum msm_vidc_inst_capability_type parents[MAX_CAP_PARENTS];
	enum msm_vidc_inst_capability_type children[MAX_CAP_CHILDREN];
	int (*adjust)(void *inst,
		struct v4l2_ctrl *ctrl);
	int (*set)(void *inst,
		enum msm_vidc_inst_capability_type cap_id);
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
	u32 max_channels;
	u32 mal_length;
	u32 highest_bank_bit;
	u32 bank_swzl_level;
	u32 bank_swz2_level;
	u32 bank_swz3_level;
	u32 bank_spreading;
};

enum vpu_version {
	VPU_VERSION_IRIS2 = 1,
	VPU_VERSION_IRIS2_2PIPE,
	VPU_VERSION_IRIS2_1PIPE,
	VENUS_VERSION_AR50LT_V1,
	VENUS_VERSION_AR50LT_V2,
};

struct msm_vidc_platform_data {
	struct msm_platform_core_capability *core_data;
	u32 core_data_size;
	struct msm_platform_inst_capability *instance_data;
	u32 instance_data_size;
	struct msm_vidc_csc_coeff csc_data;
	struct msm_vidc_ubwc_config_data *ubwc_config;
	u32 *bus_bw_nrt;
	struct msm_vidc_efuse_data *efuse_data;
	unsigned int efuse_data_size;
	unsigned int sku_version;
	unsigned int vpu_ver;
};

struct msm_vidc_platform {
	void *core;
	struct msm_vidc_platform_data data;
};

int msm_vidc_init_platform(struct platform_device *pdev);
int msm_vidc_deinit_platform(struct platform_device *pdev);
int msm_vidc_read_efuse(struct msm_vidc_core *core);
void msm_vidc_sort_table(struct msm_vidc_core *core);
void msm_vidc_ddr_ubwc_config(
	struct msm_vidc_platform_data *platform_data, u32 hbb_override_val);

#endif // _MSM_VIDC_PLATFORM_H_
