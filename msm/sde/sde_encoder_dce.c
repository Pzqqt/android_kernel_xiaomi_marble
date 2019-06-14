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
#include "sde_dsc_helper.h"

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

static int _dce_dsc_update_pic_dim(struct msm_display_dsc_info *dsc,
		int pic_width, int pic_height)
{
	if (!dsc || !pic_width || !pic_height) {
		SDE_ERROR("invalid input: pic_width=%d pic_height=%d\n",
			pic_width, pic_height);
		return -EINVAL;
	}

	if ((pic_width % dsc->config.slice_width) ||
	    (pic_height % dsc->config.slice_height)) {
		SDE_ERROR("pic_dim=%dx%d has to be multiple of slice=%dx%d\n",
			pic_width, pic_height,
			dsc->config.slice_width, dsc->config.slice_height);
		return -EINVAL;
	}

	dsc->config.pic_width = pic_width;
	dsc->config.pic_height = pic_height;

	return 0;
}

static int _dce_dsc_initial_line_calc(struct msm_display_dsc_info *dsc,
		int enc_ip_width,
		int dsc_cmn_mode)
{
	int max_ssm_delay, max_se_size, max_muxword_size;
	int compress_bpp_group, obuf_latency, input_ssm_out_latency;
	int base_hs_latency, chunk_bits, ob_data_width;
	int output_rate_extra_budget_bits, multi_hs_extra_budget_bits;
	int multi_hs_extra_latency,  mux_word_size;
	int ob_data_width_4comps, ob_data_width_3comps;
	int output_rate_ratio_complement, container_slice_width;
	int rtl_num_components, multi_hs_c, multi_hs_d;

	int bpc = dsc->config.bits_per_component;
	int bpp = DSC_BPP(dsc->config);
	int num_of_active_ss = dsc->config.slice_count;
	bool native_422 = dsc->config.native_422;
	bool native_420 = dsc->config.native_420;

	/* Hardent core config */
	int multiplex_mode_enable = 0, split_panel_enable = 0;
	int rtl_max_bpc = 10, rtl_output_data_width = 64;
	int pipeline_latency = 28;

	if (dsc_cmn_mode & DSC_MODE_MULTIPLEX)
		multiplex_mode_enable = 1;
	if (dsc_cmn_mode & DSC_MODE_SPLIT_PANEL)
		split_panel_enable = 0;
	container_slice_width = (native_422 ?
			dsc->config.slice_width / 2 : dsc->config.slice_width);
	max_muxword_size = ((rtl_max_bpc >= 12) ? 64 : 48);
	max_se_size = 4 * (rtl_max_bpc + 1);
	max_ssm_delay = max_se_size + max_muxword_size - 1;
	mux_word_size = (bpc >= 12 ? 64 : 48);
	compress_bpp_group = (native_422 ? 2 * bpp : bpp);
	input_ssm_out_latency = pipeline_latency + (3 * (max_ssm_delay + 2)
			* num_of_active_ss);
	rtl_num_components = (native_420 | native_422 ? 4 : 3);
	ob_data_width_4comps = ((rtl_output_data_width >= (2 *
			max_muxword_size)) ?
			rtl_output_data_width :
			(2 * rtl_output_data_width));
	ob_data_width_3comps = (rtl_output_data_width >= max_muxword_size ?
			rtl_output_data_width : 2 * rtl_output_data_width);
	ob_data_width = (rtl_num_components == 4 ?
			ob_data_width_4comps : ob_data_width_3comps);
	obuf_latency = DIV_ROUND_UP((9 * ob_data_width + mux_word_size),
			compress_bpp_group) + 1;
	base_hs_latency = dsc->config.initial_xmit_delay +
		input_ssm_out_latency + obuf_latency;
	chunk_bits = 8 * dsc->config.slice_chunk_size;
	output_rate_ratio_complement = ob_data_width - compress_bpp_group;
	output_rate_extra_budget_bits =
		(output_rate_ratio_complement * chunk_bits) >>
		(ob_data_width == 128 ? 7 : 6);
	multi_hs_c = split_panel_enable * multiplex_mode_enable;
	multi_hs_d = (num_of_active_ss > 1) * (ob_data_width >
			compress_bpp_group);
	multi_hs_extra_budget_bits = (multi_hs_c ?
				chunk_bits : (multi_hs_d ? chunk_bits :
					output_rate_extra_budget_bits));
	multi_hs_extra_latency = DIV_ROUND_UP(multi_hs_extra_budget_bits,
			compress_bpp_group);
	dsc->initial_lines = DIV_ROUND_UP((base_hs_latency +
				multi_hs_extra_latency),
			container_slice_width);

	return 0;
}

static bool _dce_dsc_ich_reset_override_needed(bool pu_en,
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
	return pu_en && dsc && (dsc->config.slice_count > 1) &&
		(dsc->config.slice_width == dsc->config.pic_width);
}

static void _dce_dsc_pipe_cfg(struct sde_hw_dsc *hw_dsc,
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

	if (!dsc || !hw_dsc || !hw_pp) {
		SDE_ERROR("invalid params %d %d %d\n", !dsc, !hw_dsc,
				!hw_pp);
		return;
	}

	if (hw_dsc->ops.dsc_config)
		hw_dsc->ops.dsc_config(hw_dsc, dsc, common_mode, ich_reset);

	if (hw_dsc->ops.dsc_config_thresh)
		hw_dsc->ops.dsc_config_thresh(hw_dsc, dsc);

	if (hw_dsc_pp && hw_dsc_pp->ops.setup_dsc)
		hw_dsc_pp->ops.setup_dsc(hw_dsc_pp);

	if (hw_dsc->ops.bind_pingpong_blk)
		hw_dsc->ops.bind_pingpong_blk(hw_dsc, true, hw_pp->idx);

	if (hw_dsc_pp && hw_dsc_pp->ops.enable_dsc)
		hw_dsc_pp->ops.enable_dsc(hw_dsc_pp);
}

static int _dce_dsc_setup(struct sde_encoder_virt *sde_enc,
		struct sde_encoder_kickoff_params *params)
{
	struct sde_kms *sde_kms;
	struct msm_drm_private *priv;
	struct drm_encoder *drm_enc;
	struct drm_connector *drm_conn;
	struct sde_encoder_phys *enc_master;
	struct sde_hw_dsc *hw_dsc[MAX_CHANNELS_PER_ENC];
	struct sde_hw_pingpong *hw_pp[MAX_CHANNELS_PER_ENC];
	struct sde_hw_pingpong *hw_dsc_pp[MAX_CHANNELS_PER_ENC];
	struct msm_display_dsc_info *dsc = NULL;
	enum sde_rm_topology_name topology;
	const struct sde_rm_topology_def *def;
	const struct sde_rect *roi;
	struct sde_hw_ctl *hw_ctl;
	struct sde_ctl_dsc_cfg cfg;
	bool half_panel_partial_update, dsc_merge;
	int this_frame_slices;
	int intf_ip_w, enc_ip_w;
	int num_intf, num_dsc;
	int ich_res;
	int dsc_common_mode = 0;
	int i;

	if (!sde_enc || !params || !sde_enc->phys_encs[0] ||
			!sde_enc->phys_encs[0]->connector)
		return -EINVAL;

	drm_conn = sde_enc->phys_encs[0]->connector;
	drm_enc = &sde_enc->base;
	priv = drm_enc->dev->dev_private;
	sde_kms = to_sde_kms(priv->kms);

	topology = sde_connector_get_topology_name(drm_conn);
	if (topology == SDE_RM_TOPOLOGY_NONE) {
		SDE_ERROR_DCE(sde_enc, "topology not set yet\n");
		return -EINVAL;
	}

	SDE_DEBUG_DCE(sde_enc, "topology:%d\n", topology);

	if (sde_kms_rect_is_equal(&sde_enc->cur_conn_roi,
			&sde_enc->prv_conn_roi))
		return 0;

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

	memset(&cfg, 0, sizeof(cfg));
	enc_master = sde_enc->cur_master;
	roi = &sde_enc->cur_conn_roi;
	hw_ctl = enc_master->hw_ctl;
	dsc = &sde_enc->mode_info.comp_info.dsc_info;

	def = sde_rm_topology_get_topology_def(&sde_kms->rm, topology);
	if (IS_ERR_OR_NULL(def))
		return -EINVAL;

	num_dsc = def->num_comp_enc;
	num_intf = def->num_intf;

	/*
	 * If this encoder is driving more than one DSC encoder, they
	 * operate in tandem, same pic dimension needs to be used by
	 * each of them.(pp-split is assumed to be not supported)
	 */
	_dce_dsc_update_pic_dim(dsc, roi->w, roi->h);

	half_panel_partial_update = (num_dsc > 1) ?
			(hweight_long(params->affected_displays) != num_dsc) :
			false;
	dsc_merge = (num_dsc > num_intf) ? true : false;

	if (!half_panel_partial_update)
		dsc_common_mode |= DSC_MODE_SPLIT_PANEL;
	if (dsc_merge)
		dsc_common_mode |= DSC_MODE_MULTIPLEX;
	if (enc_master->intf_mode == INTF_MODE_VIDEO)
		dsc_common_mode |= DSC_MODE_VIDEO;

	this_frame_slices = roi->w / dsc->config.slice_width;
	intf_ip_w = this_frame_slices * dsc->config.slice_width;

	if ((!half_panel_partial_update) && (num_intf > 1))
		intf_ip_w /= 2;

	sde_dsc_populate_dsc_private_params(dsc, intf_ip_w);

	/*
	 * in dsc merge case: when using 2 encoders for the same stream,
	 * no. of slices need to be same on both the encoders.
	 */
	enc_ip_w = intf_ip_w;
	if (dsc_merge)
		enc_ip_w = intf_ip_w / 2;

	_dce_dsc_initial_line_calc(dsc, enc_ip_w, dsc_common_mode);

	/*
	 * __is_ich_reset_override_needed should be called only after
	 * updating pic dimension, mdss_panel_dsc_update_pic_dim.
	 */
	ich_res = _dce_dsc_ich_reset_override_needed(
			half_panel_partial_update, dsc);

	SDE_DEBUG_DCE(sde_enc, "pic_w: %d pic_h: %d mode:%d\n",
				roi->w, roi->h, dsc_common_mode);

	for (i = 0; i < num_dsc; i++) {
		bool active = !!((1 << i) & params->affected_displays);

		hw_pp[i] = sde_enc->hw_pp[i];
		hw_dsc[i] = sde_enc->hw_dsc[i];
		hw_dsc_pp[i] = sde_enc->hw_dsc_pp[i];

		if (!hw_pp[i] || !hw_dsc[i]) {
			SDE_ERROR_DCE(sde_enc, "invalid params for DSC\n");
			SDE_EVT32(DRMID(&sde_enc->base), roi->w, roi->h,
				dsc_common_mode, i, active);
			return -EINVAL;
		}

		_dce_dsc_pipe_cfg(hw_dsc[i], hw_pp[i], dsc,
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

		_dce_dsc_pipe_cfg(hw_dsc, hw_pp, NULL,
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
