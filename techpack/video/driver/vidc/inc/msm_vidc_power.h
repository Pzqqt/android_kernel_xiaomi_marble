/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_POWER_H_
#define _MSM_VIDC_POWER_H_

#include "fixedpoint.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_inst.h"

#define COMPRESSION_RATIO_MAX 5

/* TODO: Move to dtsi OR use source clock instead of branch clock.*/

enum vidc_bus_type {
	PERF,
	DDR,
	LLCC,
};

/*
 * Minimum dimensions for which to calculate bandwidth.
 * This means that anything bandwidth(0, 0) ==
 * bandwidth(BASELINE_DIMENSIONS.width, BASELINE_DIMENSIONS.height)
 */
static const struct {
	int height, width;
} BASELINE_DIMENSIONS = {
	.width = 1280,
	.height = 720,
};

/* converts Mbps to bps (the "b" part can be bits or bytes based on context) */
#define kbps(__mbps) ((__mbps) * 1000)
#define bps(__mbps) (kbps(__mbps) * 1000)

#define GENERATE_COMPRESSION_PROFILE(__bpp, __worst) {              \
	.bpp = __bpp,                                                          \
	.ratio = __worst,                \
}

/*
 * The below table is a structural representation of the following table:
 *  Resolution |    Bitrate |              Compression Ratio          |
 * ............|............|.........................................|
 * Width Height|Average High|Avg_8bpc Worst_8bpc Avg_10bpc Worst_10bpc|
 *  1280    720|      7   14|    1.69       1.28      1.49        1.23|
 *  1920   1080|     20   40|    1.69       1.28      1.49        1.23|
 *  2560   1440|     32   64|     2.2       1.26      1.97        1.22|
 *  3840   2160|     42   84|     2.2       1.26      1.97        1.22|
 *  4096   2160|     44   88|     2.2       1.26      1.97        1.22|
 *  4096   2304|     48   96|     2.2       1.26      1.97        1.22|
 */
static struct lut {
	int frame_size; /* width x height */
	int frame_rate;
	unsigned long bitrate;
	struct {
		int bpp;
		fp_t ratio;
	} compression_ratio[COMPRESSION_RATIO_MAX];
} const LUT[] = {
	{
		.frame_size = 1280 * 720,
		.frame_rate = 30,
		.bitrate = 14,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 28, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 23, 100)),
		}
	},
	{
		.frame_size = 1280 * 720,
		.frame_rate = 60,
		.bitrate = 22,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 28, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 23, 100)),
		}
	},
	{
		.frame_size = 1920 * 1088,
		.frame_rate = 30,
		.bitrate = 40,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 28, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 23, 100)),
		}
	},
	{
		.frame_size = 1920 * 1088,
		.frame_rate = 60,
		.bitrate = 64,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 28, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 23, 100)),
		}
	},
	{
		.frame_size = 2560 * 1440,
		.frame_rate = 30,
		.bitrate = 64,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 2560 * 1440,
		.frame_rate = 60,
		.bitrate = 102,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 3840 * 2160,
		.frame_rate = 30,
		.bitrate = 84,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 3840 * 2160,
		.frame_rate = 60,
		.bitrate = 134,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 4096 * 2160,
		.frame_rate = 30,
		.bitrate = 88,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 4096 * 2160,
		.frame_rate = 60,
		.bitrate = 141,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 4096 * 2304,
		.frame_rate = 30,
		.bitrate = 96,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
	{
		.frame_size = 4096 * 2304,
		.frame_rate = 60,
		.bitrate = 154,
		.compression_ratio = {
			GENERATE_COMPRESSION_PROFILE(8,
					FP(1, 26, 100)),
			GENERATE_COMPRESSION_PROFILE(10,
					FP(1, 22, 100)),
		}
	},
};

static inline u32 get_type_frm_name(const char* name)
{
	if (!strcmp(name, "venus-llcc"))
		return LLCC;
	else if (!strcmp(name, "venus-ddr"))
		return DDR;
	else
		return PERF;
}

#define DUMP_HEADER_MAGIC 0xdeadbeef
#define DUMP_FP_FMT "%FP" /* special format for fp_t */

struct dump {
	char* key;
	char* format;
	size_t val;
};

struct lut const* __lut(int width, int height, int fps);
fp_t __compression_ratio(struct lut const* entry, int bpp);
void __dump(struct dump dump[], int len);

static inline bool __ubwc(enum msm_vidc_colorformat_type f)
{
	switch (f) {
	case MSM_VIDC_FMT_NV12C:
	case MSM_VIDC_FMT_TP10C:
		return true;
	default:
		return false;
	}
}

static inline int __bpp(enum msm_vidc_colorformat_type f)
{
	switch (f) {
	case MSM_VIDC_FMT_NV12:
	case MSM_VIDC_FMT_NV21:
	case MSM_VIDC_FMT_NV12C:
	case MSM_VIDC_FMT_RGBA8888C:
		return 8;
	case MSM_VIDC_FMT_P010:
	case MSM_VIDC_FMT_TP10C:
		return 10;
	default:
		d_vpr_e("Unsupported colorformat (%x)", f);
		return INT_MAX;
	}
}

u64 msm_vidc_max_freq(struct msm_vidc_inst* inst);
int msm_vidc_get_inst_load(struct msm_vidc_inst *inst);
int msm_vidc_get_mbps(struct msm_vidc_inst *inst);
int msm_vidc_scale_power(struct msm_vidc_inst *inst, bool scale_buses);
void msm_vidc_power_data_reset(struct msm_vidc_inst *inst);
#endif
