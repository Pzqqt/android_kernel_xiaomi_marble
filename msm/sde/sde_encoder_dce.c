// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 */

#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/sde_rsc.h>

#include "msm_drv.h"
#include "sde_kms.h"
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include "sde_hwio.h"
#include "sde_hw_catalog.h"
#include "sde_hw_intf.h"
#include "sde_hw_ctl.h"
#include "sde_formats.h"
#include "sde_encoder_phys.h"
#include "sde_power_handle.h"
#include "sde_hw_dsc.h"
#include "sde_crtc.h"
#include "sde_trace.h"
#include "sde_core_irq.h"

#define SDE_DEBUG_DCE(e, fmt, ...) SDE_DEBUG("enc%d " fmt,\
		(e) ? (e)->base.base.id : -1, ##__VA_ARGS__)

#define SDE_ERROR_DCE(e, fmt, ...) SDE_ERROR("enc%d " fmt,\
		(e) ? (e)->base.base.id : -1, ##__VA_ARGS__)

bool sde_encoder_is_dsc_merge(struct drm_encoder *drm_enc)
{
	enum sde_rm_topology_name topology;
	struct sde_encoder_virt *sde_enc;
	struct drm_connector *drm_conn;

	if (!drm_enc)
		return false;

	sde_enc = to_sde_encoder_virt(drm_enc);
	if (!sde_enc->cur_master)
		return false;

	drm_conn = sde_enc->cur_master->connector;
	if (!drm_conn)
		return false;

	topology = sde_connector_get_topology_name(drm_conn);
	if (topology == SDE_RM_TOPOLOGY_DUALPIPE_DSCMERGE)
		return true;

	return false;
}

static int _sde_encoder_dsc_update_pic_dim(struct msm_display_dsc_info *dsc,

		int pic_width, int pic_height)
{
	if (!dsc || !pic_width || !pic_height) {
		SDE_ERROR("invalid input: pic_width=%d pic_height=%d\n",
			pic_width, pic_height);
		return -EINVAL;
	}

	if ((pic_width % dsc->slice_width) ||
	    (pic_height % dsc->slice_height)) {
		SDE_ERROR("pic_dim=%dx%d has to be multiple of slice=%dx%d\n",
			pic_width, pic_height,
			dsc->slice_width, dsc->slice_height);
		return -EINVAL;
	}

	dsc->pic_width = pic_width;
	dsc->pic_height = pic_height;

	return 0;
}

static void _sde_encoder_dsc_pclk_param_calc(struct msm_display_dsc_info *dsc,
		int intf_width)
{
	int slice_per_pkt, slice_per_intf;
	int bytes_in_slice, total_bytes_per_intf;

	if (!dsc || !dsc->slice_width || !dsc->slice_per_pkt ||
	    (intf_width < dsc->slice_width)) {
		SDE_ERROR("invalid input: intf_width=%d slice_width=%d\n",
			intf_width, dsc ? dsc->slice_width : -1);
		return;
	}

	slice_per_pkt = dsc->slice_per_pkt;
	slice_per_intf = DIV_ROUND_UP(intf_width, dsc->slice_width);

	/*
	 * If slice_per_pkt is greater than slice_per_intf then default to 1.
	 * This can happen during partial update.
	 */
	if (slice_per_pkt > slice_per_intf)
		slice_per_pkt = 1;

	bytes_in_slice = DIV_ROUND_UP(dsc->slice_width * dsc->bpp, 8);
	total_bytes_per_intf = bytes_in_slice * slice_per_intf;

	dsc->eol_byte_num = total_bytes_per_intf % 3;
	dsc->pclk_per_line =  DIV_ROUND_UP(total_bytes_per_intf, 3);
	dsc->bytes_in_slice = bytes_in_slice;
	dsc->bytes_per_pkt = bytes_in_slice * slice_per_pkt;
	dsc->pkt_per_line = slice_per_intf / slice_per_pkt;
}

static int _sde_encoder_dsc_initial_line_calc(struct msm_display_dsc_info *dsc,
		int enc_ip_width)
{
	int max_ssm_delay, max_se_size, obuf_latency;
	int input_ssm_out_latency, base_hs_latency;
	int multi_hs_extra_latency,  mux_word_size;

	/* Hardent core config */
	int max_muxword_size = 48;
	int output_rate = 64;
	int rtl_max_bpc = 10;
	int pipeline_latency = 28;

	max_se_size = 4 * (rtl_max_bpc + 1);
	max_ssm_delay = max_se_size + max_muxword_size - 1;
	mux_word_size = (dsc->bpc >= 12 ? 64 : 48);
	input_ssm_out_latency = pipeline_latency + (3 * (max_ssm_delay + 2));
	obuf_latency = DIV_ROUND_UP((9 * output_rate +
				mux_word_size), dsc->bpp) + 1;
	base_hs_latency = dsc->initial_xmit_delay + input_ssm_out_latency
				+ obuf_latency;
	multi_hs_extra_latency = DIV_ROUND_UP((8 * dsc->chunk_size), dsc->bpp);
	dsc->initial_lines = DIV_ROUND_UP((base_hs_latency +
				multi_hs_extra_latency), dsc->slice_width);

	return 0;
}

static bool _sde_encoder_dsc_ich_reset_override_needed(bool pu_en,
		struct msm_display_dsc_info *dsc)
{
	/*
	 * As per the DSC spec, ICH_RESET can be either end of the slice line
	 * or at the end of the slice. HW internally generates ich_reset at
	 * end of the slice line if DSC_MERGE is used or encoder has two
	 * soft slices. However, if encoder has only 1 soft slice and DSC_MERGE
	 * is not used then it will generate ich_reset at the end of slice.
	 *
	 * Now as per the spec, during one PPS session, position where
	 * ich_reset is generated should not change. Now if full-screen frame
	 * has more than 1 soft slice then HW will automatically generate
	 * ich_reset at the end of slice_line. But for the same panel, if
	 * partial frame is enabled and only 1 encoder is used with 1 slice,
	 * then HW will generate ich_reset at end of the slice. This is a
	 * mismatch. Prevent this by overriding HW's decision.
	 */
	return pu_en && dsc && (dsc->full_frame_slices > 1) &&
		(dsc->slice_width == dsc->pic_width);
}

static void _sde_encoder_dsc_pipe_cfg(struct sde_hw_dsc *hw_dsc,
		struct sde_hw_pingpong *hw_pp, struct msm_display_dsc_info *dsc,
		u32 common_mode, bool ich_reset, bool enable,
		struct sde_hw_pingpong *hw_dsc_pp)
{
	if (!enable) {
		if (hw_dsc_pp && hw_dsc_pp->ops.disable_dsc)
			hw_dsc_pp->ops.disable_dsc(hw_dsc_pp);

		if (hw_dsc && hw_dsc->ops.dsc_disable)
			hw_dsc->ops.dsc_disable(hw_dsc);

		if (hw_dsc && hw_dsc->ops.bind_pingpong_blk)
			hw_dsc->ops.bind_pingpong_blk(hw_dsc, false,
					PINGPONG_MAX);
		return;
	}

	if (!dsc || !hw_dsc || !hw_pp || !hw_dsc_pp) {
		SDE_ERROR("invalid params %d %d %d %d\n", !dsc, !hw_dsc,
				!hw_pp, !hw_dsc_pp);
		return;
	}

	if (hw_dsc->ops.dsc_config)
		hw_dsc->ops.dsc_config(hw_dsc, dsc, common_mode, ich_reset);

	if (hw_dsc->ops.dsc_config_thresh)
		hw_dsc->ops.dsc_config_thresh(hw_dsc, dsc);

	if (hw_dsc_pp->ops.setup_dsc)
		hw_dsc_pp->ops.setup_dsc(hw_dsc_pp);

	if (hw_dsc->ops.bind_pingpong_blk)
		hw_dsc->ops.bind_pingpong_blk(hw_dsc, true, hw_pp->idx);

	if (hw_dsc_pp->ops.enable_dsc)
		hw_dsc_pp->ops.enable_dsc(hw_dsc_pp);
}

static int _sde_encoder_dsc_n_lm_1_enc_1_intf(struct sde_encoder_virt *sde_enc)
{
	int this_frame_slices;
	int intf_ip_w, enc_ip_w;
	int ich_res, dsc_common_mode = 0;

	struct sde_hw_pingpong *hw_pp = sde_enc->hw_pp[0];
	struct sde_hw_pingpong *hw_dsc_pp = sde_enc->hw_dsc_pp[0];
	struct sde_hw_dsc *hw_dsc = sde_enc->hw_dsc[0];
	struct sde_encoder_phys *enc_master = sde_enc->cur_master;
	const struct sde_rect *roi = &sde_enc->cur_conn_roi;
	struct msm_display_dsc_info *dsc = NULL;
	struct sde_hw_ctl *hw_ctl;
	struct sde_ctl_dsc_cfg cfg;

	if (hw_dsc == NULL || hw_pp == NULL || !enc_master) {
		SDE_ERROR_DCE(sde_enc, "invalid params for DSC\n");
		return -EINVAL;
	}


	hw_ctl = enc_master->hw_ctl;

	memset(&cfg, 0, sizeof(cfg));
	dsc = &sde_enc->mode_info.comp_info.dsc_info;
	_sde_encoder_dsc_update_pic_dim(dsc, roi->w, roi->h);

	this_frame_slices = roi->w / dsc->slice_width;
	intf_ip_w = this_frame_slices * dsc->slice_width;
	_sde_encoder_dsc_pclk_param_calc(dsc, intf_ip_w);

	enc_ip_w = intf_ip_w;
	_sde_encoder_dsc_initial_line_calc(dsc, enc_ip_w);

	ich_res = _sde_encoder_dsc_ich_reset_override_needed(false, dsc);

	if (enc_master->intf_mode == INTF_MODE_VIDEO)
		dsc_common_mode = DSC_MODE_VIDEO;

	SDE_DEBUG_DCE(sde_enc, "pic_w: %d pic_h: %d mode:%d\n",
		roi->w, roi->h, dsc_common_mode);
	SDE_EVT32(DRMID(&sde_enc->base), roi->w, roi->h, dsc_common_mode);

	_sde_encoder_dsc_pipe_cfg(hw_dsc, hw_pp, dsc, dsc_common_mode,
			ich_res, true, hw_dsc_pp);
	cfg.dsc[cfg.dsc_count++] = hw_dsc->idx;

	/* setup dsc active configuration in the control path */
	if (hw_ctl->ops.setup_dsc_cfg) {
		hw_ctl->ops.setup_dsc_cfg(hw_ctl, &cfg);
		SDE_DEBUG_DCE(sde_enc,
				"setup dsc_cfg hw_ctl[%d], count:%d,dsc[0]:%d, dsc[1]:%d\n",
				hw_ctl->idx,
				cfg.dsc_count,
				cfg.dsc[0],
				cfg.dsc[1]);
	}

	if (hw_ctl->ops.update_bitmask_dsc)
		hw_ctl->ops.update_bitmask_dsc(hw_ctl, hw_dsc->idx, 1);

	return 0;
}

static int _sde_encoder_dsc_2_lm_2_enc_2_intf(struct sde_encoder_virt *sde_enc,
		struct sde_encoder_kickoff_params *params)
{
	int this_frame_slices;
	int intf_ip_w, enc_ip_w;
	int ich_res, dsc_common_mode;

	struct sde_encoder_phys *enc_master = sde_enc->cur_master;
	const struct sde_rect *roi = &sde_enc->cur_conn_roi;
	struct sde_hw_dsc *hw_dsc[MAX_CHANNELS_PER_ENC];
	struct sde_hw_pingpong *hw_pp[MAX_CHANNELS_PER_ENC];
	struct sde_hw_pingpong *hw_dsc_pp[MAX_CHANNELS_PER_ENC];
	struct msm_display_dsc_info dsc[MAX_CHANNELS_PER_ENC];
	bool half_panel_partial_update;
	struct sde_hw_ctl *hw_ctl = enc_master->hw_ctl;
	struct sde_ctl_dsc_cfg cfg;
	int i;

	memset(&cfg, 0, sizeof(cfg));

	for (i = 0; i < MAX_CHANNELS_PER_ENC; i++) {
		hw_pp[i] = sde_enc->hw_pp[i];
		hw_dsc[i] = sde_enc->hw_dsc[i];
		hw_dsc_pp[i] = sde_enc->hw_dsc_pp[i];

		if (!hw_pp[i] || !hw_dsc[i] || !hw_dsc_pp[i]) {
			SDE_ERROR_DCE(sde_enc, "invalid params for DSC\n");
			return -EINVAL;
		}
	}

	half_panel_partial_update =
			hweight_long(params->affected_displays) == 1;

	dsc_common_mode = 0;
	if (!half_panel_partial_update)
		dsc_common_mode |= DSC_MODE_SPLIT_PANEL;
	if (enc_master->intf_mode == INTF_MODE_VIDEO)
		dsc_common_mode |= DSC_MODE_VIDEO;

	memcpy(&dsc[0], &sde_enc->mode_info.comp_info.dsc_info, sizeof(dsc[0]));
	memcpy(&dsc[1], &sde_enc->mode_info.comp_info.dsc_info, sizeof(dsc[1]));

	/*
	 * Since both DSC use same pic dimension, set same pic dimension
	 * to both DSC structures.
	 */
	_sde_encoder_dsc_update_pic_dim(&dsc[0], roi->w, roi->h);
	_sde_encoder_dsc_update_pic_dim(&dsc[1], roi->w, roi->h);

	this_frame_slices = roi->w / dsc[0].slice_width;
	intf_ip_w = this_frame_slices * dsc[0].slice_width;

	if (!half_panel_partial_update)
		intf_ip_w /= 2;

	/*
	 * In this topology when both interfaces are active, they have same
	 * load so intf_ip_w will be same.
	 */
	_sde_encoder_dsc_pclk_param_calc(&dsc[0], intf_ip_w);
	_sde_encoder_dsc_pclk_param_calc(&dsc[1], intf_ip_w);

	/*
	 * In this topology, since there is no dsc_merge, uncompressed input
	 * to encoder and interface is same.
	 */
	enc_ip_w = intf_ip_w;
	_sde_encoder_dsc_initial_line_calc(&dsc[0], enc_ip_w);
	_sde_encoder_dsc_initial_line_calc(&dsc[1], enc_ip_w);

	/*
	 * __is_ich_reset_override_needed should be called only after
	 * updating pic dimension, mdss_panel_dsc_update_pic_dim.
	 */
	ich_res = _sde_encoder_dsc_ich_reset_override_needed(
			half_panel_partial_update, &dsc[0]);

	SDE_DEBUG_DCE(sde_enc, "pic_w: %d pic_h: %d mode:%d\n",
			roi->w, roi->h, dsc_common_mode);

	for (i = 0; i < sde_enc->num_phys_encs; i++) {
		bool active = !!((1 << i) & params->affected_displays);

		SDE_EVT32(DRMID(&sde_enc->base), roi->w, roi->h,
				dsc_common_mode, i, active);
		_sde_encoder_dsc_pipe_cfg(hw_dsc[i], hw_pp[i], &dsc[i],
				dsc_common_mode, ich_res, active, hw_dsc_pp[i]);

		if (active) {
			if (cfg.dsc_count >= MAX_DSC_PER_CTL_V1) {
				pr_err("Invalid dsc count:%d\n",
						cfg.dsc_count);
				return -EINVAL;
			}
			cfg.dsc[cfg.dsc_count++] = hw_dsc[i]->idx;

			if (hw_ctl->ops.update_bitmask_dsc)
				hw_ctl->ops.update_bitmask_dsc(hw_ctl,
						hw_dsc[i]->idx, 1);
		}
	}

	/* setup dsc active configuration in the control path */
	if (hw_ctl->ops.setup_dsc_cfg) {
		hw_ctl->ops.setup_dsc_cfg(hw_ctl, &cfg);
		SDE_DEBUG_DCE(sde_enc,
				"setup dsc_cfg hw_ctl[%d], count:%d,dsc[0]:%d, dsc[1]:%d\n",
				hw_ctl->idx,
				cfg.dsc_count,
				cfg.dsc[0],
				cfg.dsc[1]);
	}
	return 0;
}

static int _sde_encoder_dsc_2_lm_2_enc_1_intf(struct sde_encoder_virt *sde_enc,
		struct sde_encoder_kickoff_params *params)
{
	int this_frame_slices;
	int intf_ip_w, enc_ip_w;
	int ich_res, dsc_common_mode;

	struct sde_encoder_phys *enc_master = sde_enc->cur_master;
	const struct sde_rect *roi = &sde_enc->cur_conn_roi;
	struct sde_hw_dsc *hw_dsc[MAX_CHANNELS_PER_ENC];
	struct sde_hw_pingpong *hw_pp[MAX_CHANNELS_PER_ENC];
	struct sde_hw_pingpong *hw_dsc_pp[MAX_CHANNELS_PER_ENC];
	struct msm_display_dsc_info *dsc = NULL;
	bool half_panel_partial_update;
	struct sde_hw_ctl *hw_ctl = enc_master->hw_ctl;
	struct sde_ctl_dsc_cfg cfg;
	int i;

	memset(&cfg, 0, sizeof(cfg));

	for (i = 0; i < MAX_CHANNELS_PER_ENC; i++) {
		hw_pp[i] = sde_enc->hw_pp[i];
		hw_dsc[i] = sde_enc->hw_dsc[i];
		hw_dsc_pp[i] = sde_enc->hw_dsc_pp[i];

		if (!hw_pp[i] || !hw_dsc[i] || !hw_dsc_pp[i]) {
			SDE_ERROR_DCE(sde_enc, "invalid params for DSC\n");
			return -EINVAL;
		}
	}

	dsc = &sde_enc->mode_info.comp_info.dsc_info;

	half_panel_partial_update =
			hweight_long(params->affected_displays) == 1;

	dsc_common_mode = 0;
	if (!half_panel_partial_update)
		dsc_common_mode |= DSC_MODE_SPLIT_PANEL | DSC_MODE_MULTIPLEX;
	if (enc_master->intf_mode == INTF_MODE_VIDEO)
		dsc_common_mode |= DSC_MODE_VIDEO;

	_sde_encoder_dsc_update_pic_dim(dsc, roi->w, roi->h);

	this_frame_slices = roi->w / dsc->slice_width;
	intf_ip_w = this_frame_slices * dsc->slice_width;
	_sde_encoder_dsc_pclk_param_calc(dsc, intf_ip_w);

	/*
	 * dsc merge case: when using 2 encoders for the same stream,
	 * no. of slices need to be same on both the encoders.
	 */
	enc_ip_w = intf_ip_w / 2;
	_sde_encoder_dsc_initial_line_calc(dsc, enc_ip_w);

	ich_res = _sde_encoder_dsc_ich_reset_override_needed(
			half_panel_partial_update, dsc);

	SDE_DEBUG_DCE(sde_enc, "pic_w: %d pic_h: %d mode:%d\n",
			roi->w, roi->h, dsc_common_mode);
	SDE_EVT32(DRMID(&sde_enc->base), roi->w, roi->h,
			dsc_common_mode, i, params->affected_displays);

	_sde_encoder_dsc_pipe_cfg(hw_dsc[0], hw_pp[0], dsc, dsc_common_mode,
			ich_res, true, hw_dsc_pp[0]);
	cfg.dsc[0] = hw_dsc[0]->idx;
	cfg.dsc_count++;
	if (hw_ctl->ops.update_bitmask_dsc)
		hw_ctl->ops.update_bitmask_dsc(hw_ctl, hw_dsc[0]->idx, 1);


	_sde_encoder_dsc_pipe_cfg(hw_dsc[1], hw_pp[1], dsc, dsc_common_mode,
			ich_res, !half_panel_partial_update, hw_dsc_pp[1]);
	if (!half_panel_partial_update) {
		cfg.dsc[1] = hw_dsc[1]->idx;
		cfg.dsc_count++;
		if (hw_ctl->ops.update_bitmask_dsc)
			hw_ctl->ops.update_bitmask_dsc(hw_ctl, hw_dsc[1]->idx,
					1);
	}
	/* setup dsc active configuration in the control path */
	if (hw_ctl->ops.setup_dsc_cfg) {
		hw_ctl->ops.setup_dsc_cfg(hw_ctl, &cfg);
		SDE_DEBUG_DCE(sde_enc,
				"setup_dsc_cfg hw_ctl[%d], count:%d,dsc[0]:%d, dsc[1]:%d\n",
				hw_ctl->idx,
				cfg.dsc_count,
				cfg.dsc[0],
				cfg.dsc[1]);
	}
	return 0;
}

static int _dce_dsc_setup(struct sde_encoder_virt *sde_enc,
		struct sde_encoder_kickoff_params *params)
{
	enum sde_rm_topology_name topology;
	struct drm_connector *drm_conn;
	int ret = 0;

	if (!sde_enc || !params || !sde_enc->phys_encs[0] ||
			!sde_enc->phys_encs[0]->connector)
		return -EINVAL;

	drm_conn = sde_enc->phys_encs[0]->connector;

	topology = sde_connector_get_topology_name(drm_conn);
	if (topology == SDE_RM_TOPOLOGY_NONE) {
		SDE_ERROR_DCE(sde_enc, "topology not set yet\n");
		return -EINVAL;
	}

	SDE_DEBUG_DCE(sde_enc, "topology:%d\n", topology);
	SDE_EVT32(DRMID(&sde_enc->base), topology,
			sde_enc->cur_conn_roi.x,
			sde_enc->cur_conn_roi.y,
			sde_enc->cur_conn_roi.w,
			sde_enc->cur_conn_roi.h,
			sde_enc->prv_conn_roi.x,
			sde_enc->prv_conn_roi.y,
			sde_enc->prv_conn_roi.w,
			sde_enc->prv_conn_roi.h,
			sde_enc->cur_master->cached_mode.hdisplay,
			sde_enc->cur_master->cached_mode.vdisplay);

	if (sde_kms_rect_is_equal(&sde_enc->cur_conn_roi,
			&sde_enc->prv_conn_roi))
		return ret;

	switch (topology) {
	case SDE_RM_TOPOLOGY_SINGLEPIPE_DSC:
	case SDE_RM_TOPOLOGY_DUALPIPE_3DMERGE_DSC:
		ret = _sde_encoder_dsc_n_lm_1_enc_1_intf(sde_enc);
		break;
	case SDE_RM_TOPOLOGY_DUALPIPE_DSCMERGE:
		ret = _sde_encoder_dsc_2_lm_2_enc_1_intf(sde_enc, params);
		break;
	case SDE_RM_TOPOLOGY_DUALPIPE_DSC:
		ret = _sde_encoder_dsc_2_lm_2_enc_2_intf(sde_enc, params);
		break;
	default:
		SDE_ERROR_DCE(sde_enc, "No DSC support for topology %d",
			topology);
		return -EINVAL;
	}

	return ret;
}

static void _dce_dsc_disable(struct sde_encoder_virt *sde_enc)
{
	int i;
	struct sde_hw_pingpong *hw_pp = NULL;
	struct sde_hw_pingpong *hw_dsc_pp = NULL;
	struct sde_hw_dsc *hw_dsc = NULL;
	struct sde_hw_ctl *hw_ctl = NULL;
	struct sde_ctl_dsc_cfg cfg;

	if (!sde_enc || !sde_enc->phys_encs[0] ||
			!sde_enc->phys_encs[0]->connector) {
		SDE_ERROR("invalid params %d %d\n",
			!sde_enc, sde_enc ? !sde_enc->phys_encs[0] : -1);
		return;
	}

	if (sde_enc->cur_master)
		hw_ctl = sde_enc->cur_master->hw_ctl;

	/* Disable DSC for all the pp's present in this topology */
	for (i = 0; i < MAX_CHANNELS_PER_ENC; i++) {
		hw_pp = sde_enc->hw_pp[i];
		hw_dsc = sde_enc->hw_dsc[i];
		hw_dsc_pp = sde_enc->hw_dsc_pp[i];

		_sde_encoder_dsc_pipe_cfg(hw_dsc, hw_pp, NULL,
						0, 0, 0, hw_dsc_pp);

		if (hw_dsc)
			sde_enc->dirty_dsc_ids[i] = hw_dsc->idx;
	}

	/* Clear the DSC ACTIVE config for this CTL */
	if (hw_ctl && hw_ctl->ops.setup_dsc_cfg) {
		memset(&cfg, 0, sizeof(cfg));
		hw_ctl->ops.setup_dsc_cfg(hw_ctl, &cfg);
	}

	/**
	 * Since pending flushes from previous commit get cleared
	 * sometime after this point, setting DSC flush bits now
	 * will have no effect. Therefore dirty_dsc_ids track which
	 * DSC blocks must be flushed for the next trigger.
	 */
}

static bool _dce_dsc_is_dirty(struct sde_encoder_virt *sde_enc)
{
	int i;

	for (i = 0; i < MAX_CHANNELS_PER_ENC; i++) {
		/**
		 * This dirty_dsc_hw field is set during DSC disable to
		 * indicate which DSC blocks need to be flushed
		 */
		if (sde_enc->dirty_dsc_ids[i])
			return true;
	}

	return false;
}


static void _dce_helper_flush_dsc(struct sde_encoder_virt *sde_enc)
{
	int i;
	struct sde_hw_ctl *hw_ctl = NULL;
	enum sde_dsc dsc_idx;

	if (sde_enc->cur_master)
		hw_ctl = sde_enc->cur_master->hw_ctl;

	for (i = 0; i < MAX_CHANNELS_PER_ENC; i++) {
		dsc_idx = sde_enc->dirty_dsc_ids[i];
		if (dsc_idx && hw_ctl && hw_ctl->ops.update_bitmask_dsc)
			hw_ctl->ops.update_bitmask_dsc(hw_ctl, dsc_idx, 1);

		sde_enc->dirty_dsc_ids[i] = DSC_NONE;
	}
}

void sde_encoder_dce_disable(struct sde_encoder_virt *sde_enc)
{
	enum msm_display_compression_type comp_type;

	if (!sde_enc)
		return;

	comp_type = sde_enc->mode_info.comp_info.comp_type;

	if (comp_type == MSM_DISPLAY_COMPRESSION_DSC)
		_dce_dsc_disable(sde_enc);
}

int sde_encoder_dce_flush(struct sde_encoder_virt *sde_enc)
{
	int rc = 0;

	if (!sde_enc)
		return -EINVAL;

	if (_dce_dsc_is_dirty(sde_enc))
		_dce_helper_flush_dsc(sde_enc);

	return rc;
}

int sde_encoder_dce_setup(struct sde_encoder_virt *sde_enc,
		struct sde_encoder_kickoff_params *params)
{
	enum msm_display_compression_type comp_type;
	int rc = 0;

	if (!sde_enc)
		return -EINVAL;

	comp_type = sde_enc->mode_info.comp_info.comp_type;

	if (comp_type == MSM_DISPLAY_COMPRESSION_DSC)
		rc = _dce_dsc_setup(sde_enc, params);

	return rc;
}
