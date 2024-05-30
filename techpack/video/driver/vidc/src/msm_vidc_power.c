// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "msm_vidc_power.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_buffer.h"
#include "venus_hfi.h"
#include "msm_vidc_events.h"

/* Q16 Format */
#define MSM_VIDC_MIN_UBWC_COMPLEXITY_FACTOR (1 << 16)
#define MSM_VIDC_MAX_UBWC_COMPLEXITY_FACTOR (4 << 16)
#define MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO (1 << 16)
#define MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO (5 << 16)

u64 msm_vidc_max_freq(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core* core;
	struct allowed_clock_rates_table *allowed_clks_tbl;
	u64 freq = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return freq;
	}
	core = inst->core;
	if (!core->dt || !core->dt->allowed_clks_tbl) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return freq;
	}
	allowed_clks_tbl = core->dt->allowed_clks_tbl;
	freq = allowed_clks_tbl[0].clock_rate;

	i_vpr_l(inst, "%s: rate = %llu\n", __func__, freq);
	return freq;
}

int msm_vidc_get_mbps(struct msm_vidc_inst *inst)
{
	u32 mbpf, fps;

	mbpf = msm_vidc_get_mbs_per_frame(inst);
	fps = msm_vidc_get_fps(inst);

	return mbpf * fps;
}

int msm_vidc_get_inst_load(struct msm_vidc_inst *inst)
{
	int load = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	/*
	 * NRT sessions - clock scaling is based on OPP table.
	 *              - No load based rejection.
	 * RT sessions  - clock scaling and session admission based on load.
	 */
	if (is_thumbnail_session(inst) || !is_realtime_session(inst))
		load = 0;
	else
		load = msm_vidc_get_mbps(inst);

	return load;
}

static int fill_dynamic_stats(struct msm_vidc_inst *inst,
	struct vidc_bus_vote_data *vote_data)
{
	struct msm_vidc_input_cr_data *temp, *next;
	u32 cf = MSM_VIDC_MAX_UBWC_COMPLEXITY_FACTOR;
	u32 cr = MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO;
	u32 input_cr = MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO;
	u32 frame_size;

	if (inst->power.fw_cr)
		cr = inst->power.fw_cr;

	if (inst->power.fw_cf) {
		cf = inst->power.fw_cf;
		frame_size = (msm_vidc_get_mbs_per_frame(inst) / (32 * 8) * 3) / 2;
		if (frame_size)
			cf = cf / frame_size;
	}

	list_for_each_entry_safe(temp, next, &inst->enc_input_crs, list)
		input_cr = min(input_cr, temp->input_cr);

	vote_data->compression_ratio = cr;
	vote_data->complexity_factor = cf;
	vote_data->input_cr = input_cr;

	/* Sanitize CF values from HW */
	cf = clamp_t(u32, cf, MSM_VIDC_MIN_UBWC_COMPLEXITY_FACTOR,
			MSM_VIDC_MAX_UBWC_COMPLEXITY_FACTOR);
	cr = clamp_t(u32, cr, MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO,
			MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO);
	input_cr = clamp_t(u32, input_cr, MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO,
			MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO);

	vote_data->compression_ratio = cr;
	vote_data->complexity_factor = cf;
	vote_data->input_cr = input_cr;

	i_vpr_l(inst,
		"Input CR = %d Recon CR = %d Complexity Factor = %d\n",
		vote_data->input_cr, vote_data->compression_ratio,
		vote_data->complexity_factor);

	return 0;
}

static int msm_vidc_set_buses(struct msm_vidc_inst* inst)
{
	int rc = 0;
	struct msm_vidc_core* core;
	struct msm_vidc_inst* temp;
	u64 total_bw_ddr = 0, total_bw_llcc = 0;
	u64 curr_time_ns;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core = inst->core;
	if (!core || !core->platform || !core->platform->data.bus_bw_nrt) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	curr_time_ns = ktime_get_ns();
	list_for_each_entry(temp, &core->instances, list) {
		/* skip for session where no input is there to process */
		if (!temp->max_input_data_size)
			continue;

		/* skip inactive session bus bandwidth */
		if (!is_active_session(temp->last_qbuf_time_ns, curr_time_ns)) {
			temp->active = false;
			continue;
		}

		if (temp->power.power_mode == VIDC_POWER_TURBO) {
			total_bw_ddr = total_bw_llcc = INT_MAX;
			break;
		}

		if (!is_realtime_session(inst)) {
			temp->power.ddr_bw = core->platform->data.bus_bw_nrt[0];
			temp->power.sys_cache_bw = core->platform->data.bus_bw_nrt[0];
		}

		total_bw_ddr += temp->power.ddr_bw;
		total_bw_llcc += temp->power.sys_cache_bw;
	}
	mutex_unlock(&core->lock);

	if (msm_vidc_ddr_bw) {
		d_vpr_l("msm_vidc_ddr_bw %d\n", msm_vidc_ddr_bw);
		total_bw_ddr = msm_vidc_ddr_bw;
	}

	if (msm_vidc_llc_bw) {
		d_vpr_l("msm_vidc_llc_bw %d\n", msm_vidc_llc_bw);
		total_bw_llcc = msm_vidc_llc_bw;
	}

	rc = venus_hfi_scale_buses(inst, total_bw_ddr, total_bw_llcc);
	if (rc)
		return rc;

	return 0;
}

int msm_vidc_scale_buses(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct vidc_bus_vote_data *vote_data;
	struct v4l2_format *out_f;
	struct v4l2_format *inp_f;
	int codec = 0, frame_rate, buf_ts_fps;
	struct msm_vidc_power *power;
	int initial_window = 0;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params: %pK\n", __func__, inst);
		return -EINVAL;
	}
	core = inst->core;
	power = &inst->power;
	if (!core->dt) {
		i_vpr_e(inst, "%s: invalid dt params\n", __func__);
		return -EINVAL;
	}
	vote_data = &inst->bus_data;

	vote_data->power_mode = VIDC_POWER_NORMAL;
	initial_window = power->nom_threshold ? power->nom_threshold : DCVS_WINDOW;
	if (inst->power.buffer_counter < min(initial_window, DCVS_WINDOW) ||
		is_image_session(inst))
		vote_data->power_mode = VIDC_POWER_TURBO;

	if (vote_data->power_mode == VIDC_POWER_TURBO)
		goto set_buses;

	out_f = &inst->fmts[OUTPUT_PORT];
	inp_f = &inst->fmts[INPUT_PORT];
	switch (inst->domain) {
	case MSM_VIDC_DECODER:
		codec = inp_f->fmt.pix_mp.pixelformat;
		break;
	case MSM_VIDC_ENCODER:
		codec = out_f->fmt.pix_mp.pixelformat;
		break;
	default:
		i_vpr_e(inst, "%s: invalid session_type %#x\n",
			__func__, inst->domain);
		break;
	}

	frame_rate = inst->capabilities->cap[FRAME_RATE].value;
	vote_data->codec = inst->codec;
	vote_data->input_width = inp_f->fmt.pix_mp.width;
	vote_data->input_height = inp_f->fmt.pix_mp.height;
	vote_data->output_width = out_f->fmt.pix_mp.width;
	vote_data->output_height = out_f->fmt.pix_mp.height;
	vote_data->lcu_size = (codec == V4L2_PIX_FMT_HEVC ||
			codec == V4L2_PIX_FMT_VP9) ? 32 : 16;
	vote_data->fps = msm_vidc_get_fps(inst);
	buf_ts_fps = msm_vidc_calc_window_avg_framerate(inst);
	if (buf_ts_fps > vote_data->fps) {
		i_vpr_l(inst, "%s: bitstream: fps %d, client rate %u\n", __func__,
			buf_ts_fps, vote_data->fps);
		vote_data->fps = buf_ts_fps;
	}

	if (inst->domain == MSM_VIDC_ENCODER) {
		vote_data->domain = MSM_VIDC_ENCODER;
		vote_data->bitrate = inst->capabilities->cap[BIT_RATE].value;
		vote_data->rotation = inst->capabilities->cap[ROTATION].value;
		vote_data->b_frames_enabled =
			inst->capabilities->cap[B_FRAME].value > 0;
		/* scale bitrate if operating rate is larger than fps */
		if (vote_data->fps > (frame_rate >> 16) &&
			(frame_rate >> 16)) {
			vote_data->bitrate = vote_data->bitrate /
				(frame_rate >> 16) * vote_data->fps;
		}
		vote_data->num_formats = 1;
		vote_data->color_formats[0] = v4l2_colorformat_to_driver(
			inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat, __func__);
	} else if (inst->domain == MSM_VIDC_DECODER) {
		u32 color_format;

		vote_data->domain = MSM_VIDC_DECODER;
		vote_data->bitrate = inst->max_input_data_size * vote_data->fps * 8;
		color_format = v4l2_colorformat_to_driver(
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, __func__);
		if (is_linear_colorformat(color_format)) {
			vote_data->num_formats = 2;
			/*
			 * 0 index - dpb colorformat
			 * 1 index - opb colorformat
			 */
			if (is_10bit_colorformat(color_format)) {
				vote_data->color_formats[0] = MSM_VIDC_FMT_TP10C;
			} else {
				vote_data->color_formats[0] = MSM_VIDC_FMT_NV12;
			}
			vote_data->color_formats[1] = color_format;
		} else {
			vote_data->num_formats = 1;
			vote_data->color_formats[0] = color_format;
		}
	}
	vote_data->work_mode = inst->capabilities->cap[STAGE].value;
	if (core->dt->sys_cache_res_set)
		vote_data->use_sys_cache = true;
	vote_data->num_vpp_pipes = core->capabilities[NUM_VPP_PIPE].value;
	fill_dynamic_stats(inst, vote_data);

	call_session_op(core, calc_bw, inst, vote_data);

	inst->power.ddr_bw = vote_data->calc_bw_ddr;
	inst->power.sys_cache_bw = vote_data->calc_bw_llcc;

set_buses:
	inst->power.power_mode = vote_data->power_mode;
	rc = msm_vidc_set_buses(inst);
	if (rc)
		return rc;

	return 0;
}

int msm_vidc_set_clocks(struct msm_vidc_inst* inst)
{
	int rc = 0;
	struct msm_vidc_core* core;
	struct msm_vidc_inst* temp;
	u64 freq;
	u64 rate = 0;
	bool increment, decrement;
	u64 curr_time_ns;
	int i = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	if (!core->dt || !core->dt->allowed_clks_tbl) {
		d_vpr_e("%s: invalid dt params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	increment = false;
	decrement = true;
	freq = 0;
	curr_time_ns = ktime_get_ns();
	list_for_each_entry(temp, &core->instances, list) {
		/* skip for session where no input is there to process */
		if (!temp->max_input_data_size)
			continue;

		/* skip inactive session clock rate */
		if (!is_active_session(temp->last_qbuf_time_ns, curr_time_ns)) {
			temp->active = false;
			continue;
		}
		freq += temp->power.min_freq;

		if (msm_vidc_clock_voting) {
			d_vpr_l("msm_vidc_clock_voting %d\n", msm_vidc_clock_voting);
			freq = msm_vidc_clock_voting;
			decrement = false;
			break;
		}
		/* increment even if one session requested for it */
		if (temp->power.dcvs_flags & MSM_VIDC_DCVS_INCR)
			increment = true;
		/* decrement only if all sessions requested for it */
		if (!(temp->power.dcvs_flags & MSM_VIDC_DCVS_DECR))
			decrement = false;
	}

	/*
	 * keep checking from lowest to highest rate until
	 * table rate >= requested rate
	 */
	for (i = core->dt->allowed_clks_tbl_size - 1; i >= 0; i--) {
		rate = core->dt->allowed_clks_tbl[i].clock_rate;
		if (rate >= freq)
			break;
	}
	if (i < 0)
		i = 0;
	if (increment) {
		if (i > 0)
			rate = core->dt->allowed_clks_tbl[i - 1].clock_rate;
	} else if (decrement) {
		if (i < (int) (core->dt->allowed_clks_tbl_size - 1))
			rate = core->dt->allowed_clks_tbl[i + 1].clock_rate;
	}
	core->power.clk_freq = (u32)rate;

	i_vpr_p(inst, "%s: clock rate %llu requested %llu increment %d decrement %d\n",
		__func__, rate, freq, increment, decrement);
	mutex_unlock(&core->lock);

	rc = venus_hfi_scale_clocks(inst, rate);
	if (rc)
		return rc;

	return 0;
}

static int msm_vidc_apply_dcvs(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int bufs_with_fw = 0;
	struct msm_vidc_power *power;

	if (!inst) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return -EINVAL;
	}

	/* skip dcvs */
	if (!inst->power.dcvs_mode)
		return 0;

	power = &inst->power;

	if (is_decode_session(inst)) {
		bufs_with_fw = msm_vidc_num_buffers(inst,
			MSM_VIDC_BUF_OUTPUT, MSM_VIDC_ATTR_QUEUED);
	} else {
		bufs_with_fw = msm_vidc_num_buffers(inst,
			MSM_VIDC_BUF_INPUT, MSM_VIDC_ATTR_QUEUED);
	}

	/* +1 as one buffer is going to be queued after the function */
	bufs_with_fw += 1;

	/*
	 * DCVS decides clock level based on below algorithm
	 *
	 * Limits :
	 * min_threshold : Buffers required for reference by FW.
	 * nom_threshold : Midpoint of Min and Max thresholds
	 * max_threshold : Min Threshold + DCVS extra buffers, allocated
	 *				   for smooth flow.
	 * 1) When buffers outside FW are reaching client's extra buffers,
	 *    FW is slow and will impact pipeline, Increase clock.
	 * 2) When pending buffers with FW are less than FW requested,
	 *    pipeline has cushion to absorb FW slowness, Decrease clocks.
	 * 3) When DCVS has engaged(Inc or Dec):
	 *    For decode:
	 *        - Pending buffers with FW transitions past the nom_threshold,
	 *        switch to calculated load, this smoothens the clock transitions.
	 *    For encode:
	 *        - Always switch to calculated load.
	 * 4) Otherwise maintain previous Load config.
	 */
	if (bufs_with_fw >= power->max_threshold) {
		power->dcvs_flags = MSM_VIDC_DCVS_INCR;
		goto exit;
	} else if (bufs_with_fw < power->min_threshold) {
		power->dcvs_flags = MSM_VIDC_DCVS_DECR;
		goto exit;
	}

	/* encoder: dcvs window handling */
	if (is_encode_session(inst)) {
		power->dcvs_flags = 0;
		goto exit;
	}

	/* decoder: dcvs window handling */
	if ((power->dcvs_flags & MSM_VIDC_DCVS_DECR && bufs_with_fw >= power->nom_threshold) ||
		(power->dcvs_flags & MSM_VIDC_DCVS_INCR && bufs_with_fw <= power->nom_threshold)) {
		power->dcvs_flags = 0;
	}

exit:
	i_vpr_p(inst, "dcvs: bufs_with_fw %d th[%d %d %d] flags %#x\n",
		bufs_with_fw, power->min_threshold,
		power->nom_threshold, power->max_threshold,
		power->dcvs_flags);

	return rc;
}

int msm_vidc_scale_clocks(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	struct msm_vidc_power *power;
	int initial_window = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	power = &inst->power;
	initial_window = power->nom_threshold ? power->nom_threshold : DCVS_WINDOW;

	if (inst->power.buffer_counter < min(initial_window, DCVS_WINDOW) ||
		is_image_session(inst)
		|| inst->state == MSM_VIDC_DRAIN
		|| inst->state == MSM_VIDC_DRC
		|| inst->state == MSM_VIDC_DRC_DRAIN) {
		inst->power.min_freq = msm_vidc_max_freq(inst);
		inst->power.dcvs_flags = 0;
	} else if (msm_vidc_clock_voting) {
		inst->power.min_freq = msm_vidc_clock_voting;
		inst->power.dcvs_flags = 0;
	} else {
		inst->power.min_freq =
			call_session_op(core, calc_freq, inst, inst->max_input_data_size);
		msm_vidc_apply_dcvs(inst);
	}
	inst->power.curr_freq = inst->power.min_freq;
	msm_vidc_set_clocks(inst);

	return 0;
}

int msm_vidc_scale_power(struct msm_vidc_inst *inst, bool scale_buses)
{
	struct msm_vidc_core *core;
	struct msm_vidc_buffer *vbuf;
	u32 data_size = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return -EINVAL;
	}
	core = inst->core;

	if (!inst->active) {
		/* scale buses for inactive -> active session */
		scale_buses = true;
		inst->active = true;
	}

	list_for_each_entry(vbuf, &inst->buffers.input.list, list)
		data_size = max(data_size, vbuf->data_size);

	mutex_lock(&core->lock);
	inst->max_input_data_size = data_size;
	mutex_unlock(&core->lock);

	/* no pending inputs - skip scale power */
	if (!inst->max_input_data_size)
		return 0;

	if (msm_vidc_scale_clocks(inst))
		i_vpr_e(inst, "failed to scale clock\n");

	if (scale_buses) {
		if (msm_vidc_scale_buses(inst))
			i_vpr_e(inst, "failed to scale bus\n");
	}

	i_vpr_hp(inst,
		"power: inst: clk %lld ddr %d llcc %d dcvs flags %#x, core: clk %lld ddr %lld llcc %lld\n",
		inst->power.curr_freq, inst->power.ddr_bw,
		inst->power.sys_cache_bw, inst->power.dcvs_flags,
		core->power.clk_freq, core->power.bw_ddr,
		core->power.bw_llcc);

	trace_msm_vidc_perf_power_scale(inst, core->power.clk_freq,
		core->power.bw_ddr, core->power.bw_llcc);

	return 0;
}

void msm_vidc_dcvs_data_reset(struct msm_vidc_inst *inst)
{
	struct msm_vidc_power *dcvs;
	u32 min_count, actual_count, max_count;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	dcvs = &inst->power;
	if (is_encode_session(inst)) {
		min_count = inst->buffers.input.min_count;
		actual_count = inst->buffers.input.actual_count;
		max_count = min((min_count + DCVS_ENC_EXTRA_INPUT_BUFFERS), actual_count);
	} else if (is_decode_session(inst)) {
		min_count = inst->buffers.output.min_count;
		actual_count = inst->buffers.output.actual_count;
		max_count = min((min_count + DCVS_DEC_EXTRA_OUTPUT_BUFFERS), actual_count);
	} else {
		i_vpr_e(inst, "%s: invalid domain type %d\n",
			__func__, inst->domain);
		return;
	}

	dcvs->min_threshold = min_count;
	dcvs->max_threshold = max_count;
	dcvs->dcvs_window = min_count < max_count ? max_count - min_count : 0;
	dcvs->nom_threshold = dcvs->min_threshold + (dcvs->dcvs_window / 2);
	dcvs->dcvs_flags = 0;

	i_vpr_p(inst, "%s: dcvs: thresholds [%d %d %d] flags %#x\n",
		__func__, dcvs->min_threshold,
		dcvs->nom_threshold, dcvs->max_threshold,
		dcvs->dcvs_flags);
}

void msm_vidc_power_data_reset(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	i_vpr_hp(inst, "%s\n", __func__);

	msm_vidc_dcvs_data_reset(inst);

	inst->power.buffer_counter = 0;
	inst->power.fw_cr = 0;
	inst->power.fw_cf = INT_MAX;

	rc = msm_vidc_scale_power(inst, true);
	if (rc)
		i_vpr_e(inst, "%s: failed to scale power\n", __func__);
}
