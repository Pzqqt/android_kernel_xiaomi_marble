// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include "msm_vidc_power_ar50lt.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_dt.h"

u64 msm_vidc_calc_freq_ar50lt(struct msm_vidc_inst *inst, u32 data_size)
{
	u64 freq = 0, vpp_cycles = 0, vsp_cycles = 0;
	u64 fw_cycles = 0, fw_vpp_cycles = 0;
	u32 vpp_cycles_per_mb;
	u32 mbs_per_second, mbpf;
	struct msm_vidc_core *core = NULL;
	int i = 0;
	struct allowed_clock_rates_table *allowed_clks_tbl = NULL;
	u64 rate = 0, fps;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return freq;
	}

	core = inst->core;
	if (!core->dt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return freq;
	}
	//dcvs = &inst->clk_data;

	mbpf = msm_vidc_get_mbs_per_frame(inst);
	fps = msm_vidc_get_fps(inst);
	mbs_per_second = mbpf * fps;

	/*
	 * Calculate vpp, vsp cycles separately for encoder and decoder.
	 * Even though, most part is common now, in future it may change
	 * between them.
	 */

	fw_cycles = fps * inst->capabilities->cap[MB_CYCLES_FW].value;
	fw_vpp_cycles = fps * inst->capabilities->cap[MB_CYCLES_FW_VPP].value;

	if (inst->domain == MSM_VIDC_ENCODER) {
		vpp_cycles_per_mb = is_low_power_session(inst) ?
			inst->capabilities->cap[MB_CYCLES_LP].value :
			inst->capabilities->cap[MB_CYCLES_VPP].value;

		vpp_cycles = mbs_per_second * vpp_cycles_per_mb;
		/* 21 / 20 is minimum overhead factor */
		vpp_cycles += max(vpp_cycles / 20, fw_vpp_cycles);

		vsp_cycles = mbs_per_second *  inst->capabilities->cap[MB_CYCLES_VSP].value;

		/* 10 / 7 is overhead factor */
		vsp_cycles += (inst->capabilities->cap[BIT_RATE].value * 10) / 7;
	} else if (inst->domain == MSM_VIDC_DECODER) {
		vpp_cycles = mbs_per_second * inst->capabilities->cap[MB_CYCLES_VPP].value;
		/* 21 / 20 is minimum overhead factor */
		vpp_cycles += max(vpp_cycles / 20, fw_vpp_cycles);

		vsp_cycles = mbs_per_second * inst->capabilities->cap[MB_CYCLES_VSP].value;
		/* 10 / 7 is overhead factor */
		vsp_cycles += div_u64((fps * data_size * 8 * 10), 7);

	} else {
		i_vpr_e(inst, "%s: Unknown session type\n", __func__);
		return msm_vidc_max_freq(inst);
	}

	freq = max(vpp_cycles, vsp_cycles);
	freq = max(freq, fw_cycles);

	allowed_clks_tbl = core->dt->allowed_clks_tbl;
	for (i = core->dt->allowed_clks_tbl_size - 1; i >= 0; i--) {
		rate =  core->dt->allowed_clks_tbl[i].clock_rate;
		if (rate >= freq)
			break;
	}

	if (i < 0)
		i = 0;

	i_vpr_p(inst, "%s: filled len %d, required freq %llu, fps %u, mbpf %u\n",
		__func__, data_size, freq, fps, mbpf);

	return (unsigned long) freq;
}
static u64 __calculate_encoder(struct vidc_bus_vote_data *d)
{
	/* Encoder Parameters */
	int width, height, fps, bitrate, lcu_size, lcu_per_frame,
		collocated_bytes_per_lcu, search_range_v, search_range_h,
		vertical_tile_size, num_tiles;

	unsigned int bins_to_bit_factor;
	fp_t y_bw;
	bool is_h264_category = true;
	fp_t orig_read_factor, recon_write_factor,
		ref_y_read_factor, ref_c_read_factor, overhead_factor;

	/* Output parameters */
	fp_t orig_read, recon_write,
		ref_y_read, ref_c_read,
		bse_lb_read, bse_lb_write,
		collocated_read, collocated_write,
		bitstream_read, bitstream_write,
		total_read, total_write,
		total;

	unsigned long ret = 0;

	/* Encoder Fixed Parameters setup */
	search_range_h = 96;
	search_range_v = 48;
	bins_to_bit_factor = 4;
	overhead_factor = FP(1, 3, 100);
	orig_read_factor = FP(1, 50, 100); /* L + C */
	recon_write_factor = FP(1, 50, 100); /* L + C */
	ref_c_read_factor = FP(0, 75, 100); /* 1.5/2  ( 1.5 Cache efficiency )*/

	fps = d->fps;
	width = max(d->output_width, BASELINE_DIMENSIONS.width);
	height = max(d->output_height, BASELINE_DIMENSIONS.height);
	bitrate = d->bitrate > 0 ? (d->bitrate + 1000000 - 1) / 1000000 :
		__lut(width, height, fps)->bitrate;
	lcu_size = d->lcu_size;

	/* Derived Parameters Setup*/
	lcu_per_frame = DIV_ROUND_UP(width, lcu_size) *
		DIV_ROUND_UP(height, lcu_size);

	if (d->codec == MSM_VIDC_HEVC ||
		d->codec == MSM_VIDC_VP9) {
		/* H264, VP8, MPEG2 use the same settings */
		/* HEVC, VP9 use the same setting */
		is_h264_category = false;
	}

	collocated_bytes_per_lcu = lcu_size == 16 ? 16 :
		lcu_size == 32 ? 64 : 256;

	if (width >= 1296 && width <= 1536)
		vertical_tile_size = 768;
	else
		vertical_tile_size = 640;

	num_tiles = DIV_ROUND_UP(width, vertical_tile_size);
	y_bw = fp_mult(fp_mult(FP_INT(width), FP_INT(height)), FP_INT(fps));
	y_bw = fp_div(y_bw, FP_INT(bps(1)));

	/* -1 for 1 less tile boundary penalty */
	ref_y_read_factor = (num_tiles - 1) * 2;
	ref_y_read_factor = fp_div(fp_mult(FP_INT(ref_y_read_factor),
		FP_INT(search_range_h)), FP_INT(width));
	ref_y_read_factor = ref_y_read_factor + FP_INT(1);

	orig_read = fp_mult(y_bw, orig_read_factor);
	recon_write = fp_mult(y_bw, recon_write_factor);
	ref_y_read = fp_mult(y_bw, ref_y_read_factor);
	ref_c_read = fp_mult(y_bw, ref_c_read_factor);

	bse_lb_read = fp_div(FP_INT(16 * fps * lcu_per_frame),
		FP_INT(bps(1)));
	bse_lb_write = bse_lb_read;

	collocated_read = fp_div(FP_INT(lcu_per_frame *
		collocated_bytes_per_lcu * fps), FP_INT(bps(1)));
	collocated_write = collocated_read;

	bitstream_read = fp_mult(fp_div(FP_INT(bitrate), FP_INT(8)),
		FP_INT(bins_to_bit_factor));
	bitstream_write = fp_div(FP_INT(bitrate), FP_INT(8));
	bitstream_write = bitstream_write + bitstream_read;

	total_read = orig_read + ref_y_read + ref_c_read +
		bse_lb_read + collocated_read + bitstream_read;
	total_write = recon_write + bse_lb_write +
		collocated_write + bitstream_write;

	total = total_read + total_write;
	total = fp_mult(total, overhead_factor);

	if (msm_vidc_debug & VIDC_BUS) {
		struct dump dump[] = {
		{"ENCODER PARAMETERS", "", DUMP_HEADER_MAGIC},
		{"width", "%d", width},
		{"height", "%d", height},
		{"fps", "%d", fps},
		{"bitrate (Mbit/sec)", "%lu", bitrate},
		{"lcu size", "%d", lcu_size},
		{"collocated byter per lcu", "%d", collocated_bytes_per_lcu},
		{"horizontal search range", "%d", search_range_h},
		{"vertical search range", "%d", search_range_v},
		{"bins to bit factor", "%d", bins_to_bit_factor},

		{"DERIVED PARAMETERS", "", DUMP_HEADER_MAGIC},
		{"lcu/frame", "%d", lcu_per_frame},
		{"vertical tile size", "%d", vertical_tile_size},
		{"number of tiles", "%d", num_tiles},
		{"Y BW", DUMP_FP_FMT, y_bw},

		{"original read factor", DUMP_FP_FMT, orig_read_factor},
		{"recon write factor", DUMP_FP_FMT, recon_write_factor},
		{"ref read Y factor", DUMP_FP_FMT, ref_y_read_factor},
		{"ref read C factor", DUMP_FP_FMT, ref_c_read_factor},
		{"overhead_factor", DUMP_FP_FMT, overhead_factor},

		{"INTERMEDIATE B/W DDR", "", DUMP_HEADER_MAGIC},
		{"orig read", DUMP_FP_FMT, orig_read},
		{"recon write", DUMP_FP_FMT, recon_write},
		{"ref read Y", DUMP_FP_FMT, ref_y_read},
		{"ref read C", DUMP_FP_FMT, ref_c_read},
		{"BSE lb read", DUMP_FP_FMT, bse_lb_read},
		{"BSE lb write", DUMP_FP_FMT, bse_lb_write},
		{"collocated read", DUMP_FP_FMT, collocated_read},
		{"collocated write", DUMP_FP_FMT, collocated_write},
		{"bitstream read", DUMP_FP_FMT, bitstream_read},
		{"bitstream write", DUMP_FP_FMT, bitstream_write},
		{"total read", DUMP_FP_FMT, total_read},
		{"total write", DUMP_FP_FMT, total_write},
		{"total", DUMP_FP_FMT, total},
		};
		__dump(dump, ARRAY_SIZE(dump));
	}


	d->calc_bw_ddr = kbps(fp_round(total));

	return ret;
}

static unsigned long __calculate_decoder(struct vidc_bus_vote_data *d)
{
	/* Decoder parameters */
	int width, height, fps, bitrate, lcu_size,
		lcu_per_frame, collocated_bytes_per_lcu,
		motion_complexity;

	unsigned int bins_to_bits_factor, vsp_read_factor;
	fp_t y_bw;
	bool is_h264_category = true;
	fp_t recon_write_factor, ref_read_factor,
		opb_factor, overhead_factor;

	/* Output parameters */
	fp_t opb_write, recon_write,
		ref_read,
		bse_lb_read, bse_lb_write,
		collocated_read, collocated_write,
		bitstream_read, bitstream_write,
		total_read, total_write,
		total;

	unsigned long ret = 0;

	/* Decoder Fixed Parameters */
	overhead_factor = FP(1, 3, 100);
	recon_write_factor = FP(1, 50, 100); /* L + C */
	opb_factor = FP(1, 50, 100); /* L + C */
	motion_complexity = 5; /* worst case complexity */
	bins_to_bits_factor = 4;
	vsp_read_factor = 6;

	fps = d->fps;
	width = max(d->output_width, BASELINE_DIMENSIONS.width);
	height = max(d->output_height, BASELINE_DIMENSIONS.height);
	bitrate = d->bitrate > 0 ? (d->bitrate + 1000000 - 1) / 1000000 :
		__lut(width, height, fps)->bitrate;
	lcu_size = d->lcu_size;

	/* Derived Parameters Setup*/
	lcu_per_frame = DIV_ROUND_UP(width, lcu_size) *
		DIV_ROUND_UP(height, lcu_size);

	if (d->codec == MSM_VIDC_HEVC ||
		d->codec == MSM_VIDC_VP9) {
		/* H264, VP8, MPEG2 use the same settings */
		/* HEVC, VP9 use the same setting */
		is_h264_category = false;
	}

	collocated_bytes_per_lcu = lcu_size == 16 ? 16 :
		lcu_size == 32 ? 64 : 256;

	y_bw = fp_mult(fp_mult(FP_INT(width), FP_INT(height)), FP_INT(fps));
	y_bw = fp_div(y_bw, FP_INT(bps(1)));

	ref_read_factor = FP(1, 50, 100); /* L + C */
	ref_read_factor = fp_mult(ref_read_factor, FP_INT(motion_complexity));

	recon_write = fp_mult(y_bw, recon_write_factor);
	ref_read = fp_mult(y_bw, ref_read_factor);

	if (d->codec == MSM_VIDC_HEVC)
		bse_lb_read = FP_INT(lcu_size == 32 ? 64 :
			lcu_size == 16 ? 32 : 128);
	else
		bse_lb_read = FP_INT(128);
	bse_lb_read = fp_div(fp_mult(FP_INT(lcu_per_frame * fps), bse_lb_read),
		FP_INT(bps(1)));
	bse_lb_write = bse_lb_read;

	collocated_read = fp_div(FP_INT(lcu_per_frame *
		collocated_bytes_per_lcu * fps), FP_INT(bps(1)));
	collocated_write = collocated_read;

	bitstream_read = fp_mult(fp_div(FP_INT(bitrate), FP_INT(8)),
		FP_INT(vsp_read_factor));
	bitstream_write = fp_mult(fp_div(FP_INT(bitrate), FP_INT(8)),
		FP_INT(bins_to_bits_factor));

	opb_write = fp_mult(y_bw, opb_factor);

	total_read = ref_read + bse_lb_read + collocated_read +
		bitstream_read;
	total_write = recon_write + bse_lb_write + bitstream_write +
		opb_write;

	total = total_read + total_write;
	total = fp_mult(total, overhead_factor);

	if (msm_vidc_debug & VIDC_BUS) {
		struct dump dump[] = {
		{"DECODER PARAMETERS", "", DUMP_HEADER_MAGIC},
		{"width", "%d", width},
		{"height", "%d", height},
		{"fps", "%d", fps},
		{"bitrate (Mbit/sec)", "%lu", bitrate},
		{"lcu size", "%d", lcu_size},
		{"collocated byter per lcu", "%d", collocated_bytes_per_lcu},
		{"vsp read factor", "%d", vsp_read_factor},
		{"bins to bits factor", "%d", bins_to_bits_factor},
		{"motion complexity", "%d", motion_complexity},

		{"DERIVED PARAMETERS", "", DUMP_HEADER_MAGIC},
		{"lcu/frame", "%d", lcu_per_frame},
		{"Y BW", DUMP_FP_FMT, y_bw},
		{"recon write factor", DUMP_FP_FMT, recon_write_factor},
		{"ref_read_factor", DUMP_FP_FMT, ref_read_factor},
		{"opb factor", DUMP_FP_FMT, opb_factor},
		{"overhead_factor", DUMP_FP_FMT, overhead_factor},

		{"INTERMEDIATE B/W DDR", "", DUMP_HEADER_MAGIC},
		{"recon write", DUMP_FP_FMT, recon_write},
		{"ref read", DUMP_FP_FMT, ref_read},
		{"BSE lb read", DUMP_FP_FMT, bse_lb_read},
		{"BSE lb write", DUMP_FP_FMT, bse_lb_write},
		{"collocated read", DUMP_FP_FMT, collocated_read},
		{"collocated write", DUMP_FP_FMT, collocated_write},
		{"bitstream read", DUMP_FP_FMT, bitstream_read},
		{"bitstream write", DUMP_FP_FMT, bitstream_write},
		{"opb write", DUMP_FP_FMT, opb_write},
		{"total read", DUMP_FP_FMT, total_read},
		{"total write", DUMP_FP_FMT, total_write},
		{"total", DUMP_FP_FMT, total},
		};
		__dump(dump, ARRAY_SIZE(dump));
	}

	d->calc_bw_ddr = kbps(fp_round(total));

	return ret;
}
static u64 __calculate(struct msm_vidc_inst *inst, struct vidc_bus_vote_data *d)
{
	u64 value = 0;

	switch (d->domain) {
	case MSM_VIDC_ENCODER:
		value = __calculate_encoder(d);
		break;
	case MSM_VIDC_DECODER:
		value = __calculate_decoder(d);
		break;
	default:
		i_vpr_e(inst, "%s: Unknown Domain %#x", __func__, d->domain);
	}

	return value;
}

int msm_vidc_calc_bw_ar50lt(struct msm_vidc_inst *inst,
			struct vidc_bus_vote_data *vidc_data)
{
	int ret = 0;

	if (!vidc_data)
		return ret;

	ret = __calculate(inst, vidc_data);

	return ret;
}
