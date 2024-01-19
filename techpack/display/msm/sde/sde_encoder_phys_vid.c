// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2015-2021, The Linux Foundation. All rights reserved.
 */

#define pr_fmt(fmt)	"[drm:%s:%d] " fmt, __func__, __LINE__
#include "sde_encoder_phys.h"
#include "sde_hw_interrupts.h"
#include "sde_core_irq.h"
#include "sde_formats.h"
#include "dsi_display.h"
#include "sde_trace.h"
#include "mi_sde_encoder.h"
#include "mi_dsi_display.h"
#include "mi_panel_id.h"


#define SDE_DEBUG_VIDENC(e, fmt, ...) SDE_DEBUG("enc%d intf%d " fmt, \
		(e) && (e)->base.parent ? \
		(e)->base.parent->base.id : -1, \
		(e) && (e)->base.hw_intf ? \
		(e)->base.hw_intf->idx - INTF_0 : -1, ##__VA_ARGS__)

#define SDE_ERROR_VIDENC(e, fmt, ...) SDE_ERROR("enc%d intf%d " fmt, \
		(e) && (e)->base.parent ? \
		(e)->base.parent->base.id : -1, \
		(e) && (e)->base.hw_intf ? \
		(e)->base.hw_intf->idx - INTF_0 : -1, ##__VA_ARGS__)

#define to_sde_encoder_phys_vid(x) \
	container_of(x, struct sde_encoder_phys_vid, base)

/* Poll time to do recovery during active region */
#define POLL_TIME_USEC_FOR_LN_CNT 500
#define MAX_POLL_CNT 10

static bool sde_encoder_phys_vid_is_master(
		struct sde_encoder_phys *phys_enc)
{
	bool ret = false;

	if (phys_enc->split_role != ENC_ROLE_SLAVE)
		ret = true;

	return ret;
}

static void drm_mode_to_intf_timing_params(
		const struct sde_encoder_phys_vid *vid_enc,
		const struct drm_display_mode *mode,
		struct intf_timing_params *timing)
{
	const struct sde_encoder_phys *phys_enc = &vid_enc->base;
	bool fsc_mode = false;
	struct sde_connector_state *c_state = NULL;
	struct dsi_display *display = NULL;

	if (phys_enc->connector && phys_enc->connector->state) {
		c_state = to_sde_connector_state(phys_enc->connector->state);
		if (!c_state) {
			SDE_ERROR("invalid connector state\n");
			return;
		}
	}

	memset(timing, 0, sizeof(*timing));

	if ((mode->htotal < mode->hsync_end)
			|| (mode->hsync_start < mode->hdisplay)
			|| (mode->vtotal < mode->vsync_end)
			|| (mode->vsync_start < mode->vdisplay)
			|| (mode->hsync_end < mode->hsync_start)
			|| (mode->vsync_end < mode->vsync_start)) {
		SDE_ERROR(
		    "invalid params - hstart:%d,hend:%d,htot:%d,hdisplay:%d\n",
				mode->hsync_start, mode->hsync_end,
				mode->htotal, mode->hdisplay);
		SDE_ERROR("vstart:%d,vend:%d,vtot:%d,vdisplay:%d\n",
				mode->vsync_start, mode->vsync_end,
				mode->vtotal, mode->vdisplay);
		return;
	}

	/*
	 * https://www.kernel.org/doc/htmldocs/drm/ch02s05.html
	 *  Active Region      Front Porch   Sync   Back Porch
	 * <-----------------><------------><-----><----------->
	 * <- [hv]display --->
	 * <--------- [hv]sync_start ------>
	 * <----------------- [hv]sync_end ------->
	 * <---------------------------- [hv]total ------------->
	 */
	fsc_mode = c_state ? msm_is_mode_fsc(&c_state->msm_mode) : false;

	timing->poms_align_vsync = phys_enc->poms_align_vsync;
	timing->width = GET_MODE_WIDTH(fsc_mode, mode);/* active width */
	timing->height = GET_MODE_HEIGHT(fsc_mode, mode);/* active height */
	timing->xres = timing->width;
	timing->yres = timing->height;
	timing->h_back_porch = mode->htotal - mode->hsync_end;
	timing->h_front_porch = mode->hsync_start - mode->hdisplay;
	timing->v_back_porch = mode->vtotal - mode->vsync_end;
	timing->v_front_porch = mode->vsync_start - mode->vdisplay;
	timing->hsync_pulse_width = mode->hsync_end - mode->hsync_start;
	timing->vsync_pulse_width = mode->vsync_end - mode->vsync_start;
	timing->hsync_polarity = (mode->flags & DRM_MODE_FLAG_NHSYNC) ? 1 : 0;
	timing->vsync_polarity = (mode->flags & DRM_MODE_FLAG_NVSYNC) ? 1 : 0;
	timing->border_clr = 0;
	display = mi_get_primary_dsi_display();
	if (display && (mi_get_panel_id_by_dsi_panel(display->panel) == M16T_PANEL_PA ||
		mi_get_panel_id_by_dsi_panel(display->panel) == N16_PANEL_PA ||
		mi_get_panel_id_by_dsi_panel(display->panel) == N16_PANEL_PB)) {
		timing->underflow_clr = 0;
	} else{
		timing->underflow_clr = 0xff;
	}

	timing->hsync_skew = mode->hskew;
	timing->v_front_porch_fixed = vid_enc->base.vfp_cached;
	timing->vrefresh = drm_mode_vrefresh(mode);
	timing->fsc_mode = fsc_mode;

	if (vid_enc->base.comp_type != MSM_DISPLAY_COMPRESSION_NONE) {
		timing->compression_en = true;
		timing->dce_bytes_per_line = vid_enc->base.dce_bytes_per_line;
	}

	/* DSI controller cannot handle active-low sync signals. */
	if (phys_enc->hw_intf->cap->type == INTF_DSI) {
		timing->hsync_polarity = 0;
		timing->vsync_polarity = 0;
	}

	/* for DP/EDP, Shift timings to align it to bottom right */
	if ((phys_enc->hw_intf->cap->type == INTF_DP) ||
		(phys_enc->hw_intf->cap->type == INTF_EDP)) {
		timing->h_back_porch += timing->h_front_porch;
		timing->h_front_porch = 0;
		timing->v_back_porch += timing->v_front_porch;
		timing->v_front_porch = 0;
	}

	timing->wide_bus_en = sde_encoder_is_widebus_enabled(phys_enc->parent);

	/*
	 * for DP, divide the horizonal parameters by 2 when
	 * widebus or compression is enabled, irrespective of
	 * compression ratio
	 */
	if (phys_enc->hw_intf->cap->type == INTF_DP &&
			(timing->wide_bus_en ||
			(vid_enc->base.comp_ratio > 1))) {
		timing->width = timing->width >> 1;
		timing->xres = timing->xres >> 1;
		timing->h_back_porch = timing->h_back_porch >> 1;
		timing->h_front_porch = timing->h_front_porch >> 1;
		timing->hsync_pulse_width = timing->hsync_pulse_width >> 1;

		if (vid_enc->base.comp_type == MSM_DISPLAY_COMPRESSION_DSC &&
				(vid_enc->base.comp_ratio > 1)) {
			timing->extra_dto_cycles =
				vid_enc->base.dsc_extra_pclk_cycle_cnt;
			timing->width += vid_enc->base.dsc_extra_disp_width;
			timing->h_back_porch +=
				vid_enc->base.dsc_extra_disp_width;
		}
	}

	/*
	 * for DSI, if compression is enabled, then divide the horizonal active
	 * timing parameters by compression ratio.
	 */
	if ((phys_enc->hw_intf->cap->type != INTF_DP) &&
			((vid_enc->base.comp_type ==
			MSM_DISPLAY_COMPRESSION_DSC) ||
			(vid_enc->base.comp_type ==
			MSM_DISPLAY_COMPRESSION_VDC))) {
		// adjust active dimensions
		timing->width = DIV_ROUND_UP(timing->width,
			vid_enc->base.comp_ratio);
		timing->xres = DIV_ROUND_UP(timing->xres,
			vid_enc->base.comp_ratio);
	}

	/*
	 * For edp only:
	 * DISPLAY_V_START = (VBP * HCYCLE) + HBP
	 * DISPLAY_V_END = (VBP + VACTIVE) * HCYCLE - 1 - HFP
	 */
	/*
	 * if (vid_enc->hw->cap->type == INTF_EDP) {
	 * display_v_start += mode->htotal - mode->hsync_start;
	 * display_v_end -= mode->hsync_start - mode->hdisplay;
	 * }
	 */
}

static inline u32 get_horizontal_total(const struct intf_timing_params *timing)
{
	u32 active = timing->xres;
	u32 inactive =
	    timing->h_back_porch + timing->h_front_porch +
	    timing->hsync_pulse_width;
	return active + inactive;
}

static inline u32 get_vertical_total(const struct intf_timing_params *timing)
{
	u32 active = timing->yres;
	u32 inactive = timing->v_back_porch + timing->v_front_porch +
			    timing->vsync_pulse_width;
	return active + inactive;
}

/*
 * programmable_fetch_get_num_lines:
 *	Number of fetch lines in vertical front porch
 * @timing: Pointer to the intf timing information for the requested mode
 *
 * Returns the number of fetch lines in vertical front porch at which mdp
 * can start fetching the next frame.
 *
 * Number of needed prefetch lines is anything that cannot be absorbed in the
 * start of frame time (back porch + vsync pulse width).
 *
 * Some panels have very large VFP, however we only need a total number of
 * lines based on the chip worst case latencies.
 */
static u32 programmable_fetch_get_num_lines(
		struct sde_encoder_phys_vid *vid_enc,
		const struct intf_timing_params *timing)
{
	struct sde_encoder_phys *phys_enc = &vid_enc->base;
	struct sde_mdss_cfg *m;

	u32 needed_prefill_lines, needed_vfp_lines, actual_vfp_lines;
	const u32 fixed_prefill_fps = DEFAULT_FPS;
	u32 default_prefill_lines =
		phys_enc->hw_intf->cap->prog_fetch_lines_worst_case;
	u32 start_of_frame_lines =
	    timing->v_back_porch + timing->vsync_pulse_width;
	u32 v_front_porch = timing->v_front_porch;
	u32 vrefresh, max_fps;

	m = phys_enc->sde_kms->catalog;
	max_fps = sde_encoder_get_dfps_maxfps(phys_enc->parent);
	vrefresh = (max_fps > timing->vrefresh) ? max_fps : timing->vrefresh;

	/* Avoid programmable fetch config for xr targets,
	 * with separate scid for left and right display
	 */
	if (test_bit(SDE_MDP_LLCC_DISP_LR, &m->mdp[0].features))
		return 0;

	/* minimum prefill lines are defined based on 60fps */
	needed_prefill_lines = (vrefresh > fixed_prefill_fps) ?
		((default_prefill_lines * vrefresh) /
			fixed_prefill_fps) : default_prefill_lines;
	needed_vfp_lines = needed_prefill_lines - start_of_frame_lines;

	/* Fetch must be outside active lines, otherwise undefined. */
	if (start_of_frame_lines >= needed_prefill_lines) {
		SDE_DEBUG_VIDENC(vid_enc,
				"prog fetch always enabled case\n");
		actual_vfp_lines = (m->delay_prg_fetch_start) ? 2 : 1;
	} else if (v_front_porch < needed_vfp_lines) {
		/* Warn fetch needed, but not enough porch in panel config */
		pr_warn_once
			("low vbp+vfp may lead to perf issues in some cases\n");
		SDE_DEBUG_VIDENC(vid_enc,
				"less vfp than fetch req, using entire vfp\n");
		actual_vfp_lines = v_front_porch;
	} else {
		SDE_DEBUG_VIDENC(vid_enc, "room in vfp for needed prefetch\n");
		actual_vfp_lines = needed_vfp_lines;
	}

	SDE_DEBUG_VIDENC(vid_enc,
		"vrefresh:%u v_front_porch:%u v_back_porch:%u vsync_pulse_width:%u\n",
		vrefresh, v_front_porch, timing->v_back_porch,
		timing->vsync_pulse_width);
	SDE_DEBUG_VIDENC(vid_enc,
		"prefill_lines:%u needed_vfp_lines:%u actual_vfp_lines:%u\n",
		needed_prefill_lines, needed_vfp_lines, actual_vfp_lines);

	return actual_vfp_lines;
}

/*
 * programmable_fetch_config: Programs HW to prefetch lines by offsetting
 *	the start of fetch into the vertical front porch for cases where the
 *	vsync pulse width and vertical back porch time is insufficient
 *
 *	Gets # of lines to pre-fetch, then calculate VSYNC counter value.
 *	HW layer requires VSYNC counter of first pixel of tgt VFP line.
 *
 * @timing: Pointer to the intf timing information for the requested mode
 */
static void programmable_fetch_config(struct sde_encoder_phys *phys_enc,
				      const struct intf_timing_params *timing)
{
	struct sde_encoder_phys_vid *vid_enc =
		to_sde_encoder_phys_vid(phys_enc);
	struct intf_prog_fetch f = { 0 };
	u32 vfp_fetch_lines = 0;
	u32 horiz_total = 0;
	u32 vert_total = 0;
	u32 vfp_fetch_start_vsync_counter = 0;
	unsigned long lock_flags;
	struct sde_mdss_cfg *m;

	if (WARN_ON_ONCE(!phys_enc->hw_intf->ops.setup_prg_fetch))
		return;

	m = phys_enc->sde_kms->catalog;

	vfp_fetch_lines = programmable_fetch_get_num_lines(vid_enc, timing);
	if (vfp_fetch_lines) {
		vert_total = get_vertical_total(timing);
		horiz_total = get_horizontal_total(timing);
		vfp_fetch_start_vsync_counter =
			(vert_total - vfp_fetch_lines) * horiz_total + 1;

		/**
		 * Check if we need to throttle the fetch to start
		 * from second line after the active region.
		 */
		if (m->delay_prg_fetch_start)
			vfp_fetch_start_vsync_counter += horiz_total;

		f.enable = 1;
		f.fetch_start = vfp_fetch_start_vsync_counter;
	}

	SDE_DEBUG_VIDENC(vid_enc,
		"vfp_fetch_lines %u vfp_fetch_start_vsync_counter %u\n",
		vfp_fetch_lines, vfp_fetch_start_vsync_counter);

	spin_lock_irqsave(phys_enc->enc_spinlock, lock_flags);
	phys_enc->hw_intf->ops.setup_prg_fetch(phys_enc->hw_intf, &f);
	spin_unlock_irqrestore(phys_enc->enc_spinlock, lock_flags);
}

static bool sde_encoder_phys_vid_mode_fixup(
		struct sde_encoder_phys *phys_enc,
		const struct drm_display_mode *mode,
		struct drm_display_mode *adj_mode)
{
	if (phys_enc)
		SDE_DEBUG_VIDENC(to_sde_encoder_phys_vid(phys_enc), "\n");

	/*
	 * Modifying mode has consequences when the mode comes back to us
	 */
	return true;
}

/* vid_enc timing_params must be configured before calling this function */
static void _sde_encoder_phys_vid_setup_avr(
		struct sde_encoder_phys *phys_enc, u32 qsync_min_fps)
{
	struct sde_encoder_phys_vid *vid_enc;

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	if (vid_enc->base.hw_intf->ops.avr_setup) {
		struct intf_avr_params avr_params = {0};
		u32 default_fps = drm_mode_vrefresh(&phys_enc->cached_mode);
		int ret;

		if (!default_fps) {
			SDE_ERROR_VIDENC(vid_enc,
					"invalid default fps %d\n",
					default_fps);
			return;
		}

		if (qsync_min_fps > default_fps) {
			SDE_ERROR_VIDENC(vid_enc,
				"qsync fps %d must be less than default %d\n",
				qsync_min_fps, default_fps);
			return;
		}

		avr_params.default_fps = default_fps;
		avr_params.min_fps = qsync_min_fps;

		ret = vid_enc->base.hw_intf->ops.avr_setup(
				vid_enc->base.hw_intf,
				&vid_enc->timing_params, &avr_params);
		if (ret)
			SDE_ERROR_VIDENC(vid_enc,
				"bad settings, can't configure AVR\n");

		SDE_EVT32(DRMID(phys_enc->parent), default_fps,
				qsync_min_fps, ret);
	}
}

static void _sde_encoder_phys_vid_avr_ctrl(struct sde_encoder_phys *phys_enc)
{
	struct intf_avr_params avr_params;
	struct sde_encoder_phys_vid *vid_enc = to_sde_encoder_phys_vid(phys_enc);
	u32 avr_step_fps = sde_connector_get_avr_step(phys_enc->connector);

	memset(&avr_params, 0, sizeof(avr_params));
	avr_params.avr_mode = sde_connector_get_qsync_mode(phys_enc->connector);
	if (avr_step_fps)
		avr_params.avr_step_lines = mult_frac(phys_enc->cached_mode.vtotal,
				vid_enc->timing_params.vrefresh, avr_step_fps);

	if (vid_enc->base.hw_intf->ops.avr_ctrl)
		vid_enc->base.hw_intf->ops.avr_ctrl(vid_enc->base.hw_intf, &avr_params);

	SDE_EVT32(DRMID(phys_enc->parent), phys_enc->hw_intf->idx - INTF_0,
			avr_params.avr_mode, avr_params.avr_step_lines, avr_step_fps);
}

static void sde_encoder_phys_vid_setup_timing_engine(
		struct sde_encoder_phys *phys_enc)
{
	struct sde_encoder_phys_vid *vid_enc;
	struct drm_display_mode mode;
	struct intf_timing_params timing_params = { 0 };
	const struct sde_format *fmt = NULL;
	u32 fmt_fourcc = DRM_FORMAT_RGB888;
	u32 qsync_min_fps = 0;
	unsigned long lock_flags;
	struct sde_hw_intf_cfg intf_cfg = { 0 };
	bool is_split_link = false;

	if (!phys_enc || !phys_enc->sde_kms || !phys_enc->hw_ctl ||
			!phys_enc->hw_intf || !phys_enc->connector) {
		SDE_ERROR("invalid encoder %d\n", !phys_enc);
		return;
	}

	mode = phys_enc->cached_mode;
	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	if (!phys_enc->hw_intf->ops.setup_timing_gen) {
		SDE_ERROR("timing engine setup is not supported\n");
		return;
	}

	SDE_DEBUG_VIDENC(vid_enc, "enabling mode:\n");
	drm_mode_debug_printmodeline(&mode);

	is_split_link = phys_enc->hw_intf->cfg.split_link_en;
	if (phys_enc->split_role != ENC_ROLE_SOLO || is_split_link) {
		mode.hdisplay >>= 1;
		mode.htotal >>= 1;
		mode.hsync_start >>= 1;
		mode.hsync_end >>= 1;

		SDE_DEBUG_VIDENC(vid_enc,
			"split_role %d, halve horizontal %d %d %d %d\n",
			phys_enc->split_role,
			mode.hdisplay, mode.htotal,
			mode.hsync_start, mode.hsync_end);
	}

	if (!phys_enc->vfp_cached) {
		phys_enc->vfp_cached =
			sde_connector_get_panel_vfp(phys_enc->connector, &mode);
		if (phys_enc->vfp_cached <= 0)
			phys_enc->vfp_cached = mode.vsync_start - mode.vdisplay;
	}

	drm_mode_to_intf_timing_params(vid_enc, &mode, &timing_params);

	vid_enc->timing_params = timing_params;

	if (phys_enc->cont_splash_enabled) {
		SDE_DEBUG_VIDENC(vid_enc,
			"skipping intf programming since cont splash is enabled\n");
		goto exit;
	}

	fmt = sde_get_sde_format(fmt_fourcc);
	SDE_DEBUG_VIDENC(vid_enc, "fmt_fourcc 0x%X\n", fmt_fourcc);

	spin_lock_irqsave(phys_enc->enc_spinlock, lock_flags);
	phys_enc->hw_intf->ops.setup_timing_gen(phys_enc->hw_intf,
			&timing_params, fmt);

	if (test_bit(SDE_CTL_ACTIVE_CFG,
				&phys_enc->hw_ctl->caps->features)) {
		sde_encoder_helper_update_intf_cfg(phys_enc);
	} else if (phys_enc->hw_ctl->ops.setup_intf_cfg) {
		intf_cfg.intf = phys_enc->hw_intf->idx;
		intf_cfg.intf_mode_sel = SDE_CTL_MODE_SEL_VID;
		intf_cfg.stream_sel = 0; /* Don't care value for video mode */
		intf_cfg.mode_3d =
			sde_encoder_helper_get_3d_blend_mode(phys_enc);

		phys_enc->hw_ctl->ops.setup_intf_cfg(phys_enc->hw_ctl,
				&intf_cfg);
	}
	spin_unlock_irqrestore(phys_enc->enc_spinlock, lock_flags);
	if (phys_enc->hw_intf->cap->type == INTF_DSI)
		programmable_fetch_config(phys_enc, &timing_params);

exit:
	if (phys_enc->parent_ops.get_qsync_fps)
		phys_enc->parent_ops.get_qsync_fps(
			phys_enc->parent, &qsync_min_fps, phys_enc->connector->state);

	/* only panels which support qsync will have a non-zero min fps */
	if (qsync_min_fps) {
		_sde_encoder_phys_vid_setup_avr(phys_enc, qsync_min_fps);
		_sde_encoder_phys_vid_avr_ctrl(phys_enc);
	}
}

static void sde_encoder_phys_vid_vblank_irq(void *arg, int irq_idx)
{
	struct sde_encoder_phys *phys_enc = arg;
	struct sde_hw_ctl *hw_ctl;
	struct intf_status intf_status = {0};
	unsigned long lock_flags;
	u32 flush_register = ~0;
	u32 reset_status = 0;
	int new_cnt = -1, old_cnt = -1;
	u32 event = 0;
	int pend_ret_fence_cnt = 0;

	if (!phys_enc)
		return;

	hw_ctl = phys_enc->hw_ctl;
	if (!hw_ctl)
		return;

	mi_sde_encoder_save_vsync_info(phys_enc);

	SDE_ATRACE_BEGIN("vblank_irq");

	/*
	 * only decrement the pending flush count if we've actually flushed
	 * hardware. due to sw irq latency, vblank may have already happened
	 * so we need to double-check with hw that it accepted the flush bits
	 */
	spin_lock_irqsave(phys_enc->enc_spinlock, lock_flags);

	old_cnt = atomic_read(&phys_enc->pending_kickoff_cnt);

	if (hw_ctl && hw_ctl->ops.get_flush_register)
		flush_register = hw_ctl->ops.get_flush_register(hw_ctl);

	if (flush_register)
		goto not_flushed;

	new_cnt = atomic_add_unless(&phys_enc->pending_kickoff_cnt, -1, 0);
	pend_ret_fence_cnt = atomic_read(&phys_enc->pending_retire_fence_cnt);

	/* signal only for master, where there is a pending kickoff */
	if (sde_encoder_phys_vid_is_master(phys_enc) &&
	    atomic_add_unless(&phys_enc->pending_retire_fence_cnt, -1, 0)) {
		event = SDE_ENCODER_FRAME_EVENT_DONE |
			SDE_ENCODER_FRAME_EVENT_SIGNAL_RETIRE_FENCE |
			SDE_ENCODER_FRAME_EVENT_SIGNAL_RELEASE_FENCE;
	}

not_flushed:
	if (hw_ctl && hw_ctl->ops.get_reset)
		reset_status = hw_ctl->ops.get_reset(hw_ctl);

	spin_unlock_irqrestore(phys_enc->enc_spinlock, lock_flags);

	if (event && phys_enc->parent_ops.handle_frame_done)
		phys_enc->parent_ops.handle_frame_done(phys_enc->parent,
			phys_enc, event);

	if (phys_enc->parent_ops.handle_vblank_virt)
		phys_enc->parent_ops.handle_vblank_virt(phys_enc->parent,
				phys_enc);

	if (phys_enc->hw_intf->ops.get_status)
		phys_enc->hw_intf->ops.get_status(phys_enc->hw_intf,
			&intf_status);

	SDE_EVT32_IRQ(DRMID(phys_enc->parent), phys_enc->hw_intf->idx - INTF_0,
			old_cnt, atomic_read(&phys_enc->pending_kickoff_cnt),
			reset_status ? SDE_EVTLOG_ERROR : 0,
			flush_register, event,
			atomic_read(&phys_enc->pending_retire_fence_cnt),
			intf_status.frame_count, intf_status.line_count);

	/* Signal any waiting atomic commit thread */
	wake_up_all(&phys_enc->pending_kickoff_wq);
	SDE_ATRACE_END("vblank_irq");
}

static void sde_encoder_phys_vid_underrun_irq(void *arg, int irq_idx)
{
	struct sde_encoder_phys *phys_enc = arg;

	if (!phys_enc)
		return;

	SDE_ERROR("underrun_irq, call sde_encoder_underrun_callback()\n");
	if (phys_enc->parent_ops.handle_underrun_virt)
		phys_enc->parent_ops.handle_underrun_virt(phys_enc->parent,
			phys_enc);
}

static void _sde_encoder_phys_vid_setup_irq_hw_idx(
		struct sde_encoder_phys *phys_enc)
{
	struct sde_encoder_irq *irq;

	/*
	 * Initialize irq->hw_idx only when irq is not registered.
	 * Prevent invalidating irq->irq_idx as modeset may be
	 * called many times during dfps.
	 */

	irq = &phys_enc->irq[INTR_IDX_VSYNC];
	if (irq->irq_idx < 0)
		irq->hw_idx = phys_enc->intf_idx;

	irq = &phys_enc->irq[INTR_IDX_UNDERRUN];
	if (irq->irq_idx < 0)
		irq->hw_idx = phys_enc->intf_idx;
}

static void sde_encoder_phys_vid_cont_splash_mode_set(
		struct sde_encoder_phys *phys_enc,
		struct drm_display_mode *adj_mode)
{
	if (!phys_enc || !adj_mode) {
		SDE_ERROR("invalid args\n");
		return;
	}

	phys_enc->cached_mode = *adj_mode;
	phys_enc->enable_state = SDE_ENC_ENABLED;

	_sde_encoder_phys_vid_setup_irq_hw_idx(phys_enc);

	phys_enc->kickoff_timeout_ms =
		sde_encoder_helper_get_kickoff_timeout_ms(phys_enc->parent);
}

static void sde_encoder_phys_vid_mode_set(
		struct sde_encoder_phys *phys_enc,
		struct drm_display_mode *mode,
		struct drm_display_mode *adj_mode, bool *reinit_mixers)
{
	struct sde_rm *rm;
	struct sde_rm_hw_iter iter;
	int i, instance;
	struct sde_encoder_phys_vid *vid_enc;

	if (!phys_enc || !phys_enc->sde_kms) {
		SDE_ERROR("invalid encoder/kms\n");
		return;
	}

	rm = &phys_enc->sde_kms->rm;
	vid_enc = to_sde_encoder_phys_vid(phys_enc);

	if (adj_mode) {
		phys_enc->cached_mode = *adj_mode;
		drm_mode_debug_printmodeline(adj_mode);
		SDE_DEBUG_VIDENC(vid_enc, "caching mode:\n");
	}

	instance = phys_enc->split_role == ENC_ROLE_SLAVE ? 1 : 0;

	/* Retrieve previously allocated HW Resources. Shouldn't fail */
	sde_rm_init_hw_iter(&iter, phys_enc->parent->base.id, SDE_HW_BLK_CTL);
	for (i = 0; i <= instance; i++) {
		if (sde_rm_get_hw(rm, &iter)) {
			if (phys_enc->hw_ctl && phys_enc->hw_ctl != iter.hw) {
				*reinit_mixers =  true;
				SDE_EVT32(phys_enc->hw_ctl->idx,
					((struct sde_hw_ctl *)iter.hw)->idx);
			}
			phys_enc->hw_ctl = (struct sde_hw_ctl *)iter.hw;
		}
	}
	if (IS_ERR_OR_NULL(phys_enc->hw_ctl)) {
		SDE_ERROR_VIDENC(vid_enc, "failed to init ctl, %ld\n",
				PTR_ERR(phys_enc->hw_ctl));
		phys_enc->hw_ctl = NULL;
		return;
	}

	sde_rm_init_hw_iter(&iter, phys_enc->parent->base.id, SDE_HW_BLK_INTF);
	for (i = 0; i <= instance; i++) {
		if (sde_rm_get_hw(rm, &iter))
			phys_enc->hw_intf = (struct sde_hw_intf *)iter.hw;
	}

	if (IS_ERR_OR_NULL(phys_enc->hw_intf)) {
		SDE_ERROR_VIDENC(vid_enc, "failed to init intf: %ld\n",
				PTR_ERR(phys_enc->hw_intf));
		phys_enc->hw_intf = NULL;
		return;
	}

	_sde_encoder_phys_vid_setup_irq_hw_idx(phys_enc);

	phys_enc->kickoff_timeout_ms =
		sde_encoder_helper_get_kickoff_timeout_ms(phys_enc->parent);
}

static int sde_encoder_phys_vid_control_vblank_irq(
		struct sde_encoder_phys *phys_enc,
		bool enable)
{
	int ret = 0;
	struct sde_encoder_phys_vid *vid_enc;
	int refcount;

	if (!phys_enc) {
		SDE_ERROR("invalid encoder\n");
		return -EINVAL;
	}

	mutex_lock(phys_enc->vblank_ctl_lock);
	refcount = atomic_read(&phys_enc->vblank_refcount);
	vid_enc = to_sde_encoder_phys_vid(phys_enc);

	/* Slave encoders don't report vblank */
	if (!sde_encoder_phys_vid_is_master(phys_enc))
		goto end;

	/* protect against negative */
	if (!enable && refcount == 0) {
		ret = -EINVAL;
		goto end;
	}

	SDE_DEBUG_VIDENC(vid_enc, "[%pS] enable=%d/%d\n",
			__builtin_return_address(0),
			enable, atomic_read(&phys_enc->vblank_refcount));

	SDE_EVT32(DRMID(phys_enc->parent), enable,
			atomic_read(&phys_enc->vblank_refcount));

	if (enable && atomic_inc_return(&phys_enc->vblank_refcount) == 1) {
		ret = sde_encoder_helper_register_irq(phys_enc, INTR_IDX_VSYNC);
		if (ret)
			atomic_dec_return(&phys_enc->vblank_refcount);
	} else if (!enable &&
			atomic_dec_return(&phys_enc->vblank_refcount) == 0) {
		ret = sde_encoder_helper_unregister_irq(phys_enc,
				INTR_IDX_VSYNC);
		if (ret)
			atomic_inc_return(&phys_enc->vblank_refcount);
	}

end:
	if (ret) {
		SDE_ERROR_VIDENC(vid_enc,
				"control vblank irq error %d, enable %d\n",
				ret, enable);
		SDE_EVT32(DRMID(phys_enc->parent),
				phys_enc->hw_intf->idx - INTF_0,
				enable, refcount, SDE_EVTLOG_ERROR);
	}
	mutex_unlock(phys_enc->vblank_ctl_lock);
	return ret;
}

static bool sde_encoder_phys_vid_wait_dma_trigger(
		struct sde_encoder_phys *phys_enc)
{
	struct sde_encoder_phys_vid *vid_enc;
	struct sde_hw_intf *intf;
	struct sde_hw_ctl *ctl;
	struct intf_status status;

	if (!phys_enc) {
		SDE_ERROR("invalid encoder\n");
		return false;
	}

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	intf = phys_enc->hw_intf;
	ctl = phys_enc->hw_ctl;
	if (!phys_enc->hw_intf || !phys_enc->hw_ctl) {
		SDE_ERROR("invalid hw_intf %d hw_ctl %d\n",
			phys_enc->hw_intf != NULL, phys_enc->hw_ctl != NULL);
		return false;
	}

	if (!intf->ops.get_status)
		return false;

	intf->ops.get_status(intf, &status);

	/* if interface is not enabled, return true to wait for dma trigger */
	return status.is_en ? false : true;
}

static void sde_encoder_phys_vid_enable(struct sde_encoder_phys *phys_enc)
{
	struct msm_drm_private *priv;
	struct sde_encoder_phys_vid *vid_enc;
	struct sde_hw_intf *intf;
	struct sde_hw_ctl *ctl;

	if (!phys_enc || !phys_enc->parent || !phys_enc->parent->dev ||
			!phys_enc->parent->dev->dev_private ||
			!phys_enc->sde_kms) {
		SDE_ERROR("invalid encoder/device\n");
		return;
	}
	priv = phys_enc->parent->dev->dev_private;

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	intf = phys_enc->hw_intf;
	ctl = phys_enc->hw_ctl;
	if (!phys_enc->hw_intf || !phys_enc->hw_ctl || !phys_enc->hw_pp) {
		SDE_ERROR("invalid hw_intf %d hw_ctl %d hw_pp %d\n",
				!phys_enc->hw_intf, !phys_enc->hw_ctl,
				!phys_enc->hw_pp);
		return;
	}
	if (!ctl->ops.update_bitmask) {
		SDE_ERROR("invalid hw_ctl ops %d\n", ctl->idx);
		return;
	}

	SDE_DEBUG_VIDENC(vid_enc, "\n");

	if (WARN_ON(!phys_enc->hw_intf->ops.enable_timing))
		return;

	if (!phys_enc->cont_splash_enabled)
		sde_encoder_helper_split_config(phys_enc,
				phys_enc->hw_intf->idx);

	sde_encoder_phys_vid_setup_timing_engine(phys_enc);

	/*
	 * For cases where both the interfaces are connected to same ctl,
	 * set the flush bit for both master and slave.
	 * For single flush cases (dual-ctl or pp-split), skip setting the
	 * flush bit for the slave intf, since both intfs use same ctl
	 * and HW will only flush the master.
	 */
	if (!test_bit(SDE_CTL_ACTIVE_CFG, &ctl->caps->features) &&
			sde_encoder_phys_needs_single_flush(phys_enc) &&
		!sde_encoder_phys_vid_is_master(phys_enc))
		goto skip_flush;

	/**
	 * skip flushing intf during cont. splash handoff since bootloader
	 * has already enabled the hardware and is single buffered.
	 */
	if (phys_enc->cont_splash_enabled) {
		SDE_DEBUG_VIDENC(vid_enc,
		"skipping intf flush bit set as cont. splash is enabled\n");
		goto skip_flush;
	}

	ctl->ops.update_bitmask(ctl, SDE_HW_FLUSH_INTF, intf->idx, 1);

	if (phys_enc->hw_pp->merge_3d)
		ctl->ops.update_bitmask(ctl, SDE_HW_FLUSH_MERGE_3D,
			phys_enc->hw_pp->merge_3d->idx, 1);

	if (phys_enc->hw_intf->cap->type == INTF_DP &&
		phys_enc->comp_type == MSM_DISPLAY_COMPRESSION_DSC &&
		phys_enc->comp_ratio)
		ctl->ops.update_bitmask(ctl, SDE_HW_FLUSH_PERIPH, intf->idx, 1);

skip_flush:
	SDE_DEBUG_VIDENC(vid_enc, "update pending flush ctl %d intf %d\n",
		ctl->idx - CTL_0, intf->idx);
	SDE_EVT32(DRMID(phys_enc->parent),
		atomic_read(&phys_enc->pending_retire_fence_cnt));

	/* ctl_flush & timing engine enable will be triggered by framework */
	if (phys_enc->enable_state == SDE_ENC_DISABLED)
		phys_enc->enable_state = SDE_ENC_ENABLING;
}

static void sde_encoder_phys_vid_destroy(struct sde_encoder_phys *phys_enc)
{
	struct sde_encoder_phys_vid *vid_enc;

	if (!phys_enc) {
		SDE_ERROR("invalid encoder\n");
		return;
	}

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	SDE_DEBUG_VIDENC(vid_enc, "\n");
	kfree(vid_enc);
}

static void sde_encoder_phys_vid_get_hw_resources(
		struct sde_encoder_phys *phys_enc,
		struct sde_encoder_hw_resources *hw_res,
		struct drm_connector_state *conn_state)
{
	struct sde_encoder_phys_vid *vid_enc;

	if (!phys_enc || !hw_res) {
		SDE_ERROR("invalid arg(s), enc %d hw_res %d conn_state %d\n",
				!phys_enc, !hw_res, !conn_state);
		return;
	}

	if ((phys_enc->intf_idx - INTF_0) >= INTF_MAX) {
		SDE_ERROR("invalid intf idx:%d\n", phys_enc->intf_idx);
		return;
	}

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	SDE_DEBUG_VIDENC(vid_enc, "\n");
	hw_res->intfs[phys_enc->intf_idx - INTF_0] = INTF_MODE_VIDEO;
}

static int _sde_encoder_phys_vid_wait_for_vblank(
		struct sde_encoder_phys *phys_enc, bool notify)
{
	struct sde_encoder_wait_info wait_info = {0};
	int ret = 0;
	u32 event = SDE_ENCODER_FRAME_EVENT_ERROR |
		SDE_ENCODER_FRAME_EVENT_SIGNAL_RELEASE_FENCE |
		SDE_ENCODER_FRAME_EVENT_SIGNAL_RETIRE_FENCE;
	struct drm_connector *conn;

	if (!phys_enc) {
		pr_err("invalid encoder\n");
		return -EINVAL;
	}

	conn = phys_enc->connector;

	wait_info.wq = &phys_enc->pending_kickoff_wq;
	wait_info.atomic_cnt = &phys_enc->pending_kickoff_cnt;
	wait_info.timeout_ms = phys_enc->kickoff_timeout_ms;

	/* Wait for kickoff to complete */
	ret = sde_encoder_helper_wait_for_irq(phys_enc, INTR_IDX_VSYNC,
			&wait_info);

	if (notify && (ret == -ETIMEDOUT) &&
	    atomic_add_unless(&phys_enc->pending_retire_fence_cnt, -1, 0) &&
	    phys_enc->parent_ops.handle_frame_done) {
		phys_enc->parent_ops.handle_frame_done(
			phys_enc->parent, phys_enc, event);

		if (sde_encoder_recovery_events_enabled(phys_enc->parent))
			sde_connector_event_notify(conn,
				DRM_EVENT_SDE_HW_RECOVERY,
				sizeof(uint8_t), SDE_RECOVERY_HARD_RESET);
	}

	SDE_EVT32(DRMID(phys_enc->parent), event, notify, ret,
			ret ? SDE_EVTLOG_FATAL : 0);
	return ret;
}

static int sde_encoder_phys_vid_wait_for_vblank(
		struct sde_encoder_phys *phys_enc)
{
	return _sde_encoder_phys_vid_wait_for_vblank(phys_enc, true);
}

static int sde_encoder_phys_vid_wait_for_commit_done(
		struct sde_encoder_phys *phys_enc)
{
	int rc;

	rc =  _sde_encoder_phys_vid_wait_for_vblank(phys_enc, true);
	if (rc)
		sde_encoder_helper_phys_reset(phys_enc);

	return rc;
}

static int sde_encoder_phys_vid_wait_for_vblank_no_notify(
		struct sde_encoder_phys *phys_enc)
{
	return _sde_encoder_phys_vid_wait_for_vblank(phys_enc, false);
}

static int sde_encoder_phys_vid_prepare_for_kickoff(
		struct sde_encoder_phys *phys_enc,
		struct sde_encoder_kickoff_params *params)
{
	struct sde_encoder_phys_vid *vid_enc;
	struct sde_hw_ctl *ctl;
	bool recovery_events;
	struct drm_connector *conn;
	int rc;
	int irq_enable;

	if (!phys_enc || !params || !phys_enc->hw_ctl) {
		SDE_ERROR("invalid encoder/parameters\n");
		return -EINVAL;
	}
	vid_enc = to_sde_encoder_phys_vid(phys_enc);

	ctl = phys_enc->hw_ctl;
	if (!ctl->ops.wait_reset_status)
		return 0;

	conn = phys_enc->connector;
	recovery_events = sde_encoder_recovery_events_enabled(
			phys_enc->parent);
	/*
	 * hw supports hardware initiated ctl reset, so before we kickoff a new
	 * frame, need to check and wait for hw initiated ctl reset completion
	 */
	rc = ctl->ops.wait_reset_status(ctl);
	if (rc) {
		SDE_ERROR_VIDENC(vid_enc, "ctl %d reset failure: %d\n",
				ctl->idx, rc);

		++vid_enc->error_count;

		/* to avoid flooding, only log first time, and "dead" time */
		if (vid_enc->error_count == 1) {
			SDE_EVT32(DRMID(phys_enc->parent), SDE_EVTLOG_FATAL);
			mutex_lock(phys_enc->vblank_ctl_lock);

			irq_enable = atomic_read(&phys_enc->vblank_refcount);

			if (irq_enable)
				sde_encoder_helper_unregister_irq(
					phys_enc, INTR_IDX_VSYNC);

			SDE_DBG_DUMP(SDE_DBG_BUILT_IN_ALL);

			if (irq_enable)
				sde_encoder_helper_register_irq(
					phys_enc, INTR_IDX_VSYNC);

			mutex_unlock(phys_enc->vblank_ctl_lock);
		}

		/*
		 * if the recovery event is registered by user, don't panic
		 * trigger panic on first timeout if no listener registered
		 */
		if (recovery_events)
			sde_connector_event_notify(conn, DRM_EVENT_SDE_HW_RECOVERY,
					sizeof(uint8_t), SDE_RECOVERY_CAPTURE);
		else
			SDE_DBG_DUMP(0x0, "panic");

		/* request a ctl reset before the next flush */
		phys_enc->enable_state = SDE_ENC_ERR_NEEDS_HW_RESET;
	} else {
		if (recovery_events && vid_enc->error_count)
			sde_connector_event_notify(conn,
					DRM_EVENT_SDE_HW_RECOVERY,
					sizeof(uint8_t),
					SDE_RECOVERY_SUCCESS);
		vid_enc->error_count = 0;
	}

	return rc;
}

static void sde_encoder_phys_vid_single_vblank_wait(
		struct sde_encoder_phys *phys_enc)
{
	int ret;
	struct sde_encoder_phys_vid *vid_enc
					= to_sde_encoder_phys_vid(phys_enc);

	/*
	 * Wait for a vsync so we know the ENABLE=0 latched before
	 * the (connector) source of the vsync's gets disabled,
	 * otherwise we end up in a funny state if we re-enable
	 * before the disable latches, which results that some of
	 * the settings changes for the new modeset (like new
	 * scanout buffer) don't latch properly..
	 */
	ret = sde_encoder_phys_vid_control_vblank_irq(phys_enc, true);
	if (ret) {
		SDE_ERROR_VIDENC(vid_enc,
				"failed to enable vblank irq: %d\n",
				ret);
		SDE_EVT32(DRMID(phys_enc->parent),
				phys_enc->hw_intf->idx - INTF_0, ret,
				SDE_EVTLOG_FUNC_CASE1,
				SDE_EVTLOG_ERROR);
	} else {
		ret = _sde_encoder_phys_vid_wait_for_vblank(phys_enc, false);
		if (ret) {
			atomic_set(&phys_enc->pending_kickoff_cnt, 0);
			SDE_ERROR_VIDENC(vid_enc,
					"failure waiting for disable: %d\n",
					ret);
			SDE_EVT32(DRMID(phys_enc->parent),
					phys_enc->hw_intf->idx - INTF_0, ret,
					SDE_EVTLOG_FUNC_CASE2,
					SDE_EVTLOG_ERROR);
		}
		sde_encoder_phys_vid_control_vblank_irq(phys_enc, false);
	}
}

static void sde_encoder_phys_vid_disable(struct sde_encoder_phys *phys_enc)
{
	struct msm_drm_private *priv;
	struct sde_encoder_phys_vid *vid_enc;
	unsigned long lock_flags;
	struct intf_status intf_status = {0};

	if (!phys_enc || !phys_enc->parent || !phys_enc->parent->dev ||
			!phys_enc->parent->dev->dev_private) {
		SDE_ERROR("invalid encoder/device\n");
		return;
	}
	priv = phys_enc->parent->dev->dev_private;

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	if (!phys_enc->hw_intf || !phys_enc->hw_ctl) {
		SDE_ERROR("invalid hw_intf %d hw_ctl %d\n",
				!phys_enc->hw_intf, !phys_enc->hw_ctl);
		return;
	}

	SDE_DEBUG_VIDENC(vid_enc, "\n");

	if (WARN_ON(!phys_enc->hw_intf->ops.enable_timing))
		return;
	else if (!sde_encoder_phys_vid_is_master(phys_enc))
		goto exit;

	if (phys_enc->enable_state == SDE_ENC_DISABLED) {
		SDE_ERROR("already disabled\n");
		return;
	}

	if (sde_in_trusted_vm(phys_enc->sde_kms))
		goto exit;

	spin_lock_irqsave(phys_enc->enc_spinlock, lock_flags);
	phys_enc->hw_intf->ops.enable_timing(phys_enc->hw_intf, 0);
	sde_encoder_phys_inc_pending(phys_enc);
	spin_unlock_irqrestore(phys_enc->enc_spinlock, lock_flags);

	if (phys_enc->hw_intf->ops.reset_counter)
		phys_enc->hw_intf->ops.reset_counter(phys_enc->hw_intf);

	sde_encoder_phys_vid_single_vblank_wait(phys_enc);
	if (phys_enc->hw_intf->ops.get_status)
		phys_enc->hw_intf->ops.get_status(phys_enc->hw_intf,
			&intf_status);

	if (intf_status.is_en) {
		spin_lock_irqsave(phys_enc->enc_spinlock, lock_flags);
		sde_encoder_phys_inc_pending(phys_enc);
		spin_unlock_irqrestore(phys_enc->enc_spinlock, lock_flags);

		sde_encoder_phys_vid_single_vblank_wait(phys_enc);
	}

	sde_encoder_helper_phys_disable(phys_enc, NULL);
exit:
	SDE_EVT32(DRMID(phys_enc->parent),
		atomic_read(&phys_enc->pending_retire_fence_cnt));
	phys_enc->vfp_cached = 0;
	phys_enc->enable_state = SDE_ENC_DISABLED;
}

static int sde_encoder_phys_vid_poll_for_active_region(struct sde_encoder_phys *phys_enc)
{
	struct sde_encoder_phys_vid *vid_enc;
	struct intf_timing_params *timing;
	u32 line_cnt, v_inactive, poll_time_us, trial = 0;

	if (!phys_enc || !phys_enc->hw_intf || !phys_enc->hw_intf->ops.get_line_count)
		return -EINVAL;

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	timing = &vid_enc->timing_params;

	/* if programmable fetch is not enabled return early or if it is not a DSI interface*/
	if (!programmable_fetch_get_num_lines(vid_enc, timing) ||
			phys_enc->hw_intf->cap->type != INTF_DSI)
		return 0;

	poll_time_us = DIV_ROUND_UP(1000000, timing->vrefresh) / MAX_POLL_CNT;
	v_inactive = timing->v_front_porch + timing->v_back_porch + timing->vsync_pulse_width;

	do {
		usleep_range(poll_time_us, poll_time_us + 5);
		line_cnt = phys_enc->hw_intf->ops.get_line_count(phys_enc->hw_intf);
		trial++;
	} while ((trial < MAX_POLL_CNT) || (line_cnt < v_inactive));

	return (trial >= MAX_POLL_CNT) ? -ETIMEDOUT : 0;
}

static void sde_encoder_phys_vid_handle_post_kickoff(
		struct sde_encoder_phys *phys_enc)
{
	unsigned long lock_flags;
	struct sde_encoder_phys_vid *vid_enc;
	u32 avr_mode;
	u32 ret;

	if (!phys_enc) {
		SDE_ERROR("invalid encoder\n");
		return;
	}

	vid_enc = to_sde_encoder_phys_vid(phys_enc);
	SDE_DEBUG_VIDENC(vid_enc, "enable_state %d\n", phys_enc->enable_state);

	/*
	 * Video mode must flush CTL before enabling timing engine
	 * Video encoders need to turn on their interfaces now
	 */
	if (phys_enc->enable_state == SDE_ENC_ENABLING) {
		if (sde_encoder_phys_vid_is_master(phys_enc)) {
			SDE_EVT32(DRMID(phys_enc->parent),
				phys_enc->hw_intf->idx - INTF_0);
			spin_lock_irqsave(phys_enc->enc_spinlock, lock_flags);
			phys_enc->hw_intf->ops.enable_timing(phys_enc->hw_intf,
				1);
			spin_unlock_irqrestore(phys_enc->enc_spinlock,
				lock_flags);

			ret = sde_encoder_phys_vid_poll_for_active_region(phys_enc);
			if (ret)
				SDE_DEBUG_VIDENC(vid_enc, "poll for active failed ret:%d\n", ret);
		}
		phys_enc->enable_state = SDE_ENC_ENABLED;
	}

	avr_mode = sde_connector_get_qsync_mode(phys_enc->connector);

	if (avr_mode && vid_enc->base.hw_intf->ops.avr_trigger) {
		vid_enc->base.hw_intf->ops.avr_trigger(vid_enc->base.hw_intf);
		SDE_EVT32(DRMID(phys_enc->parent),
				phys_enc->hw_intf->idx - INTF_0,
				SDE_EVTLOG_FUNC_CASE9);
	}
}

static void sde_encoder_phys_vid_prepare_for_commit(
		struct sde_encoder_phys *phys_enc)
{
	struct sde_connector_state *c_state;

	if (!phys_enc || !phys_enc->parent) {
		SDE_ERROR("invalid encoder parameters\n");
		return;
	}

	if (phys_enc->connector && phys_enc->connector->state) {
		c_state = to_sde_connector_state(phys_enc->connector->state);
		if (!c_state) {
			SDE_ERROR("invalid connector state\n");
			return;
		}

		if (!msm_is_mode_seamless_vrr(&c_state->msm_mode)
			&& sde_connector_is_qsync_updated(phys_enc->connector))
			_sde_encoder_phys_vid_avr_ctrl(phys_enc);

	}
}

static void sde_encoder_phys_vid_irq_control(struct sde_encoder_phys *phys_enc,
		bool enable)
{
	struct sde_encoder_phys_vid *vid_enc;
	int ret;

	if (!phys_enc)
		return;

	vid_enc = to_sde_encoder_phys_vid(phys_enc);

	SDE_EVT32(DRMID(phys_enc->parent), phys_enc->hw_intf->idx - INTF_0,
			enable, atomic_read(&phys_enc->vblank_refcount));

	if (enable) {
		ret = sde_encoder_phys_vid_control_vblank_irq(phys_enc, true);
		if (ret)
			return;

		sde_encoder_helper_register_irq(phys_enc, INTR_IDX_UNDERRUN);
	} else {
		sde_encoder_phys_vid_control_vblank_irq(phys_enc, false);
		sde_encoder_helper_unregister_irq(phys_enc, INTR_IDX_UNDERRUN);
	}
}

static int sde_encoder_phys_vid_get_line_count(
		struct sde_encoder_phys *phys_enc)
{
	if (!phys_enc)
		return -EINVAL;

	if (!sde_encoder_phys_vid_is_master(phys_enc))
		return -EINVAL;

	if (!phys_enc->hw_intf || !phys_enc->hw_intf->ops.get_line_count)
		return -EINVAL;

	return phys_enc->hw_intf->ops.get_line_count(phys_enc->hw_intf);
}

static u32 sde_encoder_phys_vid_get_underrun_line_count(
		struct sde_encoder_phys *phys_enc)
{
	u32 underrun_linecount = 0xebadebad;
	u32 intf_intr_status = 0xebadebad;
	struct intf_status intf_status = {0};

	if (!phys_enc)
		return -EINVAL;

	if (!sde_encoder_phys_vid_is_master(phys_enc) || !phys_enc->hw_intf)
		return -EINVAL;

	if (phys_enc->hw_intf->ops.get_status)
		phys_enc->hw_intf->ops.get_status(phys_enc->hw_intf,
			&intf_status);

	if (phys_enc->hw_intf->ops.get_underrun_line_count)
		underrun_linecount =
		  phys_enc->hw_intf->ops.get_underrun_line_count(
			phys_enc->hw_intf);

	if (phys_enc->hw_intf->ops.get_intr_status)
		intf_intr_status = phys_enc->hw_intf->ops.get_intr_status(
				phys_enc->hw_intf);

	SDE_EVT32(DRMID(phys_enc->parent), underrun_linecount,
		intf_status.frame_count, intf_status.line_count,
		intf_intr_status);

	return underrun_linecount;
}

static int sde_encoder_phys_vid_wait_for_active(
			struct sde_encoder_phys *phys_enc)
{
	struct drm_display_mode mode;
	struct sde_encoder_phys_vid *vid_enc;
	u32 ln_cnt, min_ln_cnt, active_lns_cnt;
	u32 retry = MAX_POLL_CNT;

	vid_enc =  to_sde_encoder_phys_vid(phys_enc);

	if (!phys_enc->hw_intf || !phys_enc->hw_intf->ops.get_line_count) {
		SDE_ERROR_VIDENC(vid_enc, "invalid vid_enc params\n");
		return -EINVAL;
	}

	mode = phys_enc->cached_mode;

	min_ln_cnt = (mode.vtotal - mode.vsync_start) +
		(mode.vsync_end - mode.vsync_start);
	active_lns_cnt = mode.vdisplay;

	while (retry) {
		ln_cnt = phys_enc->hw_intf->ops.get_line_count(
				phys_enc->hw_intf);

		if ((ln_cnt >= min_ln_cnt) &&
			(ln_cnt < (active_lns_cnt + min_ln_cnt))) {
			SDE_DEBUG_VIDENC(vid_enc,
					"Needed lines left line_cnt=%d\n",
					ln_cnt);
			return 0;
		}

		SDE_ERROR_VIDENC(vid_enc, "line count is less. line_cnt = %d\n", ln_cnt);
		udelay(POLL_TIME_USEC_FOR_LN_CNT);
		retry--;
	}

	return -EINVAL;
}

void sde_encoder_phys_vid_add_enc_to_minidump(struct sde_encoder_phys *phys_enc)
{
	struct sde_encoder_phys_vid *vid_enc;
	vid_enc =  to_sde_encoder_phys_vid(phys_enc);

	sde_mini_dump_add_va_region("sde_enc_phys_vid", sizeof(*vid_enc), vid_enc);
}

static void sde_encoder_phys_vid_init_ops(struct sde_encoder_phys_ops *ops)
{
	ops->is_master = sde_encoder_phys_vid_is_master;
	ops->mode_set = sde_encoder_phys_vid_mode_set;
	ops->cont_splash_mode_set = sde_encoder_phys_vid_cont_splash_mode_set;
	ops->mode_fixup = sde_encoder_phys_vid_mode_fixup;
	ops->enable = sde_encoder_phys_vid_enable;
	ops->disable = sde_encoder_phys_vid_disable;
	ops->destroy = sde_encoder_phys_vid_destroy;
	ops->get_hw_resources = sde_encoder_phys_vid_get_hw_resources;
	ops->control_vblank_irq = sde_encoder_phys_vid_control_vblank_irq;
	ops->wait_for_commit_done = sde_encoder_phys_vid_wait_for_commit_done;
	ops->wait_for_vblank = sde_encoder_phys_vid_wait_for_vblank_no_notify;
	ops->wait_for_tx_complete = sde_encoder_phys_vid_wait_for_vblank;
	ops->irq_control = sde_encoder_phys_vid_irq_control;
	ops->prepare_for_kickoff = sde_encoder_phys_vid_prepare_for_kickoff;
	ops->handle_post_kickoff = sde_encoder_phys_vid_handle_post_kickoff;
	ops->needs_single_flush = sde_encoder_phys_needs_single_flush;
	ops->setup_misr = sde_encoder_helper_setup_misr;
	ops->collect_misr = sde_encoder_helper_collect_misr;
	ops->trigger_flush = sde_encoder_helper_trigger_flush;
	ops->hw_reset = sde_encoder_helper_hw_reset;
	ops->get_line_count = sde_encoder_phys_vid_get_line_count;
	ops->wait_dma_trigger = sde_encoder_phys_vid_wait_dma_trigger;
	ops->wait_for_active = sde_encoder_phys_vid_wait_for_active;
	ops->prepare_commit = sde_encoder_phys_vid_prepare_for_commit;
	ops->get_underrun_line_count =
		sde_encoder_phys_vid_get_underrun_line_count;
	ops->add_to_minidump = sde_encoder_phys_vid_add_enc_to_minidump;
}

struct sde_encoder_phys *sde_encoder_phys_vid_init(
		struct sde_enc_phys_init_params *p)
{
	struct sde_encoder_phys *phys_enc = NULL;
	struct sde_encoder_phys_vid *vid_enc = NULL;
	struct sde_hw_mdp *hw_mdp;
	struct sde_encoder_irq *irq;
	int i, ret = 0;

	if (!p) {
		ret = -EINVAL;
		goto fail;
	}

	vid_enc = kzalloc(sizeof(*vid_enc), GFP_KERNEL);
	if (!vid_enc) {
		ret = -ENOMEM;
		goto fail;
	}

	phys_enc = &vid_enc->base;

	hw_mdp = sde_rm_get_mdp(&p->sde_kms->rm);
	if (IS_ERR_OR_NULL(hw_mdp)) {
		ret = PTR_ERR(hw_mdp);
		SDE_ERROR("failed to get mdptop\n");
		goto fail;
	}
	phys_enc->hw_mdptop = hw_mdp;
	phys_enc->intf_idx = p->intf_idx;

	SDE_DEBUG_VIDENC(vid_enc, "\n");

	sde_encoder_phys_vid_init_ops(&phys_enc->ops);
	phys_enc->parent = p->parent;
	phys_enc->parent_ops = p->parent_ops;
	phys_enc->sde_kms = p->sde_kms;
	phys_enc->split_role = p->split_role;
	phys_enc->intf_mode = INTF_MODE_VIDEO;
	phys_enc->enc_spinlock = p->enc_spinlock;
	phys_enc->vblank_ctl_lock = p->vblank_ctl_lock;
	phys_enc->comp_type = p->comp_type;
	phys_enc->kickoff_timeout_ms = DEFAULT_KICKOFF_TIMEOUT_MS;
	for (i = 0; i < INTR_IDX_MAX; i++) {
		irq = &phys_enc->irq[i];
		INIT_LIST_HEAD(&irq->cb.list);
		irq->irq_idx = -EINVAL;
		irq->hw_idx = -EINVAL;
		irq->cb.arg = phys_enc;
	}

	irq = &phys_enc->irq[INTR_IDX_VSYNC];
	irq->name = "vsync_irq";
	irq->intr_type = SDE_IRQ_TYPE_INTF_VSYNC;
	irq->intr_idx = INTR_IDX_VSYNC;
	irq->cb.func = sde_encoder_phys_vid_vblank_irq;

	irq = &phys_enc->irq[INTR_IDX_UNDERRUN];
	irq->name = "underrun";
	irq->intr_type = SDE_IRQ_TYPE_INTF_UNDER_RUN;
	irq->intr_idx = INTR_IDX_UNDERRUN;
	irq->cb.func = sde_encoder_phys_vid_underrun_irq;

	atomic_set(&phys_enc->vblank_refcount, 0);
	atomic_set(&phys_enc->pending_kickoff_cnt, 0);
	atomic_set(&phys_enc->pending_retire_fence_cnt, 0);
	init_waitqueue_head(&phys_enc->pending_kickoff_wq);
	phys_enc->enable_state = SDE_ENC_DISABLED;

	SDE_DEBUG_VIDENC(vid_enc, "created intf idx:%d\n", p->intf_idx);

	return phys_enc;

fail:
	SDE_ERROR("failed to create encoder\n");
	if (vid_enc)
		sde_encoder_phys_vid_destroy(phys_enc);

	return ERR_PTR(ret);
}
