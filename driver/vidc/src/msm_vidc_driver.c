// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/iommu.h>
#include <linux/workqueue.h>
#include <linux/hash.h>
#include <media/v4l2_vidc_extensions.h>
#include "msm_media_info.h"

#include "msm_vidc_driver.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_memory.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_power.h"
#include "msm_vidc.h"
#include "msm_vdec.h"
#include "msm_venc.h"
#include "venus_hfi.h"
#include "venus_hfi_response.h"

#define COUNT_BITS(a, out) {       \
	while ((a) >= 1) {          \
		(out) += (a) & (1); \
		(a) >>= (1);        \
	}                           \
}

void print_vidc_buffer(u32 tag, const char *str, struct msm_vidc_inst *inst,
		struct msm_vidc_buffer *vbuf)
{
	if (!(tag & msm_vidc_debug) || !inst || !vbuf)
		return;

	if (vbuf->type == MSM_VIDC_BUF_INPUT || vbuf->type == MSM_VIDC_BUF_OUTPUT) {
		dprintk(tag, inst->sid,
			"%s: %s: idx %2d fd %3d off %d daddr %#llx size %d filled %d flags %#x ts %lld attr %#x\n",
			str, vbuf->type == MSM_VIDC_BUF_INPUT ? "INPUT" : "OUTPUT",
			vbuf->index, vbuf->fd, vbuf->data_offset,
			vbuf->device_addr, vbuf->buffer_size, vbuf->data_size,
			vbuf->flags, vbuf->timestamp, vbuf->attr);
	} else if (vbuf->type == MSM_VIDC_BUF_INPUT_META ||
			   vbuf->type == MSM_VIDC_BUF_OUTPUT_META) {
		dprintk(tag, inst->sid,
			"%s: %s: idx %2d fd %3d off %d daddr %#llx size %d filled %d flags %#x ts %lld attr %#x\n",
			str, vbuf->type == MSM_VIDC_BUF_INPUT_META ? "INPUT_META" : "OUTPUT_META",
			vbuf->index, vbuf->fd, vbuf->data_offset,
			vbuf->device_addr, vbuf->buffer_size, vbuf->data_size,
			vbuf->flags, vbuf->timestamp, vbuf->attr);
	}
}

void print_vb2_buffer(const char *str, struct msm_vidc_inst *inst,
		struct vb2_buffer *vb2)
{
	if (!inst || !vb2)
		return;

	i_vpr_e(inst,
		"%s: %s: idx %2d fd %d off %d size %d filled %d\n",
		str, vb2->type == INPUT_MPLANE ? "INPUT" : "OUTPUT",
		vb2->index, vb2->planes[0].m.fd,
		vb2->planes[0].data_offset, vb2->planes[0].length,
		vb2->planes[0].bytesused);
}

enum msm_vidc_buffer_type v4l2_type_to_driver(u32 type, const char *func)
{
	enum msm_vidc_buffer_type buffer_type = 0;

	switch (type) {
	case INPUT_MPLANE:
		buffer_type = MSM_VIDC_BUF_INPUT;
		break;
	case OUTPUT_MPLANE:
		buffer_type = MSM_VIDC_BUF_OUTPUT;
		break;
	case INPUT_META_PLANE:
		buffer_type = MSM_VIDC_BUF_INPUT_META;
		break;
	case OUTPUT_META_PLANE:
		buffer_type = MSM_VIDC_BUF_OUTPUT_META;
		break;
	default:
		d_vpr_e("%s: invalid v4l2 buffer type %#x\n", func, type);
		break;
	}
	return buffer_type;
}

u32 v4l2_type_from_driver(enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	u32 type = 0;

	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		type = INPUT_MPLANE;
		break;
	case MSM_VIDC_BUF_OUTPUT:
		type = OUTPUT_MPLANE;
		break;
	case MSM_VIDC_BUF_INPUT_META:
		type = INPUT_META_PLANE;
		break;
	case MSM_VIDC_BUF_OUTPUT_META:
		type = OUTPUT_META_PLANE;
		break;
	default:
		d_vpr_e("%s: invalid driver buffer type %d\n",
			func, buffer_type);
		break;
	}
	return type;
}

enum msm_vidc_codec_type v4l2_codec_to_driver(u32 v4l2_codec, const char *func)
{
	enum msm_vidc_codec_type codec = 0;

	switch (v4l2_codec) {
	case V4L2_PIX_FMT_H264:
		codec = MSM_VIDC_H264;
		break;
	case V4L2_PIX_FMT_HEVC:
		codec = MSM_VIDC_HEVC;
		break;
	case V4L2_PIX_FMT_VP9:
		codec = MSM_VIDC_VP9;
		break;
	default:
		d_vpr_e("%s: invalid v4l2 codec %#x\n", func, v4l2_codec);
		break;
	}
	return codec;
}

u32 v4l2_codec_from_driver(enum msm_vidc_codec_type codec, const char *func)
{
	u32 v4l2_codec = 0;

	switch (codec) {
	case MSM_VIDC_H264:
		v4l2_codec = V4L2_PIX_FMT_H264;
		break;
	case MSM_VIDC_HEVC:
		v4l2_codec = V4L2_PIX_FMT_HEVC;
		break;
	case MSM_VIDC_VP9:
		v4l2_codec = V4L2_PIX_FMT_VP9;
		break;
	default:
		d_vpr_e("%s: invalid driver codec %#x\n", func, codec);
		break;
	}
	return v4l2_codec;
}

enum msm_vidc_colorformat_type v4l2_colorformat_to_driver(u32 v4l2_colorformat,
	const char *func)
{
	enum msm_vidc_colorformat_type colorformat = 0;

	switch (v4l2_colorformat) {
	case V4L2_PIX_FMT_NV12:
		colorformat = MSM_VIDC_FMT_NV12;
		break;
	case V4L2_PIX_FMT_NV21:
		colorformat = MSM_VIDC_FMT_NV21;
		break;
	case V4L2_PIX_FMT_VIDC_NV12C:
		colorformat = MSM_VIDC_FMT_NV12C;
		break;
	case V4L2_PIX_FMT_VIDC_TP10C:
		colorformat = MSM_VIDC_FMT_TP10C;
		break;
	case V4L2_PIX_FMT_VIDC_ARGB32C:
		colorformat = MSM_VIDC_FMT_RGBA8888C;
		break;
	case V4L2_PIX_FMT_VIDC_P010:
		colorformat = MSM_VIDC_FMT_P010;
		break;
	default:
		d_vpr_e("%s: invalid v4l2 color format %#x\n",
			func, v4l2_colorformat);
		break;
	}
	return colorformat;
}

u32 v4l2_colorformat_from_driver(enum msm_vidc_colorformat_type colorformat,
	const char *func)
{
	u32 v4l2_colorformat = 0;

	switch (colorformat) {
	case MSM_VIDC_FMT_NV12:
		v4l2_colorformat = V4L2_PIX_FMT_NV12;
		break;
	case MSM_VIDC_FMT_NV21:
		v4l2_colorformat = V4L2_PIX_FMT_NV21;
		break;
	case MSM_VIDC_FMT_NV12C:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_NV12C;
		break;
	case MSM_VIDC_FMT_TP10C:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_TP10C;
		break;
	case MSM_VIDC_FMT_RGBA8888C:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_ARGB32C;
		break;
	case MSM_VIDC_FMT_P010:
		v4l2_colorformat = V4L2_PIX_FMT_VIDC_P010;
		break;
	default:
		d_vpr_e("%s: invalid driver color format %#x\n",
			func, colorformat);
		break;
	}
	return v4l2_colorformat;
}

u32 v4l2_colorformat_to_media(u32 v4l2_fmt, const char *func)
{
	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV12:
		return COLOR_FMT_NV12;
	case V4L2_PIX_FMT_NV21:
		return COLOR_FMT_NV21;
	case V4L2_PIX_FMT_VIDC_P010:
		return COLOR_FMT_P010;
	case V4L2_PIX_FMT_VIDC_NV12C:
		return COLOR_FMT_NV12_UBWC;
	case V4L2_PIX_FMT_VIDC_TP10C:
		return COLOR_FMT_NV12_BPP10_UBWC;
	case V4L2_PIX_FMT_VIDC_ARGB32C:
		return COLOR_FMT_RGBA8888_UBWC;
	default:
		d_vpr_e("%s: invalid v4l2 color fmt: %#x, set default (NV12)",
			func, v4l2_fmt);
		return COLOR_FMT_NV12;
	}
}

int v4l2_type_to_driver_port(struct msm_vidc_inst *inst, u32 type,
	const char *func)
{
	int port;

	if (type == INPUT_MPLANE) {
		port = INPUT_PORT;
	} else if (type == INPUT_META_PLANE) {
		port = INPUT_META_PORT;
	} else if (type == OUTPUT_MPLANE) {
		port = OUTPUT_PORT;
	} else if (type == OUTPUT_META_PLANE) {
		port = OUTPUT_META_PORT;
	} else {
		i_vpr_e(inst, "%s: port not found for v4l2 type %d\n",
			func, type);
		port = -EINVAL;
	}

	return port;
}

u32 msm_vidc_get_buffer_region(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, const char *func)
{
	u32 region = MSM_VIDC_NON_SECURE;

	if (!is_secure_session(inst) &&
		buffer_type != MSM_VIDC_BUF_ARP) {
		return region;
	}

	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		if (is_encode_session(inst))
			region = MSM_VIDC_SECURE_PIXEL;
		else
			region = MSM_VIDC_SECURE_BITSTREAM;
		break;
	case MSM_VIDC_BUF_OUTPUT:
		if (is_encode_session(inst))
			region = MSM_VIDC_SECURE_BITSTREAM;
		else
			region = MSM_VIDC_SECURE_PIXEL;
		break;
	case MSM_VIDC_BUF_INPUT_META:
	case MSM_VIDC_BUF_OUTPUT_META:
		region = MSM_VIDC_NON_SECURE;
		break;
	case MSM_VIDC_BUF_BIN:
		region = MSM_VIDC_SECURE_BITSTREAM;
		break;
	case MSM_VIDC_BUF_COMV:
	case MSM_VIDC_BUF_NON_COMV:
	case MSM_VIDC_BUF_LINE:
		region = MSM_VIDC_SECURE_NONPIXEL;
		break;
	case MSM_VIDC_BUF_DPB:
		region = MSM_VIDC_SECURE_PIXEL;
		break;
	case MSM_VIDC_BUF_PERSIST:
	// TODO: Need to revisit for ARP
	case MSM_VIDC_BUF_ARP:
		region = MSM_VIDC_SECURE_NONPIXEL;
		break;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
	}
	return region;
}

struct msm_vidc_buffers *msm_vidc_get_buffers(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		return &inst->buffers.input;
	case MSM_VIDC_BUF_INPUT_META:
		return &inst->buffers.input_meta;
	case MSM_VIDC_BUF_OUTPUT:
		return &inst->buffers.output;
	case MSM_VIDC_BUF_OUTPUT_META:
		return &inst->buffers.output_meta;
	case MSM_VIDC_BUF_BIN:
		return &inst->buffers.bin;
	case MSM_VIDC_BUF_ARP:
		return &inst->buffers.arp;
	case MSM_VIDC_BUF_COMV:
		return &inst->buffers.comv;
	case MSM_VIDC_BUF_NON_COMV:
		return &inst->buffers.non_comv;
	case MSM_VIDC_BUF_LINE:
		return &inst->buffers.line;
	case MSM_VIDC_BUF_DPB:
		return &inst->buffers.dpb;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->buffers.persist;
	case MSM_VIDC_BUF_VPSS:
		return &inst->buffers.vpss;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
		return NULL;
	}
}

struct msm_vidc_mappings *msm_vidc_get_mappings(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		return &inst->mappings.input;
	case MSM_VIDC_BUF_INPUT_META:
		return &inst->mappings.input_meta;
	case MSM_VIDC_BUF_OUTPUT:
		return &inst->mappings.output;
	case MSM_VIDC_BUF_OUTPUT_META:
		return &inst->mappings.output_meta;
	case MSM_VIDC_BUF_BIN:
		return &inst->mappings.bin;
	case MSM_VIDC_BUF_ARP:
		return &inst->mappings.arp;
	case MSM_VIDC_BUF_COMV:
		return &inst->mappings.comv;
	case MSM_VIDC_BUF_NON_COMV:
		return &inst->mappings.non_comv;
	case MSM_VIDC_BUF_LINE:
		return &inst->mappings.line;
	case MSM_VIDC_BUF_DPB:
		return &inst->mappings.dpb;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->mappings.persist;
	case MSM_VIDC_BUF_VPSS:
		return &inst->mappings.vpss;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
		return NULL;
	}
}

struct msm_vidc_allocations *msm_vidc_get_allocations(
	struct msm_vidc_inst *inst, enum msm_vidc_buffer_type buffer_type,
	const char *func)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_BIN:
		return &inst->allocations.bin;
	case MSM_VIDC_BUF_ARP:
		return &inst->allocations.arp;
	case MSM_VIDC_BUF_COMV:
		return &inst->allocations.comv;
	case MSM_VIDC_BUF_NON_COMV:
		return &inst->allocations.non_comv;
	case MSM_VIDC_BUF_LINE:
		return &inst->allocations.line;
	case MSM_VIDC_BUF_DPB:
		return &inst->allocations.dpb;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->allocations.persist;
	case MSM_VIDC_BUF_VPSS:
		return &inst->allocations.vpss;
	default:
		i_vpr_e(inst, "%s: invalid driver buffer type %d\n",
			func, buffer_type);
		return NULL;
	}
}

const char *core_state_name(enum msm_vidc_core_state state)
{
	const char* name = "UNKNOWN";

	switch (state) {
	case MSM_VIDC_CORE_INIT:
		name = "CORE_INIT";
		break;
	case MSM_VIDC_CORE_DEINIT:
		name = "CORE_DEINIT";
		break;
	default:
		name = "UNKNOWN";
		break;
	}

	return name;
}

int msm_vidc_change_core_state(struct msm_vidc_core *core,
	enum msm_vidc_core_state request_state, const char *func)
{
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h("%s: core state changed from %s to %s\n",
		func, core_state_name(core->state),
		core_state_name(request_state));
	core->state = request_state;
	return 0;
}

const char *state_name(enum msm_vidc_inst_state state)
{
	const char *name = "UNKNOWN";

	switch (state) {
	case MSM_VIDC_OPEN:
		name = "OPEN";
		break;
	case MSM_VIDC_START_INPUT:
		name = "START_INPUT";
		break;
	case MSM_VIDC_START_OUTPUT:
		name = "START_OUTPUT";
		break;
	case MSM_VIDC_START:
		name = "START";
		break;
	case MSM_VIDC_DRC:
		name = "DRC";
		break;
	case MSM_VIDC_DRC_LAST_FLAG:
		name = "DRC_LAST_FLAG";
		break;
	case MSM_VIDC_DRAIN:
		name = "DRAIN";
		break;
	case MSM_VIDC_DRAIN_LAST_FLAG:
		name = "DRAIN_LAST_FLAG";
		break;
	case MSM_VIDC_DRC_DRAIN:
		name = "DRC_DRAIN";
		break;
	case MSM_VIDC_DRC_DRAIN_LAST_FLAG:
		name = "DRC_DRAIN_LAST_FLAG";
		break;
	case MSM_VIDC_DRAIN_START_INPUT:
		name = "DRAIN_START_INPUT";
		break;
	case MSM_VIDC_ERROR:
		name = "ERROR";
		break;
	default:
		name = "UNKNOWN";
		break;
	}

	return name;
}

int msm_vidc_change_inst_state(struct msm_vidc_inst *inst,
	enum msm_vidc_inst_state request_state, const char *func)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!request_state) {
		i_vpr_e(inst, "%s: invalid request state\n", func);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_ERROR) {
		i_vpr_h(inst,
			"%s: inst is in bad state, can not change state to %s\n",
			func, state_name(request_state));
		return 0;
	}

	i_vpr_h(inst, "%s: state changed from %s to %s\n",
		   func, state_name(inst->state), state_name(request_state));
	inst->state = request_state;
	return 0;
}

bool msm_vidc_allow_s_fmt(struct msm_vidc_inst *inst, u32 type)
{
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_OPEN) {
		allow = true;
		goto exit;
	}
	if (inst->state == MSM_VIDC_START_INPUT) {
		if (type == OUTPUT_MPLANE || type == OUTPUT_META_PLANE) {
			allow = true;
			goto exit;
		}
	}
	if (inst->state == MSM_VIDC_START_OUTPUT) {
		if (type == INPUT_MPLANE || type == INPUT_META_PLANE) {
			allow = true;
			goto exit;
		}
	}

exit:
	if (!allow)
		i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
				__func__, type, state_name(inst->state));
	return allow;
}

bool msm_vidc_allow_s_ctrl(struct msm_vidc_inst *inst, u32 id)
{
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_OPEN) {
		allow = true;
		goto exit;
	}
	if (is_decode_session(inst)) {
		if (!inst->vb2q[INPUT_PORT].streaming) {
			allow = true;
			goto exit;
		}
		if (inst->vb2q[INPUT_PORT].streaming) {
			switch (id) {
			case V4L2_CID_MPEG_VIDC_CODEC_CONFIG:
				allow = true;
				break;
			default:
				allow = false;
				break;
			}
		}
	} else if (is_encode_session(inst)) {
		if (inst->state == MSM_VIDC_START || inst->state == MSM_VIDC_START_OUTPUT) {
			switch (id) {
			case V4L2_CID_MPEG_VIDEO_BITRATE:
			case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
			case V4L2_CID_MPEG_VIDEO_FORCE_KEY_FRAME:
			case V4L2_CID_HFLIP:
			case V4L2_CID_VFLIP:
			case V4L2_CID_MPEG_VIDEO_HEVC_I_FRAME_QP:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_LAYER:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L0_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L1_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L2_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L3_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L4_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L5_BR:
			case V4L2_CID_MPEG_VIDEO_HEVC_HIER_CODING_L6_BR:
			case V4L2_CID_MPEG_VIDC_BASELAYER_PRIORITY:
			case V4L2_CID_MPEG_VIDC_USELTRFRAME:
			case V4L2_CID_MPEG_VIDC_MARKLTRFRAME:
			case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_TYPES:
			case V4L2_CID_MPEG_VIDC_VIDEO_BLUR_RESOLUTION:
			case V4L2_CID_MPEG_VIDEO_CONSTANT_QUALITY:
				allow = true;
				break;
			default:
				allow = false;
				break;
			}
		}
	}

exit:
	if (!allow)
		i_vpr_e(inst, "%s: id %d not allowed in state %s\n",
			__func__, id, state_name(inst->state));
	return allow;
}

bool msm_vidc_allow_reqbufs(struct msm_vidc_inst *inst, u32 type)
{
	bool allow = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_OPEN) {
		allow = true;
		goto exit;
	}
	if (inst->state == MSM_VIDC_START_INPUT) {
		if (type == OUTPUT_MPLANE || type == OUTPUT_META_PLANE) {
			allow = true;
			goto exit;
		}
	}
	if (inst->state == MSM_VIDC_START_OUTPUT) {
		if (type == INPUT_MPLANE || type == INPUT_META_PLANE) {
			allow = true;
			goto exit;
		}
	}

exit:
	if (!allow)
		i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
				__func__, type, state_name(inst->state));
	return allow;
}

enum msm_vidc_allow msm_vidc_allow_stop(struct msm_vidc_inst *inst)
{
	enum msm_vidc_allow allow = MSM_VIDC_DISALLOW;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return allow;
	}
	if (inst->state == MSM_VIDC_START ||
		inst->state == MSM_VIDC_DRC ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_DRAIN) {
		allow = MSM_VIDC_ALLOW;
	} else if (inst->state == MSM_VIDC_START_INPUT) {
		allow = MSM_VIDC_IGNORE;
		i_vpr_e(inst, "%s: stop ignored in state %s\n",
			__func__, state_name(inst->state));
	} else {
		i_vpr_e(inst, "%s: stop not allowed in state %s\n",
			__func__, state_name(inst->state));
	}

	return allow;
}

bool msm_vidc_allow_start(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG)
		return true;

	i_vpr_e(inst, "%s: not allowed in state %s\n",
			__func__, state_name(inst->state));
	return false;
}

bool msm_vidc_allow_streamon(struct msm_vidc_inst *inst, u32 type)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (type == INPUT_MPLANE || type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_OUTPUT)
			return true;
	} else if (type == OUTPUT_MPLANE || type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_INPUT ||
			inst->state == MSM_VIDC_DRAIN_START_INPUT)
			return true;
	}

	i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
			__func__, type, state_name(inst->state));
	return false;
}

bool msm_vidc_allow_streamoff(struct msm_vidc_inst *inst, u32 type)
{
	bool allow = true;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_OUTPUT)
			allow = false;
	} else if (type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT)
			allow = false;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN ||
			inst->state == MSM_VIDC_START_INPUT)
			allow = false;
	} else if (type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT)
			allow = false;
	}
	if (!allow)
		i_vpr_e(inst, "%s: type %d not allowed in state %s\n",
				__func__, type, state_name(inst->state));

	return allow;
}

bool msm_vidc_allow_qbuf(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_ERROR) {
		i_vpr_e(inst, "%s: inst in error state\n", __func__);
		return false;
	} else {
		return true;
	}
}

enum msm_vidc_allow msm_vidc_allow_input_psc(struct msm_vidc_inst *inst)
{
	enum msm_vidc_allow allow = MSM_VIDC_DISALLOW;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return MSM_VIDC_DISALLOW;
	}
	if (inst->state == MSM_VIDC_START ||
		inst->state == MSM_VIDC_START_INPUT ||
		inst->state == MSM_VIDC_DRAIN) {
		allow = MSM_VIDC_ALLOW;
	} else if (inst->state == MSM_VIDC_DRC ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_DRAIN ||
		inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG ||
		inst->state == MSM_VIDC_DRAIN_START_INPUT) {
		i_vpr_h(inst, "%s: defer input psc, inst state %s\n",
				__func__, state_name(inst->state));
		allow = MSM_VIDC_DEFER;
	} else {
		i_vpr_e(inst, "%s: input psc in wrong state %s\n",
				__func__, state_name(inst->state));
		allow = MSM_VIDC_DISALLOW;
	}

	return allow;
}

bool msm_vidc_allow_last_flag(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return false;
	}
	if (inst->state == MSM_VIDC_DRC ||
		inst->state == MSM_VIDC_DRAIN ||
		inst->state == MSM_VIDC_DRC_DRAIN)
		return true;

	i_vpr_e(inst, "%s: not allowed in state %s\n",
			__func__, state_name(inst->state));
	return false;
}

int msm_vidc_state_change_streamon(struct msm_vidc_inst *inst, u32 type)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;
	struct response_work *resp_work;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (type == INPUT_META_PLANE || type == OUTPUT_META_PLANE)
		return 0;

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN)
			new_state = MSM_VIDC_START_INPUT;
		else if (inst->state == MSM_VIDC_START_OUTPUT)
			new_state = MSM_VIDC_START;
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_OPEN) {
			new_state = MSM_VIDC_START_OUTPUT;
		} else if (inst->state == MSM_VIDC_START_INPUT) {
			new_state = MSM_VIDC_START;
		} else if (inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			i_vpr_h(inst,
				"%s: streamon(output) in DRAIN_START_INPUT state\n",
				__func__);
			new_state = MSM_VIDC_DRAIN;
			if (!list_empty(&inst->response_works)) {
				resp_work = list_first_entry(&inst->response_works,
							struct response_work, list);
				if (resp_work->type == RESP_WORK_INPUT_PSC) {
					i_vpr_h(inst,
						"%s: streamon(output) in DRAIN_START_INPUT state, input psc pending\n",
						__func__);
					rc = handle_session_response_work(inst, resp_work);
					if (rc) {
						i_vpr_e(inst,
							"%s: handle input psc failed\n", __func__);
						new_state = MSM_VIDC_ERROR;
					} else {
						new_state = MSM_VIDC_DRC_DRAIN;
					}
					list_del(&resp_work->list);
					kfree(resp_work->data);
					kfree(resp_work);
				}
			}
		}
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_state_change_streamoff(struct msm_vidc_inst *inst, u32 type)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;
	struct response_work *resp_work, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (type == INPUT_META_PLANE || type == OUTPUT_META_PLANE)
		return 0;

	if (type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			new_state = MSM_VIDC_OPEN;
		} else if (inst->state == MSM_VIDC_START) {
			new_state = MSM_VIDC_START_OUTPUT;
		} else if (inst->state == MSM_VIDC_DRC ||
				   inst->state == MSM_VIDC_DRC_LAST_FLAG ||
				   inst->state == MSM_VIDC_DRAIN ||
				   inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
				   inst->state == MSM_VIDC_DRC_DRAIN ||
				   inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG ||
				   inst->state == MSM_VIDC_DRAIN_START_INPUT) {
			new_state = MSM_VIDC_START_OUTPUT;
			/* discard pending port settings change if any */
			list_for_each_entry_safe(resp_work, dummy,
						&inst->response_works, list) {
				if (resp_work->type == RESP_WORK_INPUT_PSC) {
					i_vpr_h(inst,
						"%s: discard pending input psc\n", __func__);
					list_del(&resp_work->list);
					kfree(resp_work->data);
					kfree(resp_work);
				}
			}
		}
	} else if (type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			new_state = MSM_VIDC_OPEN;
		} else if (inst->state == MSM_VIDC_START ||
				   inst->state == MSM_VIDC_DRAIN ||
				   inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
				   inst->state == MSM_VIDC_DRC ||
				   inst->state == MSM_VIDC_DRC_LAST_FLAG ||
				   inst->state == MSM_VIDC_DRC_DRAIN) {
			new_state = MSM_VIDC_START_INPUT;
		} else if (inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG) {
			new_state = MSM_VIDC_DRAIN_START_INPUT;
		}
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		goto exit;

exit:
	return rc;
}

int msm_vidc_state_change_stop(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_START) {
		new_state = MSM_VIDC_DRAIN;
	} else if (inst->state == MSM_VIDC_DRC) {
		new_state = MSM_VIDC_DRC_DRAIN;
	} else if (inst->state == MSM_VIDC_DRC_DRAIN ||
			   inst->state == MSM_VIDC_DRC_LAST_FLAG) {
		new_state = MSM_VIDC_DRC_DRAIN_LAST_FLAG;
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
			__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_state_change_start(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;
	struct response_work *resp_work;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_DRAIN_LAST_FLAG ||
		inst->state == MSM_VIDC_DRC_LAST_FLAG) {
			new_state = MSM_VIDC_START;
		if (!list_empty(&inst->response_works)) {
			resp_work = list_first_entry(&inst->response_works,
				struct response_work, list);
			if (resp_work->type == RESP_WORK_INPUT_PSC) {
				i_vpr_h(inst,
					"%s: start in DRC(DRAIN)_LAST_FLAG state, input psc pending\n",
					__func__);
				rc = handle_session_response_work(inst, resp_work);
				if (rc) {
					i_vpr_e(inst,
						"%s: handle input psc failed\n", __func__);
					new_state = MSM_VIDC_ERROR;
				} else {
					new_state = MSM_VIDC_DRC;
				}
				list_del(&resp_work->list);
				kfree(resp_work->data);
				kfree(resp_work);
			}
		}
	} else if (inst->state == MSM_VIDC_DRC_DRAIN_LAST_FLAG) {
			new_state = MSM_VIDC_DRAIN;
		if (!list_empty(&inst->response_works)) {
			resp_work = list_first_entry(&inst->response_works,
				struct response_work, list);
			if (resp_work->type == RESP_WORK_INPUT_PSC) {
				i_vpr_h(inst,
					"%s: start in DRC_DRAIN_LAST_FLAG state, input psc pending\n");
				rc = handle_session_response_work(inst, resp_work);
				if (rc) {
					i_vpr_e(inst,
						"%s: handle input psc failed\n", __func__);
					new_state = MSM_VIDC_ERROR;
				} else {
					new_state = MSM_VIDC_DRC_DRAIN;
				}
				list_del(&resp_work->list);
				kfree(resp_work->data);
				kfree(resp_work);
			}
		}
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
			__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_state_change_input_psc(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	/* don't change state as output port is not started yet */
	if (inst->state == MSM_VIDC_START_INPUT)
		return 0;

	if (inst->state == MSM_VIDC_START) {
		new_state = MSM_VIDC_DRC;
	} else if (inst->state == MSM_VIDC_DRAIN) {
		new_state = MSM_VIDC_DRC_DRAIN;
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
				__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_state_change_last_flag(struct msm_vidc_inst *inst)
{
	int rc = 0;
	enum msm_vidc_inst_state new_state = MSM_VIDC_ERROR;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (inst->state == MSM_VIDC_DRC) {
		new_state = MSM_VIDC_DRC_LAST_FLAG;
	} else if (inst->state == MSM_VIDC_DRAIN) {
		new_state = MSM_VIDC_DRAIN_LAST_FLAG;
	} else if (inst->state == MSM_VIDC_DRC_DRAIN) {
		new_state = MSM_VIDC_DRC_DRAIN_LAST_FLAG;
	} else {
		i_vpr_e(inst, "%s: wrong state %s\n",
				__func__, state_name(inst->state));
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return -EINVAL;
	}
	rc = msm_vidc_change_inst_state(inst, new_state, __func__);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_get_control(struct msm_vidc_inst *inst, struct v4l2_ctrl *ctrl)
{
	int rc = 0;

	if (!inst || !ctrl) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		ctrl->val = inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
		i_vpr_h(inst, "g_min: output buffers %d\n", ctrl->val);
		break;
	case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
		ctrl->val = inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
		i_vpr_h(inst, "g_min: input buffers %d\n", ctrl->val);
		break;
	default:
		break;
	}

	return rc;
}

int msm_vidc_get_mbs_per_frame(struct msm_vidc_inst *inst)
{
	int height, width;
	struct v4l2_format *out_f;
	struct v4l2_format *inp_f;

	out_f = &inst->fmts[OUTPUT_PORT];
	inp_f = &inst->fmts[INPUT_PORT];
	height = max(out_f->fmt.pix_mp.height,
			inp_f->fmt.pix_mp.height);
	width = max(out_f->fmt.pix_mp.width,
			inp_f->fmt.pix_mp.width);

	return NUM_MBS_PER_FRAME(height, width);
}

int msm_vidc_get_fps(struct msm_vidc_inst *inst)
{
	int fps;
	u32 frame_rate, operating_rate;

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	frame_rate = inst->capabilities->cap[FRAME_RATE].value;
	operating_rate = inst->capabilities->cap[OPERATING_RATE].value;

	if (operating_rate > frame_rate)
		fps = (operating_rate >> 16) ?
			(operating_rate >> 16) : 1;
	else
		fps = frame_rate >> 16;

	return fps;
}

int msm_vidc_num_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type type, enum msm_vidc_buffer_attributes attr)
{
	int count = 0;
	struct msm_vidc_buffer *vbuf;
	struct msm_vidc_buffers *buffers;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return count;
	}
	if (type == MSM_VIDC_BUF_OUTPUT) {
		buffers = &inst->buffers.output;
	} else if (type == MSM_VIDC_BUF_INPUT) {
		buffers = &inst->buffers.input;
	} else {
		i_vpr_e(inst, "%s: invalid buffer type %#x\n",
				__func__, type);
		return count;
	}

	list_for_each_entry(vbuf, &buffers->list, list) {
		if (vbuf->type != type)
			continue;
		if (!(vbuf->attr & attr))
			continue;
		count++;
	}

	return count;
}

static int vb2_buffer_to_driver(struct vb2_buffer *vb2,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;

	if (!vb2 || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buf->valid = true;
	buf->type = v4l2_type_to_driver(vb2->type, __func__);
	if (!buf->type)
		return -EINVAL;
	buf->index = vb2->index;
	buf->fd = vb2->planes[0].m.fd;
	buf->data_offset = vb2->planes[0].data_offset;
	buf->data_size = vb2->planes[0].bytesused;
	buf->buffer_size = vb2->planes[0].length;
	buf->timestamp = vb2->timestamp;

	return rc;
}

int msm_vidc_unmap_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_map *map = NULL;
	bool found = false;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mappings = msm_vidc_get_mappings(inst, buf->type, __func__);
	if (!mappings)
		return -EINVAL;

	/* sanity check to see if it was not removed */
	list_for_each_entry(map, &mappings->list, list) {
		if (map->dmabuf == buf->dmabuf) {
			found = true;
			break;
		}
	}
	if (!found) {
		print_vidc_buffer(VIDC_ERR, "no buf in mappings", inst, buf);
		return -EINVAL;
	}

	rc = msm_vidc_memory_unmap(inst->core, map);
	if (rc) {
		print_vidc_buffer(VIDC_ERR, "unmap failed", inst, buf);
		return -EINVAL;
	}

	/* finally delete if refcount is zero */
	if (!map->refcount) {
		list_del(&map->list);
		kfree(map);
	}

	return 0;
}

int msm_vidc_put_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_unmap_driver_buf(inst, buf);
	if (rc)
		return rc;

	msm_vidc_memory_put_dmabuf(buf->dmabuf);

	/* delete the buffer from buffers->list */
	list_del(&buf->list);
	kfree(buf);

	return 0;
}

int msm_vidc_map_driver_buf(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int rc = 0;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_map *map = NULL;
	bool found = false;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mappings = msm_vidc_get_mappings(inst, buf->type, __func__);
	if (!mappings)
		return -EINVAL;

	/* check if it is an existing one */
	list_for_each_entry(map, &mappings->list, list) {
		if (map->dmabuf == buf->dmabuf) {
			found = true;
			break;
		}
	}
	if (found) {
		/* skip mapping for RO buffer */
		if (!(buf->attr & MSM_VIDC_ATTR_READ_ONLY)) {
			rc = msm_vidc_memory_map(inst->core, map);
			if (rc)
				return -ENOMEM;
			buf->device_addr = map->device_addr;
		}
		return 0;
	}
	map = kzalloc(sizeof(struct msm_vidc_map), GFP_KERNEL);
	if (!map) {
		i_vpr_e(inst, "%s: alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&map->list);
	map->type = buf->type;
	map->dmabuf = buf->dmabuf;
	map->region = msm_vidc_get_buffer_region(inst, buf->type, __func__);
	rc = msm_vidc_memory_map(inst->core, map);
	if (rc) {
		kfree(map);
		return -ENOMEM;
	}
	buf->device_addr = map->device_addr;
	list_add_tail(&map->list, &mappings->list);

	return 0;
}

struct msm_vidc_buffer *msm_vidc_get_driver_buf(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2)
{
	int rc = 0;
	struct msm_vidc_buffer *buf = NULL;
	struct msm_vidc_buffers *buffers;
	struct dma_buf *dmabuf;
	enum msm_vidc_buffer_type buf_type;
	bool found = false;

	if (!inst || !vb2) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	buf_type = v4l2_type_to_driver(vb2->type, __func__);
	if (!buf_type)
		return NULL;

	buffers = msm_vidc_get_buffers(inst, buf_type, __func__);
	if (!buffers)
		return NULL;

	dmabuf = msm_vidc_memory_get_dmabuf(vb2->planes[0].m.fd);
	if (!dmabuf)
		return NULL;

	/* check if it is an existing buffer */
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->dmabuf == dmabuf &&
		    buf->data_offset == vb2->planes[0].data_offset) {
			found = true;
			break;
		}
	}
	if (found) {
		/* only YUV buffers are allowed to repeat */
		if ((is_decode_session(inst) && vb2->type != OUTPUT_MPLANE) ||
		    (is_encode_session(inst) && vb2->type != INPUT_MPLANE)) {
			print_vidc_buffer(VIDC_ERR,
				"existing buffer", inst, buf);
			goto error;
		}
		/* for decoder, YUV with RO flag are allowed to repeat */
		if (is_decode_session(inst) &&
		    !(buf->attr & MSM_VIDC_ATTR_READ_ONLY)) {
			print_vidc_buffer(VIDC_ERR,
				"existing buffer without RO flag", inst, buf);
			goto error;
		}
		/* for encoder, treat the repeated buffer as new buffer */
		if (is_encode_session(inst) && vb2->type == INPUT_MPLANE)
			found = false;
	}
	if (!found) {
		buf = kzalloc(sizeof(struct msm_vidc_buffer), GFP_KERNEL);
		if (!buf) {
			i_vpr_e(inst, "%s: alloc failed\n", __func__);
			goto error;
		}
		buf->dmabuf = dmabuf;
		INIT_LIST_HEAD(&buf->list);
		list_add_tail(&buf->list, &buffers->list);
	}
	rc = vb2_buffer_to_driver(vb2, buf);
	if (rc)
		goto error;

	rc = msm_vidc_map_driver_buf(inst, buf);
	if (rc)
		goto error;

	return buf;

error:
	msm_vidc_memory_put_dmabuf(dmabuf);
	if (!found)
		kfree(buf);
	return NULL;
}

struct msm_vidc_buffer *get_meta_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	struct msm_vidc_buffer *mbuf;
	struct msm_vidc_buffers *buffers;
	bool found = false;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	if (buf->type == MSM_VIDC_BUF_INPUT) {
		buffers = &inst->buffers.input_meta;
	} else if (buf->type == MSM_VIDC_BUF_OUTPUT) {
		buffers = &inst->buffers.output_meta;
	} else {
		i_vpr_e(inst, "%s: invalid buffer type %d\n",
			__func__, buf->type);
		return NULL;
	}
	list_for_each_entry(mbuf, &buffers->list, list) {
		if (!mbuf->valid)
			continue;
		if (mbuf->index == buf->index) {
			found = true;
			break;
		}
	}
	if (!found)
		return NULL;

	return mbuf;
}

int msm_vidc_queue_buffer(struct msm_vidc_inst *inst, struct vb2_buffer *vb2)
{
	int rc = 0;
	struct msm_vidc_buffer *buf;
	struct msm_vidc_buffer *meta;
	int port;

	if (!inst || !vb2) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buf = msm_vidc_get_driver_buf(inst, vb2);
	if (!buf)
		return -EINVAL;

	/* meta buffer will be queued along with actual buffer */
	if (buf->type == MSM_VIDC_BUF_INPUT_META ||
	    buf->type == MSM_VIDC_BUF_OUTPUT_META) {
		buf->attr |= MSM_VIDC_ATTR_DEFERRED;
		print_vidc_buffer(VIDC_HIGH, "qbuf deferred", inst, buf);
		return 0;
	}

	/* skip queuing if streamon not completed */
	port = v4l2_type_to_driver_port(inst, vb2->type, __func__);
	if (port < 0)
		return -EINVAL;
	if (!inst->vb2q[port].streaming) {
		buf->attr |= MSM_VIDC_ATTR_DEFERRED;
		print_vidc_buffer(VIDC_HIGH, "qbuf deferred", inst, buf);
		return 0;
	}

	if (is_decode_session(inst) &&
			inst->capabilities->cap[CODEC_CONFIG].value) {
		buf->flags |= MSM_VIDC_BUF_FLAG_CODECCONFIG;
		inst->capabilities->cap[CODEC_CONFIG].value = 0;
	}

	if (buf->type == MSM_VIDC_BUF_INPUT) {
		inst->power.buffer_counter++;
		msm_vidc_scale_power(inst, true);
	}

	print_vidc_buffer(VIDC_HIGH, "qbuf", inst, buf);
	meta = get_meta_buffer(inst, buf);
	if (!meta) {
		if (is_meta_enabled(inst, buf->type)) {
			print_vidc_buffer(VIDC_ERR, "missing meta for",
				inst, buf);
			return -EINVAL;
		}
	}
	rc = venus_hfi_queue_buffer(inst, buf, meta);
	if (rc)
		return rc;

	buf->attr &= ~MSM_VIDC_ATTR_DEFERRED;
	buf->attr |= MSM_VIDC_ATTR_QUEUED;
	if (meta) {
		meta->attr &= ~MSM_VIDC_ATTR_DEFERRED;
		meta->attr |= MSM_VIDC_ATTR_QUEUED;
	}

	if (buf->type == MSM_VIDC_BUF_INPUT)
		msm_vidc_debugfs_update(inst, MSM_VIDC_DEBUGFS_EVENT_ETB);
	else if (buf->type == MSM_VIDC_BUF_OUTPUT)
		msm_vidc_debugfs_update(inst, MSM_VIDC_DEBUGFS_EVENT_FTB);

	return rc;
}

int msm_vidc_destroy_internal_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_allocations *allocations;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_alloc *alloc, *alloc_dummy;
	struct msm_vidc_map  *map, *map_dummy;
	struct msm_vidc_buffer *buf, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (!is_internal_buffer(buffer->type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer->type);
		return 0;
	}

	i_vpr_h(inst,
		"%s: destroy buffer_type %#x, size %d device_addr %#x\n",
		__func__, buffer->type, buffer->buffer_size,
		buffer->device_addr);

	buffers = msm_vidc_get_buffers(inst, buffer->type, __func__);
	if (!buffers)
		return -EINVAL;
	allocations = msm_vidc_get_allocations(inst, buffer->type, __func__);
	if (!allocations)
		return -EINVAL;
	mappings = msm_vidc_get_mappings(inst, buffer->type, __func__);
	if (!mappings)
		return -EINVAL;

	list_for_each_entry_safe(map, map_dummy, &mappings->list, list) {
		if (map->dmabuf == buffer->dmabuf) {
			msm_vidc_memory_unmap(inst->core, map);
			list_del(&map->list);
			kfree(map);
			break;
		}
	}

	list_for_each_entry_safe(alloc, alloc_dummy, &allocations->list, list) {
		if (alloc->dmabuf == buffer->dmabuf) {
			msm_vidc_memory_free(inst->core, alloc);
			list_del(&alloc->list);
			kfree(alloc);
			break;
		}
	}

	list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
		if (buf->dmabuf == buffer->dmabuf) {
			list_del(&buf->list);
			kfree(buf);
			break;
		}
	}

	buffers->size = 0;
	buffers->min_count = buffers->extra_count = buffers->actual_count = 0;

	return 0;
}

int msm_vidc_get_internal_buffers(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	u32 buf_size;
	u32 buf_count;
	struct msm_vidc_core *core;
	struct msm_vidc_buffers *buffers;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	buf_size = call_session_op(core, buffer_size,
		inst, buffer_type);

	buf_count = call_session_op(core, min_count,
		inst, buffer_type);

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buf_size <= buffers->size &&
		buf_count <= buffers->min_count) {
		buffers->reuse = true;
	} else {
		buffers->reuse = false;
		buffers->size = buf_size;
		buffers->min_count = buf_count;
	}
	return 0;
}

int msm_vidc_create_internal_buffer(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type, u32 index)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_allocations *allocations;
	struct msm_vidc_mappings *mappings;
	struct msm_vidc_buffer *buffer;
	struct msm_vidc_alloc *alloc;
	struct msm_vidc_map *map;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;
	allocations = msm_vidc_get_allocations(inst, buffer_type, __func__);
	if (!allocations)
		return -EINVAL;
	mappings = msm_vidc_get_mappings(inst, buffer_type, __func__);
	if (!mappings)
		return -EINVAL;

	if (!buffers->size) {
		i_vpr_e(inst, "%s: invalid buffer %#x\n",
			__func__, buffer_type);
		return -EINVAL;
	}
	buffer = kzalloc(sizeof(struct msm_vidc_buffer), GFP_KERNEL);
	if (!buffer) {
		i_vpr_e(inst, "%s: buf alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&buffer->list);
	buffer->valid = true;
	buffer->type = buffer_type;
	buffer->index = index;
	buffer->buffer_size = buffers->size;
	list_add_tail(&buffer->list, &buffers->list);

	alloc = kzalloc(sizeof(struct msm_vidc_alloc), GFP_KERNEL);
	if (!alloc) {
		i_vpr_e(inst, "%s: alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&alloc->list);
	alloc->type = buffer_type;
	alloc->region = msm_vidc_get_buffer_region(inst,
		buffer_type, __func__);
	alloc->size = buffer->buffer_size;
	alloc->secure = (alloc->region > MSM_VIDC_NON_SECURE) ? 1 : 0;
	rc = msm_vidc_memory_alloc(inst->core, alloc);
	if (rc)
		return -ENOMEM;
	list_add_tail(&alloc->list, &allocations->list);

	map = kzalloc(sizeof(struct msm_vidc_map), GFP_KERNEL);
	if (!map) {
		i_vpr_e(inst, "%s: map alloc failed\n", __func__);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&map->list);
	map->type = alloc->type;
	map->region = alloc->region;
	map->dmabuf = alloc->dmabuf;
	rc = msm_vidc_memory_map(inst->core, map);
	if (rc)
		return -ENOMEM;
	list_add_tail(&map->list, &mappings->list);

	buffer->dmabuf = alloc->dmabuf;
	buffer->device_addr = map->device_addr;
	i_vpr_h(inst,
		"%s: created buffer_type %#x, size %d device_addr %#x\n",
		__func__, buffer_type, buffers->size,
		buffer->device_addr);

	return 0;
}

int msm_vidc_create_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	int i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buffers->reuse) {
		i_vpr_l(inst, "%s: reuse enabled for buffer type %#x\n",
			__func__, buffer_type);
		return 0;
	}

	for (i = 0; i < buffers->min_count; i++) {
		rc = msm_vidc_create_internal_buffer(inst, buffer_type, i);
		if (rc)
			return rc;
	}

	return rc;
}

int msm_vidc_queue_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buffer, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buffers->reuse) {
		i_vpr_l(inst, "%s: reuse enabled for buffer type %#x\n",
			__func__, buffer_type);
		return 0;
	}

	list_for_each_entry_safe(buffer, dummy, &buffers->list, list) {
		/* do not queue pending release buffers */
		if (buffer->flags & MSM_VIDC_ATTR_PENDING_RELEASE)
			continue;
		/* do not queue already queued buffers */
		if (buffer->attr & MSM_VIDC_ATTR_QUEUED)
			continue;
		rc = venus_hfi_queue_buffer(inst, buffer, NULL);
		if (rc)
			return rc;
		/* mark queued */
		buffer->attr |= MSM_VIDC_ATTR_QUEUED;

		i_vpr_h(inst, "%s: queued buffer_type %#x, size %d\n",
			__func__, buffer_type, buffers->size);
	}

	return 0;
}

int msm_vidc_alloc_and_queue_session_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (buffer_type != MSM_VIDC_BUF_ARP &&
		buffer_type != MSM_VIDC_BUF_PERSIST) {
		i_vpr_e(inst, "%s: invalid buffer type: %d\n",
			__func__, buffer_type);
		rc = -EINVAL;
		goto exit;
	}

	rc = msm_vidc_get_internal_buffers(inst, buffer_type);
	if (rc)
		goto exit;

	rc = msm_vidc_create_internal_buffers(inst, buffer_type);
	if (rc)
		goto exit;

	rc = msm_vidc_queue_internal_buffers(inst, buffer_type);
	if (rc)
		goto exit;

exit:
	return rc;
}

int msm_vidc_release_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buffer, *dummy;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		i_vpr_e(inst, "%s: buffer type %#x is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffers = msm_vidc_get_buffers(inst, buffer_type, __func__);
	if (!buffers)
		return -EINVAL;

	if (buffers->reuse) {
		i_vpr_l(inst, "%s: reuse enabled for buffer type %#x\n",
			__func__, buffer_type);
		return 0;
	}

	list_for_each_entry_safe(buffer, dummy, &buffers->list, list) {
		/* do not release already pending release buffers */
		if (buffer->attr & MSM_VIDC_ATTR_PENDING_RELEASE)
			continue;
		/* release only queued buffers */
		if (!(buffer->attr & MSM_VIDC_ATTR_QUEUED))
			continue;
		rc = venus_hfi_release_buffer(inst, buffer);
		if (rc)
			return rc;
		/* mark pending release */
		buffer->attr |= MSM_VIDC_ATTR_PENDING_RELEASE;

		i_vpr_e(inst, "%s: released buffer_type %#x, size %d\n",
			__func__, buffer_type, buffers->size);
	}

	return 0;
}

int msm_vidc_vb2_buffer_done(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buf)
{
	int type, port;
	struct vb2_queue *q;
	struct vb2_buffer *vb2;
	struct vb2_v4l2_buffer *vbuf;
	bool found;

	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	type = v4l2_type_from_driver(buf->type, __func__);
	if (!type)
		return -EINVAL;
	port = v4l2_type_to_driver_port(inst, type, __func__);
	if (port < 0)
		return -EINVAL;

	q = &inst->vb2q[port];
	if (!q->streaming) {
		i_vpr_e(inst, "%s: port %d is not streaming\n",
			__func__, port);
		return -EINVAL;
	}

	found = false;
	list_for_each_entry(vb2, &q->queued_list, queued_entry) {
		if (vb2->state != VB2_BUF_STATE_ACTIVE)
			continue;
		if (vb2->index == buf->index) {
			found = true;
			break;
		}
	}
	if (!found) {
		print_vidc_buffer(VIDC_ERR, "vb2 not found for", inst, buf);
		return -EINVAL;
	}
	vbuf = to_vb2_v4l2_buffer(vb2);
	vbuf->flags = buf->flags;
	vb2->timestamp = buf->timestamp;
	vb2->planes[0].bytesused = buf->data_size;
	vb2_buffer_done(vb2, VB2_BUF_STATE_DONE);

	return 0;
}

int msm_vidc_event_queue_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int index;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (is_decode_session(inst))
		index = 0;
	else if (is_encode_session(inst))
		index = 1;
	else
		return -EINVAL;

	v4l2_fh_init(&inst->event_handler, &core->vdev[index].vdev);
	v4l2_fh_add(&inst->event_handler);

	return rc;
}

int msm_vidc_event_queue_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	v4l2_fh_del(&inst->event_handler);
	v4l2_fh_exit(&inst->event_handler);

	return rc;
}

static int vb2q_init(struct msm_vidc_inst *inst,
	struct vb2_queue *q, enum v4l2_buf_type type)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !q || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	q->type = type;
	q->io_modes = VB2_DMABUF;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	q->ops = core->vb2_ops;
	q->mem_ops = core->vb2_mem_ops;
	q->drv_priv = inst;
	q->allow_zero_bytesused = 1;
	q->copy_timestamp = 1;
	rc = vb2_queue_init(q);
	if (rc)
		i_vpr_e(inst, "%s: vb2_queue_init failed for type %d\n",
				__func__, type);
	return rc;
}

int msm_vidc_vb2_queue_init(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = vb2q_init(inst, &inst->vb2q[INPUT_PORT], INPUT_MPLANE);
	if (rc)
		return rc;

	rc = vb2q_init(inst, &inst->vb2q[OUTPUT_PORT], OUTPUT_MPLANE);
	if (rc)
		return rc;

	rc = vb2q_init(inst, &inst->vb2q[INPUT_META_PORT], INPUT_META_PLANE);
	if (rc)
		return rc;

	rc = vb2q_init(inst, &inst->vb2q[OUTPUT_META_PORT], OUTPUT_META_PLANE);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_vb2_queue_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	vb2_queue_release(&inst->vb2q[OUTPUT_META_PORT]);
	vb2_queue_release(&inst->vb2q[INPUT_META_PORT]);
	vb2_queue_release(&inst->vb2q[OUTPUT_PORT]);
	vb2_queue_release(&inst->vb2q[INPUT_PORT]);

	return rc;
}

int msm_vidc_add_session(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_inst *i;
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	core_lock(core, __func__);
	list_for_each_entry(i, &core->instances, list)
		count++;

	if (count < 0xffffff /*TODO: MAX_SUPPORTED_INSTANCES*/) {
		list_add_tail(&inst->list, &core->instances);
	} else {
		d_vpr_e("%s: total sessions %d exceeded max limit %d\n",
			__func__, count, MAX_SUPPORTED_INSTANCES);
		rc = -EINVAL;
	}
	core_unlock(core, __func__);

	/* assign session_id */
	inst->session_id = hash32_ptr(inst);
	inst->sid = inst->session_id;

	return rc;
}

int msm_vidc_remove_session(struct msm_vidc_inst *inst)
{
	struct msm_vidc_inst *i, *temp;
	struct msm_vidc_core *core;
	u32 count = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	core_lock(core, __func__);
	list_for_each_entry_safe(i, temp, &core->instances, list) {
		if (i->session_id == inst->session_id) {
			list_del_init(&i->list);
			d_vpr_h("%s: removed session %d\n",
				__func__, i->session_id);
			inst->sid = 0;
		}
	}
	list_for_each_entry(i, &core->instances, list)
		count++;
	d_vpr_h("%s: remaining sessions %d\n", __func__, count);
	core_unlock(core, __func__);

	return 0;
}

int msm_vidc_session_open(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	inst->packet_size = 4096;
	inst->packet = kzalloc(inst->packet_size, GFP_KERNEL);
	if (!inst->packet) {
		i_vpr_e(inst, "%s(): inst packet allocation failed\n", __func__);
		return -ENOMEM;
	}

	rc = venus_hfi_session_open(inst);
	if (rc)
		goto error;

	return 0;
error:
	i_vpr_e(inst, "%s(): session open failed\n", __func__);
	kfree(inst->packet);
	inst->packet = NULL;
	return rc;
}

int msm_vidc_session_set_codec(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = venus_hfi_session_set_codec(inst);
	if (rc)
		return rc;

	return 0;
}

int msm_vidc_session_streamon(struct msm_vidc_inst *inst,
		enum msm_vidc_port_type port)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	msm_vidc_scale_power(inst, true);

	rc = venus_hfi_start(inst, port);
	if (rc)
		return rc;

	return rc;
}

int msm_vidc_session_streamoff(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	int count = 0;
	struct msm_vidc_core *core;
	enum signal_session_response signal_type;
	enum msm_vidc_buffer_type buffer_type;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (port == INPUT_PORT) {
		signal_type = SIGNAL_CMD_STOP_INPUT;
		buffer_type = MSM_VIDC_BUF_INPUT;
	} else if (port == OUTPUT_PORT) {
		signal_type = SIGNAL_CMD_STOP_OUTPUT;
		buffer_type = MSM_VIDC_BUF_OUTPUT;
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_stop(inst, port);
	if (rc)
		goto error;

	core = inst->core;
	i_vpr_h(inst, "%s: wait on port: %d for time: %d ms\n",
		__func__, port, core->capabilities[HW_RESPONSE_TIMEOUT].value);
	mutex_unlock(&inst->lock);
	rc = wait_for_completion_timeout(
			&inst->completions[signal_type],
			msecs_to_jiffies(
			core->capabilities[HW_RESPONSE_TIMEOUT].value));
	if (!rc) {
		i_vpr_e(inst, "%s: session stop timed out for port: %d\n",
				__func__, port);
		rc = -ETIMEDOUT;
		msm_vidc_core_timeout(inst->core);
	} else {
		rc = 0;
	}
	mutex_lock(&inst->lock);

	/* no more queued buffers after streamoff */
	count = msm_vidc_num_buffers(inst, buffer_type, MSM_VIDC_ATTR_QUEUED);
	if (!count) {
		i_vpr_h(inst, "%s: stop successful on port: %d\n",
			__func__, port);
	} else {
		i_vpr_e(inst,
			"%s: %d buffers pending with firmware on port: %d\n",
			__func__, count, port);
		rc = -EINVAL;
		goto error;
	}
	return 0;

error:
	msm_vidc_kill_session(inst);
	msm_vidc_flush_buffers(inst, buffer_type);
	return rc;
}

int msm_vidc_session_close(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = venus_hfi_session_close(inst);
	if (rc)
		return rc;

	core = inst->core;
	i_vpr_h(inst, "%s: wait on close for time: %d ms\n",
		__func__, core->capabilities[HW_RESPONSE_TIMEOUT].value);
	mutex_unlock(&inst->lock);
	rc = wait_for_completion_timeout(
			&inst->completions[SIGNAL_CMD_CLOSE],
			msecs_to_jiffies(
			core->capabilities[HW_RESPONSE_TIMEOUT].value));
	if (!rc) {
		i_vpr_e(inst, "%s: session close timed out\n", __func__);
		rc = -ETIMEDOUT;
		msm_vidc_core_timeout(inst->core);
	} else {
		rc = 0;
		i_vpr_h(inst, "%s: close successful\n", __func__);
	}
	mutex_lock(&inst->lock);

	msm_vidc_remove_session(inst);

	i_vpr_h(inst, "%s: free session packet data\n", __func__);
	kfree(inst->packet);
	inst->packet = NULL;

	return rc;
}

int msm_vidc_kill_session(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!inst->session_id) {
		i_vpr_e(inst, "%s: already killed\n", __func__);
		return 0;
	}

	i_vpr_e(inst, "%s: killing session\n", __func__);
	msm_vidc_session_close(inst);
	msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);

	return 0;
}

int msm_vidc_get_inst_capability(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i;
	struct msm_vidc_core *core;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	for (i = 0; i < core->codecs_count; i++) {
		if (core->inst_caps[i].domain == inst->domain &&
			core->inst_caps[i].codec == inst->codec) {
			i_vpr_h(inst,
				"%s: copied capabilities with %#x codec, %#x domain\n",
				__func__, inst->codec, inst->domain);
			memcpy(inst->capabilities, &core->inst_caps[i],
				sizeof(struct msm_vidc_inst_capability));
		}
	}
	if (!inst->capabilities) {
		i_vpr_e(inst, "%s: capabilities not found\n", __func__);
		return -EINVAL;
	}

	return rc;
}

static int msm_vidc_deinit_core_caps(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s: skip freeing core capabilities\n", __func__);
	//kfree(core->capabilities);
	//core->capabilities = NULL;

	return rc;
}

static int msm_vidc_init_core_caps(struct msm_vidc_core *core)
{
	int rc = 0;
	int i, num_platform_caps;
	struct msm_platform_core_capability *platform_data;

	if (!core || !core->platform) {
		d_vpr_e("%s: invalid params\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	platform_data = core->platform->data.core_data;
	if (!platform_data) {
		d_vpr_e("%s: platform core data is NULL\n",
				__func__);
			rc = -EINVAL;
			goto exit;
	}

	if (!core->capabilities) {
		core->capabilities = kcalloc(1,
			(sizeof(struct msm_vidc_core_capability) *
			CORE_CAP_MAX), GFP_KERNEL);
		if (!core->capabilities) {
			d_vpr_e("%s: failed to allocate core capabilities\n",
				__func__);
			rc = -ENOMEM;
			goto exit;
		}
	} else {
		d_vpr_h("%s: capabilities memory is expected to be freed\n",
			__func__);
	}

	num_platform_caps = core->platform->data.core_data_size;

	/* loop over platform caps */
	for (i = 0; i < num_platform_caps; i++) {
		core->capabilities[platform_data[i].type].type = platform_data[i].type;
		core->capabilities[platform_data[i].type].value = platform_data[i].value;
	}

exit:
	if (rc)
		msm_vidc_deinit_core_caps(core);
	return rc;
}

static void update_inst_capability(struct msm_platform_inst_capability *in,
		struct msm_vidc_inst_capability *capability)
{
	if (!in || !capability) {
		d_vpr_e("%s: invalid params %pK %pK\n",
			__func__, in, capability);
		return;
	}
	if (in->cap < INST_CAP_MAX) {
		capability->cap[in->cap].cap = in->cap;
		capability->cap[in->cap].min = in->min;
		capability->cap[in->cap].max = in->max;
		capability->cap[in->cap].step_or_mask = in->step_or_mask;
		capability->cap[in->cap].value = in->value;
		capability->cap[in->cap].flags = in->flags;
		capability->cap[in->cap].v4l2_id = in->v4l2_id;
		capability->cap[in->cap].hfi_id = in->hfi_id;
		memcpy(capability->cap[in->cap].parents, in->parents,
			sizeof(capability->cap[in->cap].parents));
		memcpy(capability->cap[in->cap].children, in->children,
			sizeof(capability->cap[in->cap].children));
		capability->cap[in->cap].adjust = in->adjust;
		capability->cap[in->cap].set = in->set;
	} else {
		d_vpr_e("%s: invalid cap %d\n",
			__func__, in->cap);
	}
}

static int msm_vidc_deinit_instance_caps(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s: skip freeing core->instance capabilities\n", __func__);
	//kfree(core->inst_caps);
	//core->inst_caps = NULL;

	return rc;
}

static int msm_vidc_init_instance_caps(struct msm_vidc_core *core)
{
	int rc = 0;
	u8 enc_valid_codecs, dec_valid_codecs;
	u8 count_bits, enc_codec_count;
	u8 codecs_count = 0;
	int i, j, check_bit, num_platform_caps;
	struct msm_platform_inst_capability *platform_data = NULL;

	if (!core || !core->platform || !core->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		rc = -EINVAL;
		goto error;
	}

	platform_data = core->platform->data.instance_data;
	if (!platform_data) {
		d_vpr_e("%s: platform instance data is NULL\n",
				__func__);
			rc = -EINVAL;
		goto error;
	}

	enc_valid_codecs = core->capabilities[ENC_CODECS].value;
	count_bits = enc_valid_codecs;
	COUNT_BITS(count_bits, codecs_count);
	enc_codec_count = codecs_count;

	dec_valid_codecs = core->capabilities[DEC_CODECS].value;
	count_bits = dec_valid_codecs;
	COUNT_BITS(count_bits, codecs_count);

	core->codecs_count = codecs_count;

	if (!core->inst_caps) {
		core->inst_caps = kcalloc(codecs_count,
			sizeof(struct msm_vidc_inst_capability),
			GFP_KERNEL);
		if (!core->inst_caps) {
			d_vpr_e("%s: failed to allocate core capabilities\n",
				__func__);
			rc = -ENOMEM;
			goto error;
		}
	} else {
		d_vpr_h("%s: capabilities memory is expected to be freed\n",
			__func__);
	}

	check_bit = 0;
	/* determine codecs for enc domain */
	for (i = 0; i < enc_codec_count; i++) {
		while (check_bit < (sizeof(enc_valid_codecs) * 8)) {
			if (enc_valid_codecs & BIT(check_bit)) {
				core->inst_caps[i].domain = MSM_VIDC_ENCODER;
				core->inst_caps[i].codec = enc_valid_codecs &
						BIT(check_bit);
				check_bit++;
				break;
			}
			check_bit++;
		}
	}

	/* reset checkbit to check from 0th bit of decoder codecs set bits*/
	check_bit = 0;
	/* determine codecs for dec domain */
	for (; i < codecs_count; i++) {
		while (check_bit < (sizeof(dec_valid_codecs) * 8)) {
			if (dec_valid_codecs & BIT(check_bit)) {
				core->inst_caps[i].domain = MSM_VIDC_DECODER;
				core->inst_caps[i].codec = dec_valid_codecs &
						BIT(check_bit);
				check_bit++;
				break;
			}
			check_bit++;
		}
	}

	num_platform_caps = core->platform->data.instance_data_size;

	d_vpr_h("%s: num caps %d\n", __func__, num_platform_caps);
	/* loop over each platform capability */
	for (i = 0; i < num_platform_caps; i++) {
		/* select matching core codec and update it */
		for (j = 0; j < codecs_count; j++) {
			if ((platform_data[i].domain &
				core->inst_caps[j].domain) &&
				(platform_data[i].codec &
				core->inst_caps[j].codec)) {
				/* update core capability */
				update_inst_capability(&platform_data[i],
					&core->inst_caps[j]);
			}
		}
	}

	return 0;
error:
	if (rc)
		msm_vidc_deinit_instance_caps(core);
	return rc;
}

int msm_vidc_core_deinit(struct msm_vidc_core *core, bool force)
{
	int rc = 0;
	struct msm_vidc_inst *inst, *dummy;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core_lock(core, __func__);
	d_vpr_h("%s()\n", __func__);
	if (core->state == MSM_VIDC_CORE_DEINIT)
		goto unlock;
	if (!force)
		if (!list_empty(&core->instances))
			goto unlock;

	venus_hfi_core_deinit(core);
	msm_vidc_deinit_instance_caps(core);
	msm_vidc_deinit_core_caps(core);
	/* unlink all sessions from core, if any */
	list_for_each_entry_safe(inst, dummy, &core->instances, list) {
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		list_del(&inst->list);
	}
	msm_vidc_change_core_state(core, MSM_VIDC_CORE_DEINIT, __func__);

unlock:
	core_unlock(core, __func__);
	return rc;
}

int msm_vidc_core_init(struct msm_vidc_core *core)
{
	int rc = 0;

	if (!core || !core->platform) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core_lock(core, __func__);
	if (core->state == MSM_VIDC_CORE_INIT) {
		rc = 0;
		goto unlock;
	}

	rc = msm_vidc_init_core_caps(core);
	if (rc)
		goto unlock;
	rc = msm_vidc_init_instance_caps(core);
	if (rc)
		goto unlock;

	msm_vidc_change_core_state(core, MSM_VIDC_CORE_INIT, __func__);
	init_completion(&core->init_done);
	core->smmu_fault_handled = false;
	core->ssr.trigger = false;

	rc = venus_hfi_core_init(core);
	if (rc) {
		d_vpr_e("%s: core init failed\n", __func__);
		goto unlock;
	}

	d_vpr_h("%s(): waiting for sys_init_done, %d ms\n", __func__,
		core->capabilities[HW_RESPONSE_TIMEOUT].value);
	core_unlock(core, __func__);
	rc = wait_for_completion_timeout(&core->init_done, msecs_to_jiffies(
			core->capabilities[HW_RESPONSE_TIMEOUT].value));
	core_lock(core, __func__);
	if (!rc) {
		d_vpr_e("%s: core init timed out\n", __func__);
		rc = -ETIMEDOUT;
	} else {
		d_vpr_h("%s: system init wait completed\n", __func__);
		rc = 0;
	}

unlock:
	core_unlock(core, __func__);
	if (rc)
		msm_vidc_core_deinit(core, true);
	return rc;
}

int msm_vidc_core_timeout(struct msm_vidc_core *core)
{
	return msm_vidc_core_deinit(core, true);
}

int msm_vidc_smmu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *data)
{
	return -EINVAL;
}

int msm_vidc_trigger_ssr(struct msm_vidc_core *core,
		enum msm_vidc_ssr_trigger_type type)
{
	return 0;
}

void msm_vidc_ssr_handler(struct work_struct *work)
{
}

void msm_vidc_pm_work_handler(struct work_struct *work)
{
}

void msm_vidc_fw_unload_handler(struct work_struct *work)
{
	struct msm_vidc_core *core = NULL;
	int rc = 0;

	core = container_of(work, struct msm_vidc_core, fw_unload_work.work);
	if (!core) {
		d_vpr_e("%s: invalid work or core handle\n", __func__);
		return;
	}

	d_vpr_h("%s: deinitializing video core\n",__func__);
	rc = msm_vidc_core_deinit(core, false);
	if (rc)
		d_vpr_e("%s: Failed to deinit core\n", __func__);

}

void msm_vidc_batch_handler(struct work_struct *work)
{
}

int msm_vidc_flush_buffers(struct msm_vidc_inst* inst,
		enum msm_vidc_buffer_type type)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf, *dummy;
	enum msm_vidc_buffer_type buffer_type[2];
	int i;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (type == MSM_VIDC_BUF_INPUT) {
		buffer_type[0] = MSM_VIDC_BUF_INPUT_META;
		buffer_type[1] = MSM_VIDC_BUF_INPUT;
	} else if (type == MSM_VIDC_BUF_OUTPUT) {
		buffer_type[0] = MSM_VIDC_BUF_OUTPUT_META;
		buffer_type[1] = MSM_VIDC_BUF_OUTPUT;
	} else {
		i_vpr_h(inst, "%s: invalid buffer type %d\n",
				__func__, type);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(buffer_type); i++) {
		buffers = msm_vidc_get_buffers(inst, buffer_type[i], __func__);
		if (!buffers)
			return -EINVAL;

		list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
			if (buf->attr & MSM_VIDC_ATTR_QUEUED ||
				buf->attr & MSM_VIDC_ATTR_DEFERRED) {
				print_vidc_buffer(VIDC_ERR, "flushing buffer", inst, buf);
				msm_vidc_vb2_buffer_done(inst, buf);
				msm_vidc_put_driver_buf(inst, buf);
			}
		}
	}

	return rc;
}

void msm_vidc_destroy_buffers(struct msm_vidc_inst *inst)
{
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf, *dummy;
	enum msm_vidc_buffer_type buf_types[] = {
		MSM_VIDC_BUF_INPUT,
		MSM_VIDC_BUF_OUTPUT,
		MSM_VIDC_BUF_INPUT_META,
		MSM_VIDC_BUF_OUTPUT_META,
		MSM_VIDC_BUF_BIN,
		MSM_VIDC_BUF_ARP,
		MSM_VIDC_BUF_COMV,
		MSM_VIDC_BUF_NON_COMV,
		MSM_VIDC_BUF_LINE,
		MSM_VIDC_BUF_DPB,
		MSM_VIDC_BUF_PERSIST,
		MSM_VIDC_BUF_VPSS,
	};
	int i;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(buf_types); i++) {
		buffers = msm_vidc_get_buffers(inst, buf_types[i], __func__);
		if (!buffers)
			continue;
		list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
			i_vpr_h(inst,
				"destroying buffer: type %d idx %d fd %d addr %#x size %d\n",
				buf->type, buf->index, buf->fd, buf->device_addr, buf->buffer_size);
			if (is_internal_buffer(buf->type))
				msm_vidc_destroy_internal_buffer(inst, buf);
			else
				msm_vidc_put_driver_buf(inst, buf);
		}
	}
}

static void msm_vidc_close_helper(struct kref *kref)
{
	struct msm_vidc_inst *inst = container_of(kref,
		struct msm_vidc_inst, kref);

	i_vpr_h(inst, "%s()\n", __func__);
	msm_vidc_event_queue_deinit(inst);
	msm_vidc_vb2_queue_deinit(inst);
	msm_vidc_debugfs_deinit_inst(inst);
	if (is_decode_session(inst))
		msm_vdec_inst_deinit(inst);
	else if (is_encode_session(inst))
		msm_venc_inst_deinit(inst);
	kfree(inst->capabilities);
	if (inst->response_workq)
		destroy_workqueue(inst->response_workq);
	kfree(inst);
}

struct msm_vidc_inst *get_inst_ref(struct msm_vidc_core *core,
		struct msm_vidc_inst *instance)
{
	struct msm_vidc_inst *inst = NULL;
	bool matches = false;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		if (inst == instance) {
			matches = true;
			break;
		}
	}
	inst = (matches && kref_get_unless_zero(&inst->kref)) ? inst : NULL;
	mutex_unlock(&core->lock);
	return inst;
}

struct msm_vidc_inst *get_inst(struct msm_vidc_core *core,
		u32 session_id)
{
	struct msm_vidc_inst *inst = NULL;
	bool matches = false;

	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	mutex_lock(&core->lock);
	list_for_each_entry(inst, &core->instances, list) {
		if (inst->session_id == session_id) {
			matches = true;
			break;
		}
	}
	inst = (matches && kref_get_unless_zero(&inst->kref)) ? inst : NULL;
	mutex_unlock(&core->lock);
	return inst;
}

void put_inst(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	kref_put(&inst->kref, msm_vidc_close_helper);
}

bool core_lock_check(struct msm_vidc_core *core, const char* func)
{
	return mutex_is_locked(&core->lock);
}

void core_lock(struct msm_vidc_core *core, const char *function)
{
	mutex_lock(&core->lock);
}

void core_unlock(struct msm_vidc_core *core, const char *function)
{
	mutex_unlock(&core->lock);
}

bool inst_lock_check(struct msm_vidc_inst *inst, const char* func)
{
	return mutex_is_locked(&inst->lock);
}

void inst_lock(struct msm_vidc_inst *inst, const char *function)
{
	mutex_lock(&inst->lock);
}

void inst_unlock(struct msm_vidc_inst *inst, const char *function)
{
	mutex_unlock(&inst->lock);
}

int msm_vidc_update_meta_port_settings(struct msm_vidc_inst *inst)
{
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[INPUT_META_PORT];
	if (is_input_meta_enabled(inst)) {
		fmt->fmt.meta.buffersize = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_INPUT_META);
		inst->buffers.input_meta.min_count =
				inst->buffers.input.min_count;
		inst->buffers.input_meta.extra_count =
				inst->buffers.input.extra_count;
		inst->buffers.input_meta.actual_count =
				inst->buffers.input.actual_count;
		inst->buffers.input_meta.size = fmt->fmt.meta.buffersize;
	} else {
		fmt->fmt.meta.buffersize = 0;
		inst->buffers.input_meta.min_count = 0;
		inst->buffers.input_meta.extra_count = 0;
		inst->buffers.input_meta.actual_count = 0;
		inst->buffers.input_meta.size = 0;
	}

	fmt = &inst->fmts[OUTPUT_META_PORT];
	if (is_output_meta_enabled(inst)) {
		fmt->fmt.meta.buffersize = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_OUTPUT_META);
		inst->buffers.output_meta.min_count =
				inst->buffers.output.min_count;
		inst->buffers.output_meta.extra_count =
				inst->buffers.output.extra_count;
		inst->buffers.output_meta.actual_count =
				inst->buffers.output.actual_count;
		inst->buffers.output_meta.size = fmt->fmt.meta.buffersize;
	} else {
		fmt->fmt.meta.buffersize = 0;
		inst->buffers.output_meta.min_count = 0;
		inst->buffers.output_meta.extra_count = 0;
		inst->buffers.output_meta.actual_count = 0;
		inst->buffers.output_meta.size = 0;
	}
	return 0;
}

void msm_vidc_schedule_core_deinit(struct msm_vidc_core *core)
{
	if (!core)
		return;

	if (!core->capabilities[FW_UNLOAD].value)
		return;

	cancel_delayed_work(&core->fw_unload_work);

	schedule_delayed_work(&core->fw_unload_work,
		msecs_to_jiffies(core->capabilities[FW_UNLOAD_DELAY].value));

	d_vpr_h("firmware unload delayed by %u ms\n",
		core->capabilities[FW_UNLOAD_DELAY].value);

	return;
}

