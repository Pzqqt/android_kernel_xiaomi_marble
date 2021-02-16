// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/v4l2_vidc_extensions.h>
#include "msm_media_info.h"

#include "msm_venc.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_control.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"
#include "hfi_packet.h"

u32 msm_venc_input_set_prop[] = {
	HFI_PROP_COLOR_FORMAT,
	HFI_PROP_RAW_RESOLUTION,
	HFI_PROP_LINEAR_STRIDE_SCANLINE,
	HFI_PROP_BUFFER_HOST_MAX_COUNT,
};

u32 msm_venc_output_set_prop[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_BUFFER_HOST_MAX_COUNT,
};

u32 msm_venc_input_subscribe_for_properties[] = {
	HFI_PROP_NO_OUTPUT,
};

u32 msm_venc_output_subscribe_for_properties[] = {
	HFI_PROP_PICTURE_TYPE,
	HFI_PROP_BUFFER_MARK,
};

static int msm_venc_codec_change(struct msm_vidc_inst *inst, u32 v4l2_codec)
{
	int rc = 0;

	if (inst->codec && inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat == v4l2_codec)
		return 0;

	i_vpr_h(inst, "%s: codec changed from %#x to %#x\n",
		__func__, inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, v4l2_codec);

	inst->codec = v4l2_codec_to_driver(v4l2_codec, __func__);
	rc = msm_vidc_get_inst_capability(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_deinit(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_init(inst);
	if (rc)
		goto exit;

exit:
	return rc;
}

/* todo: add logs for each property once finalised */
static int msm_venc_set_colorformat(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 pixelformat;
	enum msm_vidc_colorformat_type colorformat;
	u32 hfi_colorformat;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	pixelformat = inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat;
	colorformat = v4l2_colorformat_to_driver(pixelformat, __func__);
	if (!(colorformat & inst->capabilities->cap[PIX_FMTS].step_or_mask)) {
		i_vpr_e(inst, "%s: invalid pixelformat %#x\n",
			__func__, pixelformat);
		return -EINVAL;
	}

	hfi_colorformat = get_hfi_colorformat(inst, colorformat);
	i_vpr_h(inst, "%s: hfi colorformat: %#x", __func__,
		hfi_colorformat);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_COLOR_FORMAT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&hfi_colorformat,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_linear_alignment_factor(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 pixelformat, stride_y, scanline_y, stride_uv, scanline_uv;
	u32 payload[2];

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	pixelformat = inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat;
	if (pixelformat != V4L2_PIX_FMT_NV12 &&
		pixelformat != V4L2_PIX_FMT_VIDC_P010) {
		i_vpr_e(inst,
			"%s: not a linear color fmt, property is not set\n",
			__func__);
		return 0;
	}

	stride_y = inst->fmts[INPUT_PORT].fmt.pix_mp.width;
	scanline_y = inst->fmts[INPUT_PORT].fmt.pix_mp.height;
	stride_uv = stride_y;
	scanline_uv = scanline_y / 2;

	payload[0] = stride_y << 16 | scanline_y;
	payload[1] = stride_uv << 16 | scanline_uv;
	i_vpr_h(inst, "%s: stride_y: %d scanline_y: %d "
		"stride_uv: %d, scanline_uv: %d", __func__,
		stride_y, scanline_y, stride_uv, scanline_uv);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LINEAR_STRIDE_SCANLINE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_64_PACKED,
			&payload,
			sizeof(u64));
	if (rc)
		return rc;

	return 0;
}

static int msm_venc_set_raw_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 resolution;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	resolution = inst->crop.width << 16 | inst->crop.height;
	i_vpr_h(inst, "%s: width: %d height: %d\n", __func__,
			inst->crop.width, inst->crop.height);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_RAW_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&resolution,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_bitstream_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 resolution;

	if (port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	resolution = (inst->fmts[port].fmt.pix_mp.width << 16) |
		inst->fmts[port].fmt.pix_mp.height;
	i_vpr_h(inst, "%s: width: %d height: %d\n", __func__,
			inst->fmts[port].fmt.pix_mp.width,
			inst->fmts[port].fmt.pix_mp.height);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BITSTREAM_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&resolution,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_crop_offsets(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 left_offset, top_offset, right_offset, bottom_offset;
	u64 crop;

	if (port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	left_offset = inst->crop.left;
	top_offset = inst->crop.top;
	right_offset = (inst->fmts[port].fmt.pix_mp.width -
		inst->crop.width);
	bottom_offset = (inst->fmts[port].fmt.pix_mp.height -
		inst->crop.height);

	crop = (u64)right_offset << 48 | (u64)bottom_offset << 32 |
		(u64)left_offset << 16 | top_offset;
	i_vpr_h(inst, "%s: left_offset: %d top_offset: %d "
		"right_offset: %d bottom_offset: %d", __func__,
		left_offset, top_offset, right_offset, bottom_offset);

	rc = venus_hfi_session_property(inst,
			HFI_PROP_CROP_OFFSETS,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_64_PACKED,
			&crop,
			sizeof(u64));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_host_max_buf_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 count = DEFAULT_MAX_HOST_BUF_COUNT;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	i_vpr_h(inst, "%s: count: %u port: %u\n", __func__, count, port);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BUFFER_HOST_MAX_COUNT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&count,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_stage(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core = inst->core;
	struct msm_vidc_inst_capability *capability = inst->capabilities;
	u32 stage;

	rc = call_session_op(core, decide_work_mode, inst);
	if (rc) {
		i_vpr_e(inst, "%s: decide_work_mode failed\n",
			__func__);
		return -EINVAL;
	}

	stage = capability->cap[STAGE].value;
	i_vpr_h(inst, "%s: stage: %u\n", __func__, stage);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_STAGE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&stage,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_pipe(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core = inst->core;
	struct msm_vidc_inst_capability *capability = inst->capabilities;
	u32 pipe;

	rc = call_session_op(core, decide_work_route, inst);
	if (rc) {
		i_vpr_e(inst, "%s: decide_work_route failed\n",
			__func__);
		return -EINVAL;
	}

	pipe = capability->cap[PIPE].value;
	i_vpr_h(inst, "%s: pipe: %u\n", __func__, pipe);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIPE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&pipe,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_quality_mode(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core* core = inst->core;
	struct msm_vidc_inst_capability *capability = inst->capabilities;
	u32 mode;

	rc = call_session_op(core, decide_quality_mode, inst);
	if (rc) {
		i_vpr_e(inst, "%s: decide_work_route failed\n",
			__func__);
		return -EINVAL;
	}

	mode = capability->cap[QUALITY_MODE].value;
	i_vpr_h(inst, "%s: quality_mode: %u\n", __func__, mode);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_QUALITY_MODE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_BITSTREAM,
			HFI_PAYLOAD_U32_ENUM,
			&mode,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_input_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_input_set_prop);
	     i++) {
		switch (msm_venc_input_set_prop[i]) {
		case HFI_PROP_COLOR_FORMAT:
			rc = msm_venc_set_colorformat(inst, INPUT_PORT);
			break;
		case HFI_PROP_RAW_RESOLUTION:
			rc = msm_venc_set_raw_resolution(inst, INPUT_PORT);
			break;
		case HFI_PROP_LINEAR_STRIDE_SCANLINE:
			rc = msm_venc_set_linear_alignment_factor(inst, INPUT_PORT);
			break;
		case HFI_PROP_BUFFER_HOST_MAX_COUNT:
			rc = msm_venc_set_host_max_buf_count(inst, INPUT_PORT);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_venc_input_set_prop[i]);
			rc = -EINVAL;
			break;
		}

		if (rc)
			goto exit;
	}

exit:
	return rc;
}

static int msm_venc_set_output_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_output_set_prop);
	     i++) {
		switch (msm_venc_output_set_prop[i]) {
		case HFI_PROP_BITSTREAM_RESOLUTION:
			rc = msm_venc_set_bitstream_resolution(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_CROP_OFFSETS:
			rc = msm_venc_set_crop_offsets(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_BUFFER_HOST_MAX_COUNT:
			rc = msm_venc_set_host_max_buf_count(inst, OUTPUT_PORT);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_venc_output_set_prop[i]);
			rc = -EINVAL;
			break;
		}

		if (rc)
			goto exit;
	}

exit:
	return rc;
}

static int msm_venc_set_internal_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	//TODO: set HFI_PORT_NONE properties at master port streamon.
	rc = msm_venc_set_stage(inst);
	if (rc)
		return rc;

	rc = msm_venc_set_pipe(inst);
	if (rc)
		return rc;

	rc = msm_venc_set_quality_mode(inst);
	if (rc)
		return rc;

	return rc;
}

static int msm_venc_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
/* TODO: VPSS
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	inst->buffers.vpss.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_VPSS) + 100000000;

	inst->buffers.dpb.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_VPSS);

	i_vpr_h(inst, "%s: internal buffer: min     size\n", __func__);
	i_vpr_h(inst, "vpss  buffer: %d      %d\n",
		inst->buffers.vpss.min_count,
		inst->buffers.vpss.size);
*/
	return rc;
}

static int msm_venc_create_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
/* TODO: VPSS
	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_VPSS);
	if (rc)
		return rc;
*/
	return rc;
}

static int msm_venc_queue_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

/* TODO: VPSS
	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_VPSS);
	if (rc)
		return rc;
*/
	return rc;
}

static int msm_venc_get_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;

	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;

	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;

	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;

	rc = msm_vidc_get_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	i_vpr_h(inst, "internal buffer: min     size\n");
	i_vpr_h(inst, "bin  buffer: %d      %d\n",
		inst->buffers.bin.min_count,
		inst->buffers.bin.size);
	i_vpr_h(inst, "comv  buffer: %d      %d\n",
		inst->buffers.comv.min_count,
		inst->buffers.comv.size);
	i_vpr_h(inst, "non_comv  buffer: %d      %d\n",
		inst->buffers.non_comv.min_count,
		inst->buffers.non_comv.size);
	i_vpr_h(inst, "line buffer: %d      %d\n",
		inst->buffers.line.min_count,
		inst->buffers.line.size);
	i_vpr_h(inst, "dpb buffer: %d      %d\n",
		inst->buffers.dpb.min_count,
		inst->buffers.dpb.size);

	return rc;
}

static int msm_venc_create_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	return 0;
}

static int msm_venc_queue_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;

	return 0;
}

static int msm_venc_property_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;
	u32 payload_size = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_PROPERTY;
	if (port == INPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(msm_venc_input_subscribe_for_properties); i++)
			payload[i + 1] = msm_venc_input_subscribe_for_properties[i];
		payload_size = (ARRAY_SIZE(msm_venc_input_subscribe_for_properties) + 1) *
				sizeof(u32);
	} else if (port == OUTPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(msm_venc_output_subscribe_for_properties); i++)
			payload[i + 1] = msm_venc_output_subscribe_for_properties[i];
		payload_size = (ARRAY_SIZE(msm_venc_output_subscribe_for_properties) + 1) *
				sizeof(u32);
	} else {
		i_vpr_e(inst, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			payload_size);

	return rc;
}

static int msm_venc_metadata_delivery(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i, count = 0;
	struct msm_vidc_inst_capability *capability;
	u32 metadata_list[] = {
		META_SEI_MASTERING_DISP,
		META_SEI_CLL,
		META_HDR10PLUS,
		META_EVA_STATS,
		META_BUF_TAG,
		META_ROI_INFO,
	};

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(metadata_list); i++) {
		if (capability->cap[metadata_list[i]].value) {
			payload[count + 1] =
				capability->cap[metadata_list[i]].hfi_id;
			count++;
		}
	};

	if (!count)
		return 0;

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));

	return rc;
}

static int msm_venc_metadata_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i, count = 0;
	struct msm_vidc_inst_capability *capability;
	u32 metadata_list[] = {
		META_LTR_MARK_USE,
		META_SEQ_HDR_NAL,
		META_TIMESTAMP,
		META_BUF_TAG,
		META_SUBFRAME_OUTPUT,
		META_ENC_QP_METADATA,
	};

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(metadata_list); i++) {
		if (capability->cap[metadata_list[i]].value) {
			payload[count + 1] =
				capability->cap[metadata_list[i]].hfi_id;
			count++;
		}
	};

	if (!count)
		return 0;

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));

	return rc;
}

int msm_venc_streamoff_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_streamoff(inst, INPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_venc_streamon_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_input_meta_enabled(inst) &&
		!inst->vb2q[INPUT_META_PORT].streaming) {
		i_vpr_e(inst,
			"%s: Meta port must be streamed on before data port\n",
			__func__);
		return -EINVAL;
	}

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;
	//rc = msm_vidc_check_scaling_supported(inst);
	if (rc)
		goto error;

	rc = msm_venc_set_input_properties(inst);
	if (rc)
		goto error;

	/* Decide bse vpp delay after work mode */
	//msm_vidc_set_bse_vpp_delay(inst);

	rc = msm_venc_get_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_venc_create_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_venc_queue_input_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_venc_property_subscription(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_venc_metadata_delivery(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vidc_session_streamon(inst, INPUT_PORT);
	if (rc)
		goto error;

	return 0;

error:
	i_vpr_e(inst, "%s: failed\n", __func__);
	msm_venc_streamoff_input(inst);
	return rc;
}

int msm_venc_qbuf(struct msm_vidc_inst *inst, struct vb2_buffer *vb2)
{
	int rc = 0;

	rc = msm_vidc_queue_buffer(inst, vb2);
	if (rc)
		return rc;

	return rc;
}

int msm_venc_process_cmd(struct msm_vidc_inst *inst, u32 cmd)
{
	int rc = 0;
	enum msm_vidc_allow allow = MSM_VIDC_DISALLOW;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (cmd == V4L2_ENC_CMD_STOP) {
		allow = msm_vidc_allow_stop(inst);
		if (allow == MSM_VIDC_DISALLOW)
			return -EBUSY;
		else if (allow == MSM_VIDC_IGNORE)
			return 0;
		else if (allow != MSM_VIDC_ALLOW)
			return -EINVAL;
		rc = venus_hfi_session_command(inst,
				HFI_CMD_DRAIN,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
		rc = msm_vidc_state_change_stop(inst);
		if (rc)
			return rc;
	} else if (cmd == V4L2_ENC_CMD_START) {
		if (!msm_vidc_allow_start(inst))
			return -EBUSY;
		vb2_clear_last_buffer_dequeued(&inst->vb2q[OUTPUT_META_PORT]);
		vb2_clear_last_buffer_dequeued(&inst->vb2q[OUTPUT_PORT]);
		rc = msm_vidc_state_change_start(inst);
		if (rc)
			return rc;
		rc = venus_hfi_session_command(inst,
				HFI_CMD_RESUME,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
	} else {
		d_vpr_e("%s: unknown cmd %d\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}

int msm_venc_streamoff_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_streamoff(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_venc_streamon_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_output_meta_enabled(inst) &&
		!inst->vb2q[OUTPUT_META_PORT].streaming) {
		i_vpr_e(inst,
			"%s: Meta port must be streamed on before data port\n",
			__func__);
		return -EINVAL;
	}

	rc = msm_venc_set_output_properties(inst);
	if (rc)
		goto error;

	rc = msm_venc_get_output_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_venc_create_output_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_venc_queue_output_internal_buffers(inst);
	if (rc)
		goto error;

	rc = msm_vidc_adjust_v4l2_properties(inst);
	if (rc)
		goto error;

	rc = msm_vidc_set_v4l2_properties(inst);
	if (rc)
		goto error;

	rc = msm_venc_set_internal_properties(inst);
	if (rc)
		goto error;

	rc = msm_venc_property_subscription(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	rc = msm_venc_metadata_subscription(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	rc = msm_vidc_session_streamon(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	return 0;

error:
	i_vpr_e(inst, "%s: failed\n", __func__);
	msm_venc_streamoff_output(inst);
	return rc;
}

static int msm_venc_s_fmt_input(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_format *fmt;
	struct msm_vidc_core *core;
	u32 codec_align;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[INPUT_PORT];
	fmt->type = INPUT_MPLANE;
	fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
	fmt->fmt.pix_mp.width = VENUS_Y_STRIDE(
		v4l2_colorformat_to_media(fmt->fmt.pix_mp.pixelformat, __func__),
		f->fmt.pix_mp.width);
	fmt->fmt.pix_mp.height = VENUS_Y_SCANLINES(
		v4l2_colorformat_to_media(fmt->fmt.pix_mp.pixelformat, __func__),
		f->fmt.pix_mp.height);

	fmt->fmt.pix_mp.num_planes = 1;
	fmt->fmt.pix_mp.plane_fmt[0].bytesperline =
		fmt->fmt.pix_mp.width;
	fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_INPUT);
	if (inst->buffers.input.actual_count <
		inst->buffers.input.min_count +
		inst->buffers.input.extra_count) {
		inst->buffers.input.actual_count =
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
	}
	inst->buffers.input.size =
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage;

	codec_align = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat ==
		V4L2_PIX_FMT_HEVC ? 32 : 16;

	/* check if resolution changed */
	if (inst->fmts[OUTPUT_PORT].fmt.pix_mp.width >
		ALIGN(f->fmt.pix_mp.width, codec_align) ||
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.height >
		ALIGN(f->fmt.pix_mp.height, codec_align)) {
		/* reset bitstream port with updated resolution */
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.width =
			ALIGN(f->fmt.pix_mp.width, codec_align);
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.height =
			ALIGN(f->fmt.pix_mp.height, codec_align);
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.plane_fmt[0].sizeimage =
			call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_OUTPUT);

		/* reset crop dimensions with updated resolution */
		inst->crop.top = inst->crop.left = 0;
		inst->crop.width = f->fmt.pix_mp.width;
		inst->crop.height = f->fmt.pix_mp.height;

		/* reset compose dimensions with updated resolution */
		inst->compose.top = inst->crop.left = 0;
		inst->compose.width = f->fmt.pix_mp.width;
		inst->compose.height = f->fmt.pix_mp.height;
	}

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		return rc;
	//update_log_ctxt(inst->sid, inst->session_type,
	//	mplane->pixelformat);
	i_vpr_h(inst,
		"%s: input: codec %#x width %d height %d size %d min_count %d extra_count %d\n",
		__func__, f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width,
		f->fmt.pix_mp.height,
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
		inst->buffers.input.min_count,
		inst->buffers.input.extra_count);

	//msm_vidc_update_dcvs(inst);
	//msm_vidc_update_batching(inst);

	memcpy(f, fmt, sizeof(struct v4l2_format));

	return rc;
}

static int msm_venc_s_fmt_input_meta(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_format *fmt;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[INPUT_META_PORT];
	fmt->type = INPUT_META_PLANE;
	fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	if (is_input_meta_enabled(inst)) {
		fmt->fmt.meta.buffersize = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_OUTPUT_META);
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
	i_vpr_h(inst,
		"%s: input meta: size %d min_count %d extra_count %d\n",
		__func__, fmt->fmt.meta.buffersize,
		inst->buffers.input_meta.min_count,
		inst->buffers.input_meta.extra_count);

	memcpy(f, fmt, sizeof(struct v4l2_format));
	return rc;
}

static int msm_venc_s_fmt_output(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_format *fmt;
	struct msm_vidc_core *core;
	u32 codec_align;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[OUTPUT_PORT];
	if (fmt->fmt.pix_mp.pixelformat != f->fmt.pix_mp.pixelformat) {
		i_vpr_h(inst,
			"%s: codec changed from %#x to %#x\n", __func__,
			fmt->fmt.pix_mp.pixelformat, f->fmt.pix_mp.pixelformat);
		rc = msm_venc_codec_change(inst, f->fmt.pix_mp.pixelformat);
		if (rc)
			return rc;
	}
	fmt->type = OUTPUT_MPLANE;

	codec_align = f->fmt.pix_mp.pixelformat ==
		V4L2_PIX_FMT_HEVC ? 32 : 16;
	/* width, height is readonly for client */
	fmt->fmt.pix_mp.width = ALIGN(inst->crop.width, codec_align);
	fmt->fmt.pix_mp.height = ALIGN(inst->crop.height, codec_align);
	fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
	fmt->fmt.pix_mp.num_planes = 1;
	fmt->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	if (inst->buffers.output.actual_count <
		inst->buffers.output.min_count +
		inst->buffers.output.extra_count) {
		inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	}
	inst->buffers.output.size =
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage;

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		return rc;

	//update_log_ctxt(inst->sid, inst->session_type,
	//	mplane->pixelformat);

	i_vpr_h(inst,
		"%s: output: format %#x width %d height %d size %d min_count %d extra_count %d\n",
		__func__, fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.width,
		fmt->fmt.pix_mp.height,
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
		inst->buffers.output.min_count,
		inst->buffers.output.extra_count);

	memcpy(f, fmt, sizeof(struct v4l2_format));

	return rc;
}

static int msm_venc_s_fmt_output_meta(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_format *fmt;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[OUTPUT_META_PORT];
	fmt->type = OUTPUT_META_PLANE;
	fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
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
	i_vpr_h(inst,
		"%s: output meta: size %d min_count %d extra_count %d\n",
		__func__, fmt->fmt.meta.buffersize,
		inst->buffers.output_meta.min_count,
		inst->buffers.output_meta.extra_count);

	memcpy(f, fmt, sizeof(struct v4l2_format));
	return rc;
}

// TODO: use PIX_FMTS caps to check supported color format
int msm_venc_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (f->type == INPUT_MPLANE) {
		rc = msm_venc_s_fmt_input(inst, f);
		if (rc)
			return rc;
	} else if (f->type == INPUT_META_PLANE) {
		rc = msm_venc_s_fmt_input_meta(inst, f);
		if (rc)
			return rc;
	} else if (f->type == OUTPUT_MPLANE) {
		rc = msm_venc_s_fmt_output(inst, f);
		if (rc)
			return rc;
	} else if (f->type == OUTPUT_META_PLANE) {
		rc = msm_venc_s_fmt_output_meta(inst, f);
		if (rc)
			return rc;
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, f->type);
		return rc;
	}

	return rc;
}

int msm_venc_g_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	int port;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	port = v4l2_type_to_driver_port(inst, f->type, __func__);
	if (port < 0)
		return -EINVAL;

	memcpy(f, &inst->fmts[port], sizeof(struct v4l2_format));

	return rc;
}

int msm_venc_s_selection(struct msm_vidc_inst* inst, struct v4l2_selection* s)
{
	int rc = 0;

	if (!inst || !s) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (s->type != INPUT_MPLANE && s->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, s->type);
		return -EINVAL;
	}

	switch (s->target) {
	case V4L2_SEL_TGT_CROP_BOUNDS:
	case V4L2_SEL_TGT_CROP_DEFAULT:
	case V4L2_SEL_TGT_CROP:
		if (s->r.left || s->r.top) {
			i_vpr_h(inst, "%s: unsupported top %d or left %d\n",
				__func__, s->r.left, s->r.top);
			s->r.left = s->r.top = 0;
		}
		if (s->r.width > inst->fmts[INPUT_PORT].fmt.pix_mp.width) {
			i_vpr_h(inst, "%s: unsupported width %d, fmt width %d\n",
				__func__, s->r.width,
				inst->fmts[INPUT_PORT].fmt.pix_mp.width);
			s->r.width = inst->fmts[INPUT_PORT].fmt.pix_mp.width;
		}
		if (s->r.height > inst->fmts[INPUT_PORT].fmt.pix_mp.height) {
			i_vpr_h(inst, "%s: unsupported height %d, fmt height %d\n",
				__func__, s->r.height,
				inst->fmts[INPUT_PORT].fmt.pix_mp.height);
			s->r.height = inst->fmts[INPUT_PORT].fmt.pix_mp.height;
		}

		inst->crop.left = s->r.left;
		inst->crop.top = s->r.top;
		inst->crop.width = s->r.width;
		inst->crop.height = s->r.height;
		/* adjust compose such that it is within crop */
		if (inst->compose.left < inst->crop.left)
			inst->compose.left = inst->crop.left;
		if (inst->compose.top < inst->crop.top)
			inst->compose.top = inst->crop.top;
		if (inst->compose.width > inst->crop.width)
			inst->compose.width = inst->crop.width;
		if (inst->compose.height > inst->crop.height)
			inst->compose.height = inst->crop.height;
		/* update output format based on new crop dimensions */
		rc = msm_venc_s_fmt_output(inst, &inst->fmts[OUTPUT_PORT]);
		if (rc)
			return rc;
		break;
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
	case V4L2_SEL_TGT_COMPOSE_PADDED:
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_COMPOSE:
		if (s->r.left < inst->crop.left) {
			i_vpr_e(inst,
				"%s: compose left (%d) less than crop left (%d)\n",
				__func__, s->r.left, inst->crop.left);
			s->r.left = inst->crop.left;
		}
		if (s->r.top < inst->crop.top) {
			i_vpr_e(inst,
				"%s: compose top (%d) less than crop top (%d)\n",
				__func__, s->r.top, inst->crop.top);
			s->r.top = inst->crop.top;
		}
		if (s->r.width > inst->crop.width) {
			i_vpr_e(inst,
				"%s: compose width (%d) greate than crop width (%d)\n",
				__func__, s->r.width, inst->crop.width);
			s->r.width = inst->crop.width;
		}
		if (s->r.height > inst->crop.height) {
			i_vpr_e(inst,
				"%s: compose height (%d) greate than crop height (%d)\n",
				__func__, s->r.height, inst->crop.height);
			s->r.height = inst->crop.height;
		}
		inst->compose.left = s->r.left;
		inst->compose.top = s->r.top;
		inst->compose.width = s->r.width;
		inst->compose.height= s->r.height;
		break;
	default:
		i_vpr_e(inst, "%s: invalid target %d\n",
				__func__, s->target);
		rc = -EINVAL;
		break;
	}
	if (!rc)
		i_vpr_h(inst, "%s: target %d, r [%d, %d, %d, %d]\n",
			__func__, s->target, s->r.top, s->r.left,
			s->r.width, s->r.height);
	return rc;
}

int msm_venc_g_selection(struct msm_vidc_inst* inst, struct v4l2_selection* s)
{
	int rc = 0;

	if (!inst || !s) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (s->type != INPUT_MPLANE && s->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, s->type);
		return -EINVAL;
	}

	switch (s->target) {
	case V4L2_SEL_TGT_CROP_BOUNDS:
	case V4L2_SEL_TGT_CROP_DEFAULT:
	case V4L2_SEL_TGT_CROP:
		s->r.left = inst->crop.left;
		s->r.top = inst->crop.top;
		s->r.width = inst->crop.width;
		s->r.height = inst->crop.height;
		break;
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
	case V4L2_SEL_TGT_COMPOSE_PADDED:
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_COMPOSE:
		s->r.left = inst->compose.left;
		s->r.top = inst->compose.top;
		s->r.width = inst->compose.width;
		s->r.height = inst->compose.height;
		break;
	default:
		i_vpr_e(inst, "%s: invalid target %d\n",
			__func__, s->target);
		rc = -EINVAL;
		break;
	}
	if (!rc)
		i_vpr_h(inst, "%s: target %d, r [%d, %d, %d, %d]\n",
			__func__, s->target, s->r.top, s->r.left,
			s->r.width, s->r.height);
	return rc;
}

int msm_venc_s_param(struct msm_vidc_inst *inst,
		struct v4l2_streamparm *s_parm)
{
	int rc = 0;
	struct msm_vidc_inst_capability *capability = NULL;
	struct v4l2_fract *timeperframe = NULL;
	u32 q16_rate, max_rate, default_rate;
	u64 us_per_frame = 0, input_rate = 0;
	bool is_frame_rate = false;

	if (!inst || !s_parm) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (s_parm->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		timeperframe = &s_parm->parm.output.timeperframe;
		max_rate = capability->cap[OPERATING_RATE].max;
		default_rate = capability->cap[OPERATING_RATE].value;
	} else {
		timeperframe = &s_parm->parm.capture.timeperframe;
		is_frame_rate = true;
		max_rate = capability->cap[FRAME_RATE].value;
		default_rate = capability->cap[FRAME_RATE].value;
	}

	if (!timeperframe->denominator || !timeperframe->numerator) {
		i_vpr_e(inst,
			"%s: invalid rate for type %u\n",
			__func__, s_parm->type);
		input_rate = default_rate >> 16;
		goto set_default;
	}

	us_per_frame = timeperframe->numerator * (u64)USEC_PER_SEC;
	do_div(us_per_frame, timeperframe->denominator);

	if (!us_per_frame) {
		i_vpr_e(inst, "%s: us_per_frame is zero\n",
			__func__);
		rc = -EINVAL;
		goto exit;
	}

	input_rate = (u64)USEC_PER_SEC;
	do_div(input_rate, us_per_frame);

	/* Check max allowed rate */
	if (input_rate > max_rate) {
		i_vpr_e(inst,
			"%s: Unsupported rate %u, max_fps %u, type: %u\n",
			__func__, input_rate, max_rate, s_parm->type);
		rc = -ENOTSUPP;
		goto exit;
	}

set_default:
	q16_rate = (u32)input_rate << 16;
	i_vpr_h(inst, "%s: type %u value %#x\n",
		__func__, s_parm->type, q16_rate);

	if (!is_frame_rate) {
		capability->cap[OPERATING_RATE].value = q16_rate;
		goto exit;
	} else {
		capability->cap[FRAME_RATE].value = q16_rate;
	}

	/*
	 * In static case, frame rate is set during via
	 * inst database set function mentioned in
	 * FRAME_RATE cap id.
	 * In dynamic case, frame rate is set like below.
	 */
	if (inst->vb2q[OUTPUT_PORT].streaming) {
		rc = venus_hfi_session_property(inst,
			HFI_PROP_FRAME_RATE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_BITSTREAM,
			HFI_PAYLOAD_Q16,
			&q16_rate,
			sizeof(u32));
		if (rc) {
			i_vpr_e(inst,
				"%s: failed to set frame rate to fw\n",
				__func__);
			goto exit;
		}
	}

exit:
	return rc;
}

int msm_venc_g_param(struct msm_vidc_inst *inst,
		struct v4l2_streamparm *s_parm)
{
	struct msm_vidc_inst_capability *capability = NULL;
	struct v4l2_fract *timeperframe = NULL;

	if (!inst || !s_parm) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	capability = inst->capabilities;

	if (s_parm->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		timeperframe = &s_parm->parm.output.timeperframe;
		timeperframe->numerator = 1;
		timeperframe->denominator =
			capability->cap[OPERATING_RATE].value >> 16;
	} else {
		timeperframe = &s_parm->parm.capture.timeperframe;
		timeperframe->numerator = 1;
		timeperframe->denominator =
			capability->cap[FRAME_RATE].value >> 16;
	}

	i_vpr_h(inst, "%s: type %u, num %u denom %u\n",
		__func__, s_parm->type, timeperframe->numerator,
		timeperframe->denominator);
	return 0;
}

int msm_venc_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 array[32] = {0};
	u32 i = 0, idx = 0;

	if (!inst || !inst->core || !inst->capabilities || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == OUTPUT_MPLANE) {
		u32 codecs = core->capabilities[DEC_CODECS].value;

		while (codecs) {
			if (i > 31)
				break;
			if (codecs & BIT(i)) {
				array[idx] = codecs & BIT(i);
				idx++;
			}
			i++;
		}
		f->pixelformat = v4l2_codec_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == INPUT_MPLANE) {
		u32 formats = inst->capabilities->cap[PIX_FMTS].step_or_mask;

		while (formats) {
			if (idx > 31)
				break;
			if (formats & BIT(i)) {
				array[idx] = formats & BIT(i);
				idx++;
			}
			i++;
		}
		f->pixelformat = v4l2_colorformat_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		strlcpy(f->description, "colorformat", sizeof(f->description));
	} else if (f->type == INPUT_META_PLANE || f->type == OUTPUT_META_PLANE) {
		if (!f->index) {
			f->pixelformat = V4L2_META_FMT_VIDC;
			strlcpy(f->description, "metadata", sizeof(f->description));
		} else {
			return -EINVAL;
		}
	}
	memset(f->reserved, 0, sizeof(f->reserved));

	i_vpr_h(inst, "%s: index %d, %s : %#x, flags %#x, driver colorfmt %#x\n",
		__func__, f->index, f->description, f->pixelformat, f->flags,
		v4l2_colorformat_to_driver(f->pixelformat, __func__));
	return rc;
}

int msm_venc_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *f;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_MPLANE;
	f->fmt.pix_mp.width = DEFAULT_WIDTH;
	f->fmt.pix_mp.height = DEFAULT_HEIGHT;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	inst->buffers.output.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;

	inst->crop.left = inst->crop.top = 0;
	inst->crop.width = f->fmt.pix_mp.width;
	inst->crop.height = f->fmt.pix_mp.height;

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = 0;
	inst->buffers.output_meta.min_count = 0;
	inst->buffers.output_meta.extra_count = 0;
	inst->buffers.output_meta.actual_count = 0;
	inst->buffers.output_meta.size = 0;

	f = &inst->fmts[INPUT_PORT];
	f->type = INPUT_MPLANE;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VIDC_NV12C;
	f->fmt.pix_mp.width = VENUS_Y_STRIDE(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_WIDTH);
	f->fmt.pix_mp.height = VENUS_Y_SCANLINES(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_HEIGHT);
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = f->fmt.pix_mp.width;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.actual_count =
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
	inst->buffers.input.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;

	f = &inst->fmts[INPUT_META_PORT];
	f->type = INPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = 0;
	inst->buffers.input_meta.min_count = 0;
	inst->buffers.input_meta.extra_count = 0;
	inst->buffers.input_meta.actual_count = 0;
	inst->buffers.input_meta.size = 0;

	inst->hfi_rc_type = HFI_RC_VBR_CFR;

	rc = msm_venc_codec_change(inst,
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat);

	return rc;
}

int msm_venc_inst_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	rc = msm_vidc_ctrl_deinit(inst);

	return rc;
}
