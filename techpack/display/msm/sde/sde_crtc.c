/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (C) 2013 Red Hat
 * Author: Rob Clark <robdclark@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define pr_fmt(fmt)	"[drm:%s:%d] " fmt, __func__, __LINE__
#include <linux/sort.h>
#include <linux/debugfs.h>
#include <linux/ktime.h>
#include <drm/sde_drm.h>
#include <drm/drm_mode.h>
#include <drm/drm_crtc.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_flip_work.h>

#include "sde_kms.h"
#include "sde_hw_lm.h"
#include "sde_hw_ctl.h"
#include "sde_crtc.h"
#include "sde_plane.h"
#include "sde_hw_util.h"
#include "sde_hw_catalog.h"
#include "sde_color_processing.h"
#include "sde_encoder.h"
#include "sde_connector.h"
#include "sde_vbif.h"
#include "sde_power_handle.h"
#include "sde_core_perf.h"
#include "sde_trace.h"
#include "msm_drv.h"
#include "sde_vm.h"

#include "mi_sde_crtc.h"

#define SDE_PSTATES_MAX (SDE_STAGE_MAX * 4)
#define SDE_MULTIRECT_PLANE_MAX (SDE_STAGE_MAX * 2)

struct sde_crtc_custom_events {
	u32 event;
	int (*func)(struct drm_crtc *crtc, bool en,
			struct sde_irq_callback *irq);
};

struct vblank_work {
	struct kthread_work work;
	int crtc_id;
	bool enable;
	struct msm_drm_private *priv;
};

static int sde_crtc_power_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *ad_irq);
static int sde_crtc_idle_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *idle_irq);
static int sde_crtc_mmrm_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *idle_irq);
static int sde_crtc_pm_event_handler(struct drm_crtc *crtc, bool en,
		struct sde_irq_callback *noirq);
static int sde_crtc_frame_data_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *idle_irq);
static int _sde_crtc_set_noise_layer(struct sde_crtc *sde_crtc,
				struct sde_crtc_state *cstate,
				void __user *usr_ptr);
static int sde_crtc_vm_release_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *irq);


static struct sde_crtc_custom_events custom_events[] = {
	{DRM_EVENT_AD_BACKLIGHT, sde_cp_ad_interrupt},
	{DRM_EVENT_CRTC_POWER, sde_crtc_power_interrupt_handler},
	{DRM_EVENT_IDLE_NOTIFY, sde_crtc_idle_interrupt_handler},
	{DRM_EVENT_HISTOGRAM, sde_cp_hist_interrupt},
	{DRM_EVENT_SDE_POWER, sde_crtc_pm_event_handler},
	{DRM_EVENT_LTM_HIST, sde_cp_ltm_hist_interrupt},
	{DRM_EVENT_LTM_WB_PB, sde_cp_ltm_wb_pb_interrupt},
	{DRM_EVENT_LTM_OFF, sde_cp_ltm_off_event_handler},
	{DRM_EVENT_MMRM_CB, sde_crtc_mmrm_interrupt_handler},
	{DRM_EVENT_VM_RELEASE, sde_crtc_vm_release_handler},
	{DRM_EVENT_FRAME_DATA, sde_crtc_frame_data_interrupt_handler},
};

/* default input fence timeout, in ms */
#define SDE_CRTC_INPUT_FENCE_TIMEOUT    10000

/*
 * The default input fence timeout is 2 seconds while max allowed
 * range is 10 seconds. Any value above 10 seconds adds glitches beyond
 * tolerance limit.
 */
#define SDE_CRTC_MAX_INPUT_FENCE_TIMEOUT 10000

/* layer mixer index on sde_crtc */
#define LEFT_MIXER 0
#define RIGHT_MIXER 1

#define MISR_BUFF_SIZE			256

/*
 * Time period for fps calculation in micro seconds.
 * Default value is set to 1 sec.
 */
#define DEFAULT_FPS_PERIOD_1_SEC	1000000
#define MAX_FPS_PERIOD_5_SECONDS	5000000
#define MAX_FRAME_COUNT			1000
#define MILI_TO_MICRO			1000

#define SKIP_STAGING_PIPE_ZPOS		255

static void sde_crtc_install_noise_layer_properties(struct sde_crtc *sde_crtc,
		struct sde_mdss_cfg *catalog, struct sde_kms_info *info);
static void sde_cp_crtc_apply_noise(struct drm_crtc *crtc,
		struct drm_crtc_state *state);

static inline struct sde_kms *_sde_crtc_get_kms(struct drm_crtc *crtc)
{
	struct msm_drm_private *priv;

	if (!crtc || !crtc->dev || !crtc->dev->dev_private) {
		SDE_ERROR("invalid crtc\n");
		return NULL;
	}
	priv = crtc->dev->dev_private;
	if (!priv || !priv->kms) {
		SDE_ERROR("invalid kms\n");
		return NULL;
	}

	return to_sde_kms(priv->kms);
}

/**
 * sde_crtc_calc_fps() - Calculates fps value.
 * @sde_crtc   : CRTC structure
 *
 * This function is called at frame done. It counts the number
 * of frames done for every 1 sec. Stores the value in measured_fps.
 * measured_fps value is 10 times the calculated fps value.
 * For example, measured_fps= 594 for calculated fps of 59.4
 */
static void sde_crtc_calc_fps(struct sde_crtc *sde_crtc)
{
	ktime_t current_time_us;
	u64 fps, diff_us;

	current_time_us = ktime_get();
	diff_us = (u64)ktime_us_delta(current_time_us,
			sde_crtc->fps_info.last_sampled_time_us);
	sde_crtc->fps_info.frame_count++;

	if (diff_us >= DEFAULT_FPS_PERIOD_1_SEC) {

		 /* Multiplying with 10 to get fps in floating point */
		fps = ((u64)sde_crtc->fps_info.frame_count)
						* DEFAULT_FPS_PERIOD_1_SEC * 10;
		do_div(fps, diff_us);
		sde_crtc->fps_info.measured_fps = (unsigned int)fps;
		SDE_DEBUG(" FPS for crtc%d is %d.%d\n",
				sde_crtc->base.base.id, (unsigned int)fps/10,
				(unsigned int)fps%10);
		sde_crtc->fps_info.last_sampled_time_us = current_time_us;
		sde_crtc->fps_info.frame_count = 0;
	}

	if (!sde_crtc->fps_info.time_buf)
		return;

	/**
	 * Array indexing is based on sliding window algorithm.
	 * sde_crtc->time_buf has a maximum capacity of MAX_FRAME_COUNT
	 * time slots. As the count increases to MAX_FRAME_COUNT + 1, the
	 * counter loops around and comes back to the first index to store
	 * the next ktime.
	 */
	sde_crtc->fps_info.time_buf[sde_crtc->fps_info.next_time_index++] =
								ktime_get();
	sde_crtc->fps_info.next_time_index %= MAX_FRAME_COUNT;
}

static void _sde_crtc_deinit_events(struct sde_crtc *sde_crtc)
{
	if (!sde_crtc)
		return;
}

#ifdef CONFIG_DEBUG_FS
static int _sde_debugfs_fps_status_show(struct seq_file *s, void *data)
{
	struct sde_crtc *sde_crtc;
	u64 fps_int, fps_float;
	ktime_t current_time_us;
	u64 fps, diff_us;

	if (!s || !s->private) {
		SDE_ERROR("invalid input param(s)\n");
		return -EAGAIN;
	}

	sde_crtc = s->private;

	current_time_us = ktime_get();
	diff_us = (u64)ktime_us_delta(current_time_us,
			sde_crtc->fps_info.last_sampled_time_us);

	if (diff_us >= DEFAULT_FPS_PERIOD_1_SEC) {

		 /* Multiplying with 10 to get fps in floating point */
		fps = ((u64)sde_crtc->fps_info.frame_count)
						* DEFAULT_FPS_PERIOD_1_SEC * 10;
		do_div(fps, diff_us);
		sde_crtc->fps_info.measured_fps = (unsigned int)fps;
		sde_crtc->fps_info.last_sampled_time_us = current_time_us;
		sde_crtc->fps_info.frame_count = 0;
		SDE_DEBUG("Measured FPS for crtc%d is %d.%d\n",
				sde_crtc->base.base.id, (unsigned int)fps/10,
				(unsigned int)fps%10);
	}

	fps_int = (unsigned int) sde_crtc->fps_info.measured_fps;
	fps_float = do_div(fps_int, 10);

	seq_printf(s, "fps: %llu.%llu\n", fps_int, fps_float);

	return 0;
}


static int _sde_debugfs_fps_status(struct inode *inode, struct file *file)
{
	return single_open(file, _sde_debugfs_fps_status_show,
			inode->i_private);
}
#endif

static ssize_t fps_periodicity_ms_store(struct device *device,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;
	int res;

	/* Base of the input */
	int cnt = 10;

	if (!device || !buf) {
		SDE_ERROR("invalid input param(s)\n");
		return -EAGAIN;
	}

	crtc = dev_get_drvdata(device);
	if (!crtc)
		return -EINVAL;

	sde_crtc = to_sde_crtc(crtc);

	res = kstrtou32(buf, cnt, &sde_crtc->fps_info.fps_periodic_duration);
	if (res < 0)
		return res;

	if (sde_crtc->fps_info.fps_periodic_duration <= 0)
		sde_crtc->fps_info.fps_periodic_duration =
						DEFAULT_FPS_PERIOD_1_SEC;
	else if ((sde_crtc->fps_info.fps_periodic_duration) * MILI_TO_MICRO >
						MAX_FPS_PERIOD_5_SECONDS)
		sde_crtc->fps_info.fps_periodic_duration =
						MAX_FPS_PERIOD_5_SECONDS;
	else
		sde_crtc->fps_info.fps_periodic_duration *= MILI_TO_MICRO;

	return count;
}

static ssize_t fps_periodicity_ms_show(struct device *device,
		struct device_attribute *attr, char *buf)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;

	if (!device || !buf) {
		SDE_ERROR("invalid input param(s)\n");
		return -EAGAIN;
	}

	crtc = dev_get_drvdata(device);
	if (!crtc)
		return -EINVAL;

	sde_crtc = to_sde_crtc(crtc);

	return scnprintf(buf, PAGE_SIZE, "%d\n",
		(sde_crtc->fps_info.fps_periodic_duration)/MILI_TO_MICRO);
}

static ssize_t measured_fps_show(struct device *device,
		struct device_attribute *attr, char *buf)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;
	uint64_t fps_int, fps_decimal;
	u64 fps = 0, frame_count = 0;
	ktime_t current_time;
	int i = 0, current_time_index;
	u64 diff_us;

	if (!device || !buf) {
		SDE_ERROR("invalid input param(s)\n");
		return -EAGAIN;
	}

	crtc = dev_get_drvdata(device);
	if (!crtc) {
		scnprintf(buf, PAGE_SIZE, "fps information not available");
		return -EINVAL;
	}

	sde_crtc = to_sde_crtc(crtc);

	if (!sde_crtc->fps_info.time_buf) {
		scnprintf(buf, PAGE_SIZE,
				"timebuf null - fps information not available");
		return -EINVAL;
	}

	/**
	 * Whenever the time_index counter comes to zero upon decrementing,
	 * it is set to the last index since it is the next index that we
	 * should check for calculating the buftime.
	 */
	current_time_index = (sde_crtc->fps_info.next_time_index == 0) ?
		MAX_FRAME_COUNT - 1 : (sde_crtc->fps_info.next_time_index - 1);

	current_time = ktime_get();

	for (i = 0; i < MAX_FRAME_COUNT; i++) {
		u64 ptime = (u64)ktime_to_us(current_time);
		u64 buftime = (u64)ktime_to_us(
			sde_crtc->fps_info.time_buf[current_time_index]);
		diff_us = (u64)ktime_us_delta(current_time,
			sde_crtc->fps_info.time_buf[current_time_index]);
		if (ptime > buftime && diff_us >= (u64)
				sde_crtc->fps_info.fps_periodic_duration) {

			/* Multiplying with 10 to get fps in floating point */
			fps = frame_count * DEFAULT_FPS_PERIOD_1_SEC * 10;
			do_div(fps, diff_us);
			sde_crtc->fps_info.measured_fps = (unsigned int)fps;
			SDE_DEBUG("measured fps: %d\n",
					sde_crtc->fps_info.measured_fps);
			break;
		}

		current_time_index = (current_time_index == 0) ?
			(MAX_FRAME_COUNT - 1) : (current_time_index - 1);
		SDE_DEBUG("current time index: %d\n", current_time_index);

		frame_count++;
	}

	if (i == MAX_FRAME_COUNT) {

		current_time_index = (sde_crtc->fps_info.next_time_index == 0) ?
		MAX_FRAME_COUNT - 1 : (sde_crtc->fps_info.next_time_index - 1);

		diff_us = (u64)ktime_us_delta(current_time,
			sde_crtc->fps_info.time_buf[current_time_index]);

		if (diff_us >= sde_crtc->fps_info.fps_periodic_duration) {

			/* Multiplying with 10 to get fps in floating point */
			fps = (frame_count) * DEFAULT_FPS_PERIOD_1_SEC * 10;
			do_div(fps, diff_us);
			sde_crtc->fps_info.measured_fps = (unsigned int)fps;
		}
	}

	fps_int = (uint64_t) sde_crtc->fps_info.measured_fps;
	fps_decimal = do_div(fps_int, 10);
	return scnprintf(buf, PAGE_SIZE,
	"fps: %lld.%lld duration:%d frame_count:%lld\n", fps_int, fps_decimal,
			sde_crtc->fps_info.fps_periodic_duration, frame_count);
}

static ssize_t vsync_event_show(struct device *device,
	struct device_attribute *attr, char *buf)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;
	struct drm_encoder *encoder;
	int avr_status = -EPIPE;

	if (!device || !buf) {
		SDE_ERROR("invalid input param(s)\n");
		return -EAGAIN;
	}

	crtc = dev_get_drvdata(device);
	sde_crtc = to_sde_crtc(crtc);

	mutex_lock(&sde_crtc->crtc_lock);
	if (sde_crtc->enabled) {
		drm_for_each_encoder_mask(encoder, crtc->dev, crtc->state->encoder_mask) {
			if (sde_encoder_in_clone_mode(encoder))
				continue;

			avr_status = sde_encoder_get_avr_status(encoder);
			break;
		}
	}
	mutex_unlock(&sde_crtc->crtc_lock);

	return scnprintf(buf, PAGE_SIZE, "VSYNC=%llu\nAVR_STATUS=%d\n",
			ktime_to_ns(sde_crtc->vblank_last_cb_time), avr_status);
}

static ssize_t retire_frame_event_show(struct device *device,
	struct device_attribute *attr, char *buf)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;

	if (!device || !buf) {
		SDE_ERROR("invalid input param(s)\n");
		return -EAGAIN;
	}

	crtc = dev_get_drvdata(device);
	sde_crtc = to_sde_crtc(crtc);
	return scnprintf(buf, PAGE_SIZE, "RETIRE_FRAME_TIME=%llu\n",
			ktime_to_ns(sde_crtc->retire_frame_event_time));
}

static DEVICE_ATTR_RO(vsync_event);
static DEVICE_ATTR_RO(measured_fps);
static DEVICE_ATTR_RW(fps_periodicity_ms);
static DEVICE_ATTR_RO(retire_frame_event);

static struct attribute *sde_crtc_dev_attrs[] = {
	&dev_attr_vsync_event.attr,
	&dev_attr_measured_fps.attr,
	&dev_attr_fps_periodicity_ms.attr,
	&dev_attr_retire_frame_event.attr,
	NULL
};

static const struct attribute_group sde_crtc_attr_group = {
	.attrs = sde_crtc_dev_attrs,
};

static const struct attribute_group *sde_crtc_attr_groups[] = {
	&sde_crtc_attr_group,
	NULL,
};

static void sde_crtc_event_notify(struct drm_crtc *crtc, uint32_t type, void *payload, uint32_t len)
{
	struct drm_event event;
	uint32_t *data = (uint32_t *)payload;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	event.type = type;
	event.length = len;
	msm_mode_object_event_notify(&crtc->base, crtc->dev, &event, (u8 *)payload);

	SDE_EVT32(DRMID(crtc), type, len, *data,
			((uint64_t)payload) >> 32, ((uint64_t)payload) & 0xFFFFFFFF);
	SDE_DEBUG("crtc:%d event(%lu) ptr(%pK) value(%lu) notified\n",
			DRMID(crtc), type, payload, *data);
}

static void sde_crtc_destroy(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);

	SDE_DEBUG("\n");

	if (!crtc)
		return;

	if (sde_crtc->vsync_event_sf)
		sysfs_put(sde_crtc->vsync_event_sf);
	if (sde_crtc->retire_frame_event_sf)
		sysfs_put(sde_crtc->retire_frame_event_sf);
	if (sde_crtc->sysfs_dev)
		device_unregister(sde_crtc->sysfs_dev);

	if (sde_crtc->blob_info)
		drm_property_blob_put(sde_crtc->blob_info);
	msm_property_destroy(&sde_crtc->property_info);
	sde_cp_crtc_destroy_properties(crtc);

	sde_fence_deinit(sde_crtc->output_fence);
	_sde_crtc_deinit_events(sde_crtc);

	drm_crtc_cleanup(crtc);
	mutex_destroy(&sde_crtc->crtc_lock);
	kfree(sde_crtc);
}

struct msm_display_mode *sde_crtc_get_msm_mode(struct drm_crtc_state *c_state)
{
	struct drm_connector *connector;
	struct drm_encoder *encoder;
	struct sde_connector_state *conn_state;
	bool encoder_valid = false;

	drm_for_each_encoder_mask(encoder, c_state->crtc->dev,
			c_state->encoder_mask) {
		if (!sde_encoder_in_clone_mode(encoder)) {
			encoder_valid = true;
			break;
		}
	}

	if (!encoder_valid)
		return NULL;

	connector = sde_encoder_get_connector(c_state->crtc->dev, encoder);
	if (!connector)
		return NULL;

	conn_state = to_sde_connector_state(connector->state);
	if (!conn_state)
		return NULL;

	return &conn_state->msm_mode;
}

static bool sde_crtc_mode_fixup(struct drm_crtc *crtc,
		const struct drm_display_mode *mode,
		struct drm_display_mode *adjusted_mode)
{
	struct msm_display_mode *msm_mode;
	struct drm_crtc_state *c_state;
	struct drm_connector *connector;
	struct drm_encoder *encoder;
	struct drm_connector_state *new_conn_state;
	struct sde_connector_state *c_conn_state = NULL;
	bool encoder_valid = false;
	int i;

	SDE_DEBUG("\n");

	c_state = container_of(adjusted_mode, struct drm_crtc_state,
				adjusted_mode);

	drm_for_each_encoder_mask(encoder, c_state->crtc->dev,
		c_state->encoder_mask) {
		if (!sde_crtc_state_in_clone_mode(encoder, c_state)) {
			encoder_valid = true;
			break;
		}
	}

	if (!encoder_valid) {
		SDE_ERROR("encoder not found\n");
		return true;
	}

	for_each_new_connector_in_state(c_state->state, connector,
			new_conn_state, i) {
		if (new_conn_state->best_encoder == encoder) {
			c_conn_state = to_sde_connector_state(new_conn_state);
			break;
		}
	}

	if (!c_conn_state) {
		SDE_ERROR("could not get connector state\n");
		return true;
	}

	msm_mode = &c_conn_state->msm_mode;
	if ((msm_is_mode_seamless(msm_mode) ||
	     (msm_is_mode_seamless_vrr(msm_mode) ||
	      msm_is_mode_seamless_dyn_clk(msm_mode))) &&
	    (!crtc->enabled)) {
		SDE_ERROR("crtc state prevents seamless transition\n");
		return false;
	}

	return true;
}

static void _sde_crtc_setup_blend_cfg(struct sde_crtc_mixer *mixer,
	struct sde_plane_state *pstate, struct sde_format *format)
{
	uint32_t blend_op, fg_alpha, bg_alpha;
	uint32_t blend_type;
	struct sde_hw_mixer *lm = mixer->hw_lm;
	struct drm_plane_state *plane_state = &pstate->base;

	/* default to opaque blending */
	fg_alpha = sde_plane_get_property(pstate, PLANE_PROP_ALPHA);
	bg_alpha = 0xFF - fg_alpha;
	if (sde_plane_property_is_dirty(plane_state, PLANE_PROP_BG_ALPHA))
		bg_alpha = sde_plane_get_property(pstate, PLANE_PROP_BG_ALPHA);
	blend_op = SDE_BLEND_FG_ALPHA_FG_CONST | SDE_BLEND_BG_ALPHA_BG_CONST;
	blend_type = sde_plane_get_property(pstate, PLANE_PROP_BLEND_OP);

	SDE_DEBUG("blend type:0x%x blend alpha:0x%x bg_alpha:0x%x\n",
			blend_type, fg_alpha, bg_alpha);

	switch (blend_type) {

	case SDE_DRM_BLEND_OP_OPAQUE:
		blend_op = SDE_BLEND_FG_ALPHA_FG_CONST |
			SDE_BLEND_BG_ALPHA_BG_CONST;
		break;

	case SDE_DRM_BLEND_OP_PREMULTIPLIED:
		if (format->alpha_enable) {
			blend_op = SDE_BLEND_FG_ALPHA_FG_CONST |
				SDE_BLEND_BG_ALPHA_FG_PIXEL;
			if (fg_alpha != 0xff) {
				bg_alpha = fg_alpha;
				blend_op |= SDE_BLEND_BG_MOD_ALPHA |
					SDE_BLEND_BG_INV_MOD_ALPHA;
			} else {
				blend_op |= SDE_BLEND_BG_INV_ALPHA;
			}
		}
		break;

	case SDE_DRM_BLEND_OP_COVERAGE:
		if (format->alpha_enable) {
			blend_op = SDE_BLEND_FG_ALPHA_FG_PIXEL |
				SDE_BLEND_BG_ALPHA_FG_PIXEL;
			if (fg_alpha != 0xff) {
				bg_alpha = fg_alpha;
				blend_op |= SDE_BLEND_FG_MOD_ALPHA |
					SDE_BLEND_BG_MOD_ALPHA |
					SDE_BLEND_BG_INV_MOD_ALPHA;
			} else {
				blend_op |= SDE_BLEND_BG_INV_ALPHA;
			}
		}
		break;

	default:
		/* do nothing */
		break;
	}

	if (lm->ops.setup_blend_config)
		lm->ops.setup_blend_config(lm, pstate->stage, fg_alpha, bg_alpha, blend_op);

	SDE_DEBUG(
		"format: %4.4s, alpha_enable %u fg alpha:0x%x bg alpha:0x%x blend_op:0x%x\n",
		(char *) &format->base.pixel_format,
		format->alpha_enable, fg_alpha, bg_alpha, blend_op);
}

static void _sde_crtc_setup_dim_layer_cfg(struct drm_crtc *crtc,
		struct sde_crtc *sde_crtc, struct sde_crtc_mixer *mixer,
		struct sde_hw_dim_layer *dim_layer)
{
	struct sde_crtc_state *cstate;
	struct sde_hw_mixer *lm;
	struct sde_hw_dim_layer split_dim_layer;
	int i;

	if (!dim_layer->rect.w || !dim_layer->rect.h) {
		SDE_DEBUG("empty dim_layer\n");
		return;
	}

	cstate = to_sde_crtc_state(crtc->state);

	SDE_DEBUG("dim_layer - flags:%d, stage:%d\n",
			dim_layer->flags, dim_layer->stage);

	split_dim_layer.stage = dim_layer->stage;
	split_dim_layer.color_fill = dim_layer->color_fill;

	/*
	 * traverse through the layer mixers attached to crtc and find the
	 * intersecting dim layer rect in each LM and program accordingly.
	 */
	for (i = 0; i < sde_crtc->num_mixers; i++) {
		split_dim_layer.flags = dim_layer->flags;

		sde_kms_rect_intersect(&cstate->lm_roi[i], &dim_layer->rect,
					&split_dim_layer.rect);
		if (sde_kms_rect_is_null(&split_dim_layer.rect)) {
			/*
			 * no extra programming required for non-intersecting
			 * layer mixers with INCLUSIVE dim layer
			 */
			if (split_dim_layer.flags & SDE_DRM_DIM_LAYER_INCLUSIVE)
				continue;

			/*
			 * program the other non-intersecting layer mixers with
			 * INCLUSIVE dim layer of full size for uniformity
			 * with EXCLUSIVE dim layer config.
			 */
			split_dim_layer.flags &= ~SDE_DRM_DIM_LAYER_EXCLUSIVE;
			split_dim_layer.flags |= SDE_DRM_DIM_LAYER_INCLUSIVE;
			memcpy(&split_dim_layer.rect, &cstate->lm_bounds[i],
					sizeof(split_dim_layer.rect));

		} else {
			split_dim_layer.rect.x =
					split_dim_layer.rect.x -
						cstate->lm_roi[i].x;
			split_dim_layer.rect.y =
					split_dim_layer.rect.y -
						cstate->lm_roi[i].y;
		}

		SDE_EVT32(DRMID(crtc), dim_layer->stage,
				cstate->lm_roi[i].x,
				cstate->lm_roi[i].y,
				cstate->lm_roi[i].w,
				cstate->lm_roi[i].h,
				dim_layer->rect.x,
				dim_layer->rect.y,
				dim_layer->rect.w,
				dim_layer->rect.h,
				split_dim_layer.rect.x,
				split_dim_layer.rect.y,
				split_dim_layer.rect.w,
				split_dim_layer.rect.h);

		SDE_DEBUG("split_dim_layer - LM:%d, rect:{%d,%d,%d,%d}}\n",
			i, split_dim_layer.rect.x, split_dim_layer.rect.y,
			split_dim_layer.rect.w, split_dim_layer.rect.h);

		lm = mixer[i].hw_lm;
		mixer[i].mixer_op_mode |= 1 << split_dim_layer.stage;
		lm->ops.setup_dim_layer(lm, &split_dim_layer);
	}
}

void sde_crtc_get_crtc_roi(struct drm_crtc_state *state,
		const struct sde_rect **crtc_roi)
{
	struct sde_crtc_state *crtc_state;

	if (!state || !crtc_roi)
		return;

	crtc_state = to_sde_crtc_state(state);
	*crtc_roi = &crtc_state->crtc_roi;
}

bool sde_crtc_is_crtc_roi_dirty(struct drm_crtc_state *state)
{
	struct sde_crtc_state *cstate;
	struct sde_crtc *sde_crtc;

	if (!state || !state->crtc)
		return false;

	sde_crtc = to_sde_crtc(state->crtc);
	cstate = to_sde_crtc_state(state);

	return msm_property_is_dirty(&sde_crtc->property_info,
			&cstate->property_state, CRTC_PROP_ROI_V1);
}

static int _sde_crtc_set_roi_v1(struct drm_crtc_state *state,
		void __user *usr_ptr)
{
	struct drm_crtc *crtc;
	struct sde_crtc_state *cstate;
	struct sde_drm_roi_v1 roi_v1;
	int i;

	if (!state) {
		SDE_ERROR("invalid args\n");
		return -EINVAL;
	}

	cstate = to_sde_crtc_state(state);
	crtc = cstate->base.crtc;

	memset(&cstate->user_roi_list, 0, sizeof(cstate->user_roi_list));
	memset(&cstate->cached_user_roi_list, 0, sizeof(cstate->cached_user_roi_list));

	if (!usr_ptr) {
		SDE_DEBUG("crtc%d: rois cleared\n", DRMID(crtc));
		return 0;
	}

	if (copy_from_user(&roi_v1, usr_ptr, sizeof(roi_v1))) {
		SDE_ERROR("crtc%d: failed to copy roi_v1 data\n", DRMID(crtc));
		return -EINVAL;
	}

	SDE_DEBUG("crtc%d: num_rects %d\n", DRMID(crtc), roi_v1.num_rects);

	if (roi_v1.num_rects == 0) {
		SDE_DEBUG("crtc%d: rois cleared\n", DRMID(crtc));
		return 0;
	}

	if (roi_v1.num_rects > SDE_MAX_ROI_V1) {
		SDE_ERROR("crtc%d: too many rects specified: %d\n", DRMID(crtc),
				roi_v1.num_rects);
		return -EINVAL;
	}

	cstate->user_roi_list.num_rects = roi_v1.num_rects;
	for (i = 0; i < roi_v1.num_rects; ++i) {
		cstate->user_roi_list.roi[i] = roi_v1.roi[i];
		SDE_DEBUG("crtc%d: roi%d: roi (%d,%d) (%d,%d)\n",
				DRMID(crtc), i,
				cstate->user_roi_list.roi[i].x1,
				cstate->user_roi_list.roi[i].y1,
				cstate->user_roi_list.roi[i].x2,
				cstate->user_roi_list.roi[i].y2);
		SDE_EVT32_VERBOSE(DRMID(crtc),
				cstate->user_roi_list.roi[i].x1,
				cstate->user_roi_list.roi[i].y1,
				cstate->user_roi_list.roi[i].x2,
				cstate->user_roi_list.roi[i].y2);
	}

	return 0;
}

static int _sde_crtc_set_crtc_roi(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct drm_connector *conn;
	struct drm_connector_state *conn_state;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *crtc_state;
	struct sde_rect *crtc_roi;
	struct msm_mode_info mode_info;
	int i = 0;
	int rc;
	bool is_crtc_roi_dirty;
	bool is_conn_roi_dirty;

	if (!crtc || !state)
		return -EINVAL;

	sde_crtc = to_sde_crtc(crtc);
	crtc_state = to_sde_crtc_state(state);
	crtc_roi = &crtc_state->crtc_roi;

	is_crtc_roi_dirty = sde_crtc_is_crtc_roi_dirty(state);

	for_each_new_connector_in_state(state->state, conn, conn_state, i) {
		struct sde_connector *sde_conn;
		struct sde_connector_state *sde_conn_state;
		struct sde_rect conn_roi;

		if (!conn_state || conn_state->crtc != crtc)
			continue;

		rc = sde_connector_state_get_mode_info(conn_state, &mode_info);
		if (rc) {
			SDE_ERROR("failed to get mode info\n");
			return -EINVAL;
		}

		sde_conn = to_sde_connector(conn_state->connector);
		sde_conn_state = to_sde_connector_state(conn_state);

		is_conn_roi_dirty = msm_property_is_dirty(&sde_conn->property_info,
						&sde_conn_state->property_state,
						CONNECTOR_PROP_ROI_V1);

		/*
		 * Check against CRTC ROI and Connector ROI not being updated together.
		 * This restriction should be relaxed when Connector ROI scaling is
		 * supported and while in clone mode.
		 */
		if (!sde_crtc_state_in_clone_mode(sde_conn->encoder, state) &&
				is_conn_roi_dirty != is_crtc_roi_dirty) {
			SDE_ERROR("connector/crtc rois not updated together\n");
			return -EINVAL;
		}

		if (!mode_info.roi_caps.enabled)
			continue;

		/*
		 * current driver only supports same connector and crtc size,
		 * but if support for different sizes is added, driver needs
		 * to check the connector roi here to make sure is full screen
		 * for dsc 3d-mux topology that doesn't support partial update.
		 */
		if (memcmp(&sde_conn_state->rois, &crtc_state->user_roi_list,
				sizeof(crtc_state->user_roi_list))) {
			SDE_ERROR("%s: crtc -> conn roi scaling unsupported\n",
					sde_crtc->name);
			return -EINVAL;
		}

		sde_kms_rect_merge_rectangles(&sde_conn_state->rois, &conn_roi);
		SDE_DEBUG("conn_roi x:%u, y:%u, w:%u, h:%u\n",
				conn_roi.x, conn_roi.y,
				conn_roi.w, conn_roi.h);
		SDE_EVT32_VERBOSE(DRMID(crtc), DRMID(conn),
				conn_roi.x, conn_roi.y,
				conn_roi.w, conn_roi.h);
	}

	sde_kms_rect_merge_rectangles(&crtc_state->user_roi_list, crtc_roi);

	/* clear the ROI to null if it matches full screen anyways */
	if (crtc_roi->x == 0 && crtc_roi->y == 0 &&
			crtc_roi->w == state->adjusted_mode.hdisplay &&
			crtc_roi->h == state->adjusted_mode.vdisplay)
		memset(crtc_roi, 0, sizeof(*crtc_roi));

	SDE_DEBUG("%s: crtc roi (%d,%d,%d,%d)\n", sde_crtc->name,
			crtc_roi->x, crtc_roi->y, crtc_roi->w, crtc_roi->h);
	SDE_EVT32_VERBOSE(DRMID(crtc), crtc_roi->x, crtc_roi->y, crtc_roi->w,
			crtc_roi->h);

	return 0;
}

static int _sde_crtc_check_autorefresh(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *crtc_state;
	struct drm_connector *conn;
	struct drm_connector_state *conn_state;
	int i;

	if (!crtc || !state)
		return -EINVAL;

	sde_crtc = to_sde_crtc(crtc);
	crtc_state = to_sde_crtc_state(state);

	if (sde_kms_rect_is_null(&crtc_state->crtc_roi))
		return 0;

	/* partial update active, check if autorefresh is also requested */
	for_each_new_connector_in_state(state->state, conn, conn_state, i) {
		uint64_t autorefresh;

		if (!conn_state || conn_state->crtc != crtc)
			continue;

		autorefresh = sde_connector_get_property(conn_state,
				CONNECTOR_PROP_AUTOREFRESH);
		if (autorefresh) {
			SDE_ERROR(
				"%s: autorefresh & partial crtc roi incompatible %llu\n",
					sde_crtc->name, autorefresh);
			return -EINVAL;
		}
	}

	return 0;
}

static int _sde_crtc_set_lm_roi(struct drm_crtc *crtc,
		struct drm_crtc_state *state, int lm_idx)
{
	struct sde_kms *sde_kms;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *crtc_state;
	const struct sde_rect *crtc_roi;
	const struct sde_rect *lm_bounds;
	struct sde_rect *lm_roi;

	if (!crtc || !state || lm_idx >= ARRAY_SIZE(crtc_state->lm_bounds))
		return -EINVAL;

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms || !sde_kms->catalog) {
		SDE_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	sde_crtc = to_sde_crtc(crtc);
	crtc_state = to_sde_crtc_state(state);
	crtc_roi = &crtc_state->crtc_roi;
	lm_bounds = &crtc_state->lm_bounds[lm_idx];
	lm_roi = &crtc_state->lm_roi[lm_idx];

	if (sde_kms_rect_is_null(crtc_roi))
		memcpy(lm_roi, lm_bounds, sizeof(*lm_roi));
	else
		sde_kms_rect_intersect(crtc_roi, lm_bounds, lm_roi);

	SDE_DEBUG("%s: lm%d roi (%d,%d,%d,%d)\n", sde_crtc->name, lm_idx,
			lm_roi->x, lm_roi->y, lm_roi->w, lm_roi->h);

	/*
	 * partial update is not supported with 3dmux dsc or dest scaler.
	 * hence, crtc roi must match the mixer dimensions.
	 */
	if (crtc_state->num_ds_enabled ||
		sde_rm_topology_is_group(&sde_kms->rm, state,
				SDE_RM_TOPOLOGY_GROUP_3DMERGE_DSC)) {
		if (memcmp(lm_roi, lm_bounds, sizeof(struct sde_rect))) {
			SDE_ERROR("Unsupported: Dest scaler/3d mux DSC + PU\n");
			return -EINVAL;
		}
	}

	/* if any dimension is zero, clear all dimensions for clarity */
	if (sde_kms_rect_is_null(lm_roi))
		memset(lm_roi, 0, sizeof(*lm_roi));

	return 0;
}

static u32 _sde_crtc_get_displays_affected(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *crtc_state;
	u32 disp_bitmask = 0;
	int i;

	if (!crtc || !state) {
		pr_err("Invalid crtc or state\n");
		return 0;
	}

	sde_crtc = to_sde_crtc(crtc);
	crtc_state = to_sde_crtc_state(state);

	/* pingpong split: one ROI, one LM, two physical displays */
	if (crtc_state->is_ppsplit) {
		u32 lm_split_width = crtc_state->lm_bounds[0].w / 2;
		struct sde_rect *roi = &crtc_state->lm_roi[0];

		if (sde_kms_rect_is_null(roi))
			disp_bitmask = 0;
		else if ((u32)roi->x + (u32)roi->w <= lm_split_width)
			disp_bitmask = BIT(0);		/* left only */
		else if (roi->x >= lm_split_width)
			disp_bitmask = BIT(1);		/* right only */
		else
			disp_bitmask = BIT(0) | BIT(1); /* left and right */
	} else if (sde_crtc->mixers_swapped) {
		disp_bitmask = BIT(0);
	} else {
		for (i = 0; i < sde_crtc->num_mixers; i++) {
			if (!sde_kms_rect_is_null(
					&crtc_state->lm_roi[i]))
				disp_bitmask |= BIT(i);
		}
	}

	SDE_DEBUG("affected displays 0x%x\n", disp_bitmask);

	return disp_bitmask;
}

static int _sde_crtc_check_rois_centered_and_symmetric(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *crtc_state;
	const struct sde_rect *roi[MAX_MIXERS_PER_CRTC];

	if (!crtc || !state)
		return -EINVAL;

	sde_crtc = to_sde_crtc(crtc);
	crtc_state = to_sde_crtc_state(state);

	if (sde_crtc->num_mixers > MAX_MIXERS_PER_CRTC) {
		SDE_ERROR("%s: unsupported number of mixers: %d\n",
				sde_crtc->name, sde_crtc->num_mixers);
		return -EINVAL;
	}

	/*
	 * If using pingpong split: one ROI, one LM, two physical displays
	 * then the ROI must be centered on the panel split boundary and
	 * be of equal width across the split.
	 */
	if (crtc_state->is_ppsplit) {
		u16 panel_split_width;
		u32 display_mask;

		roi[0] = &crtc_state->lm_roi[0];

		if (sde_kms_rect_is_null(roi[0]))
			return 0;

		display_mask = _sde_crtc_get_displays_affected(crtc, state);
		if (display_mask != (BIT(0) | BIT(1)))
			return 0;

		panel_split_width = crtc_state->lm_bounds[0].w / 2;
		if (roi[0]->x + roi[0]->w / 2 != panel_split_width) {
			SDE_ERROR("%s: roi x %d w %d split %d\n",
					sde_crtc->name, roi[0]->x, roi[0]->w,
					panel_split_width);
			return -EINVAL;
		}

		return 0;
	}

	/*
	 * On certain HW, if using 2 LM, ROIs must be split evenly between the
	 * LMs and be of equal width.
	 */
	if (sde_crtc->num_mixers < CRTC_DUAL_MIXERS_ONLY)
		return 0;

	roi[0] = &crtc_state->lm_roi[0];
	roi[1] = &crtc_state->lm_roi[1];

	/* if one of the roi is null it's a left/right-only update */
	if (sde_kms_rect_is_null(roi[0]) || sde_kms_rect_is_null(roi[1]))
		return 0;

	/* check lm rois are equal width & first roi ends at 2nd roi */
	if (roi[0]->x + roi[0]->w != roi[1]->x || roi[0]->w != roi[1]->w) {
		SDE_ERROR(
			"%s: rois not centered and symmetric: roi0 x %d w %d roi1 x %d w %d\n",
				sde_crtc->name, roi[0]->x, roi[0]->w,
				roi[1]->x, roi[1]->w);
		return -EINVAL;
	}

	return 0;
}

static int _sde_crtc_check_planes_within_crtc_roi(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *crtc_state;
	const struct sde_rect *crtc_roi;
	const struct drm_plane_state *pstate;
	struct drm_plane *plane;

	if (!crtc || !state)
		return -EINVAL;

	/*
	 * Reject commit if a Plane CRTC destination coordinates fall outside
	 * the partial CRTC ROI. LM output is determined via connector ROIs,
	 * if they are specified, not Plane CRTC ROIs.
	 */

	sde_crtc = to_sde_crtc(crtc);
	crtc_state = to_sde_crtc_state(state);
	crtc_roi = &crtc_state->crtc_roi;

	if (sde_kms_rect_is_null(crtc_roi))
		return 0;

	drm_atomic_crtc_state_for_each_plane_state(plane, pstate, state) {
		struct sde_rect plane_roi, intersection;

		if (IS_ERR_OR_NULL(pstate)) {
			int rc = PTR_ERR(pstate);

			SDE_ERROR("%s: failed to get plane%d state, %d\n",
					sde_crtc->name, plane->base.id, rc);
			return rc;
		}

		plane_roi.x = pstate->crtc_x;
		plane_roi.y = pstate->crtc_y;
		plane_roi.w = pstate->crtc_w;
		plane_roi.h = pstate->crtc_h;
		sde_kms_rect_intersect(crtc_roi, &plane_roi, &intersection);
		if (!sde_kms_rect_is_equal(&plane_roi, &intersection)) {
			SDE_ERROR(
				"%s: plane%d crtc roi (%d,%d,%d,%d) outside crtc roi (%d,%d,%d,%d)\n",
					sde_crtc->name, plane->base.id,
					plane_roi.x, plane_roi.y,
					plane_roi.w, plane_roi.h,
					crtc_roi->x, crtc_roi->y,
					crtc_roi->w, crtc_roi->h);
			return -E2BIG;
		}
	}

	return 0;
}

static int _sde_crtc_check_rois(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *sde_crtc_state;
	struct msm_mode_info mode_info;
	int rc, lm_idx, i;

	if (!crtc || !state)
		return -EINVAL;

	memset(&mode_info, 0, sizeof(mode_info));

	sde_crtc = to_sde_crtc(crtc);
	sde_crtc_state = to_sde_crtc_state(state);

	/*
	 * check connector array cached at modeset time since incoming atomic
	 * state may not include any connectors if they aren't modified
	 */
	for (i = 0; i < sde_crtc_state->num_connectors; i++) {
		struct drm_connector *conn = sde_crtc_state->connectors[i];

		if (!conn || !conn->state)
			continue;

		rc = sde_connector_state_get_mode_info(conn->state, &mode_info);
		if (rc) {
			SDE_ERROR("failed to get mode info\n");
			return -EINVAL;
		}

		if (!mode_info.roi_caps.enabled)
			continue;

		if (sde_crtc_state->user_roi_list.num_rects >
				mode_info.roi_caps.num_roi) {
			SDE_ERROR("roi count is exceeding limit, %d > %d\n",
					sde_crtc_state->user_roi_list.num_rects,
					mode_info.roi_caps.num_roi);
			return -E2BIG;
		}

		rc = _sde_crtc_set_crtc_roi(crtc, state);
		if (rc)
			return rc;

		rc = _sde_crtc_check_autorefresh(crtc, state);
		if (rc)
			return rc;

		for (lm_idx = 0; lm_idx < sde_crtc->num_mixers; lm_idx++) {
			rc = _sde_crtc_set_lm_roi(crtc, state, lm_idx);
			if (rc)
				return rc;
		}

		rc = _sde_crtc_check_rois_centered_and_symmetric(crtc, state);
		if (rc)
			return rc;

		rc = _sde_crtc_check_planes_within_crtc_roi(crtc, state);
		if (rc)
			return rc;
	}

	return 0;
}

static void _sde_crtc_program_lm_output_roi(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	const struct sde_rect *lm_roi;
	struct sde_hw_mixer *hw_lm;
	bool right_mixer = false;
	bool lm_updated = false;
	int lm_idx;

	if (!crtc)
		return;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);

	for (lm_idx = 0; lm_idx < sde_crtc->num_mixers; lm_idx++) {
		struct sde_hw_mixer_cfg cfg;

		lm_roi = &cstate->lm_roi[lm_idx];
		hw_lm = sde_crtc->mixers[lm_idx].hw_lm;
		if (!sde_crtc->mixers_swapped)
			right_mixer = lm_idx % MAX_MIXERS_PER_LAYOUT;

		if (lm_roi->w != hw_lm->cfg.out_width ||
				lm_roi->h != hw_lm->cfg.out_height ||
				right_mixer != hw_lm->cfg.right_mixer) {
			hw_lm->cfg.out_width = lm_roi->w;
			hw_lm->cfg.out_height = lm_roi->h;
			hw_lm->cfg.right_mixer = right_mixer;

			cfg.out_width = lm_roi->w;
			cfg.out_height = lm_roi->h;
			cfg.right_mixer = right_mixer;
			cfg.flags = 0;

			if (hw_lm->ops.setup_mixer_out)
				hw_lm->ops.setup_mixer_out(hw_lm, &cfg);
			lm_updated = true;
		}

		SDE_EVT32(DRMID(crtc), lm_idx, lm_roi->x, lm_roi->y, lm_roi->w,
				lm_roi->h, right_mixer, lm_updated);
	}

	if (lm_updated)
		sde_cp_crtc_res_change(crtc);
}

struct plane_state {
	struct sde_plane_state *sde_pstate;
	const struct drm_plane_state *drm_pstate;
	int stage;
	u32 pipe_id;
};

static int pstate_cmp(const void *a, const void *b)
{
	struct plane_state *pa = (struct plane_state *)a;
	struct plane_state *pb = (struct plane_state *)b;
	int rc = 0;
	int pa_zpos, pb_zpos;
	enum sde_layout pa_layout, pb_layout;

	if ((!pa || !pa->sde_pstate) || (!pb || !pb->sde_pstate))
		return rc;

	pa_zpos = sde_plane_get_property(pa->sde_pstate, PLANE_PROP_ZPOS);
	pb_zpos = sde_plane_get_property(pb->sde_pstate, PLANE_PROP_ZPOS);

	pa_layout = pa->sde_pstate->layout;
	pb_layout = pb->sde_pstate->layout;

	if (pa_zpos != pb_zpos)
		rc = pa_zpos - pb_zpos;
	else if (pa_layout != pb_layout)
		rc = pa_layout - pb_layout;
	else
		rc = pa->drm_pstate->crtc_x - pb->drm_pstate->crtc_x;

	return rc;
}

/*
 * validate and set source split:
 * use pstates sorted by stage to check planes on same stage
 * we assume that all pipes are in source split so its valid to compare
 * without taking into account left/right mixer placement
 */
static int _sde_crtc_validate_src_split_order(struct drm_crtc *crtc,
		struct plane_state *pstates, int cnt)
{
	struct plane_state *prv_pstate, *cur_pstate;
	enum sde_layout prev_layout, cur_layout;
	struct sde_rect left_rect, right_rect;
	struct sde_kms *sde_kms;
	int32_t left_pid, right_pid;
	int32_t stage;
	int i, rc = 0;

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms || !sde_kms->catalog) {
		SDE_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	for (i = 1; i < cnt; i++) {
		prv_pstate = &pstates[i - 1];
		cur_pstate = &pstates[i];
		prev_layout = prv_pstate->sde_pstate->layout;
		cur_layout = cur_pstate->sde_pstate->layout;

		if (prv_pstate->stage != cur_pstate->stage ||
				prev_layout != cur_layout)
			continue;

		stage = cur_pstate->stage;

		left_pid = prv_pstate->sde_pstate->base.plane->base.id;
		POPULATE_RECT(&left_rect, prv_pstate->drm_pstate->crtc_x,
			prv_pstate->drm_pstate->crtc_y,
			prv_pstate->drm_pstate->crtc_w,
			prv_pstate->drm_pstate->crtc_h, false);

		right_pid = cur_pstate->sde_pstate->base.plane->base.id;
		POPULATE_RECT(&right_rect, cur_pstate->drm_pstate->crtc_x,
			cur_pstate->drm_pstate->crtc_y,
			cur_pstate->drm_pstate->crtc_w,
			cur_pstate->drm_pstate->crtc_h, false);

		if (right_rect.x < left_rect.x) {
			swap(left_pid, right_pid);
			swap(left_rect, right_rect);
			swap(prv_pstate, cur_pstate);
		}

		/*
		 * - planes are enumerated in pipe-priority order such that
		 *   planes with lower drm_id must be left-most in a shared
		 *   blend-stage when using source split.
		 * - planes in source split must be contiguous in width
		 * - planes in source split must have same dest yoff and height
		 */
		if ((right_pid < left_pid) &&
			!sde_kms->catalog->pipe_order_type) {
			SDE_ERROR(
			  "invalid src split cfg, stage:%d left:%d right:%d\n",
				stage, left_pid, right_pid);
			return -EINVAL;
		} else if (right_rect.x != (left_rect.x + left_rect.w)) {
			SDE_ERROR(
			  "invalid coordinates, stage:%d l:%d-%d r:%d-%d\n",
				stage, left_rect.x, left_rect.w,
				right_rect.x, right_rect.w);
			return -EINVAL;
		} else if ((left_rect.y != right_rect.y) ||
				(left_rect.h != right_rect.h)) {
			SDE_ERROR(
			  "stage:%d invalid yoff/ht: l_yxh:%dx%d r_yxh:%dx%d\n",
				stage, left_rect.y, left_rect.h,
				right_rect.y, right_rect.h);
			return -EINVAL;
		}
	}

	return rc;
}

static void _sde_crtc_set_src_split_order(struct drm_crtc *crtc,
		struct plane_state *pstates, int cnt)
{
	struct plane_state *prv_pstate, *cur_pstate, *nxt_pstate;
	enum sde_layout prev_layout, cur_layout;
	struct sde_kms *sde_kms;
	struct sde_rect left_rect, right_rect;
	int32_t left_pid, right_pid;
	int32_t stage;
	int i;

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms || !sde_kms->catalog) {
		SDE_ERROR("invalid parameters\n");
		return;
	}

	if (!sde_kms->catalog->pipe_order_type)
		return;

	for (i = 0; i < cnt; i++) {
		prv_pstate = (i > 0) ? &pstates[i - 1] : NULL;
		cur_pstate = &pstates[i];
		nxt_pstate = ((i + 1) < cnt) ? &pstates[i + 1] : NULL;
		prev_layout = prv_pstate ? prv_pstate->sde_pstate->layout :
							SDE_LAYOUT_NONE;
		cur_layout = cur_pstate->sde_pstate->layout;

		if ((!prv_pstate) || (prv_pstate->stage != cur_pstate->stage)
				|| (prev_layout != cur_layout)) {
			/*
			 * reset if prv or nxt pipes are not in the same stage
			 * as the cur pipe
			 */
			if ((!nxt_pstate)
				    || (nxt_pstate->stage != cur_pstate->stage)
				    || (nxt_pstate->sde_pstate->layout !=
					cur_pstate->sde_pstate->layout))
				cur_pstate->sde_pstate->pipe_order_flags = 0;

			continue;
		}

		stage = cur_pstate->stage;

		left_pid = prv_pstate->sde_pstate->base.plane->base.id;
		POPULATE_RECT(&left_rect, prv_pstate->drm_pstate->crtc_x,
			prv_pstate->drm_pstate->crtc_y,
			prv_pstate->drm_pstate->crtc_w,
			prv_pstate->drm_pstate->crtc_h, false);

		right_pid = cur_pstate->sde_pstate->base.plane->base.id;
		POPULATE_RECT(&right_rect, cur_pstate->drm_pstate->crtc_x,
			cur_pstate->drm_pstate->crtc_y,
			cur_pstate->drm_pstate->crtc_w,
			cur_pstate->drm_pstate->crtc_h, false);

		if (right_rect.x < left_rect.x) {
			swap(left_pid, right_pid);
			swap(left_rect, right_rect);
			swap(prv_pstate, cur_pstate);
		}

		cur_pstate->sde_pstate->pipe_order_flags = SDE_SSPP_RIGHT;
		prv_pstate->sde_pstate->pipe_order_flags = 0;
	}

	for (i = 0; i < cnt; i++) {
		cur_pstate = &pstates[i];
		sde_plane_setup_src_split_order(
			cur_pstate->drm_pstate->plane,
			cur_pstate->sde_pstate->multirect_index,
			cur_pstate->sde_pstate->pipe_order_flags);
	}
}

static void _sde_crtc_setup_blend_cfg_by_stage(struct sde_crtc_mixer *mixer,
		int num_mixers, struct plane_state *pstates, int cnt)
{
	int i, lm_idx;
	struct sde_format *format;
	bool blend_stage[SDE_STAGE_MAX] = { false };
	u32 blend_type;

	for (i = cnt - 1; i >= 0; i--) {
		blend_type = sde_plane_get_property(pstates[i].sde_pstate,
				PLANE_PROP_BLEND_OP);
		/* stage has already been programmed or BLEND_OP_SKIP type */
		if (blend_stage[pstates[i].sde_pstate->stage] ||
				blend_type == SDE_DRM_BLEND_OP_SKIP)
			continue;

		for (lm_idx = 0; lm_idx < num_mixers; lm_idx++) {
			format = to_sde_format(msm_framebuffer_format(
					pstates[i].sde_pstate->base.fb));
			if (!format) {
				SDE_ERROR("invalid format\n");
				return;
			}

			_sde_crtc_setup_blend_cfg(mixer + lm_idx,
					pstates[i].sde_pstate, format);
			blend_stage[pstates[i].sde_pstate->stage] = true;
		}
	}
}

static void _sde_crtc_blend_setup_mixer(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state, struct sde_crtc *sde_crtc,
		struct sde_crtc_mixer *mixer)
{
	struct drm_plane *plane;
	struct drm_framebuffer *fb;
	struct drm_plane_state *state;
	struct sde_crtc_state *cstate;
	struct sde_plane_state *pstate = NULL;
	struct plane_state *pstates = NULL;
	struct sde_format *format;
	struct sde_hw_ctl *ctl;
	struct sde_hw_mixer *lm;
	struct sde_hw_stage_cfg *stage_cfg;
	struct sde_rect plane_crtc_roi;
	uint32_t stage_idx, lm_idx, layout_idx;
	int zpos_cnt[MAX_LAYOUTS_PER_CRTC][SDE_STAGE_MAX + 1];
	int i, mode, cnt = 0;
	bool bg_alpha_enable = false;
	u32 blend_type;
	struct sde_cp_crtc_skip_blend_plane skip_blend_plane;
	DECLARE_BITMAP(fetch_active, SSPP_MAX);

	if (!sde_crtc || !crtc->state || !mixer) {
		SDE_ERROR("invalid sde_crtc or mixer\n");
		return;
	}

	ctl = mixer->hw_ctl;
	lm = mixer->hw_lm;
	cstate = to_sde_crtc_state(crtc->state);
	pstates = kcalloc(SDE_PSTATES_MAX,
			sizeof(struct plane_state), GFP_KERNEL);
	if (!pstates)
		return;

	memset(fetch_active, 0, sizeof(fetch_active));
	memset(zpos_cnt, 0, sizeof(zpos_cnt));

	drm_atomic_crtc_for_each_plane(plane, crtc) {
		state = plane->state;
		if (!state)
			continue;

		plane_crtc_roi.x = state->crtc_x;
		plane_crtc_roi.y = state->crtc_y;
		plane_crtc_roi.w = state->crtc_w;
		plane_crtc_roi.h = state->crtc_h;

		pstate = to_sde_plane_state(state);
		fb = state->fb;

		mode = sde_plane_get_property(pstate,
				PLANE_PROP_FB_TRANSLATION_MODE);

		set_bit(sde_plane_pipe(plane), fetch_active);
		sde_plane_ctl_flush(plane, ctl, true);

		SDE_DEBUG("crtc %d stage:%d - plane %d sspp %d fb %d\n",
				crtc->base.id,
				pstate->stage,
				plane->base.id,
				sde_plane_pipe(plane) - SSPP_VIG0,
				state->fb ? state->fb->base.id : -1);

		format = to_sde_format(msm_framebuffer_format(pstate->base.fb));
		if (!format) {
			SDE_ERROR("invalid format\n");
			goto end;
		}

		blend_type = sde_plane_get_property(pstate,
					PLANE_PROP_BLEND_OP);

		if (blend_type == SDE_DRM_BLEND_OP_SKIP) {
			skip_blend_plane.valid_plane = true;
			skip_blend_plane.plane = sde_plane_pipe(plane);
			skip_blend_plane.height = plane_crtc_roi.h;
			skip_blend_plane.width = plane_crtc_roi.w;
			sde_cp_set_skip_blend_plane_info(crtc, &skip_blend_plane);
		}

		if (blend_type != SDE_DRM_BLEND_OP_SKIP) {
			if (pstate->stage == SDE_STAGE_BASE &&
					format->alpha_enable)
				bg_alpha_enable = true;

			SDE_EVT32(DRMID(crtc), DRMID(plane),
					state->fb ? state->fb->base.id : -1,
					state->src_x >> 16, state->src_y >> 16,
					state->src_w >> 16, state->src_h >> 16,
					state->crtc_x, state->crtc_y,
					state->crtc_w, state->crtc_h,
					pstate->rotation, mode);

			/*
			 * none or left layout will program to layer mixer
			 * group 0, right layout will program to layer mixer
			 * group 1.
			 */
			if (pstate->layout <= SDE_LAYOUT_LEFT)
				layout_idx = 0;
			else
				layout_idx = 1;

			stage_cfg = &sde_crtc->stage_cfg[layout_idx];
			stage_idx = zpos_cnt[layout_idx][pstate->stage]++;
			stage_cfg->stage[pstate->stage][stage_idx] =
						sde_plane_pipe(plane);
			stage_cfg->multirect_index[pstate->stage][stage_idx] =
						pstate->multirect_index;

			SDE_EVT32(DRMID(crtc), DRMID(plane), stage_idx,
					sde_plane_pipe(plane) - SSPP_VIG0,
					pstate->stage,
					pstate->multirect_index,
					pstate->multirect_mode,
					format->base.pixel_format,
					fb ? fb->modifier : 0,
					layout_idx);

			for (lm_idx = 0; lm_idx < sde_crtc->num_mixers;
							lm_idx++) {
				if (bg_alpha_enable && !format->alpha_enable)
					mixer[lm_idx].mixer_op_mode = 0;
				else
					mixer[lm_idx].mixer_op_mode |=
							1 << pstate->stage;
			}
		}

		if (cnt >= SDE_PSTATES_MAX)
			continue;

		pstates[cnt].sde_pstate = pstate;
		pstates[cnt].drm_pstate = state;

		if (blend_type == SDE_DRM_BLEND_OP_SKIP)
			pstates[cnt].stage = SKIP_STAGING_PIPE_ZPOS;
		else
			pstates[cnt].stage = sde_plane_get_property(
				pstates[cnt].sde_pstate, PLANE_PROP_ZPOS);
		pstates[cnt].pipe_id = sde_plane_pipe(plane);

		cnt++;
	}

	/* blend config update */
	_sde_crtc_setup_blend_cfg_by_stage(mixer, sde_crtc->num_mixers,
			pstates, cnt);

	if (ctl->ops.set_active_pipes)
		ctl->ops.set_active_pipes(ctl, fetch_active);

	sort(pstates, cnt, sizeof(pstates[0]), pstate_cmp, NULL);
	_sde_crtc_set_src_split_order(crtc, pstates, cnt);

	if (lm && lm->ops.setup_dim_layer) {
		cstate = to_sde_crtc_state(crtc->state);
		if (test_bit(SDE_CRTC_DIRTY_DIM_LAYERS, cstate->dirty)) {
			for (i = 0; i < cstate->num_dim_layers; i++)
				_sde_crtc_setup_dim_layer_cfg(crtc, sde_crtc,
						mixer, &cstate->dim_layer[i]);
			clear_bit(SDE_CRTC_DIRTY_DIM_LAYERS, cstate->dirty);
		}
	}

end:
	kfree(pstates);
}

static void _sde_crtc_swap_mixers_for_right_partial_update(
		struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_encoder *drm_enc;
	bool is_right_only;
	bool encoder_in_dsc_merge = false;

	if (!crtc || !crtc->state)
		return;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);

	if (sde_crtc->num_mixers != CRTC_DUAL_MIXERS_ONLY)
		return;

	drm_for_each_encoder_mask(drm_enc, crtc->dev,
			crtc->state->encoder_mask) {
		if (sde_encoder_is_dsc_merge(drm_enc)) {
			encoder_in_dsc_merge = true;
			break;
		}
	}

	/**
	 * For right-only partial update with DSC merge, we swap LM0 & LM1.
	 * This is due to two reasons:
	 * - On 8996, there is a DSC HW requirement that in DSC Merge Mode,
	 *   the left DSC must be used, right DSC cannot be used alone.
	 *   For right-only partial update, this means swap layer mixers to map
	 *   Left LM to Right INTF. On later HW this was relaxed.
	 * - In DSC Merge mode, the physical encoder has already registered
	 *   PP0 as the master, to switch to right-only we would have to
	 *   reprogram to be driven by PP1 instead.
	 * To support both cases, we prefer to support the mixer swap solution.
	 */
	if (!encoder_in_dsc_merge) {
		if (sde_crtc->mixers_swapped) {
			swap(sde_crtc->mixers[0], sde_crtc->mixers[1]);
			sde_crtc->mixers_swapped = false;
			SDE_EVT32(SDE_EVTLOG_FUNC_CASE1);
		}
		return;
	}

	is_right_only = sde_kms_rect_is_null(&cstate->lm_roi[0]) &&
			!sde_kms_rect_is_null(&cstate->lm_roi[1]);

	if (is_right_only && !sde_crtc->mixers_swapped) {
		/* right-only update swap mixers */
		swap(sde_crtc->mixers[0], sde_crtc->mixers[1]);
		sde_crtc->mixers_swapped = true;
	} else if (!is_right_only && sde_crtc->mixers_swapped) {
		/* left-only or full update, swap back */
		swap(sde_crtc->mixers[0], sde_crtc->mixers[1]);
		sde_crtc->mixers_swapped = false;
	}

	SDE_DEBUG("%s: right_only %d swapped %d, mix0->lm%d, mix1->lm%d\n",
			sde_crtc->name, is_right_only, sde_crtc->mixers_swapped,
			sde_crtc->mixers[0].hw_lm->idx - LM_0,
			sde_crtc->mixers[1].hw_lm->idx - LM_0);
	SDE_EVT32(DRMID(crtc), is_right_only, sde_crtc->mixers_swapped,
			sde_crtc->mixers[0].hw_lm->idx - LM_0,
			sde_crtc->mixers[1].hw_lm->idx - LM_0);
}

/**
 * _sde_crtc_blend_setup - configure crtc mixers
 * @crtc: Pointer to drm crtc structure
 * @old_state: Pointer to old crtc state
 * @add_planes: Whether or not to add planes to mixers
 */
static void _sde_crtc_blend_setup(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state, bool add_planes)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *sde_crtc_state;
	struct sde_crtc_mixer *mixer;
	struct sde_hw_ctl *ctl;
	struct sde_hw_mixer *lm;
	struct sde_ctl_flush_cfg cfg = {0,};

	int i;

	if (!crtc)
		return;

	sde_crtc = to_sde_crtc(crtc);
	sde_crtc_state = to_sde_crtc_state(crtc->state);
	mixer = sde_crtc->mixers;

	SDE_DEBUG("%s\n", sde_crtc->name);

	if (sde_crtc->num_mixers > MAX_MIXERS_PER_CRTC) {
		SDE_ERROR("invalid number mixers: %d\n", sde_crtc->num_mixers);
		return;
	}

	if (test_bit(SDE_CRTC_DIRTY_DIM_LAYERS, &sde_crtc->revalidate_mask)) {
		set_bit(SDE_CRTC_DIRTY_DIM_LAYERS, sde_crtc_state->dirty);
		clear_bit(SDE_CRTC_DIRTY_DIM_LAYERS, &sde_crtc->revalidate_mask);
	}

	for (i = 0; i < sde_crtc->num_mixers; i++) {
		if (!mixer[i].hw_lm) {
			SDE_ERROR("invalid lm or ctl assigned to mixer\n");
			return;
		}
		mixer[i].mixer_op_mode = 0;

		if (test_bit(SDE_CRTC_DIRTY_DIM_LAYERS,
				sde_crtc_state->dirty)) {
			/* clear dim_layer settings */
			lm = mixer[i].hw_lm;
			if (lm->ops.clear_dim_layer)
				lm->ops.clear_dim_layer(lm);
		}
	}

	_sde_crtc_swap_mixers_for_right_partial_update(crtc);

	/* initialize stage cfg */
	memset(&sde_crtc->stage_cfg, 0, sizeof(sde_crtc->stage_cfg));

	if (add_planes)
		_sde_crtc_blend_setup_mixer(crtc, old_state, sde_crtc, mixer);

	for (i = 0; i < sde_crtc->num_mixers; i++) {
		const struct sde_rect *lm_roi = &sde_crtc_state->lm_roi[i];
		int lm_layout = i / MAX_MIXERS_PER_LAYOUT;

		ctl = mixer[i].hw_ctl;
		lm = mixer[i].hw_lm;

		if (sde_kms_rect_is_null(lm_roi))
			sde_crtc->mixers[i].mixer_op_mode = 0;

		if (lm->ops.setup_alpha_out)
			lm->ops.setup_alpha_out(lm, mixer[i].mixer_op_mode);

		/* stage config flush mask */
		ctl->ops.update_bitmask_mixer(ctl, mixer[i].hw_lm->idx, 1);
		ctl->ops.get_pending_flush(ctl, &cfg);

		SDE_DEBUG("lm %d, op_mode 0x%X, ctl %d, flush mask 0x%x\n",
			mixer[i].hw_lm->idx - LM_0,
			mixer[i].mixer_op_mode,
			ctl->idx - CTL_0,
			cfg.pending_flush_mask);

		if (sde_kms_rect_is_null(lm_roi)) {
			SDE_DEBUG(
				"%s: lm%d leave ctl%d mask 0 since null roi\n",
					sde_crtc->name, lm->idx - LM_0,
					ctl->idx - CTL_0);
			ctl->ops.setup_blendstage(ctl, mixer[i].hw_lm->idx,
					NULL, true);
		} else {
			ctl->ops.setup_blendstage(ctl, mixer[i].hw_lm->idx,
					&sde_crtc->stage_cfg[lm_layout],
					false);
		}
	}

	_sde_crtc_program_lm_output_roi(crtc);
}

int sde_crtc_find_plane_fb_modes(struct drm_crtc *crtc,
		uint32_t *fb_ns, uint32_t *fb_sec, uint32_t *fb_sec_dir)
{
	struct drm_plane *plane;
	struct sde_plane_state *sde_pstate;
	uint32_t mode = 0;
	int rc;

	if (!crtc) {
		SDE_ERROR("invalid state\n");
		return -EINVAL;
	}

	*fb_ns = 0;
	*fb_sec = 0;
	*fb_sec_dir = 0;
	drm_atomic_crtc_for_each_plane(plane, crtc) {
		if (IS_ERR_OR_NULL(plane) || IS_ERR_OR_NULL(plane->state)) {
			rc = PTR_ERR(plane);
			SDE_ERROR("crtc%d failed to get plane%d state%d\n",
					DRMID(crtc), DRMID(plane), rc);
			return rc;
		}
		sde_pstate = to_sde_plane_state(plane->state);
		mode = sde_plane_get_property(sde_pstate,
				PLANE_PROP_FB_TRANSLATION_MODE);

		switch (mode) {
		case SDE_DRM_FB_NON_SEC:
			(*fb_ns)++;
			break;
		case SDE_DRM_FB_SEC:
			(*fb_sec)++;
			break;
		case SDE_DRM_FB_SEC_DIR_TRANS:
			(*fb_sec_dir)++;
			break;
		case SDE_DRM_FB_NON_SEC_DIR_TRANS:
			break;
		default:
			SDE_ERROR("Error: Plane[%d], fb_trans_mode:%d",
					DRMID(plane), mode);
			return -EINVAL;
		}
	}
	return 0;
}

int sde_crtc_state_find_plane_fb_modes(struct drm_crtc_state *state,
		uint32_t *fb_ns, uint32_t *fb_sec, uint32_t *fb_sec_dir)
{
	struct drm_plane *plane;
	const struct drm_plane_state *pstate;
	struct sde_plane_state *sde_pstate;
	uint32_t mode = 0;
	int rc;

	if (!state) {
		SDE_ERROR("invalid state\n");
		return -EINVAL;
	}

	*fb_ns = 0;
	*fb_sec = 0;
	*fb_sec_dir = 0;
	drm_atomic_crtc_state_for_each_plane_state(plane, pstate, state) {
		if (IS_ERR_OR_NULL(pstate)) {
			rc = PTR_ERR(pstate);
			SDE_ERROR("crtc%d failed to get plane%d state%d\n",
					DRMID(state->crtc), DRMID(plane), rc);
			return rc;
		}
		sde_pstate = to_sde_plane_state(pstate);
		mode = sde_plane_get_property(sde_pstate,
				PLANE_PROP_FB_TRANSLATION_MODE);

		switch (mode) {
		case SDE_DRM_FB_NON_SEC:
			(*fb_ns)++;
			break;
		case SDE_DRM_FB_SEC:
			(*fb_sec)++;
			break;
		case SDE_DRM_FB_SEC_DIR_TRANS:
			(*fb_sec_dir)++;
			break;
		case SDE_DRM_FB_NON_SEC_DIR_TRANS:
			break;
		default:
			SDE_ERROR("Error: Plane[%d], fb_trans_mode:%d",
					DRMID(plane), mode);
			return -EINVAL;
		}
	}
	return 0;
}

static void _sde_drm_fb_sec_dir_trans(
	struct sde_kms_smmu_state_data *smmu_state, uint32_t secure_level,
	struct sde_mdss_cfg *catalog, bool old_valid_fb, int *ops)
{
	/* secure display usecase */
	if ((smmu_state->state == ATTACHED)
			&& (secure_level == SDE_DRM_SEC_ONLY)) {
		smmu_state->state = catalog->sui_ns_allowed ?
			DETACH_SEC_REQ : DETACH_ALL_REQ;
		smmu_state->secure_level = secure_level;
		smmu_state->transition_type = PRE_COMMIT;
		*ops |= SDE_KMS_OPS_SECURE_STATE_CHANGE;
		if (old_valid_fb)
			*ops |= (SDE_KMS_OPS_WAIT_FOR_TX_DONE  |
					SDE_KMS_OPS_CLEANUP_PLANE_FB);
		if (catalog->sui_misr_supported)
			smmu_state->sui_misr_state =
				SUI_MISR_ENABLE_REQ;
	/* secure camera usecase */
	} else if (smmu_state->state == ATTACHED) {
		smmu_state->state = DETACH_SEC_REQ;
		smmu_state->secure_level = secure_level;
		smmu_state->transition_type = PRE_COMMIT;
		*ops |= SDE_KMS_OPS_SECURE_STATE_CHANGE;
	}
}

static void _sde_drm_fb_transactions(
	struct sde_kms_smmu_state_data *smmu_state,
	struct sde_mdss_cfg *catalog, bool old_valid_fb, bool post_commit,
	int *ops)
{
	if (((smmu_state->state == DETACHED)
				|| (smmu_state->state == DETACH_ALL_REQ))
			|| ((smmu_state->secure_level == SDE_DRM_SEC_ONLY)
				&& ((smmu_state->state == DETACHED_SEC)
				|| (smmu_state->state == DETACH_SEC_REQ)))) {
		smmu_state->state = catalog->sui_ns_allowed ?
			ATTACH_SEC_REQ : ATTACH_ALL_REQ;
		smmu_state->transition_type = post_commit ?
			POST_COMMIT : PRE_COMMIT;
		*ops |= SDE_KMS_OPS_SECURE_STATE_CHANGE;
		if (old_valid_fb)
			*ops |= SDE_KMS_OPS_WAIT_FOR_TX_DONE;
		if (catalog->sui_misr_supported)
			smmu_state->sui_misr_state =
				SUI_MISR_DISABLE_REQ;
	} else if ((smmu_state->state == DETACHED_SEC)
			|| (smmu_state->state == DETACH_SEC_REQ)) {
		smmu_state->state = ATTACH_SEC_REQ;
		smmu_state->transition_type = post_commit ?
			POST_COMMIT : PRE_COMMIT;
		*ops |= SDE_KMS_OPS_SECURE_STATE_CHANGE;
		if (old_valid_fb)
			*ops |= SDE_KMS_OPS_WAIT_FOR_TX_DONE;
	}
}

/**
 * sde_crtc_get_secure_transition_ops - determines the operations that
 * need to be performed before transitioning to secure state
 * This function should be called after swapping the new state
 * @crtc: Pointer to drm crtc structure
 * Returns the bitmask of operations need to be performed, -Error in
 * case of error cases
 */
int sde_crtc_get_secure_transition_ops(struct drm_crtc *crtc,
		struct drm_crtc_state *old_crtc_state,
		bool old_valid_fb)
{
	struct drm_plane *plane;
	struct drm_encoder *encoder;
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;
	struct sde_mdss_cfg *catalog;
	struct sde_kms_smmu_state_data *smmu_state;
	uint32_t translation_mode = 0, secure_level;
	int ops  = 0;
	bool post_commit = false;

	if (!crtc || !crtc->state) {
		SDE_ERROR("invalid crtc\n");
		return -EINVAL;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms)
		return -EINVAL;

	smmu_state = &sde_kms->smmu_state;
	smmu_state->prev_state = smmu_state->state;
	smmu_state->prev_secure_level = smmu_state->secure_level;

	sde_crtc = to_sde_crtc(crtc);
	secure_level = sde_crtc_get_secure_level(crtc, crtc->state);
	catalog = sde_kms->catalog;

	/*
	 * SMMU operations need to be delayed in case of video mode panels
	 * when switching back to non_secure mode
	 */
	drm_for_each_encoder_mask(encoder, crtc->dev,
			crtc->state->encoder_mask) {
		if (sde_encoder_is_dsi_display(encoder))
			post_commit |= sde_encoder_check_curr_mode(encoder,
						MSM_DISPLAY_VIDEO_MODE);
	}

	SDE_DEBUG("crtc%d: secure_level %d old_valid_fb %d post_commit %d\n",
			DRMID(crtc), secure_level, old_valid_fb, post_commit);
	SDE_EVT32_VERBOSE(DRMID(crtc), secure_level, smmu_state->state,
			old_valid_fb, post_commit, SDE_EVTLOG_FUNC_ENTRY);

	drm_atomic_crtc_for_each_plane(plane, crtc) {
		if (!plane->state)
			continue;

		translation_mode = sde_plane_get_property(
				to_sde_plane_state(plane->state),
				PLANE_PROP_FB_TRANSLATION_MODE);
		if (translation_mode > SDE_DRM_FB_SEC_DIR_TRANS) {
			SDE_ERROR("crtc%d: invalid translation_mode %d\n",
					DRMID(crtc), translation_mode);
			return -EINVAL;
		}

		/* we can break if we find sec_dir plane */
		if (translation_mode == SDE_DRM_FB_SEC_DIR_TRANS)
			break;
	}

	mutex_lock(&sde_kms->secure_transition_lock);

	switch (translation_mode) {
	case SDE_DRM_FB_SEC_DIR_TRANS:
		_sde_drm_fb_sec_dir_trans(smmu_state, secure_level,
				catalog, old_valid_fb, &ops);
		break;

	case SDE_DRM_FB_SEC:
	case SDE_DRM_FB_NON_SEC:
		_sde_drm_fb_transactions(smmu_state, catalog,
				old_valid_fb, post_commit, &ops);
		break;

	case SDE_DRM_FB_NON_SEC_DIR_TRANS:
		ops = 0;
		break;

	default:
		SDE_ERROR("crtc%d: invalid plane fb_mode %d\n",
				DRMID(crtc), translation_mode);
		ops = -EINVAL;
	}

	/* log only during actual transition times */
	if (ops) {
		SDE_DEBUG("crtc%d: state%d sec%d sec_lvl%d type%d ops%x\n",
			DRMID(crtc), smmu_state->state,
			secure_level, smmu_state->secure_level,
			smmu_state->transition_type, ops);
		SDE_EVT32(DRMID(crtc), secure_level, translation_mode,
				smmu_state->state, smmu_state->transition_type,
				smmu_state->secure_level, old_valid_fb,
				post_commit, ops, SDE_EVTLOG_FUNC_EXIT);
	}

	mutex_unlock(&sde_kms->secure_transition_lock);

	return ops;
}

/**
 * _sde_crtc_setup_scaler3_lut - Set up scaler lut
 * LUTs are configured only once during boot
 * @sde_crtc: Pointer to sde crtc
 * @cstate: Pointer to sde crtc state
 */
static int _sde_crtc_set_dest_scaler_lut(struct sde_crtc *sde_crtc,
		struct sde_crtc_state *cstate, uint32_t lut_idx)
{
	struct sde_hw_scaler3_lut_cfg *cfg;
	struct sde_kms *sde_kms;
	u32 *lut_data = NULL;
	size_t len = 0;
	int ret = 0;

	if (!sde_crtc || !cstate) {
		SDE_ERROR("invalid args\n");
		return -EINVAL;
	}

	sde_kms = _sde_crtc_get_kms(&sde_crtc->base);
	if (!sde_kms)
		return -EINVAL;

	if (is_qseed3_rev_qseed3lite(sde_kms->catalog))
		return 0;

	lut_data = msm_property_get_blob(&sde_crtc->property_info,
			&cstate->property_state, &len, lut_idx);
	if (!lut_data || !len) {
		SDE_DEBUG("%s: lut(%d): cleared: %pK, %zu\n", sde_crtc->name,
				lut_idx, lut_data, len);
		lut_data = NULL;
		len = 0;
	}

	cfg = &cstate->scl3_lut_cfg;

	switch (lut_idx) {
	case CRTC_PROP_DEST_SCALER_LUT_ED:
		cfg->dir_lut = lut_data;
		cfg->dir_len = len;
		break;
	case CRTC_PROP_DEST_SCALER_LUT_CIR:
		cfg->cir_lut = lut_data;
		cfg->cir_len = len;
		break;
	case CRTC_PROP_DEST_SCALER_LUT_SEP:
		cfg->sep_lut = lut_data;
		cfg->sep_len = len;
		break;
	default:
		ret = -EINVAL;
		SDE_ERROR("%s:invalid LUT idx(%d)\n", sde_crtc->name, lut_idx);
		SDE_EVT32(DRMID(&sde_crtc->base), lut_idx, SDE_EVTLOG_ERROR);
		break;
	}

	cfg->is_configured = cfg->dir_lut && cfg->cir_lut && cfg->sep_lut;

	SDE_EVT32_VERBOSE(DRMID(&sde_crtc->base), ret, lut_idx, len,
			cfg->is_configured);
	return ret;
}

void sde_crtc_timeline_status(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	sde_fence_timeline_status(sde_crtc->output_fence, &crtc->base);
}

static int _sde_validate_hw_resources(struct sde_crtc *sde_crtc)
{
	int i;

	/**
	 * Check if sufficient hw resources are
	 * available as per target caps & topology
	 */
	if (!sde_crtc) {
		SDE_ERROR("invalid argument\n");
		return -EINVAL;
	}

	if (!sde_crtc->num_mixers ||
		sde_crtc->num_mixers > MAX_MIXERS_PER_CRTC) {
		SDE_ERROR("%s: invalid number mixers: %d\n",
			sde_crtc->name, sde_crtc->num_mixers);
		SDE_EVT32(DRMID(&sde_crtc->base), sde_crtc->num_mixers,
			SDE_EVTLOG_ERROR);
		return -EINVAL;
	}

	for (i = 0; i < sde_crtc->num_mixers; i++) {
		if (!sde_crtc->mixers[i].hw_lm || !sde_crtc->mixers[i].hw_ctl
			|| !sde_crtc->mixers[i].hw_ds) {
			SDE_ERROR("%s:insufficient resources for mixer(%d)\n",
				sde_crtc->name, i);
			SDE_EVT32(DRMID(&sde_crtc->base), sde_crtc->num_mixers,
				i, sde_crtc->mixers[i].hw_lm,
				sde_crtc->mixers[i].hw_ctl,
				sde_crtc->mixers[i].hw_ds, SDE_EVTLOG_ERROR);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * _sde_crtc_dest_scaler_setup - Set up dest scaler block
 * @crtc: Pointer to drm crtc
 */
static void _sde_crtc_dest_scaler_setup(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct sde_hw_mixer *hw_lm;
	struct sde_hw_ctl *hw_ctl;
	struct sde_hw_ds *hw_ds;
	struct sde_hw_ds_cfg *cfg;
	struct sde_kms *kms;
	u32 op_mode = 0;
	u32 lm_idx = 0, num_mixers = 0;
	int i, count = 0;

	if (!crtc)
		return;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);
	kms = _sde_crtc_get_kms(crtc);
	num_mixers = sde_crtc->num_mixers;
	count = cstate->num_ds;

	SDE_DEBUG("crtc%d\n", crtc->base.id);
	SDE_EVT32(DRMID(crtc), num_mixers, count, cstate->dirty[0],
		cstate->num_ds_enabled);

	if (!test_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty)) {
		SDE_DEBUG("no change in settings, skip commit\n");
	} else if (!kms || !kms->catalog) {
		SDE_ERROR("crtc%d:invalid parameters\n", crtc->base.id);
	} else if (!kms->catalog->mdp[0].has_dest_scaler) {
		SDE_DEBUG("dest scaler feature not supported\n");
	} else if (_sde_validate_hw_resources(sde_crtc)) {
		//do nothing
	} else if ((!cstate->scl3_lut_cfg.is_configured) &&
			(!is_qseed3_rev_qseed3lite(kms->catalog))) {
		SDE_ERROR("crtc%d:no LUT data available\n", crtc->base.id);
	} else {
		for (i = 0; i < count; i++) {
			cfg = &cstate->ds_cfg[i];

			if (!cfg->flags)
				continue;

			lm_idx = cfg->idx;
			hw_lm  = sde_crtc->mixers[lm_idx].hw_lm;
			hw_ctl = sde_crtc->mixers[lm_idx].hw_ctl;
			hw_ds  = sde_crtc->mixers[lm_idx].hw_ds;

			/* Setup op mode - Dual/single */
			if (cfg->flags & SDE_DRM_DESTSCALER_ENABLE)
				op_mode |= BIT(hw_ds->idx - DS_0);

			if ((i == count-1) && hw_ds->ops.setup_opmode) {
				op_mode |= (cstate->num_ds_enabled ==
					CRTC_DUAL_MIXERS_ONLY) ?
					SDE_DS_OP_MODE_DUAL : 0;
				hw_ds->ops.setup_opmode(hw_ds, op_mode);
				SDE_EVT32_VERBOSE(DRMID(crtc), op_mode);
			}

			/* Setup scaler */
			if ((cfg->flags & SDE_DRM_DESTSCALER_SCALE_UPDATE) ||
				(cfg->flags &
					SDE_DRM_DESTSCALER_ENHANCER_UPDATE)) {
				if (hw_ds->ops.setup_scaler)
					hw_ds->ops.setup_scaler(hw_ds,
						&cfg->scl3_cfg,
						&cstate->scl3_lut_cfg);

			}

			/*
			 * Dest scaler shares the flush bit of the LM in control
			 */
			if (hw_ctl && hw_ctl->ops.update_bitmask_mixer)
				hw_ctl->ops.update_bitmask_mixer(
						hw_ctl, hw_lm->idx, 1);
		}
	}
}

static void _sde_crtc_put_frame_data_buffer(struct sde_frame_data_buffer *buf)
{
	if (!buf)
		return;

	msm_gem_put_buffer(buf->gem);
	kfree(buf);
	buf = NULL;
}

static int _sde_crtc_get_frame_data_buffer(struct drm_crtc *crtc, uint32_t fd)
{
	struct sde_crtc *sde_crtc;
	struct sde_frame_data_buffer *buf;
	uint32_t cur_buf;

	sde_crtc = to_sde_crtc(crtc);
	cur_buf = sde_crtc->frame_data.cnt;

	buf = kzalloc(sizeof(struct sde_frame_data_buffer), GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	sde_crtc->frame_data.buf[cur_buf] = buf;
	buf->fd = fd;
	buf->fb = drm_framebuffer_lookup(crtc->dev, NULL, fd);
	if (!buf->fb) {
		SDE_ERROR("unable to get fb");
		return -EINVAL;
	}

	buf->gem = msm_framebuffer_bo(buf->fb, 0);
	if (!buf->gem) {
		SDE_ERROR("unable to get drm gem");
		return -EINVAL;
	}

	return msm_gem_get_buffer(buf->gem, crtc->dev, buf->fb,
			sizeof(struct sde_drm_frame_data_packet));
}

static void _sde_crtc_set_frame_data_buffers(struct drm_crtc *crtc,
			struct sde_crtc_state *cstate, void __user *usr)
{
	struct sde_crtc *sde_crtc;
	struct sde_drm_frame_data_buffers_ctrl ctrl;
	int i, ret;

	if (!crtc || !cstate || !usr)
		return;

	sde_crtc = to_sde_crtc(crtc);

	ret = copy_from_user(&ctrl, usr, sizeof(ctrl));
	if (ret) {
		SDE_ERROR("failed to copy frame data ctrl, ret %d\n", ret);
		return;
	}

	if (!ctrl.num_buffers) {
		SDE_DEBUG("clearing frame data buffers");
		goto exit;
	} else if (ctrl.num_buffers > SDE_FRAME_DATA_BUFFER_MAX) {
		SDE_ERROR("invalid number of buffers %d", ctrl.num_buffers);
		return;
	}

	for (i = 0; i < ctrl.num_buffers; i++) {
		if (_sde_crtc_get_frame_data_buffer(crtc, ctrl.fds[i])) {
			SDE_ERROR("unable to set buffer for fd %d", ctrl.fds[i]);
			goto exit;
		}
		sde_crtc->frame_data.cnt++;
	}

	return;
exit:
	while (sde_crtc->frame_data.cnt--)
		_sde_crtc_put_frame_data_buffer(
				sde_crtc->frame_data.buf[sde_crtc->frame_data.cnt]);
	sde_crtc->frame_data.cnt = 0;
}

static void _sde_crtc_frame_data_notify(struct drm_crtc *crtc,
		struct sde_drm_frame_data_packet *frame_data_packet)
{
	struct sde_crtc *sde_crtc;
	struct sde_drm_frame_data_buf buf;
	struct msm_gem_object *msm_gem;
	u32 cur_buf;

	sde_crtc = to_sde_crtc(crtc);
	cur_buf = sde_crtc->frame_data.idx;
	msm_gem = to_msm_bo(sde_crtc->frame_data.buf[cur_buf]->gem);

	buf.fd = sde_crtc->frame_data.buf[cur_buf]->fd;
	buf.offset = msm_gem->offset;

	sde_crtc_event_notify(crtc, DRM_EVENT_FRAME_DATA, &buf,
			sizeof(struct sde_drm_frame_data_buf));

	sde_crtc->frame_data.idx = ++sde_crtc->frame_data.idx % sde_crtc->frame_data.cnt;
}

void sde_crtc_get_frame_data(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc = NULL;
	struct drm_plane *plane = NULL;
	struct sde_drm_frame_data_packet frame_data_packet = {0, 0};
	struct sde_drm_frame_data_packet *data = NULL;
	struct sde_frame_data *frame_data = NULL;
	int i = 0;

	if (!crtc || !crtc->state)
		return;

	sde_crtc = to_sde_crtc(crtc);
	frame_data = &sde_crtc->frame_data;

	if (frame_data->cnt) {
		struct msm_gem_object *msm_gem;

		msm_gem = to_msm_bo(frame_data->buf[frame_data->idx]->gem);
		data = (struct sde_drm_frame_data_packet *)
				(((u8 *)msm_gem->vaddr) + msm_gem->offset);
	} else {
		data = &frame_data_packet;
	}

	data->commit_count = sde_crtc->play_count;
	data->frame_count = sde_crtc->fps_info.frame_count;

	/* Collect plane specific data */
	SDE_EVT32(DRMID(crtc), frame_data->cnt, data->commit_count, data->frame_count, sde_crtc->plane_mask_old);
	drm_for_each_plane_mask(plane, crtc->dev, sde_crtc->plane_mask_old) {
		SDE_EVT32(DRMID(crtc), plane->base.id, sde_plane_pipe(plane) - SSPP_VIG0);
		sde_plane_get_frame_data(plane, &data->plane_frame_data[i]);
	}

	if (frame_data->cnt)
		_sde_crtc_frame_data_notify(crtc, data);
}

static void sde_crtc_frame_event_cb(void *data, u32 event, ktime_t ts)
{
	struct drm_crtc *crtc = (struct drm_crtc *)data;
	struct sde_crtc *sde_crtc;
	struct msm_drm_private *priv;
	struct sde_crtc_frame_event *fevent;
	struct sde_kms_frame_event_cb_data *cb_data;
	unsigned long flags;
	u32 crtc_id;

	cb_data = (struct sde_kms_frame_event_cb_data *)data;
	if (!data) {
		SDE_ERROR("invalid parameters\n");
		return;
	}

	crtc = cb_data->crtc;
	if (!crtc || !crtc->dev || !crtc->dev->dev_private) {
		SDE_ERROR("invalid parameters\n");
		return;
	}
	sde_crtc = to_sde_crtc(crtc);
	priv = crtc->dev->dev_private;
	crtc_id = drm_crtc_index(crtc);

	SDE_DEBUG("crtc%d\n", crtc->base.id);
	SDE_EVT32(DRMID(crtc), event);

	spin_lock_irqsave(&sde_crtc->fevent_spin_lock, flags);
	fevent = list_first_entry_or_null(&sde_crtc->frame_event_list,
			struct sde_crtc_frame_event, list);
	if (fevent)
		list_del_init(&fevent->list);
	spin_unlock_irqrestore(&sde_crtc->fevent_spin_lock, flags);

	if (!fevent) {
		SDE_ERROR("crtc%d event %d overflow\n", DRMID(crtc), event);
		SDE_EVT32(DRMID(crtc), event);
		return;
	}

	fevent->event = event;
	fevent->ts = ts;
	fevent->crtc = crtc;
	fevent->connector = cb_data->connector;
	kthread_queue_work(&priv->event_thread[crtc_id].worker, &fevent->work);
}

void sde_crtc_prepare_commit(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state)
{
	struct drm_device *dev;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_connector *conn;
	struct drm_encoder *encoder;
	struct drm_connector_list_iter conn_iter;

	if (!crtc || !crtc->state) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	dev = crtc->dev;
	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);
	SDE_EVT32_VERBOSE(DRMID(crtc), cstate->cwb_enc_mask);

	SDE_ATRACE_BEGIN("sde_crtc_prepare_commit");

	/* identify connectors attached to this crtc */
	cstate->num_connectors = 0;

	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(conn, &conn_iter)
		if (conn->state && conn->state->crtc == crtc &&
				cstate->num_connectors < MAX_CONNECTORS) {
			encoder = conn->state->best_encoder;
			if (encoder)
				sde_encoder_register_frame_event_callback(
						encoder,
						sde_crtc_frame_event_cb,
						crtc);

			cstate->connectors[cstate->num_connectors++] = conn;
			sde_connector_prepare_fence(conn);
			sde_encoder_set_clone_mode(encoder, crtc->state);
		}
	drm_connector_list_iter_end(&conn_iter);

	/* prepare main output fence */
	sde_fence_prepare(sde_crtc->output_fence);
	SDE_ATRACE_END("sde_crtc_prepare_commit");
}

/**
 *  sde_crtc_complete_flip - signal pending page_flip events
 * Any pending vblank events are added to the vblank_event_list
 * so that the next vblank interrupt shall signal them.
 * However PAGE_FLIP events are not handled through the vblank_event_list.
 * This API signals any pending PAGE_FLIP events requested through
 * DRM_IOCTL_MODE_PAGE_FLIP and are cached in the sde_crtc->event.
 * if file!=NULL, this is preclose potential cancel-flip path
 * @crtc: Pointer to drm crtc structure
 * @file: Pointer to drm file
 */
void sde_crtc_complete_flip(struct drm_crtc *crtc,
		struct drm_file *file)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct drm_device *dev = crtc->dev;
	struct drm_pending_vblank_event *event;
	unsigned long flags;

	spin_lock_irqsave(&dev->event_lock, flags);
	event = sde_crtc->event;
	if (!event)
		goto end;

	/*
	 * if regular vblank case (!file) or if cancel-flip from
	 * preclose on file that requested flip, then send the
	 * event:
	 */
	if (!file || (event->base.file_priv == file)) {
		sde_crtc->event = NULL;
		DRM_DEBUG_VBL("%s: send event: %pK\n",
					sde_crtc->name, event);
		SDE_EVT32_VERBOSE(DRMID(crtc));
		drm_crtc_send_vblank_event(crtc, event);
	}

end:
	spin_unlock_irqrestore(&dev->event_lock, flags);
}

enum sde_intf_mode sde_crtc_get_intf_mode(struct drm_crtc *crtc,
		struct drm_crtc_state *cstate)
{
	struct drm_encoder *encoder;

	if (!crtc || !crtc->dev || !cstate) {
		SDE_ERROR("invalid crtc\n");
		return INTF_MODE_NONE;
	}

	drm_for_each_encoder_mask(encoder, crtc->dev,
			cstate->encoder_mask) {
		/* continue if copy encoder is encountered */
		if (sde_crtc_state_in_clone_mode(encoder, cstate))
			continue;

		return sde_encoder_get_intf_mode(encoder);
	}

	return INTF_MODE_NONE;
}

u32 sde_crtc_get_fps_mode(struct drm_crtc *crtc)
{
	struct drm_encoder *encoder;

	if (!crtc || !crtc->dev) {
		SDE_ERROR("invalid crtc\n");
		return INTF_MODE_NONE;
	}

	drm_for_each_encoder(encoder, crtc->dev)
		if ((encoder->crtc == crtc)
				&& !sde_encoder_in_cont_splash(encoder))
			return sde_encoder_get_fps(encoder);

	return 0;
}

u32 sde_crtc_get_dfps_maxfps(struct drm_crtc *crtc)
{
	struct drm_encoder *encoder;

	if (!crtc || !crtc->dev) {
		SDE_ERROR("invalid crtc\n");
		return 0;
	}

	drm_for_each_encoder_mask(encoder, crtc->dev,
			crtc->state->encoder_mask) {
		if (!sde_encoder_in_cont_splash(encoder))
			return sde_encoder_get_dfps_maxfps(encoder);
	}

	return 0;
}

struct drm_encoder *sde_crtc_get_src_encoder_of_clone(struct drm_crtc *crtc)
{
	struct drm_encoder *enc;
	struct sde_crtc *sde_crtc;

	if (!crtc || !crtc->dev)
		return NULL;

	sde_crtc = to_sde_crtc(crtc);
	drm_for_each_encoder_mask(enc, crtc->dev, sde_crtc->cached_encoder_mask) {
		if (sde_encoder_in_clone_mode(enc))
			continue;

		return enc;
	}

	return NULL;
}

static void sde_crtc_vblank_cb(void *data, ktime_t ts)
{
	struct drm_crtc *crtc = (struct drm_crtc *)data;
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);

	/* keep statistics on vblank callback - with auto reset via debugfs */
	if (ktime_compare(sde_crtc->vblank_cb_time, ktime_set(0, 0)) == 0)
		sde_crtc->vblank_cb_time = ts;
	else
		sde_crtc->vblank_cb_count++;

	sde_crtc->vblank_last_cb_time = ts;
	sysfs_notify_dirent(sde_crtc->vsync_event_sf);

	drm_crtc_handle_vblank(crtc);
	DRM_DEBUG_VBL("crtc%d, ts:%llu\n", crtc->base.id, ktime_to_us(ts));
	SDE_EVT32_VERBOSE(DRMID(crtc), ktime_to_us(ts));
}

static void _sde_crtc_retire_event(struct drm_connector *connector,
		ktime_t ts, enum sde_fence_event fence_event)
{
	if (!connector) {
		SDE_ERROR("invalid param\n");
		return;
	}

	SDE_ATRACE_BEGIN("signal_retire_fence");
	sde_connector_complete_commit(connector, ts, fence_event);
	SDE_ATRACE_END("signal_retire_fence");
}

static void sde_crtc_frame_event_work(struct kthread_work *work)
{
	struct msm_drm_private *priv;
	struct sde_crtc_frame_event *fevent;
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;
	unsigned long flags;
	bool in_clone_mode = false;
	int ret;

	if (!work) {
		SDE_ERROR("invalid work handle\n");
		return;
	}

	fevent = container_of(work, struct sde_crtc_frame_event, work);
	if (!fevent->crtc || !fevent->crtc->state) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	crtc = fevent->crtc;
	sde_crtc = to_sde_crtc(crtc);

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid kms handle\n");
		return;
	}
	priv = sde_kms->dev->dev_private;
	SDE_ATRACE_BEGIN("crtc_frame_event");

	SDE_DEBUG("crtc%d event:%u ts:%lld\n", crtc->base.id, fevent->event,
			ktime_to_ns(fevent->ts));

	SDE_EVT32_VERBOSE(DRMID(crtc), fevent->event, SDE_EVTLOG_FUNC_ENTRY);

	in_clone_mode = (fevent->event & SDE_ENCODER_FRAME_EVENT_CWB_DONE) ?
			true : false;

	if (!in_clone_mode && (fevent->event & (SDE_ENCODER_FRAME_EVENT_ERROR
					| SDE_ENCODER_FRAME_EVENT_PANEL_DEAD
					| SDE_ENCODER_FRAME_EVENT_DONE))) {

		ret = pm_runtime_resume_and_get(crtc->dev->dev);
		if (ret < 0) {
			SDE_ERROR("failed to enable power resource %d\n", ret);
			SDE_EVT32(ret, SDE_EVTLOG_ERROR);
		} else {
			/* log and clear plane ubwc errors if any */
			sde_crtc_get_frame_data(crtc);
			pm_runtime_put_sync(crtc->dev->dev);
		}

		if (atomic_read(&sde_crtc->frame_pending) < 1) {
			/* this should not happen */
			SDE_ERROR("crtc%d ts:%lld invalid frame_pending:%d\n",
					crtc->base.id,
					ktime_to_ns(fevent->ts),
					atomic_read(&sde_crtc->frame_pending));
			SDE_EVT32(DRMID(crtc), fevent->event,
							SDE_EVTLOG_FUNC_CASE1);
		} else if (atomic_dec_return(&sde_crtc->frame_pending) == 0) {
			/* release bandwidth and other resources */
			SDE_DEBUG("crtc%d ts:%lld last pending\n",
					crtc->base.id,
					ktime_to_ns(fevent->ts));
			SDE_EVT32(DRMID(crtc), fevent->event,
							SDE_EVTLOG_FUNC_CASE2);
			sde_core_perf_crtc_release_bw(crtc);
		} else {
			SDE_EVT32_VERBOSE(DRMID(crtc), fevent->event,
							SDE_EVTLOG_FUNC_CASE3);
		}
	}

	if (fevent->event & SDE_ENCODER_FRAME_EVENT_SIGNAL_RELEASE_FENCE) {
		SDE_ATRACE_BEGIN("signal_release_fence");
		sde_fence_signal(sde_crtc->output_fence, fevent->ts,
				(fevent->event & SDE_ENCODER_FRAME_EVENT_ERROR)
				? SDE_FENCE_SIGNAL_ERROR : SDE_FENCE_SIGNAL);
		SDE_ATRACE_END("signal_release_fence");
	}

	if (fevent->event & SDE_ENCODER_FRAME_EVENT_SIGNAL_RETIRE_FENCE) {
		if (sde_crtc->retire_frame_event_sf) {
			sde_crtc->retire_frame_event_time = fevent->ts;
			sysfs_notify_dirent(sde_crtc->retire_frame_event_sf);
		}

		/* this api should be called without spin_lock */
		_sde_crtc_retire_event(fevent->connector, fevent->ts,
				(fevent->event & SDE_ENCODER_FRAME_EVENT_ERROR)
				? SDE_FENCE_SIGNAL_ERROR : SDE_FENCE_SIGNAL);
	}

	if (fevent->event & SDE_ENCODER_FRAME_EVENT_PANEL_DEAD)
		SDE_ERROR("crtc%d ts:%lld received panel dead event\n",
				crtc->base.id, ktime_to_ns(fevent->ts));

	spin_lock_irqsave(&sde_crtc->fevent_spin_lock, flags);
	list_add_tail(&fevent->list, &sde_crtc->frame_event_list);
	spin_unlock_irqrestore(&sde_crtc->fevent_spin_lock, flags);
	SDE_ATRACE_END("crtc_frame_event");
}

void sde_crtc_complete_commit(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state)
{
	struct sde_crtc *sde_crtc;
	struct sde_splash_display *splash_display = NULL;
	struct sde_kms *sde_kms;
	bool cont_splash_enabled = false;
	int i;
	u32 power_on = 1;

	if (!crtc || !crtc->state) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid kms\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	SDE_EVT32_VERBOSE(DRMID(crtc));

	for (i = 0; i < MAX_DSI_DISPLAYS; i++) {
		splash_display = &sde_kms->splash_data.splash_display[i];
		if (splash_display && splash_display->cont_splash_enabled &&
				crtc == splash_display->encoder->crtc)
			cont_splash_enabled = true;
	}

	if ((crtc->state->active_changed || cont_splash_enabled) && crtc->state->active)
		sde_crtc_event_notify(crtc, DRM_EVENT_CRTC_POWER, &power_on, sizeof(u32));

	sde_core_perf_crtc_update(crtc, 0, false);
}

/**
 * _sde_crtc_set_input_fence_timeout - update ns version of in fence timeout
 * @cstate: Pointer to sde crtc state
 */
static void _sde_crtc_set_input_fence_timeout(struct sde_crtc_state *cstate)
{
	if (!cstate) {
		SDE_ERROR("invalid cstate\n");
		return;
	}
	cstate->input_fence_timeout_ns =
		sde_crtc_get_property(cstate, CRTC_PROP_INPUT_FENCE_TIMEOUT);
	cstate->input_fence_timeout_ns *= NSEC_PER_MSEC;
}

void _sde_crtc_clear_dim_layers_v1(struct drm_crtc_state *state)
{
	u32 i;
	struct sde_crtc_state *cstate;

	if (!state)
		return;

	cstate = to_sde_crtc_state(state);

	for (i = 0; i < cstate->num_dim_layers; i++)
		memset(&cstate->dim_layer[i], 0, sizeof(cstate->dim_layer[i]));

	cstate->num_dim_layers = 0;
}

/**
 * _sde_crtc_set_dim_layer_v1 - copy dim layer settings from userspace
 * @cstate:      Pointer to sde crtc state
 * @user_ptr:    User ptr for sde_drm_dim_layer_v1 struct
 */
static void _sde_crtc_set_dim_layer_v1(struct drm_crtc *crtc,
	struct sde_crtc_state *cstate, void __user *usr_ptr)
{
	struct sde_drm_dim_layer_v1 dim_layer_v1;
	struct sde_drm_dim_layer_cfg *user_cfg;
	struct sde_hw_dim_layer *dim_layer;
	u32 count, i;
	struct sde_kms *kms;

	if (!crtc || !cstate) {
		SDE_ERROR("invalid crtc or cstate\n");
		return;
	}
	dim_layer = cstate->dim_layer;

	if (!usr_ptr) {
		/* usr_ptr is null when setting the default property value */
		_sde_crtc_clear_dim_layers_v1(&cstate->base);
		SDE_DEBUG("dim_layer data removed\n");
		goto clear;
	}

	kms = _sde_crtc_get_kms(crtc);
	if (!kms || !kms->catalog) {
		SDE_ERROR("invalid kms\n");
		return;
	}

	if (copy_from_user(&dim_layer_v1, usr_ptr, sizeof(dim_layer_v1))) {
		SDE_ERROR("failed to copy dim_layer data\n");
		return;
	}

	count = dim_layer_v1.num_layers;
	if (count > SDE_MAX_DIM_LAYERS) {
		SDE_ERROR("invalid number of dim_layers:%d", count);
		return;
	}

	/* populate from user space */
	cstate->num_dim_layers = count;
	for (i = 0; i < count; i++) {
		user_cfg = &dim_layer_v1.layer_cfg[i];

		dim_layer[i].flags = user_cfg->flags;
		dim_layer[i].stage = (kms->catalog->has_base_layer) ?
					user_cfg->stage : user_cfg->stage +
					SDE_STAGE_0;

		dim_layer[i].rect.x = user_cfg->rect.x1;
		dim_layer[i].rect.y = user_cfg->rect.y1;
		dim_layer[i].rect.w = user_cfg->rect.x2 - user_cfg->rect.x1;
		dim_layer[i].rect.h = user_cfg->rect.y2 - user_cfg->rect.y1;

		dim_layer[i].color_fill = (struct sde_mdss_color) {
				user_cfg->color_fill.color_0,
				user_cfg->color_fill.color_1,
				user_cfg->color_fill.color_2,
				user_cfg->color_fill.color_3,
		};

		SDE_DEBUG("dim_layer[%d] - flags:%d, stage:%d\n",
				i, dim_layer[i].flags, dim_layer[i].stage);
		SDE_DEBUG(" rect:{%d,%d,%d,%d}, color:{%d,%d,%d,%d}\n",
				dim_layer[i].rect.x, dim_layer[i].rect.y,
				dim_layer[i].rect.w, dim_layer[i].rect.h,
				dim_layer[i].color_fill.color_0,
				dim_layer[i].color_fill.color_1,
				dim_layer[i].color_fill.color_2,
				dim_layer[i].color_fill.color_3);
	}
clear:
	set_bit(SDE_CRTC_DIRTY_DIM_LAYERS, cstate->dirty);
}

/**
 * _sde_crtc_set_dest_scaler - copy dest scaler settings from userspace
 * @sde_crtc   :  Pointer to sde crtc
 * @cstate :  Pointer to sde crtc state
 * @usr_ptr:  User ptr for sde_drm_dest_scaler_data struct
 */
static int _sde_crtc_set_dest_scaler(struct sde_crtc *sde_crtc,
				struct sde_crtc_state *cstate,
				void __user *usr_ptr)
{
	struct sde_drm_dest_scaler_data ds_data;
	struct sde_drm_dest_scaler_cfg *ds_cfg_usr;
	struct sde_drm_scaler_v2 scaler_v2;
	void __user *scaler_v2_usr;
	int i, count;

	if (!sde_crtc || !cstate) {
		SDE_ERROR("invalid sde_crtc/state\n");
		return -EINVAL;
	}

	SDE_DEBUG("crtc %s\n", sde_crtc->name);

	if (!usr_ptr) {
		SDE_DEBUG("ds data removed\n");
		return 0;
	}

	if (copy_from_user(&ds_data, usr_ptr, sizeof(ds_data))) {
		SDE_ERROR("%s:failed to copy dest scaler data from user\n",
			sde_crtc->name);
		return -EINVAL;
	}

	count = ds_data.num_dest_scaler;
	if (!count) {
		SDE_DEBUG("no ds data available\n");
		return 0;
	}

	if (count > SDE_MAX_DS_COUNT) {
		SDE_ERROR("%s: invalid config: num_ds(%d) max(%d)\n",
			sde_crtc->name, count, SDE_MAX_DS_COUNT);
		SDE_EVT32(DRMID(&sde_crtc->base), count, SDE_EVTLOG_ERROR);
		return -EINVAL;
	}

	/* Populate from user space */
	for (i = 0; i < count; i++) {
		ds_cfg_usr = &ds_data.ds_cfg[i];

		cstate->ds_cfg[i].idx = ds_cfg_usr->index;
		cstate->ds_cfg[i].flags = ds_cfg_usr->flags;
		cstate->ds_cfg[i].lm_width = ds_cfg_usr->lm_width;
		cstate->ds_cfg[i].lm_height = ds_cfg_usr->lm_height;
		memset(&scaler_v2, 0, sizeof(scaler_v2));

		if (ds_cfg_usr->scaler_cfg) {
			scaler_v2_usr =
			(void __user *)((uintptr_t)ds_cfg_usr->scaler_cfg);

			if (copy_from_user(&scaler_v2, scaler_v2_usr,
					sizeof(scaler_v2))) {
				SDE_ERROR("%s:scaler: copy from user failed\n",
					sde_crtc->name);
				return -EINVAL;
			}
		}

		sde_set_scaler_v2(&cstate->ds_cfg[i].scl3_cfg, &scaler_v2);

		SDE_DEBUG("en(%d)dir(%d)de(%d) src(%dx%d) dst(%dx%d)\n",
			scaler_v2.enable, scaler_v2.dir_en, scaler_v2.de.enable,
			scaler_v2.src_width[0], scaler_v2.src_height[0],
			scaler_v2.dst_width, scaler_v2.dst_height);
		SDE_EVT32_VERBOSE(DRMID(&sde_crtc->base),
			scaler_v2.enable, scaler_v2.dir_en, scaler_v2.de.enable,
			scaler_v2.src_width[0], scaler_v2.src_height[0],
			scaler_v2.dst_width, scaler_v2.dst_height);

		SDE_DEBUG("ds cfg[%d]-ndx(%d) flags(%d) lm(%dx%d)\n",
			i, ds_cfg_usr->index, ds_cfg_usr->flags,
			ds_cfg_usr->lm_width, ds_cfg_usr->lm_height);
		SDE_EVT32_VERBOSE(DRMID(&sde_crtc->base), i, ds_cfg_usr->index,
			ds_cfg_usr->flags, ds_cfg_usr->lm_width,
			ds_cfg_usr->lm_height);
	}

	cstate->num_ds = count;
	set_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty);
	SDE_EVT32_VERBOSE(DRMID(&sde_crtc->base), count);

	return 0;
}

static int _sde_crtc_check_dest_scaler_lm(struct drm_crtc *crtc,
	struct drm_display_mode *mode, struct sde_hw_ds_cfg *cfg, u32 hdisplay,
	struct sde_hw_ds_cfg *prev_cfg)
{
	if (cfg->lm_width > hdisplay || cfg->lm_height > mode->vdisplay
		|| !cfg->lm_width || !cfg->lm_height) {
		SDE_ERROR("crtc%d: lm size[%d,%d] display [%d,%d]\n",
			crtc->base.id, cfg->lm_width, cfg->lm_height,
			hdisplay, mode->vdisplay);
		SDE_EVT32(DRMID(crtc), cfg->lm_width, cfg->lm_height,
			hdisplay, mode->vdisplay, SDE_EVTLOG_ERROR);
		return -E2BIG;
	}

	if (prev_cfg && (cfg->lm_width != prev_cfg->lm_width ||
			cfg->lm_height != prev_cfg->lm_height)) {
		SDE_ERROR("crtc%d: uneven lm split [%d,%d], [%d %d]\n",
				crtc->base.id, cfg->lm_width,
				cfg->lm_height, prev_cfg->lm_width,
				prev_cfg->lm_height);
		SDE_EVT32(DRMID(crtc), cfg->lm_width, cfg->lm_height,
				prev_cfg->lm_width, prev_cfg->lm_height,
				SDE_EVTLOG_ERROR);
		return -EINVAL;
	}

	return 0;
}

static int _sde_crtc_check_dest_scaler_cfg(struct drm_crtc *crtc,
	struct sde_crtc *sde_crtc, struct drm_display_mode *mode,
	struct sde_hw_ds *hw_ds, struct sde_hw_ds_cfg *cfg, u32 hdisplay,
	u32 max_in_width, u32 max_out_width)
{
	if (cfg->flags & SDE_DRM_DESTSCALER_SCALE_UPDATE ||
		cfg->flags & SDE_DRM_DESTSCALER_ENHANCER_UPDATE) {

		/**
		 * Scaler src and dst width shouldn't exceed the maximum
		 * width limitation. Also, if there is no partial update
		 * dst width and height must match display resolution.
		 */
		if (cfg->scl3_cfg.src_width[0] > max_in_width ||
			cfg->scl3_cfg.dst_width > max_out_width ||
			!cfg->scl3_cfg.src_width[0] ||
			!cfg->scl3_cfg.dst_width ||
			(!(cfg->flags & SDE_DRM_DESTSCALER_PU_ENABLE)
			 && (cfg->scl3_cfg.dst_width != hdisplay ||
			 cfg->scl3_cfg.dst_height != mode->vdisplay))) {
			SDE_ERROR("crtc%d: ", crtc->base.id);
			SDE_ERROR("src_w(%d) dst(%dx%d) display(%dx%d)",
				cfg->scl3_cfg.src_width[0],
				cfg->scl3_cfg.dst_width,
				cfg->scl3_cfg.dst_height,
				hdisplay, mode->vdisplay);
			SDE_ERROR("num_mixers(%d) flags(%d) ds-%d:\n",
				sde_crtc->num_mixers, cfg->flags,
				hw_ds->idx - DS_0);
			SDE_ERROR("scale_en = %d, DE_en =%d\n",
				cfg->scl3_cfg.enable,
				cfg->scl3_cfg.de.enable);

			SDE_EVT32(DRMID(crtc), cfg->scl3_cfg.enable,
				cfg->scl3_cfg.de.enable, cfg->flags,
				max_in_width, max_out_width,
				cfg->scl3_cfg.src_width[0],
				cfg->scl3_cfg.dst_width,
				cfg->scl3_cfg.dst_height, hdisplay,
				mode->vdisplay, sde_crtc->num_mixers,
				SDE_EVTLOG_ERROR);

			cfg->flags &=
				~SDE_DRM_DESTSCALER_SCALE_UPDATE;
			cfg->flags &=
				~SDE_DRM_DESTSCALER_ENHANCER_UPDATE;

			return -EINVAL;
		}
	}
	return 0;
}

static int _sde_crtc_check_dest_scaler_validate_ds(struct drm_crtc *crtc,
	struct sde_crtc *sde_crtc, struct sde_crtc_state *cstate,
	struct drm_display_mode *mode, struct sde_hw_ds *hw_ds,
	u32 hdisplay, u32 *num_ds_enable, u32 max_in_width, u32 max_out_width)
{
	int i, ret;
	u32 lm_idx;
	struct sde_hw_ds_cfg *cfg, *prev_cfg;

	for (i = 0; i < cstate->num_ds; i++) {
		cfg = &cstate->ds_cfg[i];
		prev_cfg = (i > 0) ? &cstate->ds_cfg[i - 1] : NULL;
		lm_idx = cfg->idx;

		/**
		 * Validate against topology
		 * No of dest scalers should match the num of mixers
		 * unless it is partial update left only/right only use case
		 */
		if (lm_idx >= sde_crtc->num_mixers || (i != lm_idx &&
			!(cfg->flags & SDE_DRM_DESTSCALER_PU_ENABLE))) {
			SDE_ERROR("crtc%d: ds_cfg id(%d):idx(%d), flags(%d)\n",
				crtc->base.id, i, lm_idx, cfg->flags);
			SDE_EVT32(DRMID(crtc), i, lm_idx, cfg->flags,
				SDE_EVTLOG_ERROR);
			return -EINVAL;
		}

		hw_ds = sde_crtc->mixers[lm_idx].hw_ds;

		if (!max_in_width && !max_out_width) {
			max_in_width = hw_ds->scl->top->maxinputwidth;
			max_out_width = hw_ds->scl->top->maxoutputwidth;

			if (cstate->num_ds == CRTC_DUAL_MIXERS_ONLY)
				max_in_width -= SDE_DS_OVERFETCH_SIZE;

			SDE_DEBUG("max DS width [%d,%d] for num_ds = %d\n",
				max_in_width, max_out_width, cstate->num_ds);
		}

		/* Check LM width and height */
		ret = _sde_crtc_check_dest_scaler_lm(crtc, mode, cfg, hdisplay,
				prev_cfg);
		if (ret)
			return ret;

		/* Check scaler data */
		ret = _sde_crtc_check_dest_scaler_cfg(crtc, sde_crtc, mode,
				hw_ds, cfg, hdisplay,
				max_in_width, max_out_width);
		if (ret)
			return ret;

		if (cfg->flags & SDE_DRM_DESTSCALER_ENABLE)
			(*num_ds_enable)++;

		SDE_DEBUG("ds[%d]: flags[0x%X]\n",
			hw_ds->idx - DS_0, cfg->flags);
		SDE_EVT32_VERBOSE(DRMID(crtc), hw_ds->idx - DS_0, cfg->flags);
	}

	return 0;
}

static void _sde_crtc_check_dest_scaler_data_disable(struct drm_crtc *crtc,
	struct sde_crtc_state *cstate, u32 num_ds_enable)
{
	struct sde_hw_ds_cfg *cfg;
	int i;

	SDE_DEBUG("dest scaler status : %d -> %d\n",
		cstate->num_ds_enabled, num_ds_enable);
	SDE_EVT32_VERBOSE(DRMID(crtc), cstate->num_ds_enabled, num_ds_enable,
			cstate->num_ds, cstate->dirty[0]);

	if (cstate->num_ds_enabled != num_ds_enable) {
		/* Disabling destination scaler */
		if (!num_ds_enable) {
			for (i = 0; i < cstate->num_ds; i++) {
				cfg = &cstate->ds_cfg[i];
				cfg->idx = i;
				/* Update scaler settings in disable case */
				cfg->flags = SDE_DRM_DESTSCALER_SCALE_UPDATE;
				cfg->scl3_cfg.enable = 0;
				cfg->scl3_cfg.de.enable = 0;
			}
		}
		cstate->num_ds_enabled = num_ds_enable;
		set_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty);
	} else {
		if (!cstate->num_ds_enabled)
			clear_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty);
	}
}

/**
 * _sde_crtc_check_dest_scaler_data - validate the dest scaler data
 * @crtc  :  Pointer to drm crtc
 * @state :  Pointer to drm crtc state
 */
static int _sde_crtc_check_dest_scaler_data(struct drm_crtc *crtc,
				struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_display_mode *mode;
	struct sde_kms *kms;
	struct sde_hw_ds *hw_ds = NULL;
	u32 ret = 0;
	u32 num_ds_enable = 0, hdisplay = 0;
	u32 max_in_width = 0, max_out_width = 0;

	if (!crtc || !state)
		return -EINVAL;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);
	kms = _sde_crtc_get_kms(crtc);
	mode = &state->adjusted_mode;

	SDE_DEBUG("crtc%d\n", crtc->base.id);

	if (!test_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty)) {
		SDE_DEBUG("dest scaler property not set, skip validation\n");
		return 0;
	}

	if (!kms || !kms->catalog) {
		SDE_ERROR("crtc%d: invalid parameters\n", crtc->base.id);
		return -EINVAL;
	}

	if (!kms->catalog->mdp[0].has_dest_scaler) {
		SDE_DEBUG("dest scaler feature not supported\n");
		return 0;
	}

	if (!sde_crtc->num_mixers) {
		SDE_DEBUG("mixers not allocated\n");
		return 0;
	}

	ret = _sde_validate_hw_resources(sde_crtc);
	if (ret)
		goto err;

	/**
	 * No of dest scalers shouldn't exceed hw ds block count and
	 * also, match the num of mixers unless it is partial update
	 * left only/right only use case - currently PU + DS is not supported
	 */
	if (cstate->num_ds > kms->catalog->ds_count ||
		((cstate->num_ds != sde_crtc->num_mixers) &&
		!(cstate->ds_cfg[0].flags & SDE_DRM_DESTSCALER_PU_ENABLE))) {
		SDE_ERROR("crtc%d: num_ds(%d), hw_ds_cnt(%d) flags(%d)\n",
			crtc->base.id, cstate->num_ds, kms->catalog->ds_count,
			cstate->ds_cfg[0].flags);
		ret = -EINVAL;
		goto err;
	}

	/**
	 * Check if DS needs to be enabled or disabled
	 * In case of enable, validate the data
	 */
	if (!(cstate->ds_cfg[0].flags & SDE_DRM_DESTSCALER_ENABLE)) {
		SDE_DEBUG("disable dest scaler, num(%d) flags(%d)\n",
			cstate->num_ds, cstate->ds_cfg[0].flags);
		goto disable;
	}

	/* Display resolution */
	hdisplay = mode->hdisplay/sde_crtc->num_mixers;

	/* Validate the DS data */
	ret = _sde_crtc_check_dest_scaler_validate_ds(crtc, sde_crtc, cstate,
			mode, hw_ds, hdisplay, &num_ds_enable,
			max_in_width, max_out_width);
	if (ret)
		goto err;

disable:
	_sde_crtc_check_dest_scaler_data_disable(crtc, cstate, num_ds_enable);
	return 0;

err:
	clear_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty);
	return ret;
}

/**
 * _sde_crtc_wait_for_fences - wait for incoming framebuffer sync fences
 * @crtc: Pointer to CRTC object
 */
static void _sde_crtc_wait_for_fences(struct drm_crtc *crtc)
{
	struct drm_plane *plane = NULL;
	uint32_t wait_ms = 1;
	ktime_t kt_end, kt_wait;
	int rc = 0;

	SDE_DEBUG("\n");

	if (!crtc || !crtc->state) {
		SDE_ERROR("invalid crtc/state %pK\n", crtc);
		return;
	}

	/* use monotonic timer to limit total fence wait time */
	kt_end = ktime_add_ns(ktime_get(),
		to_sde_crtc_state(crtc->state)->input_fence_timeout_ns);

	/*
	 * Wait for fences sequentially, as all of them need to be signalled
	 * before we can proceed.
	 *
	 * Limit total wait time to INPUT_FENCE_TIMEOUT, but still call
	 * sde_plane_wait_input_fence with wait_ms == 0 after the timeout so
	 * that each plane can check its fence status and react appropriately
	 * if its fence has timed out. Call input fence wait multiple times if
	 * fence wait is interrupted due to interrupt call.
	 */
	SDE_ATRACE_BEGIN("plane_wait_input_fence");
	drm_atomic_crtc_for_each_plane(plane, crtc) {
		do {
			kt_wait = ktime_sub(kt_end, ktime_get());
			if (ktime_compare(kt_wait, ktime_set(0, 0)) >= 0)
				wait_ms = ktime_to_ms(kt_wait);
			else
				wait_ms = 0;

			rc = sde_plane_wait_input_fence(plane, wait_ms);
		} while (wait_ms && rc == -ERESTARTSYS);
	}
	SDE_ATRACE_END("plane_wait_input_fence");
}

static void _sde_crtc_setup_mixer_for_encoder(
		struct drm_crtc *crtc,
		struct drm_encoder *enc)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct sde_kms *sde_kms = _sde_crtc_get_kms(crtc);
	struct sde_rm *rm = &sde_kms->rm;
	struct sde_crtc_mixer *mixer;
	struct sde_hw_ctl *last_valid_ctl = NULL;
	int i;
	struct sde_rm_hw_iter lm_iter, ctl_iter, dspp_iter, ds_iter;

	sde_rm_init_hw_iter(&lm_iter, enc->base.id, SDE_HW_BLK_LM);
	sde_rm_init_hw_iter(&ctl_iter, enc->base.id, SDE_HW_BLK_CTL);
	sde_rm_init_hw_iter(&dspp_iter, enc->base.id, SDE_HW_BLK_DSPP);
	sde_rm_init_hw_iter(&ds_iter, enc->base.id, SDE_HW_BLK_DS);

	/* Set up all the mixers and ctls reserved by this encoder */
	for (i = sde_crtc->num_mixers; i < ARRAY_SIZE(sde_crtc->mixers); i++) {
		mixer = &sde_crtc->mixers[i];

		if (!sde_rm_get_hw(rm, &lm_iter))
			break;
		mixer->hw_lm = (struct sde_hw_mixer *)lm_iter.hw;

		/* CTL may be <= LMs, if <, multiple LMs controlled by 1 CTL */
		if (!sde_rm_get_hw(rm, &ctl_iter)) {
			SDE_DEBUG("no ctl assigned to lm %d, using previous\n",
					mixer->hw_lm->idx - LM_0);
			mixer->hw_ctl = last_valid_ctl;
		} else {
			mixer->hw_ctl = (struct sde_hw_ctl *)ctl_iter.hw;
			last_valid_ctl = mixer->hw_ctl;
			sde_crtc->num_ctls++;
		}

		/* Shouldn't happen, mixers are always >= ctls */
		if (!mixer->hw_ctl) {
			SDE_ERROR("no valid ctls found for lm %d\n",
					mixer->hw_lm->idx - LM_0);
			return;
		}

		/* Dspp may be null */
		(void) sde_rm_get_hw(rm, &dspp_iter);
		mixer->hw_dspp = (struct sde_hw_dspp *)dspp_iter.hw;

		/* DS may be null */
		(void) sde_rm_get_hw(rm, &ds_iter);
		mixer->hw_ds = (struct sde_hw_ds *)ds_iter.hw;

		mixer->encoder = enc;

		sde_crtc->num_mixers++;
		SDE_DEBUG("setup mixer %d: lm %d\n",
				i, mixer->hw_lm->idx - LM_0);
		SDE_DEBUG("setup mixer %d: ctl %d\n",
				i, mixer->hw_ctl->idx - CTL_0);
		if (mixer->hw_ds)
			SDE_DEBUG("setup mixer %d: ds %d\n",
				i, mixer->hw_ds->idx - DS_0);
	}
}

static void _sde_crtc_setup_mixers(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct drm_encoder *enc;

	sde_crtc->num_ctls = 0;
	sde_crtc->num_mixers = 0;
	sde_crtc->mixers_swapped = false;
	memset(sde_crtc->mixers, 0, sizeof(sde_crtc->mixers));

	mutex_lock(&sde_crtc->crtc_lock);
	/* Check for mixers on all encoders attached to this crtc */
	list_for_each_entry(enc, &crtc->dev->mode_config.encoder_list, head) {
		if (enc->crtc != crtc)
			continue;

		/* avoid overwriting mixers info from a copy encoder */
		if (sde_encoder_in_clone_mode(enc))
			continue;

		_sde_crtc_setup_mixer_for_encoder(crtc, enc);
	}

	mutex_unlock(&sde_crtc->crtc_lock);
	_sde_crtc_check_dest_scaler_data(crtc, crtc->state);
}

static void _sde_crtc_setup_is_ppsplit(struct drm_crtc_state *state)
{
	int i;
	struct sde_crtc_state *cstate;

	cstate = to_sde_crtc_state(state);

	cstate->is_ppsplit = false;
	for (i = 0; i < cstate->num_connectors; i++) {
		struct drm_connector *conn = cstate->connectors[i];

		if (sde_connector_get_topology_name(conn) ==
				SDE_RM_TOPOLOGY_PPSPLIT)
			cstate->is_ppsplit = true;
	}
}

static void _sde_crtc_setup_lm_bounds(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_display_mode *adj_mode;
	u32 crtc_split_width;
	int i;

	if (!crtc || !state) {
		SDE_ERROR("invalid args\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);

	adj_mode = &state->adjusted_mode;
	crtc_split_width = sde_crtc_get_mixer_width(sde_crtc, cstate, adj_mode);

	for (i = 0; i < sde_crtc->num_mixers; i++) {
		cstate->lm_bounds[i].x = crtc_split_width * i;
		cstate->lm_bounds[i].y = 0;
		cstate->lm_bounds[i].w = crtc_split_width;
		cstate->lm_bounds[i].h =
			sde_crtc_get_mixer_height(sde_crtc, cstate, adj_mode);
		memcpy(&cstate->lm_roi[i], &cstate->lm_bounds[i],
				sizeof(cstate->lm_roi[i]));
		SDE_EVT32_VERBOSE(DRMID(crtc), i,
				cstate->lm_bounds[i].x, cstate->lm_bounds[i].y,
				cstate->lm_bounds[i].w, cstate->lm_bounds[i].h);
		SDE_DEBUG("%s: lm%d bnd&roi (%d,%d,%d,%d)\n", sde_crtc->name, i,
				cstate->lm_roi[i].x, cstate->lm_roi[i].y,
				cstate->lm_roi[i].w, cstate->lm_roi[i].h);
	}

	drm_mode_debug_printmodeline(adj_mode);
}

static void _sde_crtc_clear_all_blend_stages(struct sde_crtc *sde_crtc)
{
	struct sde_crtc_mixer mixer;

	/*
	 * Use mixer[0] to get hw_ctl which will use ops to clear
	 * all blendstages. Clear all blendstages will iterate through
	 * all mixers.
	 */
	if (sde_crtc->num_mixers) {
		mixer = sde_crtc->mixers[0];
		if (mixer.hw_ctl && mixer.hw_ctl->ops.clear_all_blendstages)
			mixer.hw_ctl->ops.clear_all_blendstages(mixer.hw_ctl);
		if (mixer.hw_ctl && mixer.hw_ctl->ops.set_active_pipes)
			mixer.hw_ctl->ops.set_active_pipes(mixer.hw_ctl, NULL);
	}
}

static void sde_crtc_atomic_begin(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state)
{
	struct sde_crtc *sde_crtc;
	struct drm_encoder *encoder;
	struct drm_device *dev;
	struct sde_kms *sde_kms;
	struct sde_splash_display *splash_display;
	bool cont_splash_enabled = false;
	size_t i;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	if (!crtc->state->enable) {
		SDE_DEBUG("crtc%d -> enable %d, skip atomic_begin\n",
				crtc->base.id, crtc->state->enable);
		return;
	}

	if (!sde_kms_power_resource_is_enabled(crtc->dev)) {
		SDE_ERROR("power resource is not enabled\n");
		return;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms)
		return;

	SDE_ATRACE_BEGIN("crtc_atomic_begin");
	SDE_DEBUG("crtc%d\n", crtc->base.id);

	sde_crtc = to_sde_crtc(crtc);
	dev = crtc->dev;

	if (!sde_crtc->num_mixers) {
		_sde_crtc_setup_mixers(crtc);
		_sde_crtc_setup_is_ppsplit(crtc->state);
		_sde_crtc_setup_lm_bounds(crtc, crtc->state);
		_sde_crtc_clear_all_blend_stages(sde_crtc);
	} else if (sde_crtc->num_mixers && sde_crtc->reinit_crtc_mixers) {
		_sde_crtc_setup_mixers(crtc);
		sde_crtc->reinit_crtc_mixers = false;
	}

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (encoder->crtc != crtc)
			continue;

		sde_encoder_trigger_rsc_state_change(encoder);
		/* encoder will trigger pending mask now */
		sde_encoder_trigger_kickoff_pending(encoder);
	}

	/* update performance setting */
	sde_core_perf_crtc_update(crtc, 1, false);

	/*
	 * If no mixers have been allocated in sde_crtc_atomic_check(),
	 * it means we are trying to flush a CRTC whose state is disabled:
	 * nothing else needs to be done.
	 */
	if (unlikely(!sde_crtc->num_mixers))
		goto end;

	_sde_crtc_blend_setup(crtc, old_state, true);
	_sde_crtc_dest_scaler_setup(crtc);
	sde_cp_crtc_apply_noise(crtc, old_state);

	if (crtc->state->mode_changed || sde_kms->perf.catalog->uidle_cfg.dirty)
		sde_core_perf_crtc_update_uidle(crtc, true);

	/* update cached_encoder_mask if new conn is added or removed */
	if (crtc->state->connectors_changed)
		sde_crtc->cached_encoder_mask = crtc->state->encoder_mask;

	/*
	 * Since CP properties use AXI buffer to program the
	 * HW, check if context bank is in attached state,
	 * apply color processing properties only if
	 * smmu state is attached,
	 */
	for (i = 0; i < MAX_DSI_DISPLAYS; i++) {
		splash_display = &sde_kms->splash_data.splash_display[i];
		if (splash_display->cont_splash_enabled &&
			splash_display->encoder &&
			crtc == splash_display->encoder->crtc)
			cont_splash_enabled = true;
	}

	if (sde_kms_is_cp_operation_allowed(sde_kms))
		sde_cp_crtc_apply_properties(crtc);

	if (!sde_crtc->enabled)
		sde_cp_crtc_mark_features_dirty(crtc);

	/*
	 * PP_DONE irq is only used by command mode for now.
	 * It is better to request pending before FLUSH and START trigger
	 * to make sure no pp_done irq missed.
	 * This is safe because no pp_done will happen before SW trigger
	 * in command mode.
	 */

end:
	SDE_ATRACE_END("crtc_atomic_begin");
}

static void sde_crtc_atomic_flush(struct drm_crtc *crtc,
		struct drm_crtc_state *old_crtc_state)
{
	struct drm_encoder *encoder;
	struct sde_crtc *sde_crtc;
	struct drm_device *dev;
	struct drm_plane *plane;
	struct msm_drm_private *priv;
	struct sde_crtc_state *cstate;
	struct sde_kms *sde_kms;
	int i;

	if (!crtc || !crtc->dev || !crtc->dev->dev_private) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	if (!crtc->state->enable) {
		SDE_DEBUG("crtc%d -> enable %d, skip atomic_flush\n",
				crtc->base.id, crtc->state->enable);
		return;
	}

	if (!sde_kms_power_resource_is_enabled(crtc->dev)) {
		SDE_ERROR("power resource is not enabled\n");
		return;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid kms\n");
		return;
	}

	SDE_DEBUG("crtc%d\n", crtc->base.id);

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);
	dev = crtc->dev;
	priv = dev->dev_private;

	if ((sde_crtc->cache_state == CACHE_STATE_NORMAL) &&
			sde_crtc_get_property(cstate, CRTC_PROP_CACHE_STATE))
		sde_crtc_static_img_control(crtc, CACHE_STATE_FRAME_WRITE,
				false);
	else
		sde_crtc_static_img_control(crtc, CACHE_STATE_NORMAL, false);

	/*
	 * If no mixers has been allocated in sde_crtc_atomic_check(),
	 * it means we are trying to flush a CRTC whose state is disabled:
	 * nothing else needs to be done.
	 */
	if (unlikely(!sde_crtc->num_mixers))
		return;

	SDE_ATRACE_BEGIN("sde_crtc_atomic_flush");

	/*
	 * For planes without commit update, drm framework will not add
	 * those planes to current state since hardware update is not
	 * required. However, if those planes were power collapsed since
	 * last commit cycle, driver has to restore the hardware state
	 * of those planes explicitly here prior to plane flush.
	 * Also use this iteration to see if any plane requires cache,
	 * so during the perf update driver can activate/deactivate
	 * the cache accordingly.
	 */
	for (i = 0; i < SDE_SYS_CACHE_MAX; i++)
		sde_crtc->new_perf.llcc_active[i] = false;

	drm_atomic_crtc_for_each_plane(plane, crtc) {
		sde_plane_restore(plane);

		for (i = 0; i < SDE_SYS_CACHE_MAX; i++) {
			if (sde_plane_is_cache_required(plane, i))
				sde_crtc->new_perf.llcc_active[i] = true;
		}
	}
	sde_core_perf_crtc_update_llcc(crtc);

	/* wait for acquire fences before anything else is done */
	_sde_crtc_wait_for_fences(crtc);

	if (!cstate->rsc_update) {
		drm_for_each_encoder_mask(encoder, dev,
				crtc->state->encoder_mask) {
			cstate->rsc_client =
				sde_encoder_get_rsc_client(encoder);
		}
		cstate->rsc_update = true;
	}

	/*
	 * Final plane updates: Give each plane a chance to complete all
	 *                      required writes/flushing before crtc's "flush
	 *                      everything" call below.
	 */
	drm_atomic_crtc_for_each_plane(plane, crtc) {
		if (sde_kms->smmu_state.transition_error)
			sde_plane_set_error(plane, true);
		sde_plane_flush(plane);
	}

	/* Kickoff will be scheduled by outer layer */
	SDE_ATRACE_END("sde_crtc_atomic_flush");
}

/**
 * sde_crtc_destroy_state - state destroy hook
 * @crtc: drm CRTC
 * @state: CRTC state object to release
 */
static void sde_crtc_destroy_state(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_encoder *enc;
	struct sde_kms *sde_kms;
	u32 encoder_mask;

	if (!crtc || !state) {
		SDE_ERROR("invalid argument(s)\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);
	sde_kms = _sde_crtc_get_kms(crtc);

	if (!sde_kms) {
		SDE_ERROR("invalid sde_kms\n");
		return;
	}

	encoder_mask = state->encoder_mask ? state->encoder_mask :
				crtc->state->encoder_mask;
	SDE_DEBUG("crtc%d, encoder_mask=%d\n", crtc->base.id, encoder_mask);

	drm_for_each_encoder_mask(enc, crtc->dev, encoder_mask)
		sde_rm_release(&sde_kms->rm, enc, true);

	sde_cp_clear_state_info(state);
	__drm_atomic_helper_crtc_destroy_state(state);

	/* destroy value helper */
	msm_property_destroy_state(&sde_crtc->property_info, cstate,
			&cstate->property_state);
}

static int _sde_crtc_flush_frame_events(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	int i;

	if (!crtc) {
		SDE_ERROR("invalid argument\n");
		return -EINVAL;
	}
	sde_crtc = to_sde_crtc(crtc);

	if (!atomic_read(&sde_crtc->frame_pending)) {
		SDE_DEBUG("no frames pending\n");
		return 0;
	}

	SDE_EVT32(DRMID(crtc), SDE_EVTLOG_FUNC_ENTRY);

	/*
	 * flush all the event thread work to make sure all the
	 * FRAME_EVENTS from encoder are propagated to crtc
	 */
	for (i = 0; i < ARRAY_SIZE(sde_crtc->frame_events); i++) {
		if (list_empty(&sde_crtc->frame_events[i].list))
			kthread_flush_work(&sde_crtc->frame_events[i].work);
	}

	SDE_EVT32_VERBOSE(DRMID(crtc), SDE_EVTLOG_FUNC_EXIT);

	return 0;
}

/**
 * _sde_crtc_remove_pipe_flush - remove staged pipes from flush mask
 * @crtc: Pointer to crtc structure
 */
static void _sde_crtc_remove_pipe_flush(struct drm_crtc *crtc)
{
	struct drm_plane *plane;
	struct drm_plane_state *state;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_mixer *mixer;
	struct sde_hw_ctl *ctl;

	if (!crtc)
		return;

	sde_crtc = to_sde_crtc(crtc);
	mixer = sde_crtc->mixers;
	if (!mixer)
		return;
	ctl = mixer->hw_ctl;

	drm_atomic_crtc_for_each_plane(plane, crtc) {
		state = plane->state;
		if (!state)
			continue;

		/* clear plane flush bitmask */
		sde_plane_ctl_flush(plane, ctl, false);
	}
}

/**
 * sde_crtc_reset_hw - attempt hardware reset on errors
 * @crtc: Pointer to DRM crtc instance
 * @old_state: Pointer to crtc state for previous commit
 * @recovery_events: Whether or not recovery events are enabled
 * Returns: Zero if current commit should still be attempted
 */
int sde_crtc_reset_hw(struct drm_crtc *crtc, struct drm_crtc_state *old_state,
	bool recovery_events)
{
	struct drm_plane *plane_halt[MAX_PLANES];
	struct drm_plane *plane;
	struct drm_encoder *encoder;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct sde_hw_ctl *ctl;
	signed int i, plane_count;
	int rc;

	if (!crtc || !crtc->dev || !old_state || !crtc->state)
		return -EINVAL;
	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);

	SDE_EVT32(DRMID(crtc), recovery_events, SDE_EVTLOG_FUNC_ENTRY);

	/* optionally generate a panic instead of performing a h/w reset */
	SDE_DBG_CTRL("stop_ftrace", "reset_hw_panic");

	for (i = 0; i < sde_crtc->num_ctls; ++i) {
		ctl = sde_crtc->mixers[i].hw_ctl;
		if (!ctl || !ctl->ops.reset)
			continue;

		rc = ctl->ops.reset(ctl);
		if (rc) {
			SDE_DEBUG("crtc%d: ctl%d reset failure\n",
					crtc->base.id, ctl->idx - CTL_0);
			SDE_EVT32(DRMID(crtc), ctl->idx - CTL_0,
					SDE_EVTLOG_ERROR);
			break;
		}
	}

	/*
	 * Early out if simple ctl reset succeeded or reset is
	 * being performed after timeout
	 */
	if (i == sde_crtc->num_ctls || crtc->state == old_state)
		return 0;

	SDE_DEBUG("crtc%d: issuing hard reset\n", DRMID(crtc));

	/* force all components in the system into reset at the same time */
	for (i = 0; i < sde_crtc->num_ctls; ++i) {
		ctl = sde_crtc->mixers[i].hw_ctl;
		if (!ctl || !ctl->ops.hard_reset)
			continue;

		SDE_EVT32(DRMID(crtc), ctl->idx - CTL_0);
		ctl->ops.hard_reset(ctl, true);
	}

	plane_count = 0;
	drm_atomic_crtc_state_for_each_plane(plane, old_state) {
		if (plane_count >= ARRAY_SIZE(plane_halt))
			break;

		plane_halt[plane_count++] = plane;
		sde_plane_halt_requests(plane, true);
		sde_plane_set_revalidate(plane, true);
	}

	/* provide safe "border color only" commit configuration for later */
	_sde_crtc_remove_pipe_flush(crtc);
	_sde_crtc_blend_setup(crtc, old_state, false);

	/* take h/w components out of reset */
	for (i = plane_count - 1; i >= 0; --i)
		sde_plane_halt_requests(plane_halt[i], false);

	/* attempt to poll for start of frame cycle before reset release */
	list_for_each_entry(encoder,
			&crtc->dev->mode_config.encoder_list, head) {
		if (encoder->crtc != crtc)
			continue;
		if (sde_encoder_get_intf_mode(encoder) == INTF_MODE_VIDEO)
			sde_encoder_poll_line_counts(encoder);
	}

	for (i = 0; i < sde_crtc->num_ctls; ++i) {
		ctl = sde_crtc->mixers[i].hw_ctl;
		if (!ctl || !ctl->ops.hard_reset)
			continue;

		ctl->ops.hard_reset(ctl, false);
	}

	list_for_each_entry(encoder,
			&crtc->dev->mode_config.encoder_list, head) {
		if (encoder->crtc != crtc)
			continue;

		if (sde_encoder_get_intf_mode(encoder) == INTF_MODE_VIDEO)
			sde_encoder_kickoff(encoder, true);
	}

	/* panic the device if VBIF is not in good state */
	return !recovery_events ? 0 : -EAGAIN;
}

void sde_crtc_commit_kickoff(struct drm_crtc *crtc,
		struct drm_crtc_state *old_state)
{
	struct drm_encoder *encoder;
	struct drm_device *dev;
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;
	struct sde_crtc_state *cstate;
	bool is_error = false;
	unsigned long flags;
	enum sde_crtc_idle_pc_state idle_pc_state;
	struct sde_encoder_kickoff_params params = { 0 };

	if (!crtc) {
		SDE_ERROR("invalid argument\n");
		return;
	}
	dev = crtc->dev;
	sde_crtc = to_sde_crtc(crtc);
	sde_kms = _sde_crtc_get_kms(crtc);

	if (!sde_kms || !sde_kms->dev || !sde_kms->dev->dev_private) {
		SDE_ERROR("invalid argument\n");
		return;
	}

	cstate = to_sde_crtc_state(crtc->state);

	/*
	 * If no mixers has been allocated in sde_crtc_atomic_check(),
	 * it means we are trying to start a CRTC whose state is disabled:
	 * nothing else needs to be done.
	 */
	if (unlikely(!sde_crtc->num_mixers))
		return;

	SDE_ATRACE_BEGIN("crtc_commit");

	idle_pc_state = sde_crtc_get_property(cstate, CRTC_PROP_IDLE_PC_STATE);

	mi_sde_crtc_update_layer_state(cstate);

	sde_crtc->kickoff_in_progress = true;
	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (encoder->crtc != crtc)
			continue;

		/*
		 * Encoder will flush/start now, unless it has a tx pending.
		 * If so, it may delay and flush at an irq event (e.g. ppdone)
		 */
		params.affected_displays = _sde_crtc_get_displays_affected(crtc,
				crtc->state);
		if (sde_encoder_prepare_for_kickoff(encoder, &params, old_state))
			sde_crtc->needs_hw_reset = true;

		if (idle_pc_state != IDLE_PC_NONE)
			sde_encoder_control_idle_pc(encoder,
			    (idle_pc_state == IDLE_PC_ENABLE) ? true : false);
	}

	/*
	 * Optionally attempt h/w recovery if any errors were detected while
	 * preparing for the kickoff
	 */
	if (sde_crtc->needs_hw_reset) {
		sde_crtc->frame_trigger_mode = params.frame_trigger_mode;
		if (sde_crtc->frame_trigger_mode
					!= FRAME_DONE_WAIT_POSTED_START &&
		    sde_crtc_reset_hw(crtc, old_state,
						params.recovery_events_enabled))
			is_error = true;
		sde_crtc->needs_hw_reset = false;
	}

	sde_crtc_calc_fps(sde_crtc);
	SDE_ATRACE_BEGIN("flush_event_thread");
	_sde_crtc_flush_frame_events(crtc);
	SDE_ATRACE_END("flush_event_thread");
	sde_crtc->plane_mask_old = crtc->state->plane_mask;
	SDE_EVT32(DRMID(crtc), sde_crtc->plane_mask_old);

	if (atomic_inc_return(&sde_crtc->frame_pending) == 1) {
		/* acquire bandwidth and other resources */
		SDE_DEBUG("crtc%d first commit\n", crtc->base.id);
		SDE_EVT32(DRMID(crtc), SDE_EVTLOG_FUNC_CASE1);
	} else {
		SDE_DEBUG("crtc%d commit\n", crtc->base.id);
		SDE_EVT32(DRMID(crtc), SDE_EVTLOG_FUNC_CASE2);
	}
	sde_crtc->play_count++;

	sde_vbif_clear_errors(sde_kms);

	if (is_error) {
		_sde_crtc_remove_pipe_flush(crtc);
		_sde_crtc_blend_setup(crtc, old_state, false);
	}

	list_for_each_entry(encoder, &dev->mode_config.encoder_list, head) {
		if (encoder->crtc != crtc)
			continue;

		sde_encoder_kickoff(encoder, true);
	}
	sde_crtc->kickoff_in_progress = false;

	/* store the event after frame trigger */
	if (sde_crtc->event) {
		WARN_ON(sde_crtc->event);
	} else {
		spin_lock_irqsave(&dev->event_lock, flags);
		sde_crtc->event = crtc->state->event;
		spin_unlock_irqrestore(&dev->event_lock, flags);
	}

	SDE_ATRACE_END("crtc_commit");
}

/**
 * _sde_crtc_vblank_enable - update power resource and vblank request
 * @sde_crtc: Pointer to sde crtc structure
 * @enable: Whether to enable/disable vblanks
 *
 * @Return: error code
 */
static int _sde_crtc_vblank_enable(
		struct sde_crtc *sde_crtc, bool enable)
{
	struct drm_crtc *crtc;
	struct drm_encoder *enc;

	if (!sde_crtc) {
		SDE_ERROR("invalid crtc\n");
		return -EINVAL;
	}

	crtc = &sde_crtc->base;
	SDE_EVT32(DRMID(crtc), enable, sde_crtc->enabled,
			crtc->state->encoder_mask,
			sde_crtc->cached_encoder_mask);

	if (enable) {
		int ret;

		ret = pm_runtime_get_sync(crtc->dev->dev);
		if (ret < 0)
			return ret;

		mutex_lock(&sde_crtc->crtc_lock);
		drm_for_each_encoder_mask(enc, crtc->dev,
				sde_crtc->cached_encoder_mask) {
			SDE_EVT32(DRMID(crtc), DRMID(enc));

			sde_encoder_register_vblank_callback(enc,
					sde_crtc_vblank_cb, (void *)crtc);
		}

		mutex_unlock(&sde_crtc->crtc_lock);
	} else {
		mutex_lock(&sde_crtc->crtc_lock);
		drm_for_each_encoder_mask(enc, crtc->dev,
				sde_crtc->cached_encoder_mask) {
			SDE_EVT32(DRMID(crtc), DRMID(enc));

			sde_encoder_register_vblank_callback(enc, NULL, NULL);
		}

		mutex_unlock(&sde_crtc->crtc_lock);
		pm_runtime_put_sync(crtc->dev->dev);
	}

	return 0;
}

static void _sde_crtc_reserve_resource(struct drm_crtc *crtc, struct drm_connector *conn)
{
	u32 min_transfer_time = 0, lm_count = 1;
	u64 mode_clock_hz = 0, updated_fps = 0, topology_id;
	struct drm_encoder *encoder;

	if (!crtc || !conn)
		return;

	encoder = conn->state->best_encoder;
	if (!sde_encoder_is_built_in_display(encoder))
		return;

	if (sde_encoder_check_curr_mode(encoder, MSM_DISPLAY_CMD_MODE))
		sde_encoder_get_transfer_time(encoder, &min_transfer_time);

	if (min_transfer_time)
		updated_fps = DIV_ROUND_UP(1000000, min_transfer_time);
	else
		updated_fps = drm_mode_vrefresh(&crtc->mode);

	topology_id = sde_connector_get_topology_name(conn);
	if (TOPOLOGY_DUALPIPE_MODE(topology_id))
		lm_count = 2;
	else if (TOPOLOGY_QUADPIPE_MODE(topology_id))
		lm_count = 4;

	/* mode clock = [(h * v * fps * 1.05) / (num_lm)] */
	mode_clock_hz = mult_frac(crtc->mode.htotal * crtc->mode.vtotal * updated_fps, 105, 100);
	mode_clock_hz = div_u64(mode_clock_hz, lm_count);
	SDE_DEBUG("[%s] h=%d v=%d fps=%d lm=%d mode_clk=%u\n",
			crtc->mode.name, crtc->mode.htotal, crtc->mode.vtotal,
			updated_fps, lm_count, mode_clock_hz);

	sde_core_perf_crtc_reserve_res(crtc, mode_clock_hz);
}

/**
 * sde_crtc_duplicate_state - state duplicate hook
 * @crtc: Pointer to drm crtc structure
 * @Returns: Pointer to new drm_crtc_state structure
 */
static struct drm_crtc_state *sde_crtc_duplicate_state(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate, *old_cstate;

	if (!crtc || !crtc->state) {
		SDE_ERROR("invalid argument(s)\n");
		return NULL;
	}

	sde_crtc = to_sde_crtc(crtc);
	old_cstate = to_sde_crtc_state(crtc->state);

	if (old_cstate->cont_splash_populated) {
		crtc->state->plane_mask = 0;
		crtc->state->connector_mask = 0;
		crtc->state->encoder_mask = 0;

		crtc->state->enable = false;
		old_cstate->cont_splash_populated = false;
	}

	cstate = msm_property_alloc_state(&sde_crtc->property_info);
	if (!cstate) {
		SDE_ERROR("failed to allocate state\n");
		return NULL;
	}
	/* duplicate value helper */
	msm_property_duplicate_state(&sde_crtc->property_info,
			old_cstate, cstate,
			&cstate->property_state, cstate->property_values);
	sde_cp_duplicate_state_info(&old_cstate->base, &cstate->base);

	/* duplicate base helper */
	__drm_atomic_helper_crtc_duplicate_state(crtc, &cstate->base);

	return &cstate->base;
}

/**
 * sde_crtc_reset - reset hook for CRTCs
 * Resets the atomic state for @crtc by freeing the state pointer (which might
 * be NULL, e.g. at driver load time) and allocating a new empty state object.
 * @crtc: Pointer to drm crtc structure
 */
static void sde_crtc_reset(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	/* revert suspend actions, if necessary */
	if (!sde_crtc_is_reset_required(crtc)) {
		SDE_DEBUG("avoiding reset for crtc:%d\n", crtc->base.id);
		return;
	}

	/* remove previous state, if present */
	if (crtc->state) {
		sde_crtc_destroy_state(crtc, crtc->state);
		crtc->state = 0;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = msm_property_alloc_state(&sde_crtc->property_info);
	if (!cstate) {
		SDE_ERROR("failed to allocate state\n");
		return;
	}

	/* reset value helper */
	msm_property_reset_state(&sde_crtc->property_info, cstate,
			&cstate->property_state,
			cstate->property_values);
	_sde_crtc_set_input_fence_timeout(cstate);

	cstate->base.crtc = crtc;
	crtc->state = &cstate->base;
}

static void sde_crtc_clear_cached_mixer_cfg(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct sde_hw_mixer *hw_lm;
	int lm_idx;

	/* clearing lm cfg marks it dirty to force reprogramming next update */
	for (lm_idx = 0; lm_idx < sde_crtc->num_mixers; lm_idx++) {
		hw_lm = sde_crtc->mixers[lm_idx].hw_lm;
		hw_lm->cfg.out_width = 0;
		hw_lm->cfg.out_height = 0;
	}

	SDE_EVT32(DRMID(crtc));
}

void sde_crtc_reset_sw_state(struct drm_crtc *crtc)
{
	struct sde_crtc_state *cstate = to_sde_crtc_state(crtc->state);
	struct drm_plane *plane;
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);

	/* mark planes, mixers, and other blocks dirty for next update */
	drm_atomic_crtc_for_each_plane(plane, crtc)
		sde_plane_set_revalidate(plane, true);

	/* mark mixers dirty for next update */
	sde_crtc_clear_cached_mixer_cfg(crtc);

	/* mark other properties which need to be dirty for next update */
	set_bit(SDE_CRTC_DIRTY_DIM_LAYERS, &sde_crtc->revalidate_mask);
	if (cstate->num_ds_enabled)
		set_bit(SDE_CRTC_DIRTY_DEST_SCALER, cstate->dirty);
}

static void sde_crtc_post_ipc(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_encoder *encoder;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);

	/* restore encoder; crtc will be programmed during commit */
	drm_for_each_encoder_mask(encoder, crtc->dev, crtc->state->encoder_mask)
		sde_encoder_virt_restore(encoder);

	/* restore UIDLE */
	sde_core_perf_crtc_update_uidle(crtc, true);

	sde_cp_crtc_post_ipc(crtc);
}

static void sde_crtc_mmrm_cb_notification(struct drm_crtc *crtc)
{
	struct msm_drm_private *priv;
	unsigned long requested_clk;
	struct sde_kms *kms = NULL;

	if (!crtc->dev->dev_private) {
		pr_err("invalid crtc priv\n");
		return;
	}
	priv = crtc->dev->dev_private;
	kms = to_sde_kms(priv->kms);
	if (!kms) {
		SDE_ERROR("invalid parameters\n");
		return;
	}

	requested_clk = sde_power_mmrm_get_requested_clk(&priv->phandle,
			kms->perf.clk_name);

	/* notify user space the reduced clk rate */
	sde_crtc_event_notify(crtc, DRM_EVENT_MMRM_CB, &requested_clk, sizeof(unsigned long));

	SDE_DEBUG("crtc[%d]: MMRM cb notified clk:%d\n",
		crtc->base.id, requested_clk);
}

static void sde_crtc_handle_power_event(u32 event_type, void *arg)
{
	struct drm_crtc *crtc = arg;
	struct sde_crtc *sde_crtc;
	struct drm_encoder *encoder;
	u32 power_on;
	unsigned long flags;
	struct sde_crtc_irq_info *node = NULL;
	int ret = 0;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return;
	}
	sde_crtc = to_sde_crtc(crtc);

	mutex_lock(&sde_crtc->crtc_lock);

	SDE_EVT32(DRMID(crtc), event_type);

	switch (event_type) {
	case SDE_POWER_EVENT_POST_ENABLE:
		spin_lock_irqsave(&sde_crtc->spin_lock, flags);
		list_for_each_entry(node, &sde_crtc->user_event_list, list) {
			ret = 0;
			if (node->func)
				ret = node->func(crtc, true, &node->irq);
			if (ret)
				SDE_ERROR("%s failed to enable event %x\n",
						sde_crtc->name, node->event);
		}
		spin_unlock_irqrestore(&sde_crtc->spin_lock, flags);

		sde_crtc_post_ipc(crtc);
		break;
	case SDE_POWER_EVENT_PRE_DISABLE:
		drm_for_each_encoder_mask(encoder, crtc->dev,
				crtc->state->encoder_mask) {
			/*
			 * disable the vsync source after updating the
			 * rsc state. rsc state update might have vsync wait
			 * and vsync source must be disabled after it.
			 * It will avoid generating any vsync from this point
			 * till mode-2 entry. It is SW workaround for HW
			 * limitation and should not be removed without
			 * checking the updated design.
			 */
			sde_encoder_control_te(encoder, false);
		}

		spin_lock_irqsave(&sde_crtc->spin_lock, flags);
		node = NULL;
		list_for_each_entry(node, &sde_crtc->user_event_list, list) {
			ret = 0;
			if (node->func)
				ret = node->func(crtc, false, &node->irq);
			if (ret)
				SDE_ERROR("%s failed to disable event %x\n",
						sde_crtc->name, node->event);
		}
		spin_unlock_irqrestore(&sde_crtc->spin_lock, flags);

		sde_cp_crtc_pre_ipc(crtc);
		break;
	case SDE_POWER_EVENT_POST_DISABLE:
		sde_crtc_reset_sw_state(crtc);
		sde_cp_crtc_suspend(crtc);
		power_on = 0;
		sde_crtc_event_notify(crtc, DRM_EVENT_SDE_POWER, &power_on, sizeof(u32));
		break;
	case SDE_POWER_EVENT_MMRM_CALLBACK:
		sde_crtc_mmrm_cb_notification(crtc);

		break;
	default:
		SDE_DEBUG("event:%d not handled\n", event_type);
		break;
	}

	mutex_unlock(&sde_crtc->crtc_lock);
}

static void _sde_crtc_reset(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct sde_crtc_state *cstate = to_sde_crtc_state(crtc->state);

	/* mark mixer cfgs dirty before wiping them */
	sde_crtc_clear_cached_mixer_cfg(crtc);

	sde_crtc->num_mixers = 0;
	memset(sde_crtc->mixers, 0, sizeof(sde_crtc->mixers));
	sde_crtc->mixers_swapped = false;

	/* disable clk & bw control until clk & bw properties are set */
	cstate->bw_control = false;
	cstate->bw_split_vote = false;

	sde_crtc_static_img_control(crtc, CACHE_STATE_DISABLED, false);
}

static void sde_crtc_disable(struct drm_crtc *crtc)
{
	struct sde_kms *sde_kms;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_encoder *encoder;
	struct msm_drm_private *priv;
	unsigned long flags;
	struct sde_crtc_irq_info *node = NULL;
	u32 power_on;
	bool in_cont_splash = false;
	int ret, i;

	if (!crtc || !crtc->dev || !crtc->dev->dev_private || !crtc->state) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid kms\n");
		return;
	}

	if (!sde_kms_power_resource_is_enabled(crtc->dev)) {
		SDE_ERROR("power resource is not enabled\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);
	priv = crtc->dev->dev_private;

	SDE_DEBUG("crtc%d\n", crtc->base.id);

	drm_crtc_vblank_off(crtc);

	mutex_lock(&sde_crtc->crtc_lock);
	SDE_EVT32_VERBOSE(DRMID(crtc));

	/* update color processing on suspend */
	sde_cp_crtc_suspend(crtc);

	mutex_unlock(&sde_crtc->crtc_lock);
	kthread_flush_worker(&priv->event_thread[crtc->index].worker);
	mutex_lock(&sde_crtc->crtc_lock);

	kthread_cancel_delayed_work_sync(&sde_crtc->static_cache_read_work);

	SDE_EVT32(DRMID(crtc), sde_crtc->enabled, crtc->state->active,
			crtc->state->enable, sde_crtc->cached_encoder_mask);
	sde_crtc->enabled = false;
	sde_crtc->cached_encoder_mask = 0;

	/* Try to disable uidle */
	sde_core_perf_crtc_update_uidle(crtc, false);

	if (atomic_read(&sde_crtc->frame_pending)) {
		SDE_ERROR("crtc%d frame_pending%d\n", crtc->base.id,
				atomic_read(&sde_crtc->frame_pending));
		SDE_EVT32(DRMID(crtc), atomic_read(&sde_crtc->frame_pending),
							SDE_EVTLOG_FUNC_CASE2);
		sde_core_perf_crtc_release_bw(crtc);
		atomic_set(&sde_crtc->frame_pending, 0);
	}

	spin_lock_irqsave(&sde_crtc->spin_lock, flags);
	list_for_each_entry(node, &sde_crtc->user_event_list, list) {
		ret = 0;
		if (node->func)
			ret = node->func(crtc, false, &node->irq);
		if (ret)
			SDE_ERROR("%s failed to disable event %x\n",
					sde_crtc->name, node->event);
	}
	spin_unlock_irqrestore(&sde_crtc->spin_lock, flags);

	drm_for_each_encoder_mask(encoder, crtc->dev,
			crtc->state->encoder_mask) {
		if (sde_encoder_in_cont_splash(encoder)) {
			in_cont_splash = true;
			break;
		}
	}

	/* avoid clk/bw downvote if cont-splash is enabled */
	if (!in_cont_splash)
		sde_core_perf_crtc_update(crtc, 0, true);

	drm_for_each_encoder_mask(encoder, crtc->dev,
			crtc->state->encoder_mask) {
		sde_encoder_register_frame_event_callback(encoder, NULL, NULL);
		cstate->rsc_client = NULL;
		cstate->rsc_update = false;

		/*
		 * reset idle power-collapse to original state during suspend;
		 * user-mode will change the state on resume, if required
		 */
		if (sde_kms->catalog->has_idle_pc)
			sde_encoder_control_idle_pc(encoder, true);
	}

	if (sde_crtc->power_event) {
		sde_power_handle_unregister_event(&priv->phandle,
				sde_crtc->power_event);
		sde_crtc->power_event = NULL;
	}

	/**
	 * All callbacks are unregistered and frame done waits are complete
	 * at this point. No buffers are accessed by hardware.
	 * reset the fence timeline if crtc will not be enabled for this commit
	 */
	if (!crtc->state->active || !crtc->state->enable) {
		sde_fence_signal(sde_crtc->output_fence,
				ktime_get(), SDE_FENCE_RESET_TIMELINE);
		for (i = 0; i < cstate->num_connectors; ++i)
			sde_connector_commit_reset(cstate->connectors[i],
					ktime_get());
	}

	_sde_crtc_reset(crtc);
	sde_cp_crtc_disable(crtc);

	power_on = 0;
	sde_crtc_event_notify(crtc, DRM_EVENT_CRTC_POWER, &power_on, sizeof(u32));

	mutex_unlock(&sde_crtc->crtc_lock);
}

static void sde_crtc_enable(struct drm_crtc *crtc,
		struct drm_crtc_state *old_crtc_state)
{
	struct sde_crtc *sde_crtc;
	struct drm_encoder *encoder;
	struct msm_drm_private *priv;
	unsigned long flags;
	struct sde_crtc_irq_info *node = NULL;
	int ret, i;
	struct sde_crtc_state *cstate;
	struct msm_display_mode *msm_mode;
	struct sde_kms *kms;

	if (!crtc || !crtc->dev || !crtc->dev->dev_private) {
		SDE_ERROR("invalid crtc\n");
		return;
	}
	kms = _sde_crtc_get_kms(crtc);
	if (!kms || !kms->catalog) {
		SDE_ERROR("invalid kms handle\n");
		return;
	}

	priv = crtc->dev->dev_private;
	cstate = to_sde_crtc_state(crtc->state);

	if (!sde_kms_power_resource_is_enabled(crtc->dev)) {
		SDE_ERROR("power resource is not enabled\n");
		return;
	}

	SDE_DEBUG("crtc%d\n", crtc->base.id);
	SDE_EVT32_VERBOSE(DRMID(crtc));
	sde_crtc = to_sde_crtc(crtc);

	/*
	 * Avoid drm_crtc_vblank_on during seamless DMS case
	 * when CRTC is already in enabled state
	 */
	if (!sde_crtc->enabled) {
		/* cache the encoder mask now for vblank work */
		sde_crtc->cached_encoder_mask = crtc->state->encoder_mask;
		/* max possible vsync_cnt(atomic_t) soft counter */
		if (kms->catalog->has_precise_vsync_ts)
			drm_crtc_set_max_vblank_count(crtc, INT_MAX);
		drm_crtc_vblank_on(crtc);
	}

	mutex_lock(&sde_crtc->crtc_lock);
	SDE_EVT32(DRMID(crtc), sde_crtc->enabled);

	/*
	 * Try to enable uidle (if possible), we do this before the call
	 * to return early during seamless dms mode, so any fps
	 * change is also consider to enable/disable UIDLE
	 */
	sde_core_perf_crtc_update_uidle(crtc, true);
	msm_mode = sde_crtc_get_msm_mode(crtc->state);
	if (!msm_mode){
		SDE_ERROR("invalid msm mode, %s\n",
			crtc->state->adjusted_mode.name);
		return;
	}

	/* return early if crtc is already enabled, do this after UIDLE check */
	if (sde_crtc->enabled) {
		if (msm_is_mode_seamless_dms(msm_mode) ||
				msm_is_mode_seamless_dyn_clk(msm_mode))
			SDE_DEBUG("%s extra crtc enable expected during DMS\n",
					sde_crtc->name);
		else
			WARN(1, "%s unexpected crtc enable\n", sde_crtc->name);

		mutex_unlock(&sde_crtc->crtc_lock);
		return;
	}

	drm_for_each_encoder_mask(encoder, crtc->dev,
			crtc->state->encoder_mask) {
		sde_encoder_register_frame_event_callback(encoder,
				sde_crtc_frame_event_cb, crtc);
		sde_crtc_static_img_control(crtc, CACHE_STATE_NORMAL,
				sde_encoder_check_curr_mode(encoder,
				MSM_DISPLAY_VIDEO_MODE));
	}

	sde_crtc->enabled = true;
	sde_cp_crtc_enable(crtc);
	/* update color processing on resume */
	sde_cp_crtc_resume(crtc);

	mutex_unlock(&sde_crtc->crtc_lock);

	spin_lock_irqsave(&sde_crtc->spin_lock, flags);
	list_for_each_entry(node, &sde_crtc->user_event_list, list) {
		ret = 0;
		if (node->func)
			ret = node->func(crtc, true, &node->irq);
		if (ret)
			SDE_ERROR("%s failed to enable event %x\n",
				sde_crtc->name, node->event);
	}
	spin_unlock_irqrestore(&sde_crtc->spin_lock, flags);

	sde_crtc->power_event = sde_power_handle_register_event(
		&priv->phandle,
		SDE_POWER_EVENT_POST_ENABLE | SDE_POWER_EVENT_POST_DISABLE |
		SDE_POWER_EVENT_PRE_DISABLE | SDE_POWER_EVENT_MMRM_CALLBACK,
		sde_crtc_handle_power_event, crtc, sde_crtc->name);

	/* Enable ESD thread */
	for (i = 0; i < cstate->num_connectors; i++) {
		sde_connector_schedule_status_work(cstate->connectors[i], true);
		_sde_crtc_reserve_resource(crtc, cstate->connectors[i]);
	}
}

/* no input validation - caller API has all the checks */
static int _sde_crtc_excl_dim_layer_check(struct drm_crtc_state *state,
		struct plane_state pstates[], int cnt)
{
	struct sde_crtc_state *cstate = to_sde_crtc_state(state);
	struct drm_display_mode *mode = &state->adjusted_mode;
	const struct drm_plane_state *pstate;
	struct sde_plane_state *sde_pstate;
	int rc = 0, i;
	bool is_fsc = sde_crtc_is_connector_fsc(cstate);

	/* Check dim layer rect bounds and stage */
	for (i = 0; i < cstate->num_dim_layers; i++) {
		if ((CHECK_LAYER_BOUNDS(cstate->dim_layer[i].rect.y,
			cstate->dim_layer[i].rect.h, GET_MODE_HEIGHT(is_fsc, mode))) ||
		    (CHECK_LAYER_BOUNDS(cstate->dim_layer[i].rect.x,
			cstate->dim_layer[i].rect.w, GET_MODE_WIDTH(is_fsc, mode))) ||
		    (cstate->dim_layer[i].stage >= SDE_STAGE_MAX) ||
		    (!cstate->dim_layer[i].rect.w) ||
		    (!cstate->dim_layer[i].rect.h)) {
			SDE_ERROR("invalid dim_layer:{%d,%d,%d,%d}, stage:%d\n",
					cstate->dim_layer[i].rect.x,
					cstate->dim_layer[i].rect.y,
					cstate->dim_layer[i].rect.w,
					cstate->dim_layer[i].rect.h,
					cstate->dim_layer[i].stage);
			SDE_ERROR("display: %dx%d\n", GET_MODE_WIDTH(is_fsc, mode),
					GET_MODE_HEIGHT(is_fsc, mode));
			rc = -E2BIG;
			goto end;
		}
	}

	/* log all src and excl_rect, useful for debugging */
	for (i = 0; i < cnt; i++) {
		pstate = pstates[i].drm_pstate;
		sde_pstate = to_sde_plane_state(pstate);
		SDE_DEBUG("p %d z %d src{%d,%d,%d,%d} excl_rect{%d,%d,%d,%d}\n",
			pstate->plane->base.id, pstates[i].stage,
			pstate->crtc_x, pstate->crtc_y,
			pstate->crtc_w, pstate->crtc_h,
			sde_pstate->excl_rect.x, sde_pstate->excl_rect.y,
			sde_pstate->excl_rect.w, sde_pstate->excl_rect.h);
	}

end:
	return rc;
}

static int _sde_crtc_check_secure_blend_config(struct drm_crtc *crtc,
	struct drm_crtc_state *state, struct plane_state pstates[],
	struct sde_crtc_state *cstate, struct sde_kms *sde_kms,
	int cnt, int secure, int fb_ns, int fb_sec, int fb_sec_dir)
{
	struct drm_plane *plane;
	int i;
	if (secure == SDE_DRM_SEC_ONLY) {
		/*
		 * validate planes - only fb_sec_dir is allowed during sec_crtc
		 * - fb_sec_dir is for secure camera preview and
		 * secure display use case
		 * - fb_sec is for secure video playback
		 * - fb_ns is for normal non secure use cases
		 */
		if (fb_ns || fb_sec) {
			SDE_ERROR(
			 "crtc%d: invalid fb_modes Sec:%d, NS:%d, Sec_Dir:%d\n",
				DRMID(crtc), fb_sec, fb_ns, fb_sec_dir);
			return -EINVAL;
		}

		/*
		 * - only one blending stage is allowed in sec_crtc
		 * - validate if pipe is allowed for sec-ui updates
		 */
		for (i = 1; i < cnt; i++) {
			if (!pstates[i].drm_pstate
					|| !pstates[i].drm_pstate->plane) {
				SDE_ERROR("crtc%d: invalid pstate at i:%d\n",
						DRMID(crtc), i);
				return -EINVAL;
			}
			plane = pstates[i].drm_pstate->plane;

			if (!sde_plane_is_sec_ui_allowed(plane)) {
				SDE_ERROR("crtc%d: sec-ui not allowed in p%d\n",
						DRMID(crtc), plane->base.id);
				return -EINVAL;

			} else if (pstates[i].stage != pstates[i-1].stage) {
				SDE_ERROR(
				  "crtc%d: invalid blend stages %d:%d, %d:%d\n",
				  DRMID(crtc), i, pstates[i].stage,
				  i-1, pstates[i-1].stage);
				return -EINVAL;
			}
		}

		/* check if all the dim_layers are in the same stage */
		for (i = 1; i < cstate->num_dim_layers; i++) {
			if (cstate->dim_layer[i].stage !=
					cstate->dim_layer[i-1].stage) {
				SDE_ERROR(
				"crtc%d: invalid dimlayer stage %d:%d, %d:%d\n",
					DRMID(crtc),
					i, cstate->dim_layer[i].stage,
					i-1, cstate->dim_layer[i-1].stage);
				return -EINVAL;
			}
		}

		/*
		 * if secure-ui supported blendstage is specified,
		 * - fail empty commit
		 * - validate dim_layer or plane is staged in the supported
		 *   blendstage
		 */
		if (sde_kms->catalog->sui_supported_blendstage) {
			int sec_stage = cnt ? pstates[0].sde_pstate->stage :
						cstate->dim_layer[0].stage;

			if (!sde_kms->catalog->has_base_layer)
				sec_stage -= SDE_STAGE_0;

			if ((!cnt && !cstate->num_dim_layers) ||
				(sde_kms->catalog->sui_supported_blendstage
						!= sec_stage)) {
				SDE_ERROR(
				  "crtc%d: empty cnt%d/dim%d or bad stage%d\n",
					DRMID(crtc), cnt,
					cstate->num_dim_layers, sec_stage);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int _sde_crtc_check_secure_single_encoder(struct drm_crtc *crtc,
	struct drm_crtc_state *state, int fb_sec_dir)
{
	struct drm_encoder *encoder;
	int encoder_cnt = 0;

	if (fb_sec_dir) {
		drm_for_each_encoder_mask(encoder, crtc->dev,
				state->encoder_mask)
			encoder_cnt++;

		if (encoder_cnt > MAX_ALLOWED_ENCODER_CNT_PER_SECURE_CRTC) {
			SDE_ERROR("crtc:%d invalid number of encoders:%d\n",
				DRMID(crtc), encoder_cnt);
			return -EINVAL;
		}
	}
	return 0;
}

static int _sde_crtc_check_secure_state_smmu_translation(struct drm_crtc *crtc,
	struct drm_crtc_state *state, struct sde_kms *sde_kms, int secure,
	int fb_ns, int fb_sec, int fb_sec_dir)
{
	struct sde_kms_smmu_state_data *smmu_state = &sde_kms->smmu_state;
	struct drm_encoder *encoder;
	int is_video_mode = false;

	drm_for_each_encoder_mask(encoder, crtc->dev, state->encoder_mask) {
		if (sde_encoder_is_dsi_display(encoder))
			is_video_mode |= sde_encoder_check_curr_mode(encoder,
						MSM_DISPLAY_VIDEO_MODE);
	}

	/*
	 * Secure display to secure camera needs without direct
	 * transition is currently not allowed
	 */
	if (fb_sec_dir && secure == SDE_DRM_SEC_NON_SEC &&
		smmu_state->state != ATTACHED &&
		smmu_state->secure_level == SDE_DRM_SEC_ONLY) {

		SDE_EVT32(DRMID(crtc), fb_ns, fb_sec_dir,
			smmu_state->state, smmu_state->secure_level,
			secure);
		goto sec_err;
	}

	/*
	 * In video mode check for null commit before transition
	 * from secure to non secure and vice versa
	 */
	if (is_video_mode && smmu_state &&
		state->plane_mask && crtc->state->plane_mask &&
		((fb_sec_dir && ((smmu_state->state == ATTACHED) &&
			(secure == SDE_DRM_SEC_ONLY))) ||
		    (fb_ns && ((smmu_state->state == DETACHED) ||
			(smmu_state->state == DETACH_ALL_REQ))) ||
		    (fb_ns && ((smmu_state->state == DETACHED_SEC) ||
			(smmu_state->state == DETACH_SEC_REQ)) &&
			(smmu_state->secure_level == SDE_DRM_SEC_ONLY)))) {

		SDE_EVT32(DRMID(crtc), fb_ns, fb_sec_dir,
			smmu_state->state, smmu_state->secure_level,
			secure, crtc->state->plane_mask, state->plane_mask);
		goto sec_err;
	}

	return 0;

sec_err:
	SDE_ERROR(
	 "crtc%d Invalid transition;sec%d state%d slvl%d ns%d sdir%d\n",
		DRMID(crtc), secure, smmu_state->state,
		smmu_state->secure_level, fb_ns, fb_sec_dir);
	return -EINVAL;
}

static int _sde_crtc_check_secure_conn(struct drm_crtc *crtc,
		struct drm_crtc_state *state, uint32_t fb_sec)
{

	bool conn_secure = false, is_wb = false;
	struct drm_connector *conn;
	struct drm_connector_state *conn_state;
	int i;

	for_each_new_connector_in_state(state->state, conn, conn_state, i) {
		if (conn_state && conn_state->crtc == crtc) {
			if (conn->connector_type ==
					DRM_MODE_CONNECTOR_VIRTUAL)
				is_wb = true;
			if (sde_connector_get_property(conn_state,
					CONNECTOR_PROP_FB_TRANSLATION_MODE) ==
					SDE_DRM_FB_SEC)
				conn_secure = true;
		}
	}

	/*
	 * If any input buffers are secure for wb,
	 * the output buffer must also be secure.
	 */
	if (is_wb && fb_sec && !conn_secure) {
		SDE_ERROR("crtc%d: input fb sec %d, output fb secure %d\n",
			DRMID(crtc), fb_sec, conn_secure);
		return -EINVAL;
	}

	return 0;
}

static int _sde_crtc_check_secure_state(struct drm_crtc *crtc,
		struct drm_crtc_state *state, struct plane_state pstates[],
		int cnt)
{
	struct sde_crtc_state *cstate;
	struct sde_kms *sde_kms;
	uint32_t secure;
	uint32_t fb_ns = 0, fb_sec = 0, fb_sec_dir = 0;
	int rc;

	if (!crtc || !state) {
		SDE_ERROR("invalid arguments\n");
		return -EINVAL;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms || !sde_kms->catalog) {
		SDE_ERROR("invalid kms\n");
		return -EINVAL;
	}

	cstate = to_sde_crtc_state(state);

	secure = sde_crtc_get_property(cstate, CRTC_PROP_SECURITY_LEVEL);

	rc = sde_crtc_state_find_plane_fb_modes(state, &fb_ns,
					&fb_sec, &fb_sec_dir);
	if (rc)
		return rc;

	rc = _sde_crtc_check_secure_blend_config(crtc, state, pstates, cstate,
			sde_kms, cnt, secure, fb_ns, fb_sec, fb_sec_dir);
	if (rc)
		return rc;

	rc = _sde_crtc_check_secure_conn(crtc, state, fb_sec);
	if (rc)
		return rc;

	/*
	 * secure_crtc is not allowed in a shared toppolgy
	 * across different encoders.
	 */
	rc = _sde_crtc_check_secure_single_encoder(crtc, state, fb_sec_dir);
	if (rc)
		return rc;

	rc = _sde_crtc_check_secure_state_smmu_translation(crtc, state, sde_kms,
			secure, fb_ns, fb_sec, fb_sec_dir);
	if (rc)
		return rc;

	SDE_DEBUG("crtc:%d Secure validation successful\n", DRMID(crtc));

	return 0;
}

bool sde_crtc_is_connector_fsc(struct sde_crtc_state *cstate)
{
	struct sde_connector *sde_conn;
	int i;

	for (i = 0; i < cstate->num_connectors; i++) {
		sde_conn = to_sde_connector(cstate->connectors[i]);
		if (sde_crtc_state_in_clone_mode(
				sde_conn->encoder, &cstate->base))
			continue;
		return sde_conn->is_fsc;
	}
	return false;
}

static int _sde_crtc_check_get_pstates(struct drm_crtc *crtc,
		struct drm_crtc_state *state,
		struct drm_display_mode *mode,
		struct plane_state *pstates,
		struct drm_plane *plane,
		struct sde_multirect_plane_states *multirect_plane,
		int *cnt)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	const struct drm_plane_state *pstate;
	const struct drm_plane_state *pipe_staged[SSPP_MAX];
	int rc = 0, multirect_count = 0, i, mixer_width, mixer_height;
	int inc_sde_stage = 0;
	struct sde_kms *kms;
	u32 blend_type;
	bool is_fsc;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);

	kms = _sde_crtc_get_kms(crtc);
	if (!kms || !kms->catalog) {
		SDE_ERROR("invalid kms\n");
		return -EINVAL;
	}

	memset(pipe_staged, 0, sizeof(pipe_staged));
	is_fsc = sde_crtc_is_connector_fsc(cstate);
	mixer_width = sde_crtc_get_mixer_width(sde_crtc, cstate, mode);
	mixer_height = sde_crtc_get_mixer_height(sde_crtc, cstate, mode);

	if (cstate->num_ds_enabled)
		mixer_width = mixer_width * cstate->num_ds_enabled;

	drm_atomic_crtc_state_for_each_plane_state(plane, pstate, state) {
		if (IS_ERR_OR_NULL(pstate)) {
			rc = PTR_ERR(pstate);
			SDE_ERROR("%s: failed to get plane%d state, %d\n",
					sde_crtc->name, plane->base.id, rc);
			return rc;
		}

		if (*cnt >= SDE_PSTATES_MAX)
			continue;

		pstates[*cnt].sde_pstate = to_sde_plane_state(pstate);
		pstates[*cnt].drm_pstate = pstate;
		pstates[*cnt].stage = sde_plane_get_property(
				pstates[*cnt].sde_pstate, PLANE_PROP_ZPOS);
		pstates[*cnt].pipe_id = sde_plane_pipe(plane);
		blend_type = sde_plane_get_property(pstates[*cnt].sde_pstate,
			PLANE_PROP_BLEND_OP);

		if (!kms->catalog->has_base_layer)
			inc_sde_stage = SDE_STAGE_0;

		/* check dim layer stage with every plane */
		for (i = 0; i < cstate->num_dim_layers; i++) {
			if (cstate->dim_layer[i].stage ==
				(pstates[*cnt].stage + inc_sde_stage)) {
				SDE_ERROR(
					"plane:%d/dim_layer:%i-same stage:%d\n",
					plane->base.id, i,
					cstate->dim_layer[i].stage);
				return -EINVAL;
			}
		}

		if (pipe_staged[pstates[*cnt].pipe_id]) {
			multirect_plane[multirect_count].r0 =
				pipe_staged[pstates[*cnt].pipe_id];
			multirect_plane[multirect_count].r1 = pstate;
			multirect_count++;

			pipe_staged[pstates[*cnt].pipe_id] = NULL;
		} else {
			pipe_staged[pstates[*cnt].pipe_id] = pstate;
		}

		(*cnt)++;

		if (CHECK_LAYER_BOUNDS(pstate->crtc_y, pstate->crtc_h,
				GET_MODE_HEIGHT(is_fsc, mode)) ||
		    CHECK_LAYER_BOUNDS(pstate->crtc_x, pstate->crtc_w,
				GET_MODE_WIDTH(is_fsc, mode))) {
			SDE_ERROR("invalid vertical/horizontal destination\n");
			SDE_ERROR("y:%d h:%d vdisp:%d x:%d w:%d hdisp:%d\n",
				pstate->crtc_y, pstate->crtc_h,
				GET_MODE_HEIGHT(is_fsc, mode),
				pstate->crtc_x, pstate->crtc_w,
				GET_MODE_WIDTH(is_fsc, mode));
			return -E2BIG;
		}

		if (blend_type != SDE_DRM_BLEND_OP_SKIP && cstate->num_ds_enabled &&
				((pstate->crtc_h > mixer_height) ||
				(pstate->crtc_w > mixer_width))) {
			SDE_ERROR("plane w/h:%x*%x > mixer w/h:%x*%x\n",
					pstate->crtc_w, pstate->crtc_h,
					mixer_width, mixer_height);
			return -E2BIG;
		}
	}

	for (i = 1; i < SSPP_MAX; i++) {
		if (pipe_staged[i]) {
			sde_plane_clear_multirect(pipe_staged[i]);
			if (is_sde_plane_virtual(pipe_staged[i]->plane)) {
				struct sde_plane_state *psde_state;

				SDE_DEBUG("r1 only virt plane:%d staged\n",
					 pipe_staged[i]->plane->base.id);

				psde_state = to_sde_plane_state(
						pipe_staged[i]);

				psde_state->multirect_index = SDE_SSPP_RECT_1;
			}
		}
	}

	for (i = 0; i < multirect_count; i++) {
		if (sde_plane_validate_multirect_v2(&multirect_plane[i])) {
			SDE_ERROR(
			"multirect validation failed for planes (%d - %d)\n",
					multirect_plane[i].r0->plane->base.id,
					multirect_plane[i].r1->plane->base.id);
			return -EINVAL;
		}
	}
	return rc;
}

static int _sde_crtc_noise_layer_check_zpos(struct sde_crtc_state *cstate,
                                            u32 zpos) {
	if (!test_bit(SDE_CRTC_NOISE_LAYER, cstate->dirty) ||
		!cstate->noise_layer_en) {
		SDE_DEBUG("noise layer not enabled %d\n", cstate->noise_layer_en);
		return 0;
	}

	if (cstate->layer_cfg.zposn == zpos ||
		cstate->layer_cfg.zposattn == zpos) {
		SDE_ERROR("invalid zpos %d zposn %d zposattn %d\n", zpos,
		     cstate->layer_cfg.zposn, cstate->layer_cfg.zposattn);
		return -EINVAL;
	}
	return 0;
}

static int _sde_crtc_check_zpos(struct drm_crtc_state *state,
		struct sde_crtc *sde_crtc,
		struct plane_state *pstates,
		struct sde_crtc_state *cstate,
		struct drm_display_mode *mode,
		int cnt)
{
	int rc = 0, i, z_pos;
	u32 zpos_cnt = 0;
	struct drm_crtc *crtc;
	struct sde_kms *kms;
	enum sde_layout layout;

	crtc = &sde_crtc->base;
	kms = _sde_crtc_get_kms(crtc);

	if (!kms || !kms->catalog) {
		SDE_ERROR("Invalid kms\n");
		return -EINVAL;
	}

	sort(pstates, cnt, sizeof(pstates[0]), pstate_cmp, NULL);

	rc = _sde_crtc_excl_dim_layer_check(state, pstates, cnt);
	if (rc)
		return rc;

	if (!sde_is_custom_client()) {
		int stage_old = pstates[0].stage;

		z_pos = 0;
		for (i = 0; i < cnt; i++) {
			if (stage_old != pstates[i].stage)
				++z_pos;
			stage_old = pstates[i].stage;
			pstates[i].stage = z_pos;
		}
	}

	z_pos = -1;
	layout = SDE_LAYOUT_NONE;
	for (i = 0; i < cnt; i++) {
		/* reset counts at every new blend stage */
		if (pstates[i].stage != z_pos ||
				pstates[i].sde_pstate->layout != layout) {
			zpos_cnt = 0;
			z_pos = pstates[i].stage;
			layout = pstates[i].sde_pstate->layout;
		}

		/* verify z_pos setting before using it */
		if (z_pos >= SDE_STAGE_MAX - SDE_STAGE_0) {
			SDE_ERROR("> %d plane stages assigned\n",
					SDE_STAGE_MAX - SDE_STAGE_0);
			return -EINVAL;
		} else if (zpos_cnt == 2) {
			SDE_ERROR("> 2 planes @ stage %d\n", z_pos);
			return -EINVAL;
		} else {
			zpos_cnt++;
		}
		rc = _sde_crtc_noise_layer_check_zpos(cstate, z_pos);
		if (rc)
			break;
		if (!kms->catalog->has_base_layer)
			pstates[i].sde_pstate->stage = z_pos + SDE_STAGE_0;
		else
			pstates[i].sde_pstate->stage = z_pos;

		SDE_DEBUG("%s: layout %d, zpos %d", sde_crtc->name, layout,
				z_pos);
	}
	return rc;
}

static int _sde_crtc_atomic_check_pstates(struct drm_crtc *crtc,
		struct drm_crtc_state *state,
		struct plane_state *pstates,
		struct sde_multirect_plane_states *multirect_plane)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct sde_kms *kms;
	struct drm_plane *plane = NULL;
	struct drm_display_mode *mode;
	int rc = 0, cnt = 0;

	kms = _sde_crtc_get_kms(crtc);

	if (!kms || !kms->catalog) {
		SDE_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);
	mode = &state->adjusted_mode;

	/* get plane state for all drm planes associated with crtc state */
	rc = _sde_crtc_check_get_pstates(crtc, state, mode, pstates,
			plane, multirect_plane, &cnt);
	if (rc)
		return rc;

	/* assign mixer stages based on sorted zpos property */
	rc = _sde_crtc_check_zpos(state, sde_crtc, pstates, cstate, mode, cnt);
	if (rc)
		return rc;

	rc = _sde_crtc_check_secure_state(crtc, state, pstates, cnt);
	if (rc)
		return rc;

	/*
	 * validate and set source split:
	 * use pstates sorted by stage to check planes on same stage
	 * we assume that all pipes are in source split so its valid to compare
	 * without taking into account left/right mixer placement
	 */
	rc = _sde_crtc_validate_src_split_order(crtc, pstates, cnt);
	if (rc)
		return rc;

	return 0;
}

static int _sde_crtc_check_plane_layout(struct drm_crtc *crtc,
		struct drm_crtc_state *crtc_state)
{
	struct sde_kms *kms;
	struct drm_plane *plane;
	struct drm_plane_state *plane_state;
	struct sde_plane_state *pstate;
	int layout_split;

	kms = _sde_crtc_get_kms(crtc);

	if (!kms || !kms->catalog) {
		SDE_ERROR("invalid parameters\n");
		return -EINVAL;
	}

	if (!sde_rm_topology_is_group(&kms->rm, crtc_state,
			SDE_RM_TOPOLOGY_GROUP_QUADPIPE))
		return 0;

	drm_atomic_crtc_state_for_each_plane(plane, crtc_state) {
		plane_state = drm_atomic_get_existing_plane_state(
				crtc_state->state, plane);
		if (!plane_state)
			continue;

		pstate = to_sde_plane_state(plane_state);
		layout_split = crtc_state->mode.hdisplay >> 1;
		if (sde_crtc_is_connector_fsc(to_sde_crtc_state(crtc_state)))
			layout_split /= PLANAR_RGB_PACKING;

		if (plane_state->crtc_x >= layout_split) {
			plane_state->crtc_x -= layout_split;
			pstate->layout_offset = layout_split;
			pstate->layout = SDE_LAYOUT_RIGHT;
		} else {
			pstate->layout_offset = -1;
			pstate->layout = SDE_LAYOUT_LEFT;
		}
		SDE_DEBUG("plane%d updated: crtc_x=%d layout=%d\n",
				DRMID(plane), plane_state->crtc_x,
				pstate->layout);

		/* check layout boundary */
		if (CHECK_LAYER_BOUNDS(plane_state->crtc_x,
				plane_state->crtc_w, layout_split)) {
			SDE_ERROR("invalid horizontal destination\n");
			SDE_ERROR("x:%d w:%d hdisp:%d layout:%d\n",
					plane_state->crtc_x,
					plane_state->crtc_w,
					layout_split, pstate->layout);
			return -E2BIG;
		}
	}

	return 0;
}

static int _sde_crtc_check_fsc_planes(struct drm_crtc *crtc,
		struct drm_crtc_state *crtc_state)
{
	struct sde_format *format = NULL;
	struct drm_plane *plane = NULL;
	struct drm_plane_state *plane_state = NULL;
	struct sde_plane_state *pstate = NULL;
	struct sde_crtc_state *cstate;
	int fsc_plane_count = 0, non_fsc_plane_count = 0;
	bool is_fsc_mode, is_dirty;
	uint32_t bg_alpha;

	cstate = to_sde_crtc_state(crtc_state);
	is_fsc_mode = sde_crtc_is_connector_fsc(cstate);

	drm_atomic_crtc_state_for_each_plane(plane, crtc_state) {
		plane_state = drm_atomic_get_new_plane_state(
				crtc_state->state, plane);
		if (!plane_state)
			continue;

		pstate = to_sde_plane_state(plane_state);
		format = to_sde_format(msm_framebuffer_format(
				plane_state->fb));
		if (!format) {
			SDE_ERROR("invalid format\n");
			return -EINVAL;
		} else if (SDE_FORMAT_IS_FSC(format)) {
			is_dirty = sde_plane_property_is_dirty(
					plane_state, PLANE_PROP_BG_ALPHA);
			bg_alpha = sde_plane_get_property(pstate, PLANE_PROP_BG_ALPHA);
			if (!is_fsc_mode && (!is_dirty || bg_alpha != 0xFF)) {
				/* return if bg_alpha is not set or not equal to FF
				 * for fsc to rgb use case
				 */
				SDE_ERROR("Invalid params is_dirty: %d\n", is_dirty);
				return -EINVAL;
			}
			fsc_plane_count++;
		} else {
			non_fsc_plane_count++;
		}
	}

	if (fsc_plane_count && non_fsc_plane_count) {
		SDE_ERROR("%d fsc and %d other planes detected together\n",
				fsc_plane_count, non_fsc_plane_count);
		return -EINVAL;
	}

	return 0;
}

void sde_crtc_state_setup_connectors(struct drm_crtc_state *state, struct drm_device *dev)
{
	struct drm_connector_list_iter conn_iter;
	struct drm_connector *conn;
	struct sde_crtc_state *cstate = to_sde_crtc_state(state);

	/* identify connectors attached to this crtc */
	cstate->num_connectors = 0;

	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(conn, &conn_iter)
		if ((state->connector_mask & (1 << drm_connector_index(conn)))
				&& cstate->num_connectors < MAX_CONNECTORS) {
			cstate->connectors[cstate->num_connectors++] = conn;
		}
	drm_connector_list_iter_end(&conn_iter);
}

static int sde_crtc_atomic_check(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct drm_device *dev;
	struct sde_crtc *sde_crtc;
	struct plane_state *pstates = NULL;
	struct drm_display_mode *mode;
	int rc = 0;
	struct sde_multirect_plane_states *multirect_plane = NULL;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return -EINVAL;
	}

	dev = crtc->dev;
	sde_crtc = to_sde_crtc(crtc);

	if (!state->enable || !state->active) {
		SDE_DEBUG("crtc%d -> enable %d, active %d, skip atomic_check\n",
				crtc->base.id, state->enable, state->active);
		goto end;
	}

	pstates = kcalloc(SDE_PSTATES_MAX,
			sizeof(struct plane_state), GFP_KERNEL);

	multirect_plane = kcalloc(SDE_MULTIRECT_PLANE_MAX,
			sizeof(struct sde_multirect_plane_states),
			GFP_KERNEL);

	if (!pstates || !multirect_plane) {
		rc = -ENOMEM;
		goto end;
	}

	mode = &state->adjusted_mode;
	SDE_DEBUG("%s: check", sde_crtc->name);

	/* force a full mode set if active state changed */
	if (state->active_changed)
		state->mode_changed = true;

	rc = _sde_crtc_check_dest_scaler_data(crtc, state);
	if (rc) {
		SDE_ERROR("crtc%d failed dest scaler check %d\n",
			crtc->base.id, rc);
		goto end;
	}

	rc = _sde_crtc_check_plane_layout(crtc, state);
	if (rc) {
		SDE_ERROR("crtc%d failed plane layout check %d\n",
				crtc->base.id, rc);
		goto end;
	}

	rc = _sde_crtc_check_fsc_planes(crtc, state);
	if (rc) {
		SDE_ERROR("crtc%d failed fsc planes check %d\n",
				crtc->base.id, rc);
		goto end;
	}

	_sde_crtc_setup_is_ppsplit(state);
	_sde_crtc_setup_lm_bounds(crtc, state);

	rc = _sde_crtc_atomic_check_pstates(crtc, state, pstates,
			multirect_plane);
	if (rc) {
		SDE_ERROR("crtc%d failed pstate check %d\n", crtc->base.id, rc);
		goto end;
	}

	rc = sde_core_perf_crtc_check(crtc, state);
	if (rc) {
		SDE_ERROR("crtc%d failed performance check %d\n",
				crtc->base.id, rc);
		goto end;
	}

	rc = _sde_crtc_check_rois(crtc, state);
	if (rc) {
		SDE_ERROR("crtc%d failed roi check %d\n", crtc->base.id, rc);
		goto end;
	}

	rc = sde_cp_crtc_check_properties(crtc, state);
	if (rc) {
		SDE_ERROR("crtc%d failed cp properties check %d\n",
				crtc->base.id, rc);
		goto end;
	}
end:
	kfree(pstates);
	kfree(multirect_plane);
	return rc;
}

/**
 * sde_crtc_get_num_datapath - get the number of layermixers active
 *				on primary connector
 * @crtc: Pointer to DRM crtc object
 * @virtual_conn: Pointer to DRM connector object of WB in CWB case
 * @crtc_state:	Pointer to DRM crtc state
 */
int sde_crtc_get_num_datapath(struct drm_crtc *crtc,
	struct drm_connector *virtual_conn, struct drm_crtc_state *crtc_state)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct drm_connector *conn, *primary_conn = NULL;
	struct sde_connector_state *sde_conn_state = NULL;
	struct drm_connector_list_iter conn_iter;
	int num_lm = 0;

	if (!sde_crtc || !virtual_conn || !crtc_state) {
		SDE_DEBUG("Invalid argument\n");
		return 0;
	}

	/* return num_mixers used for primary when available in sde_crtc */
	if (sde_crtc->num_mixers)
		return sde_crtc->num_mixers;

	drm_connector_list_iter_begin(crtc->dev, &conn_iter);
	drm_for_each_connector_iter(conn, &conn_iter) {
		if ((drm_connector_mask(conn) & crtc_state->connector_mask)
			 && conn != virtual_conn) {
			sde_conn_state = to_sde_connector_state(conn->state);
			primary_conn = conn;
			break;
		}
	}
	drm_connector_list_iter_end(&conn_iter);

	/* if primary sde_conn_state has mode info available, return num_lm from here */
	if (sde_conn_state)
		num_lm = sde_conn_state->mode_info.topology.num_lm;

	/* if PM resume occurs with CWB enabled, retrieve num_lm from primary dsi panel mode */
	if (primary_conn && !num_lm) {
		num_lm = sde_connector_get_lm_cnt_from_topology(primary_conn,
				&crtc_state->adjusted_mode);
		if (num_lm < 0) {
			SDE_DEBUG("lm cnt fail for conn:%d num_lm:%d\n",
					 primary_conn->base.id, num_lm);
			num_lm = 0;
		}
	}

	return num_lm;
}

int sde_crtc_vblank(struct drm_crtc *crtc, bool en)
{
	struct sde_crtc *sde_crtc;
	int ret;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return -EINVAL;
	}
	sde_crtc = to_sde_crtc(crtc);

	ret = _sde_crtc_vblank_enable(sde_crtc, en);
	if (ret)
		SDE_ERROR("%s vblank enable failed: %d\n",
				sde_crtc->name, ret);

	return 0;
}

static u32 sde_crtc_get_vblank_counter(struct drm_crtc *crtc)
{
	struct drm_encoder *encoder;
	struct sde_crtc *sde_crtc;
	bool is_built_in;
	u32 vblank_cnt;

	if (!crtc)
		return 0;

	sde_crtc = to_sde_crtc(crtc);

	drm_for_each_encoder_mask(encoder, crtc->dev, sde_crtc->cached_encoder_mask) {
		if (sde_encoder_in_clone_mode(encoder))
			continue;

		is_built_in = sde_encoder_is_built_in_display(encoder);
		vblank_cnt = sde_encoder_get_frame_count(encoder);

		SDE_EVT32(DRMID(crtc), DRMID(encoder), is_built_in, vblank_cnt);
		SDE_DEBUG("crtc:%d enc:%d is_built_in:%d vblank_cnt:%d\n",
				DRMID(crtc), DRMID(encoder), is_built_in, vblank_cnt);

		return vblank_cnt;
	}

	return 0;
}

static bool sde_crtc_get_vblank_timestamp(struct drm_crtc *crtc, int *max_error,
				ktime_t *tvblank, bool in_vblank_irq)
{
	struct drm_encoder *encoder;
	struct sde_crtc *sde_crtc;

	if (!crtc)
		return false;

	sde_crtc = to_sde_crtc(crtc);

	drm_for_each_encoder_mask(encoder, crtc->dev, sde_crtc->cached_encoder_mask) {
		if (sde_encoder_in_clone_mode(encoder))
			continue;

		return sde_encoder_get_vblank_timestamp(encoder, tvblank);
	}

	return false;
}

static void sde_crtc_install_dest_scale_properties(struct sde_crtc *sde_crtc,
		struct sde_mdss_cfg *catalog, struct sde_kms_info *info)
{
	sde_kms_info_add_keyint(info, "has_dest_scaler",
			catalog->mdp[0].has_dest_scaler);
	sde_kms_info_add_keyint(info, "dest_scaler_count",
				catalog->ds_count);

	if (catalog->ds[0].top) {
		sde_kms_info_add_keyint(info,
				"max_dest_scaler_input_width",
				catalog->ds[0].top->maxinputwidth);
		sde_kms_info_add_keyint(info,
				"max_dest_scaler_output_width",
				catalog->ds[0].top->maxoutputwidth);
		sde_kms_info_add_keyint(info, "max_dest_scale_up",
				catalog->ds[0].top->maxupscale);
	}

	if (catalog->ds[0].features & BIT(SDE_SSPP_SCALER_QSEED3)) {
		msm_property_install_volatile_range(
				&sde_crtc->property_info, "dest_scaler",
				0x0, 0, ~0, 0, CRTC_PROP_DEST_SCALER);
		msm_property_install_blob(&sde_crtc->property_info,
				"ds_lut_ed", 0,
				CRTC_PROP_DEST_SCALER_LUT_ED);
		msm_property_install_blob(&sde_crtc->property_info,
				"ds_lut_cir", 0,
				CRTC_PROP_DEST_SCALER_LUT_CIR);
		msm_property_install_blob(&sde_crtc->property_info,
				"ds_lut_sep", 0,
				CRTC_PROP_DEST_SCALER_LUT_SEP);
	} else if (catalog->ds[0].features
			& BIT(SDE_SSPP_SCALER_QSEED3LITE)) {
		msm_property_install_volatile_range(
				&sde_crtc->property_info, "dest_scaler",
				0x0, 0, ~0, 0, CRTC_PROP_DEST_SCALER);
	}
}

static void sde_crtc_install_perf_properties(struct sde_crtc *sde_crtc,
		struct sde_kms *sde_kms, struct sde_mdss_cfg *catalog,
		struct sde_kms_info *info)
{
	msm_property_install_range(&sde_crtc->property_info,
			"core_clk", 0x0, 0, U64_MAX,
			sde_kms->perf.max_core_clk_rate,
			CRTC_PROP_CORE_CLK);
	msm_property_install_range(&sde_crtc->property_info,
			"core_ab", 0x0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_CORE_AB);
	msm_property_install_range(&sde_crtc->property_info,
			"core_ib", 0x0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_CORE_IB);
	msm_property_install_range(&sde_crtc->property_info,
			"llcc_ab", 0x0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_LLCC_AB);
	msm_property_install_range(&sde_crtc->property_info,
			"llcc_ib", 0x0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_LLCC_IB);
	msm_property_install_range(&sde_crtc->property_info,
			"dram_ab", 0x0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_DRAM_AB);
	msm_property_install_range(&sde_crtc->property_info,
			"dram_ib", 0x0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_DRAM_IB);
	msm_property_install_range(&sde_crtc->property_info,
			"rot_prefill_bw", 0, 0, U64_MAX,
			catalog->perf.max_bw_high * 1000ULL,
			CRTC_PROP_ROT_PREFILL_BW);
	msm_property_install_range(&sde_crtc->property_info,
			"rot_clk", 0, 0, U64_MAX,
			sde_kms->perf.max_core_clk_rate,
			CRTC_PROP_ROT_CLK);

	if (catalog->perf.max_bw_low)
		sde_kms_info_add_keyint(info, "max_bandwidth_low",
				catalog->perf.max_bw_low * 1000LL);
	if (catalog->perf.max_bw_high)
		sde_kms_info_add_keyint(info, "max_bandwidth_high",
				catalog->perf.max_bw_high * 1000LL);
	if (catalog->perf.min_core_ib)
		sde_kms_info_add_keyint(info, "min_core_ib",
				catalog->perf.min_core_ib * 1000LL);
	if (catalog->perf.min_llcc_ib)
		sde_kms_info_add_keyint(info, "min_llcc_ib",
				catalog->perf.min_llcc_ib * 1000LL);
	if (catalog->perf.min_dram_ib)
		sde_kms_info_add_keyint(info, "min_dram_ib",
				catalog->perf.min_dram_ib * 1000LL);
	if (sde_kms->perf.max_core_clk_rate)
		sde_kms_info_add_keyint(info, "max_mdp_clk",
				sde_kms->perf.max_core_clk_rate);
}

static void sde_crtc_setup_capabilities_blob(struct sde_kms_info *info,
		struct sde_mdss_cfg *catalog)
{
	sde_kms_info_reset(info);

	sde_kms_info_add_keyint(info, "hw_version", catalog->hwversion);
	sde_kms_info_add_keyint(info, "max_linewidth",
			catalog->max_mixer_width);
	sde_kms_info_add_keyint(info, "max_blendstages",
			catalog->max_mixer_blendstages);

	if (catalog->qseed_sw_lib_rev == SDE_SSPP_SCALER_QSEED2)
		sde_kms_info_add_keystr(info, "qseed_type", "qseed2");
	if (catalog->qseed_sw_lib_rev == SDE_SSPP_SCALER_QSEED3)
		sde_kms_info_add_keystr(info, "qseed_type", "qseed3");
	if (catalog->qseed_sw_lib_rev == SDE_SSPP_SCALER_QSEED3LITE)
		sde_kms_info_add_keystr(info, "qseed_type", "qseed3lite");

	if (catalog->ubwc_version) {
		sde_kms_info_add_keyint(info, "UBWC version",
				catalog->ubwc_version);
		sde_kms_info_add_keyint(info, "UBWC macrotile_mode",
				catalog->macrotile_mode);
		sde_kms_info_add_keyint(info, "UBWC highest banking bit",
				catalog->mdp[0].highest_bank_bit);
		sde_kms_info_add_keyint(info, "UBWC swizzle",
				catalog->mdp[0].ubwc_swizzle);
	}

	if (of_fdt_get_ddrtype() == LP_DDR4_TYPE)
		sde_kms_info_add_keystr(info, "DDR version", "DDR4");
	else
		sde_kms_info_add_keystr(info, "DDR version", "DDR5");

	if (sde_is_custom_client()) {
		/* No support for SMART_DMA_V1 yet */
		if (catalog->smart_dma_rev == SDE_SSPP_SMART_DMA_V2)
			sde_kms_info_add_keystr(info,
					"smart_dma_rev", "smart_dma_v2");
		else if (catalog->smart_dma_rev == SDE_SSPP_SMART_DMA_V2p5)
			sde_kms_info_add_keystr(info,
					"smart_dma_rev", "smart_dma_v2p5");
	}

	sde_kms_info_add_keyint(info, "has_src_split", catalog->has_src_split);
	sde_kms_info_add_keyint(info, "has_hdr", catalog->has_hdr);
	sde_kms_info_add_keyint(info, "has_hdr_plus", catalog->has_hdr_plus);
	sde_kms_info_add_keyint(info, "skip_inline_rot_threshold",
			catalog->skip_inline_rot_threshold);

	if (catalog->allowed_dsc_reservation_switch)
		sde_kms_info_add_keyint(info, "allowed_dsc_reservation_switch",
			catalog->allowed_dsc_reservation_switch);

	if (catalog->uidle_cfg.uidle_rev)
		sde_kms_info_add_keyint(info, "has_uidle",
			true);

	sde_kms_info_add_keystr(info, "core_ib_ff",
			catalog->perf.core_ib_ff);
	sde_kms_info_add_keystr(info, "core_clk_ff",
			catalog->perf.core_clk_ff);
	sde_kms_info_add_keystr(info, "comp_ratio_rt",
			catalog->perf.comp_ratio_rt);
	sde_kms_info_add_keystr(info, "comp_ratio_nrt",
			catalog->perf.comp_ratio_nrt);
	sde_kms_info_add_keyint(info, "dest_scale_prefill_lines",
			catalog->perf.dest_scale_prefill_lines);
	sde_kms_info_add_keyint(info, "undersized_prefill_lines",
			catalog->perf.undersized_prefill_lines);
	sde_kms_info_add_keyint(info, "macrotile_prefill_lines",
			catalog->perf.macrotile_prefill_lines);
	sde_kms_info_add_keyint(info, "yuv_nv12_prefill_lines",
			catalog->perf.yuv_nv12_prefill_lines);
	sde_kms_info_add_keyint(info, "linear_prefill_lines",
			catalog->perf.linear_prefill_lines);
	sde_kms_info_add_keyint(info, "downscaling_prefill_lines",
			catalog->perf.downscaling_prefill_lines);
	sde_kms_info_add_keyint(info, "xtra_prefill_lines",
			catalog->perf.xtra_prefill_lines);
	sde_kms_info_add_keyint(info, "amortizable_threshold",
			catalog->perf.amortizable_threshold);
	sde_kms_info_add_keyint(info, "min_prefill_lines",
			catalog->perf.min_prefill_lines);
	sde_kms_info_add_keyint(info, "num_mnoc_ports",
			catalog->perf.num_mnoc_ports);
	sde_kms_info_add_keyint(info, "axi_bus_width",
			catalog->perf.axi_bus_width);

	sde_kms_info_add_keyint(info, "sec_ui_blendstage",
			catalog->sui_supported_blendstage);

	if (catalog->ubwc_bw_calc_version)
		sde_kms_info_add_keyint(info, "ubwc_bw_calc_ver",
				catalog->ubwc_bw_calc_version);
}

/**
 * sde_crtc_install_properties - install all drm properties for crtc
 * @crtc: Pointer to drm crtc structure
 */
static void sde_crtc_install_properties(struct drm_crtc *crtc,
				struct sde_mdss_cfg *catalog)
{
	struct sde_crtc *sde_crtc;
	struct sde_kms_info *info;
	struct sde_kms *sde_kms;

	static const struct drm_prop_enum_list e_secure_level[] = {
		{SDE_DRM_SEC_NON_SEC, "sec_and_non_sec"},
		{SDE_DRM_SEC_ONLY, "sec_only"},
	};

	static const struct drm_prop_enum_list e_cwb_data_points[] = {
		{CAPTURE_MIXER_OUT, "capture_mixer_out"},
		{CAPTURE_DSPP_OUT, "capture_pp_out"},
	};

	static const struct drm_prop_enum_list e_dcwb_data_points[] = {
		{CAPTURE_MIXER_OUT, "capture_mixer_out"},
		{CAPTURE_DSPP_OUT, "capture_pp_out"},
	};

	static const struct drm_prop_enum_list e_idle_pc_state[] = {
		{IDLE_PC_NONE, "idle_pc_none"},
		{IDLE_PC_ENABLE, "idle_pc_enable"},
		{IDLE_PC_DISABLE, "idle_pc_disable"},
	};

	static const struct drm_prop_enum_list e_cache_state[] = {
		{CACHE_STATE_DISABLED, "cache_state_disabled"},
		{CACHE_STATE_ENABLED, "cache_state_enabled"},
	};

	static const struct drm_prop_enum_list e_vm_req_state[] = {
		{VM_REQ_NONE, "vm_req_none"},
		{VM_REQ_RELEASE, "vm_req_release"},
		{VM_REQ_ACQUIRE, "vm_req_acquire"},
	};

	SDE_DEBUG("\n");

	if (!crtc || !catalog) {
		SDE_ERROR("invalid crtc or catalog\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid argument\n");
		return;
	}

	info = vzalloc(sizeof(struct sde_kms_info));
	if (!info) {
		SDE_ERROR("failed to allocate info memory\n");
		return;
	}

	sde_crtc_setup_capabilities_blob(info, catalog);

	/* mi properties */
	mi_sde_crtc_install_properties(&sde_crtc->property_info);

	msm_property_install_range(&sde_crtc->property_info,
		"input_fence_timeout", 0x0, 0,
		SDE_CRTC_MAX_INPUT_FENCE_TIMEOUT, SDE_CRTC_INPUT_FENCE_TIMEOUT,
		CRTC_PROP_INPUT_FENCE_TIMEOUT);
	msm_property_install_volatile_range(&sde_crtc->property_info,
		"output_fence", 0x0, 0, ~0, 0, CRTC_PROP_OUTPUT_FENCE);
	msm_property_install_range(&sde_crtc->property_info,
			"output_fence_offset", 0x0, 0, 1, 0,
			CRTC_PROP_OUTPUT_FENCE_OFFSET);

	sde_crtc_install_perf_properties(sde_crtc, sde_kms, catalog, info);

	if (catalog->has_trusted_vm_support) {
		int init_idx = sde_in_trusted_vm(sde_kms) ? 1 : 0;

		msm_property_install_enum(&sde_crtc->property_info,
			"vm_request_state", 0x0, 0, e_vm_req_state,
			ARRAY_SIZE(e_vm_req_state), init_idx,
			CRTC_PROP_VM_REQ_STATE);
	}

	if (catalog->has_idle_pc)
		msm_property_install_enum(&sde_crtc->property_info,
			"idle_pc_state", 0x0, 0, e_idle_pc_state,
			ARRAY_SIZE(e_idle_pc_state), 0,
			CRTC_PROP_IDLE_PC_STATE);

	if (catalog->has_dedicated_cwb_support)
		msm_property_install_enum(&sde_crtc->property_info,
				"capture_mode", 0, 0, e_dcwb_data_points,
				ARRAY_SIZE(e_dcwb_data_points), 0,
				CRTC_PROP_CAPTURE_OUTPUT);
	else if (catalog->has_cwb_support)
		msm_property_install_enum(&sde_crtc->property_info,
				"capture_mode", 0, 0, e_cwb_data_points,
				ARRAY_SIZE(e_cwb_data_points), 0,
				CRTC_PROP_CAPTURE_OUTPUT);

	msm_property_install_volatile_range(&sde_crtc->property_info,
		"sde_drm_roi_v1", 0x0, 0, ~0, 0, CRTC_PROP_ROI_V1);

	msm_property_install_enum(&sde_crtc->property_info, "security_level",
			0x0, 0, e_secure_level,
			ARRAY_SIZE(e_secure_level), 0,
			CRTC_PROP_SECURITY_LEVEL);

	if (catalog->syscache_supported)
		msm_property_install_enum(&sde_crtc->property_info, "cache_state",
			0x0, 0, e_cache_state,
			ARRAY_SIZE(e_cache_state), 0,
			CRTC_PROP_CACHE_STATE);

	if (catalog->has_dim_layer) {
		msm_property_install_volatile_range(&sde_crtc->property_info,
			"dim_layer_v1", 0x0, 0, ~0, 0, CRTC_PROP_DIM_LAYER_V1);
		sde_kms_info_add_keyint(info, "dim_layer_v1_max_layers",
				SDE_MAX_DIM_LAYERS);
	}

	if (catalog->mdp[0].has_dest_scaler)
		sde_crtc_install_dest_scale_properties(sde_crtc, catalog,
				info);

	if (catalog->dspp_count) {
		sde_kms_info_add_keyint(info, "dspp_count",
				catalog->dspp_count);

		if (catalog->rc_count) {
			sde_kms_info_add_keyint(info, "rc_count", catalog->rc_count);
			sde_kms_info_add_keyint(info, "rc_mem_size",
				catalog->dspp[0].sblk->rc.mem_total_size);
		}

		if (catalog->demura_count)
			sde_kms_info_add_keyint(info, "demura_count",
					catalog->demura_count);
	}

	sde_kms_info_add_keyint(info, "dsc_block_count", catalog->dsc_count);

	msm_property_install_blob(&sde_crtc->property_info, "capabilities",
		DRM_MODE_PROP_IMMUTABLE, CRTC_PROP_INFO);

	sde_kms_info_add_keyint(info, "use_baselayer_for_stage",
			 catalog->has_base_layer);

	msm_property_set_blob(&sde_crtc->property_info, &sde_crtc->blob_info,
			info->data, SDE_KMS_INFO_DATALEN(info),
			CRTC_PROP_INFO);

	sde_crtc_install_noise_layer_properties(sde_crtc, catalog, info);

	if (catalog->has_ubwc_stats)
		msm_property_install_range(&sde_crtc->property_info, "frame_data",
				0x0, 0, ~0, 0, CRTC_PROP_FRAME_DATA_BUF);

	vfree(info);
}

static int _sde_crtc_get_output_fence(struct drm_crtc *crtc,
	const struct drm_crtc_state *state, uint64_t *val)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	uint32_t offset;
	bool is_vid = false;
	struct drm_encoder *encoder;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);

	drm_for_each_encoder_mask(encoder, crtc->dev, state->encoder_mask) {
		if (sde_encoder_check_curr_mode(encoder,
						MSM_DISPLAY_VIDEO_MODE))
			is_vid = true;
		if (is_vid)
			break;
	}

	offset = sde_crtc_get_property(cstate, CRTC_PROP_OUTPUT_FENCE_OFFSET);

	/*
	 * Increment trigger offset for vidoe mode alone as its release fence
	 * can be triggered only after the next frame-update. For cmd mode &
	 * virtual displays the release fence for the current frame can be
	 * triggered right after PP_DONE/WB_DONE interrupt
	 */
	if (is_vid)
		offset++;

	/*
	 * Hwcomposer now queries the fences using the commit list in atomic
	 * commit ioctl. The offset should be set to next timeline
	 * which will be incremented during the prepare commit phase
	 */
	offset++;

	return sde_fence_create(sde_crtc->output_fence, val, offset);
}

/**
 * sde_crtc_atomic_set_property - atomically set a crtc drm property
 * @crtc: Pointer to drm crtc structure
 * @state: Pointer to drm crtc state structure
 * @property: Pointer to targeted drm property
 * @val: Updated property value
 * @Returns: Zero on success
 */
static int sde_crtc_atomic_set_property(struct drm_crtc *crtc,
		struct drm_crtc_state *state,
		struct drm_property *property,
		uint64_t val)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	int idx, ret;
	uint64_t fence_user_fd;
	uint64_t __user prev_user_fd;

	if (!crtc || !state || !property) {
		SDE_ERROR("invalid argument(s)\n");
		return -EINVAL;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);

	SDE_ATRACE_BEGIN("sde_crtc_atomic_set_property");
	/* check with cp property system first */
	ret = sde_cp_crtc_set_property(crtc, state, property, val);
	if (ret != -ENOENT)
		goto exit;

	/* if not handled by cp, check msm_property system */
	ret = msm_property_atomic_set(&sde_crtc->property_info,
			&cstate->property_state, property, val);
	if (ret)
		goto exit;

	idx = msm_property_index(&sde_crtc->property_info, property);
	switch (idx) {
	case CRTC_PROP_INPUT_FENCE_TIMEOUT:
		_sde_crtc_set_input_fence_timeout(cstate);
		break;
	case CRTC_PROP_DIM_LAYER_V1:
		_sde_crtc_set_dim_layer_v1(crtc, cstate,
					(void __user *)(uintptr_t)val);
		break;
	case CRTC_PROP_ROI_V1:
		ret = _sde_crtc_set_roi_v1(state,
					(void __user *)(uintptr_t)val);
		break;
	case CRTC_PROP_DEST_SCALER:
		ret = _sde_crtc_set_dest_scaler(sde_crtc, cstate,
				(void __user *)(uintptr_t)val);
		break;
	case CRTC_PROP_DEST_SCALER_LUT_ED:
	case CRTC_PROP_DEST_SCALER_LUT_CIR:
	case CRTC_PROP_DEST_SCALER_LUT_SEP:
		ret = _sde_crtc_set_dest_scaler_lut(sde_crtc, cstate, idx);
		break;
	case CRTC_PROP_CORE_CLK:
	case CRTC_PROP_CORE_AB:
	case CRTC_PROP_CORE_IB:
		cstate->bw_control = true;
		break;
	case CRTC_PROP_LLCC_AB:
	case CRTC_PROP_LLCC_IB:
	case CRTC_PROP_DRAM_AB:
	case CRTC_PROP_DRAM_IB:
		cstate->bw_control = true;
		cstate->bw_split_vote = true;
		break;
	case CRTC_PROP_OUTPUT_FENCE:
		if (!val)
			goto exit;

		ret = copy_from_user(&prev_user_fd, (void __user *)val,
				sizeof(uint64_t));
		if (ret) {
			SDE_ERROR("copy from user failed rc:%d\n", ret);
			ret = -EFAULT;
			goto exit;
		}

		/*
		 * client is expected to reset the property to -1 before
		 * requesting for the release fence
		 */
		if (prev_user_fd == -1) {
			ret = _sde_crtc_get_output_fence(crtc, state,
					&fence_user_fd);
			if (ret) {
				SDE_ERROR("fence create failed rc:%d\n", ret);
				goto exit;
			}

			ret = copy_to_user((uint64_t __user *)(uintptr_t)val,
					&fence_user_fd, sizeof(uint64_t));
			if (ret) {
				SDE_ERROR("copy to user failed rc:%d\n", ret);
				put_unused_fd(fence_user_fd);
				ret = -EFAULT;
				goto exit;
			}
		}
		break;
	case CRTC_PROP_NOISE_LAYER_V1:
		_sde_crtc_set_noise_layer(sde_crtc, cstate,
					(void __user *)(uintptr_t)val);
		break;
	case CRTC_PROP_FRAME_DATA_BUF:
		_sde_crtc_set_frame_data_buffers(crtc, cstate, (void __user *)(uintptr_t)val);
		break;
	default:
		/* nothing to do */
		break;
	}

exit:
	if (ret) {
		if (ret != -EPERM)
			SDE_ERROR("%s: failed to set property%d %s: %d\n",
				crtc->name, DRMID(property),
				property->name, ret);
		else
			SDE_DEBUG("%s: failed to set property%d %s: %d\n",
				crtc->name, DRMID(property),
				property->name, ret);
	} else {
		SDE_DEBUG("%s: %s[%d] <= 0x%llx\n", crtc->name, property->name,
				property->base.id, val);
	}

	SDE_ATRACE_END("sde_crtc_atomic_set_property");
	return ret;
}

static void sde_crtc_update_line_time(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct drm_encoder *encoder;
	u32 min_transfer_time = 0, updated_fps = 0;

	drm_for_each_encoder_mask(encoder, crtc->dev, crtc->state->encoder_mask) {
		if (sde_encoder_check_curr_mode(encoder, MSM_DISPLAY_CMD_MODE))
			sde_encoder_get_transfer_time(encoder, &min_transfer_time);
	}

	if (min_transfer_time) {
		/* get fps by doing 1000 ms / transfer_time */
		updated_fps = DIV_ROUND_UP(1000000, min_transfer_time);
		/* get line time by doing 1000ns / (fps * vactive) */
		sde_crtc->line_time_in_ns = DIV_ROUND_UP(1000000000,
				updated_fps * crtc->mode.vdisplay);
	} else {
		/* get line time by doing 1000ns / (fps * vtotal) */
		sde_crtc->line_time_in_ns = DIV_ROUND_UP(1000000000,
				drm_mode_vrefresh(&crtc->mode) * crtc->mode.vtotal);
	}

	SDE_EVT32(min_transfer_time, updated_fps, crtc->mode.vdisplay, crtc->mode.vtotal,
		drm_mode_vrefresh(&crtc->mode), sde_crtc->line_time_in_ns);
}

void sde_crtc_set_qos_dirty(struct drm_crtc *crtc)
{
	struct drm_plane *plane;
	struct drm_plane_state *state;
	struct sde_plane_state *pstate;

	drm_atomic_crtc_for_each_plane(plane, crtc) {
		state = plane->state;
		if (!state)
			continue;

		pstate = to_sde_plane_state(state);

		pstate->dirty |= SDE_PLANE_DIRTY_QOS;
	}
	sde_crtc_update_line_time(crtc);
}

/**
 * sde_crtc_atomic_get_property - retrieve a crtc drm property
 * @crtc: Pointer to drm crtc structure
 * @state: Pointer to drm crtc state structure
 * @property: Pointer to targeted drm property
 * @val: Pointer to variable for receiving property value
 * @Returns: Zero on success
 */
static int sde_crtc_atomic_get_property(struct drm_crtc *crtc,
		const struct drm_crtc_state *state,
		struct drm_property *property,
		uint64_t *val)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	int ret = -EINVAL, i;

	if (!crtc || !state) {
		SDE_ERROR("invalid argument(s)\n");
		goto end;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(state);

	i = msm_property_index(&sde_crtc->property_info, property);
	if (i == CRTC_PROP_OUTPUT_FENCE) {
		*val = ~0;
		ret = 0;
	} else {
		ret = msm_property_atomic_get(&sde_crtc->property_info,
			&cstate->property_state, property, val);
		if (ret)
			ret = sde_cp_crtc_get_property(crtc, property, val);
	}
	if (ret)
		DRM_ERROR("get property failed\n");

end:
	return ret;
}

int sde_crtc_helper_reset_custom_properties(struct drm_crtc *crtc,
		struct drm_crtc_state *crtc_state)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct drm_property *drm_prop;
	enum msm_mdp_crtc_property prop_idx;

	if (!crtc || !crtc_state) {
		SDE_ERROR("invalid params\n");
		return -EINVAL;
	}

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc_state);

	sde_cp_crtc_clear(crtc);

	for (prop_idx = 0; prop_idx < CRTC_PROP_COUNT; prop_idx++) {
		uint64_t val = cstate->property_values[prop_idx].value;
		uint64_t def;
		int ret;

		drm_prop = msm_property_index_to_drm_property(
				&sde_crtc->property_info, prop_idx);
		if (!drm_prop) {
			/* not all props will be installed, based on caps */
			SDE_DEBUG("%s: invalid property index %d\n",
					sde_crtc->name, prop_idx);
			continue;
		}

		def = msm_property_get_default(&sde_crtc->property_info,
				prop_idx);
		if (val == def)
			continue;

		SDE_DEBUG("%s: set prop %s idx %d from %llu to %llu\n",
				sde_crtc->name, drm_prop->name, prop_idx, val,
				def);

		ret = sde_crtc_atomic_set_property(crtc, crtc_state, drm_prop,
				def);
		if (ret) {
			SDE_ERROR("%s: set property failed, idx %d ret %d\n",
					sde_crtc->name, prop_idx, ret);
			continue;
		}
	}

	/* disable clk and bw control until clk & bw properties are set */
	cstate->bw_control = false;
	cstate->bw_split_vote = false;
	return 0;
}

void sde_crtc_misr_setup(struct drm_crtc *crtc, bool enable, u32 frame_count)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_mixer *m;
	int i;

	if (!crtc) {
		SDE_ERROR("invalid argument\n");
		return;
	}
	sde_crtc = to_sde_crtc(crtc);

	sde_crtc->misr_enable_sui = enable;
	sde_crtc->misr_frame_count = frame_count;
	for (i = 0; i < sde_crtc->num_mixers; ++i) {
		m = &sde_crtc->mixers[i];
		if (!m->hw_lm || !m->hw_lm->ops.setup_misr)
			continue;

		m->hw_lm->ops.setup_misr(m->hw_lm, enable, frame_count);
	}
}

void sde_crtc_get_misr_info(struct drm_crtc *crtc,
			struct sde_crtc_misr_info *crtc_misr_info)
{
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;

	if (!crtc_misr_info) {
		SDE_ERROR("invalid misr info\n");
		return;
	}

	crtc_misr_info->misr_enable = false;
	crtc_misr_info->misr_frame_count = 0;

	if (!crtc) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid sde_kms\n");
		return;
	}

	if (sde_kms_is_secure_session_inprogress(sde_kms))
		return;

	sde_crtc = to_sde_crtc(crtc);
	crtc_misr_info->misr_enable =
			sde_crtc->misr_enable_debugfs ? true : false;
	crtc_misr_info->misr_frame_count = sde_crtc->misr_frame_count;
}

#ifdef CONFIG_DEBUG_FS
static int _sde_debugfs_status_show(struct seq_file *s, void *data)
{
	struct sde_crtc *sde_crtc;
	struct sde_plane_state *pstate = NULL;
	struct sde_crtc_mixer *m;

	struct drm_crtc *crtc;
	struct drm_plane *plane;
	struct drm_display_mode *mode;
	struct drm_framebuffer *fb;
	struct drm_plane_state *state;
	struct sde_crtc_state *cstate;

	int i, out_width, out_height;

	if (!s || !s->private)
		return -EINVAL;

	sde_crtc = s->private;
	crtc = &sde_crtc->base;
	cstate = to_sde_crtc_state(crtc->state);

	mutex_lock(&sde_crtc->crtc_lock);
	mode = &crtc->state->adjusted_mode;
	out_width = sde_crtc_get_mixer_width(sde_crtc, cstate, mode);
	out_height = sde_crtc_get_mixer_height(sde_crtc, cstate, mode);

	seq_printf(s, "crtc:%d width:%d height:%d\n", crtc->base.id,
				mode->hdisplay, mode->vdisplay);

	seq_puts(s, "\n");

	for (i = 0; i < sde_crtc->num_mixers; ++i) {
		m = &sde_crtc->mixers[i];
		if (!m->hw_lm)
			seq_printf(s, "\tmixer[%d] has no lm\n", i);
		else if (!m->hw_ctl)
			seq_printf(s, "\tmixer[%d] has no ctl\n", i);
		else
			seq_printf(s, "\tmixer:%d ctl:%d width:%d height:%d\n",
				m->hw_lm->idx - LM_0, m->hw_ctl->idx - CTL_0,
				out_width, out_height);
	}

	seq_puts(s, "\n");

	for (i = 0; i < cstate->num_dim_layers; i++) {
		struct sde_hw_dim_layer *dim_layer = &cstate->dim_layer[i];

		seq_printf(s, "\tdim_layer:%d] stage:%d flags:%d\n",
				i, dim_layer->stage, dim_layer->flags);
		seq_printf(s, "\tdst_x:%d dst_y:%d dst_w:%d dst_h:%d\n",
				dim_layer->rect.x, dim_layer->rect.y,
				dim_layer->rect.w, dim_layer->rect.h);
		seq_printf(s,
			"\tcolor_0:%d color_1:%d color_2:%d color_3:%d\n",
				dim_layer->color_fill.color_0,
				dim_layer->color_fill.color_1,
				dim_layer->color_fill.color_2,
				dim_layer->color_fill.color_3);
		seq_puts(s, "\n");
	}

	drm_atomic_crtc_for_each_plane(plane, crtc) {
		pstate = to_sde_plane_state(plane->state);
		state = plane->state;

		if (!pstate || !state)
			continue;

		seq_printf(s, "\tplane:%u stage:%d rotation:%d\n",
			plane->base.id, pstate->stage, pstate->rotation);

		if (plane->state->fb) {
			fb = plane->state->fb;

			seq_printf(s, "\tfb:%d image format:%4.4s wxh:%ux%u ",
				fb->base.id, (char *) &fb->format->format,
				fb->width, fb->height);
			for (i = 0; i < ARRAY_SIZE(fb->format->cpp); ++i)
				seq_printf(s, "cpp[%d]:%u ",
						i, fb->format->cpp[i]);
			seq_puts(s, "\n\t");

			seq_printf(s, "modifier:%8llu ", fb->modifier);
			seq_puts(s, "\n");

			seq_puts(s, "\t");
			for (i = 0; i < ARRAY_SIZE(fb->pitches); i++)
				seq_printf(s, "pitches[%d]:%8u ", i,
							fb->pitches[i]);
			seq_puts(s, "\n");

			seq_puts(s, "\t");
			for (i = 0; i < ARRAY_SIZE(fb->offsets); i++)
				seq_printf(s, "offsets[%d]:%8u ", i,
							fb->offsets[i]);
			seq_puts(s, "\n");
		}

		seq_printf(s, "\tsrc_x:%4d src_y:%4d src_w:%4d src_h:%4d\n",
			state->src_x >> 16, state->src_y >> 16,
			state->src_w >> 16, state->src_h >> 16);

		seq_printf(s, "\tdst x:%4d dst_y:%4d dst_w:%4d dst_h:%4d\n",
			state->crtc_x, state->crtc_y, state->crtc_w,
			state->crtc_h);
		seq_printf(s, "\tmultirect: mode: %d index: %d\n",
			pstate->multirect_mode, pstate->multirect_index);

		seq_printf(s, "\texcl_rect: x:%4d y:%4d w:%4d h:%4d\n",
			pstate->excl_rect.x, pstate->excl_rect.y,
			pstate->excl_rect.w, pstate->excl_rect.h);

		seq_puts(s, "\n");
	}

	if (sde_crtc->vblank_cb_count) {
		ktime_t diff = ktime_sub(ktime_get(), sde_crtc->vblank_cb_time);
		u32 diff_ms = ktime_to_ms(diff);
		u64 fps = diff_ms ? DIV_ROUND_CLOSEST(
				sde_crtc->vblank_cb_count * 1000, diff_ms) : 0;

		seq_printf(s,
			"vblank fps:%lld count:%u total:%llums total_framecount:%llu\n",
				fps, sde_crtc->vblank_cb_count,
				ktime_to_ms(diff), sde_crtc->play_count);

		/* reset time & count for next measurement */
		sde_crtc->vblank_cb_count = 0;
		sde_crtc->vblank_cb_time = ktime_set(0, 0);
	}

	mutex_unlock(&sde_crtc->crtc_lock);

	return 0;
}

static int _sde_debugfs_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, _sde_debugfs_status_show, inode->i_private);
}

static ssize_t _sde_crtc_misr_setup(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;
	char buf[MISR_BUFF_SIZE + 1];
	u32 frame_count, enable;
	size_t buff_copy;
	struct sde_kms *sde_kms;

	if (!file || !file->private_data)
		return -EINVAL;

	sde_crtc = file->private_data;
	crtc = &sde_crtc->base;

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms) {
		SDE_ERROR("invalid sde_kms\n");
		return -EINVAL;
	}

	buff_copy = min_t(size_t, count, MISR_BUFF_SIZE);
	if (copy_from_user(buf, user_buf, buff_copy)) {
		SDE_ERROR("buffer copy failed\n");
		return -EINVAL;
	}

	buf[buff_copy] = 0; /* end of string */

	if (sscanf(buf, "%u %u", &enable, &frame_count) != 2)
		return -EINVAL;

	if (sde_kms_is_secure_session_inprogress(sde_kms)) {
		SDE_DEBUG("crtc:%d misr enable/disable not allowed\n",
				DRMID(crtc));
		return -EINVAL;
	}

	sde_crtc->misr_enable_debugfs = enable;
	sde_crtc->misr_frame_count = frame_count;
	sde_crtc->misr_reconfigure = true;

	return count;
}

static ssize_t _sde_crtc_misr_read(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	struct drm_crtc *crtc;
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;
	struct sde_crtc_mixer *m;
	int i = 0, rc;
	ssize_t len = 0;
	char buf[MISR_BUFF_SIZE + 1] = {'\0'};

	if (*ppos)
		return 0;

	if (!file || !file->private_data)
		return -EINVAL;

	sde_crtc = file->private_data;
	crtc = &sde_crtc->base;
	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms)
		return -EINVAL;

	rc = pm_runtime_get_sync(crtc->dev->dev);
	if (rc < 0)
		return rc;

	sde_vm_lock(sde_kms);
	if (!sde_vm_owns_hw(sde_kms)) {
		SDE_DEBUG("op not supported due to HW unavailability\n");
		rc = -EOPNOTSUPP;
		goto end;
	}

	if (sde_kms_is_secure_session_inprogress(sde_kms)) {
		SDE_DEBUG("crtc:%d misr read not allowed\n", DRMID(crtc));
		rc = -EOPNOTSUPP;
		goto end;
	}

	if (!sde_crtc->misr_enable_debugfs) {
		len += scnprintf(buf + len, MISR_BUFF_SIZE - len,
				"disabled\n");
		goto buff_check;
	}

	for (i = 0; i < sde_crtc->num_mixers; ++i) {
		u32 misr_value = 0;

		m = &sde_crtc->mixers[i];
		if (!m->hw_lm || !m->hw_lm->ops.collect_misr) {
			if (!m->hw_lm || !m->hw_lm->cap->dummy_mixer) {
				len += scnprintf(buf + len, MISR_BUFF_SIZE - len, "invalid\n");
				SDE_ERROR("crtc:%d invalid misr ops\n", DRMID(crtc));
			}
			continue;
		}

		rc = m->hw_lm->ops.collect_misr(m->hw_lm, false, &misr_value);
		if (rc) {
			len += scnprintf(buf + len, MISR_BUFF_SIZE - len, "invalid\n");
			SDE_ERROR("crtc:%d failed to collect misr %d\n", DRMID(crtc), rc);
			continue;
		} else {
			len += scnprintf(buf + len, MISR_BUFF_SIZE - len,
					"lm idx:%d\n", m->hw_lm->idx - LM_0);
			len += scnprintf(buf + len, MISR_BUFF_SIZE - len, "0x%x\n", misr_value);
		}
	}

buff_check:
	if (count <= len) {
		len = 0;
		goto end;
	}

	if (copy_to_user(user_buff, buf, len)) {
		len = -EFAULT;
		goto end;
	}

	*ppos += len;   /* increase offset */

end:
	sde_vm_unlock(sde_kms);
	pm_runtime_put_sync(crtc->dev->dev);
	return len;
}

#define DEFINE_SDE_DEBUGFS_SEQ_FOPS(__prefix)                          \
static int __prefix ## _open(struct inode *inode, struct file *file)	\
{									\
	return single_open(file, __prefix ## _show, inode->i_private);	\
}									\
static const struct file_operations __prefix ## _fops = {		\
	.owner = THIS_MODULE,						\
	.open = __prefix ## _open,					\
	.release = single_release,					\
	.read = seq_read,						\
	.llseek = seq_lseek,						\
}

static int sde_crtc_debugfs_state_show(struct seq_file *s, void *v)
{
	struct drm_crtc *crtc = (struct drm_crtc *) s->private;
	struct sde_crtc *sde_crtc = to_sde_crtc(crtc);
	struct sde_crtc_state *cstate = to_sde_crtc_state(crtc->state);
	int i;

	seq_printf(s, "num_connectors: %d\n", cstate->num_connectors);
	seq_printf(s, "client type: %d\n", sde_crtc_get_client_type(crtc));
	seq_printf(s, "intf_mode: %d\n", sde_crtc_get_intf_mode(crtc,
				crtc->state));
	seq_printf(s, "core_clk_rate: %llu\n",
			sde_crtc->cur_perf.core_clk_rate);
	for (i = SDE_POWER_HANDLE_DBUS_ID_MNOC;
			i < SDE_POWER_HANDLE_DBUS_ID_MAX; i++) {
		seq_printf(s, "bw_ctl[%s]: %llu\n",
				sde_power_handle_get_dbus_name(i),
				sde_crtc->cur_perf.bw_ctl[i]);
		seq_printf(s, "max_per_pipe_ib[%s]: %llu\n",
				sde_power_handle_get_dbus_name(i),
				sde_crtc->cur_perf.max_per_pipe_ib[i]);
	}

	return 0;
}
DEFINE_SDE_DEBUGFS_SEQ_FOPS(sde_crtc_debugfs_state);

static int _sde_debugfs_fence_status_show(struct seq_file *s, void *data)
{
	struct drm_crtc *crtc;
	struct drm_plane *plane;
	struct drm_connector *conn;
	struct drm_mode_object *drm_obj;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	struct sde_fence_context *ctx;
	struct drm_connector_list_iter conn_iter;
	struct drm_device *dev;

	if (!s || !s->private)
		return -EINVAL;

	sde_crtc = s->private;
	crtc = &sde_crtc->base;
	dev = crtc->dev;
	cstate = to_sde_crtc_state(crtc->state);

	if (!sde_crtc->kickoff_in_progress)
		goto skip_input_fence;

	/* Dump input fence info */
	seq_puts(s, "===Input fence===\n");
	drm_atomic_crtc_for_each_plane(plane, crtc) {
		struct sde_plane_state *pstate;
		struct dma_fence *fence;

		pstate = to_sde_plane_state(plane->state);
		if (!pstate)
			continue;

		seq_printf(s, "plane:%u stage:%d\n", plane->base.id,
			pstate->stage);

		SDE_EVT32(DRMID(crtc), plane->base.id, pstate->input_fence);
		if (pstate->input_fence) {

			rcu_read_lock();
			fence = dma_fence_get_rcu(pstate->input_fence);
			rcu_read_unlock();

			if (fence) {
				sde_fence_list_dump(fence, &s);
				dma_fence_put(fence);
			}
		}
	}

skip_input_fence:
	/* Dump release fence info */
	seq_puts(s, "\n");
	seq_puts(s, "===Release fence===\n");
	ctx = sde_crtc->output_fence;
	drm_obj = &crtc->base;
	sde_debugfs_timeline_dump(ctx, drm_obj, &s);
	seq_puts(s, "\n");

	/* Dump retire fence info */
	seq_puts(s, "===Retire fence===\n");
	drm_connector_list_iter_begin(dev, &conn_iter);
	drm_for_each_connector_iter(conn, &conn_iter)
		if (conn->state && conn->state->crtc == crtc &&
				cstate->num_connectors < MAX_CONNECTORS) {
			struct sde_connector *c_conn;

			c_conn = to_sde_connector(conn);
			ctx = c_conn->retire_fence;
			drm_obj = &conn->base;
			sde_debugfs_timeline_dump(ctx, drm_obj, &s);
		}
	drm_connector_list_iter_end(&conn_iter);
	seq_puts(s, "\n");

	return 0;
}

static int _sde_debugfs_fence_status(struct inode *inode, struct file *file)
{
	return single_open(file, _sde_debugfs_fence_status_show,
				inode->i_private);
}

static int _sde_crtc_init_debugfs(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;

	static const struct file_operations debugfs_status_fops = {
		.open =		_sde_debugfs_status_open,
		.read =		seq_read,
		.llseek =	seq_lseek,
		.release =	single_release,
	};
	static const struct file_operations debugfs_misr_fops = {
		.open =		simple_open,
		.read =		_sde_crtc_misr_read,
		.write =	_sde_crtc_misr_setup,
	};
	static const struct file_operations debugfs_fps_fops = {
		.open =		_sde_debugfs_fps_status,
		.read =		seq_read,
	};
	static const struct file_operations debugfs_fence_fops = {
		.open =		_sde_debugfs_fence_status,
		.read =		seq_read,
	};

	if (!crtc)
		return -EINVAL;
	sde_crtc = to_sde_crtc(crtc);

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms)
		return -EINVAL;

	sde_crtc->debugfs_root = debugfs_create_dir(sde_crtc->name,
			crtc->dev->primary->debugfs_root);
	if (!sde_crtc->debugfs_root)
		return -ENOMEM;

	/* don't error check these */
	debugfs_create_file("status", 0400,
			sde_crtc->debugfs_root,
			sde_crtc, &debugfs_status_fops);
	debugfs_create_file("state", 0400,
			sde_crtc->debugfs_root,
			&sde_crtc->base,
			&sde_crtc_debugfs_state_fops);
	debugfs_create_file("misr_data", 0600, sde_crtc->debugfs_root,
					sde_crtc, &debugfs_misr_fops);
	debugfs_create_file("fps", 0400, sde_crtc->debugfs_root,
					sde_crtc, &debugfs_fps_fops);
	debugfs_create_file("fence_status", 0400, sde_crtc->debugfs_root,
					sde_crtc, &debugfs_fence_fops);

	return 0;
}

static void _sde_crtc_destroy_debugfs(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;

	if (!crtc)
		return;
	sde_crtc = to_sde_crtc(crtc);
	debugfs_remove_recursive(sde_crtc->debugfs_root);
}
#else
static int _sde_crtc_init_debugfs(struct drm_crtc *crtc)
{
	return 0;
}

static void _sde_crtc_destroy_debugfs(struct drm_crtc *crtc)
{
}
#endif /* CONFIG_DEBUG_FS */

static void vblank_ctrl_worker(struct kthread_work *work)
{
	struct vblank_work *cur_work = container_of(work,
					struct vblank_work, work);
	struct msm_drm_private *priv = cur_work->priv;

	sde_crtc_vblank(priv->crtcs[cur_work->crtc_id], cur_work->enable);

	kfree(cur_work);
}

static int vblank_ctrl_queue_work(struct msm_drm_private *priv,
					int crtc_id, bool enable)
{
	struct vblank_work *cur_work;
	struct drm_crtc *crtc;
	struct kthread_worker *worker;

	if (!priv || crtc_id >= priv->num_crtcs)
		return -EINVAL;

	cur_work = kzalloc(sizeof(*cur_work), GFP_ATOMIC);
	if (!cur_work)
		return -ENOMEM;

	crtc = priv->crtcs[crtc_id];

	kthread_init_work(&cur_work->work, vblank_ctrl_worker);
	cur_work->crtc_id = crtc_id;
	cur_work->enable = enable;
	cur_work->priv = priv;
	worker = &priv->event_thread[crtc_id].worker;

	kthread_queue_work(worker, &cur_work->work);
	return 0;
}

static int sde_crtc_enable_vblank(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	unsigned int pipe = crtc->index;
	struct msm_drm_private *priv = dev->dev_private;
	struct msm_kms *kms = priv->kms;

	if (!kms)
		return -ENXIO;

	DBG("dev=%pK, crtc=%u", dev, pipe);
	return vblank_ctrl_queue_work(priv, pipe, true);
}

static void sde_crtc_disable_vblank(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	unsigned int pipe = crtc->index;
	struct msm_drm_private *priv = dev->dev_private;
	struct msm_kms *kms = priv->kms;

	if (!kms)
		return;
	DBG("dev=%pK, crtc=%u", dev, pipe);

	vblank_ctrl_queue_work(priv, pipe, false);
}

static int sde_crtc_late_register(struct drm_crtc *crtc)
{
	return _sde_crtc_init_debugfs(crtc);
}

static void sde_crtc_early_unregister(struct drm_crtc *crtc)
{
	_sde_crtc_destroy_debugfs(crtc);
}

static const struct drm_crtc_funcs sde_crtc_funcs = {
	.set_config = drm_atomic_helper_set_config,
	.destroy = sde_crtc_destroy,
	.enable_vblank = sde_crtc_enable_vblank,
	.disable_vblank = sde_crtc_disable_vblank,
	.page_flip = drm_atomic_helper_page_flip,
	.atomic_set_property = sde_crtc_atomic_set_property,
	.atomic_get_property = sde_crtc_atomic_get_property,
	.reset = sde_crtc_reset,
	.atomic_duplicate_state = sde_crtc_duplicate_state,
	.atomic_destroy_state = sde_crtc_destroy_state,
	.late_register = sde_crtc_late_register,
	.early_unregister = sde_crtc_early_unregister,
};

static const struct drm_crtc_funcs sde_crtc_funcs_v1 = {
	.set_config = drm_atomic_helper_set_config,
	.destroy = sde_crtc_destroy,
	.enable_vblank = sde_crtc_enable_vblank,
	.disable_vblank = sde_crtc_disable_vblank,
	.page_flip = drm_atomic_helper_page_flip,
	.atomic_set_property = sde_crtc_atomic_set_property,
	.atomic_get_property = sde_crtc_atomic_get_property,
	.reset = sde_crtc_reset,
	.atomic_duplicate_state = sde_crtc_duplicate_state,
	.atomic_destroy_state = sde_crtc_destroy_state,
	.late_register = sde_crtc_late_register,
	.early_unregister = sde_crtc_early_unregister,
	.get_vblank_timestamp = sde_crtc_get_vblank_timestamp,
	.get_vblank_counter = sde_crtc_get_vblank_counter,
};

static const struct drm_crtc_helper_funcs sde_crtc_helper_funcs = {
	.mode_fixup = sde_crtc_mode_fixup,
	.disable = sde_crtc_disable,
	.atomic_enable = sde_crtc_enable,
	.atomic_check = sde_crtc_atomic_check,
	.atomic_begin = sde_crtc_atomic_begin,
	.atomic_flush = sde_crtc_atomic_flush,
};

static void _sde_crtc_event_cb(struct kthread_work *work)
{
	struct sde_crtc_event *event;
	struct sde_crtc *sde_crtc;
	unsigned long irq_flags;

	if (!work) {
		SDE_ERROR("invalid work item\n");
		return;
	}

	event = container_of(work, struct sde_crtc_event, kt_work);

	/* set sde_crtc to NULL for static work structures */
	sde_crtc = event->sde_crtc;
	if (!sde_crtc)
		return;

	if (event->cb_func)
		event->cb_func(&sde_crtc->base, event->usr);

	spin_lock_irqsave(&sde_crtc->event_lock, irq_flags);
	list_add_tail(&event->list, &sde_crtc->event_free_list);
	spin_unlock_irqrestore(&sde_crtc->event_lock, irq_flags);
}

int sde_crtc_event_queue(struct drm_crtc *crtc,
		void (*func)(struct drm_crtc *crtc, void *usr),
		void *usr, bool color_processing_event)
{
	unsigned long irq_flags;
	struct sde_crtc *sde_crtc;
	struct msm_drm_private *priv;
	struct sde_crtc_event *event = NULL;
	u32 crtc_id;

	if (!crtc || !crtc->dev || !crtc->dev->dev_private || !func) {
		SDE_ERROR("invalid parameters\n");
		return -EINVAL;
	}
	sde_crtc = to_sde_crtc(crtc);
	priv = crtc->dev->dev_private;
	crtc_id = drm_crtc_index(crtc);

	/*
	 * Obtain an event struct from the private cache. This event
	 * queue may be called from ISR contexts, so use a private
	 * cache to avoid calling any memory allocation functions.
	 */
	spin_lock_irqsave(&sde_crtc->event_lock, irq_flags);
	if (!list_empty(&sde_crtc->event_free_list)) {
		event = list_first_entry(&sde_crtc->event_free_list,
				struct sde_crtc_event, list);
		list_del_init(&event->list);
	}
	spin_unlock_irqrestore(&sde_crtc->event_lock, irq_flags);

	if (!event)
		return -ENOMEM;

	/* populate event node */
	event->sde_crtc = sde_crtc;
	event->cb_func = func;
	event->usr = usr;

	/* queue new event request */
	kthread_init_work(&event->kt_work, _sde_crtc_event_cb);
	if (color_processing_event)
		kthread_queue_work(&priv->pp_event_worker,
			&event->kt_work);
	else
		kthread_queue_work(&priv->event_thread[crtc_id].worker,
			&event->kt_work);

	return 0;
}

static int _sde_crtc_init_events(struct sde_crtc *sde_crtc)
{
	int i, rc = 0;

	if (!sde_crtc) {
		SDE_ERROR("invalid crtc\n");
		return -EINVAL;
	}

	spin_lock_init(&sde_crtc->event_lock);

	INIT_LIST_HEAD(&sde_crtc->event_free_list);
	for (i = 0; i < SDE_CRTC_MAX_EVENT_COUNT; ++i)
		list_add_tail(&sde_crtc->event_cache[i].list,
				&sde_crtc->event_free_list);

	return rc;
}

void sde_crtc_static_img_control(struct drm_crtc *crtc,
		enum sde_crtc_cache_state state,
		bool is_vidmode)
{
	struct drm_plane *plane;
	struct sde_crtc *sde_crtc;
	struct sde_kms *sde_kms;

	if (!crtc || !crtc->dev)
		return;

	sde_kms = _sde_crtc_get_kms(crtc);
	if (!sde_kms || !sde_kms->catalog) {
		SDE_ERROR("invalid params\n");
		return;
	}

	if (!sde_kms->catalog->syscache_supported) {
		SDE_DEBUG("syscache not supported\n");
		return;
	}

	sde_crtc = to_sde_crtc(crtc);
	if (sde_crtc->cache_state == state)
		return;

	if (test_bit(SDE_MDP_LLCC_DISP_LR, &sde_kms->catalog->mdp[0].features)) {
		SDE_DEBUG("Cache state is directly programmed to frame read\n");
		state = CACHE_STATE_FRAME_READ;
		goto end;
	}

	switch (state) {
	case CACHE_STATE_NORMAL:
		if (sde_crtc->cache_state == CACHE_STATE_DISABLED
				&& !is_vidmode)
			return;

		kthread_cancel_delayed_work_sync(
				&sde_crtc->static_cache_read_work);
		break;
	case CACHE_STATE_FRAME_WRITE:
		if (sde_crtc->cache_state != CACHE_STATE_NORMAL)
			return;
		break;
	case CACHE_STATE_FRAME_READ:
		if (sde_crtc->cache_state != CACHE_STATE_FRAME_WRITE)
			return;
		break;
	case CACHE_STATE_DISABLED:
		break;
	default:
		return;
	}

end:
	sde_crtc->cache_state = state;
	drm_atomic_crtc_for_each_plane(plane, crtc)
		sde_plane_static_img_control(plane, state);
}

/*
 * __sde_crtc_static_cache_read_work - transition to cache read
 */
void __sde_crtc_static_cache_read_work(struct kthread_work *work)
{
	struct sde_crtc *sde_crtc = container_of(work, struct sde_crtc,
			static_cache_read_work.work);
	struct drm_crtc *crtc = &sde_crtc->base;
	struct sde_hw_ctl *ctl = sde_crtc->mixers[0].hw_ctl;
	struct drm_encoder *enc, *drm_enc = NULL;
	struct drm_plane *plane;

	if (sde_crtc->cache_state != CACHE_STATE_FRAME_WRITE)
		return;

	drm_for_each_encoder_mask(enc, crtc->dev, crtc->state->encoder_mask) {
		drm_enc = enc;
		if (sde_encoder_in_clone_mode(drm_enc))
			return;
	}

	if (!drm_enc || !ctl || !sde_crtc->num_mixers) {
		SDE_ERROR("invalid object, drm_enc:%d, ctl:%d\n", !drm_enc,
				!ctl);
		return;
	}

	SDE_EVT32(DRMID(crtc), SDE_EVTLOG_FUNC_ENTRY);

	sde_crtc_static_img_control(crtc, CACHE_STATE_FRAME_READ, false);

	/* flush only the sys-cache enabled SSPPs */
	if (ctl->ops.clear_pending_flush)
		ctl->ops.clear_pending_flush(ctl);

	drm_atomic_crtc_for_each_plane(plane, crtc)
		sde_plane_ctl_flush(plane, ctl, true);

	/* kickoff encoder and wait for VBLANK */
	sde_encoder_kickoff(drm_enc, false);
	sde_encoder_wait_for_event(drm_enc, MSM_ENC_VBLANK);

	SDE_EVT32(DRMID(crtc), SDE_EVTLOG_FUNC_EXIT);
}

void sde_crtc_static_cache_read_kickoff(struct drm_crtc *crtc)
{
	struct drm_device *dev;
	struct msm_drm_private *priv;
	struct msm_drm_thread *disp_thread;
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	u32 msecs_fps = 0;

	if (!crtc)
		return;

	dev = crtc->dev;
	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);

	if (!dev || !dev->dev_private || !sde_crtc)
		return;

	priv = dev->dev_private;
	disp_thread = &priv->disp_thread[crtc->index];

	if (sde_crtc->cache_state != CACHE_STATE_FRAME_WRITE)
		return;

	msecs_fps = DIV_ROUND_UP((1 * 1000), sde_crtc_get_fps_mode(crtc));

	/* Kickoff transition to read state after next vblank */
	kthread_queue_delayed_work(&disp_thread->worker,
			&sde_crtc->static_cache_read_work,
			msecs_to_jiffies(msecs_fps));
}

void sde_crtc_cancel_delayed_work(struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	struct sde_crtc_state *cstate;
	bool cache_status;

	if (!crtc || !crtc->state)
		return;

	sde_crtc = to_sde_crtc(crtc);
	cstate = to_sde_crtc_state(crtc->state);

	cache_status = kthread_cancel_delayed_work_sync(&sde_crtc->static_cache_read_work);
	SDE_EVT32(DRMID(crtc), cache_status);
}

/* initialize crtc */
struct drm_crtc *sde_crtc_init(struct drm_device *dev, struct drm_plane *plane)
{
	struct drm_crtc *crtc = NULL;
	struct sde_crtc *sde_crtc = NULL;
	struct msm_drm_private *priv = NULL;
	struct sde_kms *kms = NULL;
	const struct drm_crtc_funcs *crtc_funcs;
	int i, rc;

	priv = dev->dev_private;
	kms = to_sde_kms(priv->kms);

	sde_crtc = kzalloc(sizeof(*sde_crtc), GFP_KERNEL);
	if (!sde_crtc)
		return ERR_PTR(-ENOMEM);

	crtc = &sde_crtc->base;
	crtc->dev = dev;

	mutex_init(&sde_crtc->crtc_lock);
	spin_lock_init(&sde_crtc->spin_lock);
	spin_lock_init(&sde_crtc->fevent_spin_lock);
	atomic_set(&sde_crtc->frame_pending, 0);

	sde_crtc->enabled = false;
	sde_crtc->kickoff_in_progress = false;

	/* Below parameters are for fps calculation for sysfs node */
	sde_crtc->fps_info.fps_periodic_duration = DEFAULT_FPS_PERIOD_1_SEC;
	sde_crtc->fps_info.time_buf = kmalloc_array(MAX_FRAME_COUNT,
			sizeof(ktime_t), GFP_KERNEL);

	if (!sde_crtc->fps_info.time_buf)
		SDE_ERROR("invalid buffer\n");
	else
		memset(sde_crtc->fps_info.time_buf, 0,
			sizeof(*(sde_crtc->fps_info.time_buf)));

	INIT_LIST_HEAD(&sde_crtc->frame_event_list);
	INIT_LIST_HEAD(&sde_crtc->user_event_list);
	for (i = 0; i < ARRAY_SIZE(sde_crtc->frame_events); i++) {
		INIT_LIST_HEAD(&sde_crtc->frame_events[i].list);
		list_add(&sde_crtc->frame_events[i].list,
				&sde_crtc->frame_event_list);
		kthread_init_work(&sde_crtc->frame_events[i].work,
				sde_crtc_frame_event_work);
	}

	crtc_funcs = kms->catalog->has_precise_vsync_ts ? &sde_crtc_funcs_v1 : &sde_crtc_funcs;
	drm_crtc_init_with_planes(dev, crtc, plane, NULL, crtc_funcs, NULL);
	drm_crtc_helper_add(crtc, &sde_crtc_helper_funcs);

	/* save user friendly CRTC name for later */
	snprintf(sde_crtc->name, SDE_CRTC_NAME_SIZE, "crtc%u", crtc->base.id);

	/* initialize event handling */
	rc = _sde_crtc_init_events(sde_crtc);
	if (rc) {
		drm_crtc_cleanup(crtc);
		kfree(sde_crtc);
		return ERR_PTR(rc);
	}

	/* initialize output fence support */
	sde_crtc->output_fence = sde_fence_init(sde_crtc->name, crtc->base.id);

	if (IS_ERR(sde_crtc->output_fence)) {
		rc = PTR_ERR(sde_crtc->output_fence);
		SDE_ERROR("failed to init fence, %d\n", rc);
		drm_crtc_cleanup(crtc);
		kfree(sde_crtc);
		return ERR_PTR(rc);
	}

	/* create CRTC properties */
	msm_property_init(&sde_crtc->property_info, &crtc->base, dev,
			priv->crtc_property, sde_crtc->property_data,
			CRTC_PROP_COUNT, CRTC_PROP_BLOBCOUNT,
			sizeof(struct sde_crtc_state));

	sde_crtc_install_properties(crtc, kms->catalog);

	/* Install color processing properties */
	sde_cp_crtc_init(crtc);
	sde_cp_crtc_install_properties(crtc);

	for (i = 0; i < SDE_SYS_CACHE_MAX; i++) {
		sde_crtc->cur_perf.llcc_active[i] = false;
		sde_crtc->new_perf.llcc_active[i] = false;
	}

	kthread_init_delayed_work(&sde_crtc->static_cache_read_work,
			__sde_crtc_static_cache_read_work);

	SDE_DEBUG("%s: successfully initialized crtc\n", sde_crtc->name);
	return crtc;
}

int sde_crtc_post_init(struct drm_device *dev, struct drm_crtc *crtc)
{
	struct sde_crtc *sde_crtc;
	int rc = 0;

	if (!dev || !dev->primary || !dev->primary->kdev || !crtc) {
		SDE_ERROR("invalid input param(s)\n");
		rc = -EINVAL;
		goto end;
	}

	sde_crtc = to_sde_crtc(crtc);
	sde_crtc->sysfs_dev = device_create_with_groups(
		dev->primary->kdev->class, dev->primary->kdev, 0, crtc,
		sde_crtc_attr_groups, "sde-crtc-%d", crtc->index);
	if (IS_ERR_OR_NULL(sde_crtc->sysfs_dev)) {
		SDE_ERROR("crtc:%d sysfs create failed rc:%ld\n", crtc->index,
			PTR_ERR(sde_crtc->sysfs_dev));
		if (!sde_crtc->sysfs_dev)
			rc = -EINVAL;
		else
			rc = PTR_ERR(sde_crtc->sysfs_dev);
		goto end;
	}

	sde_crtc->vsync_event_sf = sysfs_get_dirent(
		sde_crtc->sysfs_dev->kobj.sd, "vsync_event");
	if (!sde_crtc->vsync_event_sf)
		SDE_ERROR("crtc:%d vsync_event sysfs create failed\n",
						crtc->base.id);

	sde_crtc->retire_frame_event_sf = sysfs_get_dirent(
		sde_crtc->sysfs_dev->kobj.sd, "retire_frame_event");
	if (!sde_crtc->retire_frame_event_sf)
		SDE_ERROR("crtc:%d retire frame event sysfs create failed\n",
						crtc->base.id);
end:
	return rc;
}

static int _sde_crtc_event_enable(struct sde_kms *kms,
		struct drm_crtc *crtc_drm, u32 event)
{
	struct sde_crtc *crtc = NULL;
	struct sde_crtc_irq_info *node;
	unsigned long flags;
	bool found = false;
	int ret, i = 0;
	bool add_event = false;

	crtc = to_sde_crtc(crtc_drm);
	spin_lock_irqsave(&crtc->spin_lock, flags);
	list_for_each_entry(node, &crtc->user_event_list, list) {
		if (node->event == event) {
			found = true;
			break;
		}
	}
	spin_unlock_irqrestore(&crtc->spin_lock, flags);

	/* event already enabled */
	if (found)
		return 0;

	node = NULL;
	for (i = 0; i < ARRAY_SIZE(custom_events); i++) {
		if (custom_events[i].event == event &&
			custom_events[i].func) {
			node = kzalloc(sizeof(*node), GFP_KERNEL);
			if (!node)
				return -ENOMEM;
			INIT_LIST_HEAD(&node->list);
			INIT_LIST_HEAD(&node->irq.list);
			node->func = custom_events[i].func;
			node->event = event;
			node->state = IRQ_NOINIT;
			spin_lock_init(&node->state_lock);
			break;
		}
	}

	if (!node) {
		SDE_ERROR("unsupported event %x\n", event);
		return -EINVAL;
	}

	ret = 0;
	if (crtc_drm->enabled) {
		ret = pm_runtime_get_sync(crtc_drm->dev->dev);
		if (ret < 0) {
			SDE_EVT32(ret, SDE_EVTLOG_ERROR);
			kfree(node);
			return ret;
		}

		INIT_LIST_HEAD(&node->irq.list);

		mutex_lock(&crtc->crtc_lock);
		ret = node->func(crtc_drm, true, &node->irq);
		if (!ret) {
			spin_lock_irqsave(&crtc->spin_lock, flags);
			list_add_tail(&node->list, &crtc->user_event_list);
			add_event = true;
			spin_unlock_irqrestore(&crtc->spin_lock, flags);
		}
		mutex_unlock(&crtc->crtc_lock);

		pm_runtime_put_sync(crtc_drm->dev->dev);
	}

	if (add_event)
		return 0;

	if (!ret) {
		spin_lock_irqsave(&crtc->spin_lock, flags);
		list_add_tail(&node->list, &crtc->user_event_list);
		spin_unlock_irqrestore(&crtc->spin_lock, flags);
	} else {
		kfree(node);
	}

	return ret;
}

static int _sde_crtc_event_disable(struct sde_kms *kms,
		struct drm_crtc *crtc_drm, u32 event)
{
	struct sde_crtc *crtc = NULL;
	struct sde_crtc_irq_info *node = NULL;
	unsigned long flags;
	bool found = false;
	int ret;

	crtc = to_sde_crtc(crtc_drm);
	spin_lock_irqsave(&crtc->spin_lock, flags);
	list_for_each_entry(node, &crtc->user_event_list, list) {
		if (node->event == event) {
			list_del_init(&node->list);
			found = true;
			break;
		}
	}
	spin_unlock_irqrestore(&crtc->spin_lock, flags);

	/* event already disabled */
	if (!found)
		return 0;

	/**
	 * crtc is disabled interrupts are cleared remove from the list,
	 * no need to disable/de-register.
	 */
	if (!crtc_drm->enabled) {
		kfree(node);
		return 0;
	}
	ret = pm_runtime_get_sync(crtc_drm->dev->dev);
	if (ret < 0) {
		SDE_ERROR("failed to enable power resource %d\n", ret);
		SDE_EVT32(ret, SDE_EVTLOG_ERROR);
		kfree(node);
		return ret;
	}

	mutex_lock(&crtc->crtc_lock);
	ret = node->func(crtc_drm, false, &node->irq);
	if (ret) {
		spin_lock_irqsave(&crtc->spin_lock, flags);
		list_add_tail(&node->list, &crtc->user_event_list);
		spin_unlock_irqrestore(&crtc->spin_lock, flags);
	} else {
		kfree(node);
	}
	mutex_unlock(&crtc->crtc_lock);

	pm_runtime_put_sync(crtc_drm->dev->dev);
	return ret;
}

int sde_crtc_register_custom_event(struct sde_kms *kms,
		struct drm_crtc *crtc_drm, u32 event, bool en)
{
	struct sde_crtc *crtc = NULL;
	int ret;

	crtc = to_sde_crtc(crtc_drm);
	if (!crtc || !kms || !kms->dev) {
		DRM_ERROR("invalid sde_crtc %pK kms %pK dev %pK\n", crtc,
			kms, ((kms) ? (kms->dev) : NULL));
		return -EINVAL;
	}

	if (en)
		ret = _sde_crtc_event_enable(kms, crtc_drm, event);
	else
		ret = _sde_crtc_event_disable(kms, crtc_drm, event);

	return ret;
}

static int sde_crtc_power_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *irq)
{
	return 0;
}

static int sde_crtc_pm_event_handler(struct drm_crtc *crtc, bool en,
		struct sde_irq_callback *noirq)
{
	/*
	 * IRQ object noirq is not being used here since there is
	 * no crtc irq from pm event.
	 */
	return 0;
}

static int sde_crtc_idle_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *irq)
{
	return 0;
}

static int sde_crtc_mmrm_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *irq)
{
	return 0;
}

static int sde_crtc_vm_release_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *irq)
{
	return 0;
}

static int sde_crtc_frame_data_interrupt_handler(struct drm_crtc *crtc_drm,
	bool en, struct sde_irq_callback *irq)
{
	return 0;
}

/**
 * sde_crtc_update_cont_splash_settings - update mixer settings
 *	and initial clk during device bootup for cont_splash use case
 * @crtc: Pointer to drm crtc structure
 */
void sde_crtc_update_cont_splash_settings(struct drm_crtc *crtc)
{
	struct sde_kms *kms = NULL;
	struct msm_drm_private *priv;
	struct sde_crtc *sde_crtc;
	u64 rate;

	if (!crtc || !crtc->state || !crtc->dev || !crtc->dev->dev_private) {
		SDE_ERROR("invalid crtc\n");
		return;
	}

	priv = crtc->dev->dev_private;
	kms = to_sde_kms(priv->kms);
	if (!kms || !kms->catalog) {
		SDE_ERROR("invalid parameters\n");
		return;
	}

	_sde_crtc_setup_mixers(crtc);
	sde_cp_crtc_refresh_status_properties(crtc);
	crtc->enabled = true;

	/* update core clk value for initial state with cont-splash */
	sde_crtc = to_sde_crtc(crtc);
	rate = sde_power_clk_get_rate(&priv->phandle, kms->perf.clk_name);
	sde_crtc->cur_perf.core_clk_rate = (rate > 0) ?
					rate : kms->perf.max_core_clk_rate;
	sde_crtc->cur_perf.core_clk_rate = kms->perf.max_core_clk_rate;
}

static void sde_crtc_install_noise_layer_properties(struct sde_crtc *sde_crtc,
		struct sde_mdss_cfg *catalog, struct sde_kms_info *info)
{
	struct sde_lm_cfg *lm;
	char feature_name[256];
	u32 version;

	if (!catalog->mixer_count)
		return;

	lm = &catalog->mixer[0];
	if (!(lm->features & BIT(SDE_MIXER_NOISE_LAYER)))
		return;

	version = lm->sblk->nlayer.version >> 16;
	snprintf(feature_name, ARRAY_SIZE(feature_name), "%s%d", "noise_layer_v", version);

	switch (version) {
	case 1:
		sde_kms_info_add_keyint(info, "has_noise_layer", 1);
		msm_property_install_volatile_range(&sde_crtc->property_info,
			feature_name, 0x0, 0, ~0, 0, CRTC_PROP_NOISE_LAYER_V1);
		break;
	default:
		SDE_ERROR("unsupported noise layer version %d\n", version);
		break;
	}
}

static int _sde_crtc_set_noise_layer(struct sde_crtc *sde_crtc,
				struct sde_crtc_state *cstate,
				void __user *usr_ptr)
{
	int ret;

	if (!sde_crtc || !cstate) {
		SDE_ERROR("invalid sde_crtc/state\n");
		return -EINVAL;
	}

	SDE_DEBUG("crtc %s\n", sde_crtc->name);

	if (!usr_ptr) {
		SDE_DEBUG("noise layer removed\n");
		cstate->noise_layer_en = false;
		set_bit(SDE_CRTC_NOISE_LAYER, cstate->dirty);
		return 0;
	}
	ret = copy_from_user(&cstate->layer_cfg, usr_ptr,
		sizeof(cstate->layer_cfg));
	if (ret) {
		SDE_ERROR("failed to copy noise layer %d\n", ret);
		return -EFAULT;
	}
	if (cstate->layer_cfg.zposn != cstate->layer_cfg.zposattn - 1 ||
		cstate->layer_cfg.zposattn >= SDE_STAGE_MAX ||
		!cstate->layer_cfg.attn_factor ||
		cstate->layer_cfg.attn_factor > DRM_NOISE_ATTN_MAX ||
		cstate->layer_cfg.strength > DRM_NOISE_STREN_MAX ||
		!cstate->layer_cfg.alpha_noise ||
		cstate->layer_cfg.alpha_noise > DRM_NOISE_ATTN_MAX) {
		SDE_ERROR("invalid param zposn %d zposattn %d attn_factor %d \
			   strength %d alpha noise %d\n", cstate->layer_cfg.zposn,
			   cstate->layer_cfg.zposattn, cstate->layer_cfg.attn_factor,
			   cstate->layer_cfg.strength, cstate->layer_cfg.alpha_noise);
		return -EINVAL;
	}
	cstate->noise_layer_en = true;
	set_bit(SDE_CRTC_NOISE_LAYER, cstate->dirty);
	return 0;
}

static void sde_cp_crtc_apply_noise(struct drm_crtc *crtc,
		struct drm_crtc_state *state)
{
	struct sde_crtc *scrtc = to_sde_crtc(crtc);
	struct sde_crtc_state *cstate = to_sde_crtc_state(crtc->state);
	struct sde_hw_mixer *lm;
	int i;
	struct sde_hw_noise_layer_cfg cfg;
	struct sde_kms *kms;

	if (!test_bit(SDE_CRTC_NOISE_LAYER, cstate->dirty))
		return;

	kms = _sde_crtc_get_kms(crtc);
	if (!kms || !kms->catalog) {
		SDE_ERROR("Invalid kms\n");
		return;
	}

	cfg.flags = cstate->layer_cfg.flags;
	cfg.alpha_noise = cstate->layer_cfg.alpha_noise;
	cfg.attn_factor = cstate->layer_cfg.attn_factor;
	cfg.strength = cstate->layer_cfg.strength;
	if (!kms->catalog->has_base_layer) {
		cfg.noise_blend_stage = cstate->layer_cfg.zposn + SDE_STAGE_0;
		cfg.attn_blend_stage = cstate->layer_cfg.zposattn + SDE_STAGE_0;
	} else {
		cfg.noise_blend_stage = cstate->layer_cfg.zposn;
		cfg.attn_blend_stage = cstate->layer_cfg.zposattn;
	}

	for (i = 0; i < scrtc->num_mixers; i++) {
		lm = scrtc->mixers[i].hw_lm;
		if (!lm->ops.setup_noise_layer)
			break;
		if (!cstate->noise_layer_en)
			lm->ops.setup_noise_layer(lm, NULL);
		else
			lm->ops.setup_noise_layer(lm, &cfg);
	}
	if (!cstate->noise_layer_en)
		clear_bit(SDE_CRTC_NOISE_LAYER, cstate->dirty);
}

void sde_crtc_disable_cp_features(struct drm_crtc *crtc)
{
	sde_cp_disable_features(crtc);
}

void _sde_crtc_vm_release_notify(struct drm_crtc *crtc)
{
	uint32_t val = 1;

	sde_crtc_event_notify(crtc, DRM_EVENT_VM_RELEASE, &val, sizeof(uint32_t));
}
