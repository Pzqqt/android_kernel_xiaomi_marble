// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

/* Q16 Format */
#define MSM_VIDC_MIN_UBWC_COMPLEXITY_FACTOR (1 << 16)
#define MSM_VIDC_MAX_UBWC_COMPLEXITY_FACTOR (4 << 16)
#define MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO (1 << 16)
#define MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO (5 << 16)

/* TODO: Move to dtsi OR use source clock instead of branch clock.*/
#define MSM_VIDC_CLOCK_SOURCE_SCALING_RATIO 3

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

	i_vpr_l(inst, "%s: rate = %lu\n", __func__, freq);
	return freq;
}

static int msm_vidc_get_mbps(struct msm_vidc_inst *inst,
		enum load_calc_quirks quirks)
{
	int input_port_mbs, output_port_mbs;
	int fps, operating_rate, frame_rate;
	struct v4l2_format *f;

	f = &inst->fmts[INPUT_PORT];
	input_port_mbs = NUM_MBS_PER_FRAME(f->fmt.pix_mp.width,
		f->fmt.pix_mp.height);

	f = &inst->fmts[OUTPUT_PORT];
	output_port_mbs = NUM_MBS_PER_FRAME(f->fmt.pix_mp.width,
		f->fmt.pix_mp.height);

	frame_rate = inst->capabilities->cap[FRAME_RATE].value;
	operating_rate = inst->capabilities->cap[OPERATING_RATE].value;

	fps = frame_rate;

	/* For admission control operating rate is ignored */
	if (quirks == LOAD_POWER)
		fps = max(operating_rate, frame_rate);

	/* In case of fps < 1 we assume 1 */
	fps = max(fps >> 16, 1);

	return max(input_port_mbs, output_port_mbs) * fps;
}

int msm_vidc_get_inst_load(struct msm_vidc_inst *inst,
		enum load_calc_quirks quirks)
{
	int load = 0;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_OPEN ||
		inst->state == MSM_VIDC_ERROR)
		goto exit;

	/*
	 * Clock and Load calculations for REALTIME/NON-REALTIME
	 * Operating rate will either Default or Client value.
	 * Session admission control will be based on Load.
	 * Power requests based of calculated Clock/Freq.
	 * ----------------|----------------------------|
	 * REALTIME        | Admission Control Load =   |
	 *                 |          res * fps         |
	 *                 | Power Request Load =       |
	 *                 |          res * max(op, fps)|
	 * ----------------|----------------------------|
	 * NON-REALTIME/   | Admission Control Load = 0 |
	 * THUMBNAIL/      | Power Request Load =       |
	 * IMAGE           |          res * max(op, fps)|
	 *                 |                            |
	 * ----------------|----------------------------|
	 */
	if (is_thumbnail_session(inst) || is_image_session(inst))
		goto exit;

	if (!is_realtime_session(inst) && quirks == LOAD_ADMISSION_CONTROL)
		goto exit;

	load = msm_vidc_get_mbps(inst, quirks);

exit:
	return load;
}

static int fill_dynamic_stats(struct msm_vidc_inst *inst,
	struct vidc_bus_vote_data *vote_data)
{
	struct msm_vidc_input_cr_data *temp, *next;
	u32 max_cr = MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO;
	u32 max_cf = MSM_VIDC_MIN_UBWC_COMPLEXITY_FACTOR;
	u32 max_input_cr = MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO;
	u32 min_cf = MSM_VIDC_MAX_UBWC_COMPLEXITY_FACTOR;
	u32 min_input_cr = MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO;
	u32 min_cr = MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO;

	/* TODO: get ubwc stats from firmware */
	min_cr = inst->power.fw_cr;
	max_cf = inst->power.fw_cf;
	max_cf = max_cf / ((msm_vidc_get_mbs_per_frame(inst)) / (32 * 8) * 3) / 2;

	list_for_each_entry_safe(temp, next, &inst->enc_input_crs, list) {
		min_input_cr = min(min_input_cr, temp->input_cr);
		max_input_cr = max(max_input_cr, temp->input_cr);
	}


	/* Sanitize CF values from HW */
	max_cf = min_t(u32, max_cf, MSM_VIDC_MAX_UBWC_COMPLEXITY_FACTOR);
	min_cf = max_t(u32, min_cf, MSM_VIDC_MIN_UBWC_COMPLEXITY_FACTOR);
	max_cr = min_t(u32, max_cr, MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO);
	min_cr = max_t(u32, min_cr, MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO);
	max_input_cr = min_t(u32,
		max_input_cr, MSM_VIDC_MAX_UBWC_COMPRESSION_RATIO);
	min_input_cr = max_t(u32,
		min_input_cr, MSM_VIDC_MIN_UBWC_COMPRESSION_RATIO);

	vote_data->compression_ratio = min_cr;
	vote_data->complexity_factor = max_cf;
	vote_data->input_cr = min_input_cr;

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

	mutex_lock(&core->lock);
	curr_time_ns = ktime_get_ns();
	list_for_each_entry(temp, &core->instances, list) {
		struct msm_vidc_buffer *vbuf, *next;
		u32 data_size = 0;

		/* TODO: accessing temp without lock */
		list_for_each_entry_safe(vbuf, next, &temp->buffers.input.list, list)
			data_size = max(data_size, vbuf->data_size);
		if (!data_size)
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
		total_bw_ddr += temp->power.ddr_bw;
		total_bw_llcc += temp->power.sys_cache_bw;
	}
	mutex_unlock(&core->lock);

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
	struct msm_vidc_buffer *vbuf;
	u32 data_size = 0;
	int codec = 0, frame_rate;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params: %pK\n", __func__, inst);
		return -EINVAL;
	}
	core = inst->core;
	if (!core->dt) {
		i_vpr_e(inst, "%s: invalid dt params\n", __func__);
		return -EINVAL;
	}
	vote_data = &inst->bus_data;

	list_for_each_entry(vbuf, &inst->buffers.input.list, list)
		data_size = max(data_size, vbuf->data_size);
	if (!data_size)
		return 0;

	vote_data->power_mode = VIDC_POWER_NORMAL;
	if (inst->power.buffer_counter < DCVS_FTB_WINDOW || is_image_session(inst))
		vote_data->power_mode = VIDC_POWER_TURBO;
	if (msm_vidc_clock_voting)
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
		vote_data->bitrate = data_size * vote_data->fps * 8;
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
			vote_data->color_formats[0] = color_format;
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
	u32 data_size;
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
		struct msm_vidc_buffer* vbuf, *next;

		data_size = 0;
		list_for_each_entry_safe(vbuf, next, &temp->buffers.input.list, list)
			data_size = max(data_size, vbuf->data_size);
		if (!data_size)
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

	i_vpr_p(inst, "%s: clock rate %lu requested %lu increment %d decrement %d\n",
		__func__, rate, freq, increment, decrement);
	mutex_unlock(&core->lock);

	/*
	 * This conversion is necessary since we are scaling clock values based on
	 * the branch clock. However, mmrm driver expects source clock to be registered
	 * and used for scaling.
	 * TODO: Remove this scaling if using source clock instead of branch clock.
	 */
	rate = rate * MSM_VIDC_CLOCK_SOURCE_SCALING_RATIO;
	i_vpr_h(inst, "%s: scaled clock rate %lu\n", __func__, rate);

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

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return -EINVAL;
	}

	if (!inst->power.dcvs_mode || inst->decode_batch.enable || is_image_session(inst)) {
		i_vpr_l(inst, "Skip DCVS (dcvs %d, batching %d)\n",
			inst->power.dcvs_mode, inst->decode_batch.enable);
		inst->power.dcvs_flags = 0;
		return 0;
	}
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
	 * 3) When DCVS has engaged(Inc or Dec) and pending buffers with FW
	 *    transitions past the nom_threshold, switch to calculated load.
	 *    This smoothens the clock transitions.
	 * 4) Otherwise maintain previous Load config.
	 */
	if (bufs_with_fw >= power->max_threshold) {
		power->dcvs_flags = MSM_VIDC_DCVS_INCR;
	} else if (bufs_with_fw < power->min_threshold) {
		power->dcvs_flags = MSM_VIDC_DCVS_DECR;
	} else if ((power->dcvs_flags & MSM_VIDC_DCVS_DECR &&
			   bufs_with_fw >= power->nom_threshold) ||
			   (power->dcvs_flags & MSM_VIDC_DCVS_INCR &&
			   bufs_with_fw <= power->nom_threshold))
		power->dcvs_flags = 0;

	i_vpr_p(inst, "DCVS: bufs_with_fw %d th[%d %d %d] flags %#x\n",
		bufs_with_fw, power->min_threshold,
		power->nom_threshold, power->max_threshold,
		power->dcvs_flags);

	return rc;
}

int msm_vidc_scale_clocks(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core* core;
	struct msm_vidc_buffer *vbuf;
	u32 data_size = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	list_for_each_entry(vbuf, &inst->buffers.input.list, list)
		data_size = max(data_size, vbuf->data_size);
	if (!data_size)
		return 0;

	//todo: add turbo session check
	if (inst->power.buffer_counter < DCVS_FTB_WINDOW || is_image_session(inst)) {
		inst->power.min_freq = msm_vidc_max_freq(inst);
		inst->power.dcvs_flags = 0;
	} else if (msm_vidc_clock_voting) {
		inst->power.min_freq = msm_vidc_clock_voting;
		inst->power.dcvs_flags = 0;
	} else {
		inst->power.min_freq =
			call_session_op(core, calc_freq, inst, data_size);
		msm_vidc_apply_dcvs(inst);
	}
	inst->power.curr_freq = inst->power.min_freq;
	msm_vidc_set_clocks(inst);

	return 0;
}

int msm_vidc_scale_power(struct msm_vidc_inst *inst, bool scale_buses)
{
	struct msm_vidc_core *core;

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

	if (msm_vidc_scale_clocks(inst))
		i_vpr_e(inst, "failed to scale clock\n");

	if (scale_buses) {
		if (msm_vidc_scale_buses(inst))
			i_vpr_e(inst, "failed to scale bus\n");
	}

	i_vpr_h(inst,
		"power: inst: clk %lld ddr %d llcc %d dcvs flags %#x, core: clk %lld ddr %lld llcc %lld\n",
		inst->power.curr_freq, inst->power.ddr_bw,
		inst->power.sys_cache_bw, inst->power.dcvs_flags,
		core->power.clk_freq, core->power.bw_ddr,
		core->power.bw_llcc);
	return 0;
}

void msm_vidc_dcvs_data_reset(struct msm_vidc_inst *inst)
{
	struct msm_vidc_power *dcvs;
	u32 min_count, actual_count;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	dcvs = &inst->power;
	if (inst->domain == MSM_VIDC_ENCODER) {
		min_count = inst->buffers.input.min_count;
		actual_count = inst->buffers.input.actual_count;
	} else if (inst->domain == MSM_VIDC_DECODER) {
		min_count = inst->buffers.output.min_count;
		actual_count = inst->buffers.output.actual_count;
	} else {
		i_vpr_e(inst, "%s: invalid domain type %d\n",
			__func__, inst->domain);
		return;
	}

	dcvs->min_threshold = min_count;
	if (inst->domain == MSM_VIDC_ENCODER)
		dcvs->max_threshold = min((min_count + DCVS_ENC_EXTRA_INPUT_BUFFERS),
								  actual_count);
	else
		dcvs->max_threshold = min((min_count + DCVS_DEC_EXTRA_OUTPUT_BUFFERS),
								  actual_count);

	dcvs->dcvs_window =
		dcvs->max_threshold < dcvs->min_threshold ? 0 :
		dcvs->max_threshold - dcvs->min_threshold;
	dcvs->nom_threshold = dcvs->min_threshold +
		(dcvs->dcvs_window ?
		(dcvs->dcvs_window / 2) : 0);

	dcvs->dcvs_flags = 0;

	i_vpr_p(inst, "%s: DCVS: thresholds [%d %d %d] flags %#x\n",
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
	i_vpr_h(inst, "%s\n", __func__);

	msm_vidc_dcvs_data_reset(inst);

	inst->power.buffer_counter = 0;
	inst->power.fw_cr = 0;
	inst->power.fw_cf = INT_MAX;

	rc = msm_vidc_scale_power(inst, true);
	if (rc)
		i_vpr_e(inst, "%s: failed to scale power\n", __func__);
}
