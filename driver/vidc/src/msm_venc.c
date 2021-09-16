// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
#include "msm_vidc_power.h"
#include "venus_hfi.h"
#include "hfi_packet.h"

static const u32 msm_venc_input_set_prop[] = {
	HFI_PROP_COLOR_FORMAT,
	HFI_PROP_RAW_RESOLUTION,
	HFI_PROP_LINEAR_STRIDE_SCANLINE,
	HFI_PROP_BUFFER_HOST_MAX_COUNT,
	HFI_PROP_SIGNAL_COLOR_INFO,
};

static const u32 msm_venc_output_set_prop[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_BUFFER_HOST_MAX_COUNT,
	HFI_PROP_CSC,
};

static const u32 msm_venc_input_subscribe_for_properties[] = {
	HFI_PROP_NO_OUTPUT,
};

static const u32 msm_venc_output_subscribe_for_properties[] = {
	HFI_PROP_PICTURE_TYPE,
	HFI_PROP_BUFFER_MARK,
	HFI_PROP_WORST_COMPRESSION_RATIO,
};

static const u32 msm_venc_output_internal_buffer_type[] = {
	MSM_VIDC_BUF_BIN,
	MSM_VIDC_BUF_COMV,
	MSM_VIDC_BUF_NON_COMV,
	MSM_VIDC_BUF_LINE,
	MSM_VIDC_BUF_DPB,
};

static const u32 msm_venc_input_internal_buffer_type[] = {
	MSM_VIDC_BUF_VPSS,
};

struct msm_venc_prop_type_handle {
	u32 type;
	int (*handle)(struct msm_vidc_inst *inst, enum msm_vidc_port_type port);
};

static int msm_venc_codec_change(struct msm_vidc_inst *inst, u32 v4l2_codec)
{
	int rc = 0;

	if (inst->codec && inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat == v4l2_codec)
		return 0;

	i_vpr_h(inst, "%s: codec changed from %s to %s\n",
		__func__, v4l2_pixelfmt_name(inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat),
		v4l2_pixelfmt_name(v4l2_codec));

	inst->codec = v4l2_codec_to_driver(v4l2_codec, __func__);
	inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat = v4l2_codec;
	rc = msm_vidc_update_debug_str(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_get_inst_capability(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_deinit(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_init(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_update_buffer_count(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vidc_update_buffer_count(inst, OUTPUT_PORT);
	if (rc)
		return rc;

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
		i_vpr_e(inst, "%s: invalid pixelformat %s\n",
			__func__, v4l2_pixelfmt_name(pixelformat));
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

static int msm_venc_set_stride_scanline(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 color_format, stride_y, scanline_y;
	u32 stride_uv = 0, scanline_uv = 0;
	u32 payload[2];

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	color_format = inst->capabilities->cap[PIX_FMTS].value;
	if (!is_linear_colorformat(color_format)) {
		i_vpr_h(inst,
			"%s: not a linear color fmt, property is not set\n",
			__func__);
		return 0;
	}

	stride_y = inst->fmts[INPUT_PORT].fmt.pix_mp.width;
	scanline_y = inst->fmts[INPUT_PORT].fmt.pix_mp.height;
	if (color_format == MSM_VIDC_FMT_NV12 ||
		color_format == MSM_VIDC_FMT_P010) {
		stride_uv = stride_y;
		scanline_uv = scanline_y / 2;
	}

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
	u32 crop[2] = {0};
	u32 width, height;

	if (port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	left_offset = inst->compose.left;
	top_offset = inst->compose.top;

	width = inst->compose.width;
	height = inst->compose.height;
	if (is_rotation_90_or_270(inst)) {
		width = inst->compose.height;
		height = inst->compose.width;
	}

	right_offset = (inst->fmts[port].fmt.pix_mp.width - width);
	bottom_offset = (inst->fmts[port].fmt.pix_mp.height - height);

	if (is_image_session(inst))
		right_offset = bottom_offset = 0;

	crop[0] = left_offset << 16 | top_offset;
	crop[1] = right_offset << 16 | bottom_offset;
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

	if (msm_vidc_is_super_buffer(inst) || is_image_session(inst))
		count = DEFAULT_MAX_HOST_BURST_BUF_COUNT;

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

static int msm_venc_set_colorspace(struct msm_vidc_inst* inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 primaries = MSM_VIDC_PRIMARIES_RESERVED;
	u32 matrix_coeff = MSM_VIDC_MATRIX_COEFF_RESERVED;
	u32 transfer_char = MSM_VIDC_TRANSFER_RESERVED;
	u32 full_range = 0;
	u32 colour_description_present_flag = 0;
	u32 video_signal_type_present_flag = 0, payload = 0;
	/* Unspecified video format */
	u32 video_format = 5;
	struct v4l2_format *input_fmt;
	u32 pix_fmt;

	if (port != INPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	input_fmt = &inst->fmts[INPUT_PORT];
	pix_fmt = v4l2_colorformat_to_driver(
		input_fmt->fmt.pix_mp.pixelformat, __func__);
	if (inst->fmts[port].fmt.pix_mp.colorspace != V4L2_COLORSPACE_DEFAULT ||
	    inst->fmts[port].fmt.pix_mp.ycbcr_enc != V4L2_YCBCR_ENC_DEFAULT ||
	    inst->fmts[port].fmt.pix_mp.xfer_func != V4L2_XFER_FUNC_DEFAULT) {
		colour_description_present_flag = 1;
		video_signal_type_present_flag = 1;
		primaries = v4l2_color_primaries_to_driver(inst,
			inst->fmts[port].fmt.pix_mp.colorspace, __func__);
		matrix_coeff = v4l2_matrix_coeff_to_driver(inst,
			inst->fmts[port].fmt.pix_mp.ycbcr_enc, __func__);
		transfer_char = v4l2_transfer_char_to_driver(inst,
			inst->fmts[port].fmt.pix_mp.xfer_func, __func__);
	} else if (is_rgba_colorformat(pix_fmt)) {
		colour_description_present_flag = 1;
		video_signal_type_present_flag = 1;
		primaries = MSM_VIDC_PRIMARIES_BT709;
		matrix_coeff = MSM_VIDC_MATRIX_COEFF_BT709;
		transfer_char = MSM_VIDC_TRANSFER_BT709;
		full_range = 0;
	}

	if (inst->fmts[port].fmt.pix_mp.quantization !=
	    V4L2_QUANTIZATION_DEFAULT) {
		video_signal_type_present_flag = 1;
		full_range = inst->fmts[port].fmt.pix_mp.quantization ==
			V4L2_QUANTIZATION_FULL_RANGE ? 1 : 0;
	}

	payload = (matrix_coeff & 0xFF) |
		((transfer_char << 8) & 0xFF00) |
		((primaries << 16) & 0xFF0000) |
		((colour_description_present_flag << 24) & 0x1000000) |
		((full_range << 25) & 0x2000000) |
		((video_format << 26) & 0x1C000000) |
		((video_signal_type_present_flag << 29) & 0x20000000);
	i_vpr_h(inst, "%s: color info: %#x\n", __func__, payload);
	rc = venus_hfi_session_property(inst,
		HFI_PROP_SIGNAL_COLOR_INFO,
		HFI_HOST_FLAGS_NONE,
		get_hfi_port(inst, port),
		HFI_PAYLOAD_32_PACKED,
		&payload,
		sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static bool msm_venc_csc_required(struct msm_vidc_inst* inst)
{
	struct v4l2_format *in_fmt = &inst->fmts[INPUT_PORT];
	struct v4l2_format *out_fmt = &inst->fmts[OUTPUT_PORT];

	/* video hardware supports conversion to REC709 CSC only */
	if (in_fmt->fmt.pix_mp.colorspace != out_fmt->fmt.pix_mp.colorspace &&
		out_fmt->fmt.pix_mp.colorspace == V4L2_COLORSPACE_REC709)
		return true;

	return false;
}

static int msm_venc_set_csc(struct msm_vidc_inst* inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 csc = 0;

	if (port != OUTPUT_PORT) {
		i_vpr_e(inst, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	msm_vidc_update_cap_value(inst, CSC,
		msm_venc_csc_required(inst) ? 1 : 0, __func__);

	csc = inst->capabilities->cap[CSC].value;
	i_vpr_h(inst, "%s: csc: %u\n", __func__, csc);
	rc = venus_hfi_session_property(inst,
		HFI_PROP_CSC,
		HFI_HOST_FLAGS_NONE,
		get_hfi_port(inst, port),
		HFI_PAYLOAD_U32,
		&csc,
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
	int i, j, rc = 0;
	static const struct msm_venc_prop_type_handle prop_type_handle_arr[] = {
		{HFI_PROP_COLOR_FORMAT,               msm_venc_set_colorformat                 },
		{HFI_PROP_RAW_RESOLUTION,             msm_venc_set_raw_resolution              },
		{HFI_PROP_LINEAR_STRIDE_SCANLINE,     msm_venc_set_stride_scanline             },
		{HFI_PROP_BUFFER_HOST_MAX_COUNT,      msm_venc_set_host_max_buf_count          },
		{HFI_PROP_SIGNAL_COLOR_INFO,          msm_venc_set_colorspace                  },
	};

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	i_vpr_h(inst, "%s()\n", __func__);
	for (i = 0; i < ARRAY_SIZE(msm_venc_input_set_prop); i++) {
		/* set session input properties */
		for (j = 0; j < ARRAY_SIZE(prop_type_handle_arr); j++) {
			if (prop_type_handle_arr[j].type == msm_venc_input_set_prop[i]) {
				rc = prop_type_handle_arr[j].handle(inst, INPUT_PORT);
				if (rc)
					goto exit;
				break;
			}
		}

		/* is property type unknown ? */
		if (j == ARRAY_SIZE(prop_type_handle_arr))
			i_vpr_e(inst, "%s: unknown property %#x\n", __func__,
				msm_venc_input_set_prop[i]);
	}

exit:
	return rc;
}

static int msm_venc_set_output_properties(struct msm_vidc_inst *inst)
{
	int i, j, rc = 0;
	static const struct msm_venc_prop_type_handle prop_type_handle_arr[] = {
		{HFI_PROP_BITSTREAM_RESOLUTION,       msm_venc_set_bitstream_resolution    },
		{HFI_PROP_CROP_OFFSETS,               msm_venc_set_crop_offsets            },
		{HFI_PROP_BUFFER_HOST_MAX_COUNT,      msm_venc_set_host_max_buf_count      },
		{HFI_PROP_CSC,                        msm_venc_set_csc                     },
	};

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	i_vpr_h(inst, "%s()\n", __func__);
	for (i = 0; i < ARRAY_SIZE(msm_venc_output_set_prop); i++) {
		/* set session output properties */
		for (j = 0; j < ARRAY_SIZE(prop_type_handle_arr); j++) {
			if (prop_type_handle_arr[j].type == msm_venc_output_set_prop[i]) {
				rc = prop_type_handle_arr[j].handle(inst, OUTPUT_PORT);
				if (rc)
					goto exit;
				break;
			}
		}

		/* is property type unknown ? */
		if (j == ARRAY_SIZE(prop_type_handle_arr))
			i_vpr_e(inst, "%s: unknown property %#x\n", __func__,
				msm_venc_output_set_prop[i]);
	}

exit:
	return rc;
}

static int msm_venc_set_internal_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	rc = msm_venc_set_quality_mode(inst);
	if (rc)
		return rc;

	return rc;
}

static int msm_venc_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int i, rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	for (i = 0; i < ARRAY_SIZE(msm_venc_input_internal_buffer_type); i++) {
		rc = msm_vidc_get_internal_buffers(inst,
			msm_venc_input_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_venc_create_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int i, rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_input_internal_buffer_type); i++) {
		rc = msm_vidc_create_internal_buffers(inst,
			msm_venc_input_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_venc_queue_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int i, rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_input_internal_buffer_type); i++) {
		rc = msm_vidc_queue_internal_buffers(inst,
			msm_venc_input_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_venc_get_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int i, rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	for (i = 0; i < ARRAY_SIZE(msm_venc_output_internal_buffer_type); i++) {
		rc = msm_vidc_get_internal_buffers(inst,
			msm_venc_output_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return rc;
}

static int msm_venc_create_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int i, rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_output_internal_buffer_type); i++) {
		rc = msm_vidc_create_internal_buffers(inst,
			msm_venc_output_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

	return 0;
}

static int msm_venc_queue_output_internal_buffers(struct msm_vidc_inst *inst)
{
	int i, rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_output_internal_buffer_type); i++) {
		rc = msm_vidc_queue_internal_buffers(inst,
			msm_venc_output_internal_buffer_type[i]);
		if (rc)
			return rc;
	}

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
	i_vpr_h(inst, "%s()\n", __func__);

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
	if (rc)
		return rc;

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
	static const u32 metadata_list[] = {
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
	i_vpr_h(inst, "%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(metadata_list); i++) {
		if (capability->cap[metadata_list[i]].value) {
			payload[count + 1] =
				capability->cap[metadata_list[i]].hfi_id;
			count++;
		}
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));
	if (rc)
		return rc;

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
	static const u32 metadata_list[] = {
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
	i_vpr_h(inst, "%s()\n", __func__);

	capability = inst->capabilities;
	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(metadata_list); i++) {
		if (capability->cap[metadata_list[i]].value) {
			payload[count + 1] =
				capability->cap[metadata_list[i]].hfi_id;
			count++;
		}
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(count + 1) * sizeof(u32));
	if (rc)
		return rc;

	return rc;
}

int msm_venc_streamoff_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core || !inst->capabilities) {
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

	if (!inst || !inst->core || !inst->capabilities) {
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

	rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;

	rc = msm_vidc_check_scaling_supported(inst);
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

	if (!inst || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (inst->firmware_priority != (inst->priority_level +
		inst->capabilities->cap[PRIORITY].value * 2))
		msm_vidc_set_session_priority(inst, PRIORITY);

	rc = msm_vidc_queue_buffer_single(inst, vb2);
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
		i_vpr_h(inst, "received cmd: drain\n");
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
		i_vpr_h(inst, "received cmd: resume\n");
		if (!msm_vidc_allow_start(inst))
			return -EBUSY;
		vb2_clear_last_buffer_dequeued(&inst->vb2q[OUTPUT_META_PORT]);
		vb2_clear_last_buffer_dequeued(&inst->vb2q[OUTPUT_PORT]);

		rc = msm_vidc_state_change_start(inst);
		if (rc)
			return rc;

		/* tune power features */
		msm_vidc_allow_dcvs(inst);
		msm_vidc_power_data_reset(inst);

		/* print final buffer counts & size details */
		msm_vidc_print_buffer_info(inst);

		rc = venus_hfi_session_command(inst,
				HFI_CMD_RESUME,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
	} else {
		i_vpr_e(inst, "%s: unknown cmd %d\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}

int msm_venc_streamoff_output(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	core = inst->core;
	if (!core->capabilities) {
		i_vpr_e(inst, "%s: core capabilities is NULL\n", __func__);
		return -EINVAL;
	}

	/* restore LAYER_COUNT max allowed value */
	inst->capabilities->cap[ENH_LAYER_COUNT].max =
		core->capabilities[MAX_ENH_LAYER_COUNT].value;

	rc = msm_vidc_session_streamoff(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_venc_streamon_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core || !inst->capabilities) {
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

	rc = msm_vidc_adjust_v4l2_properties(inst);
	if (rc)
		goto error;

	rc = msm_venc_set_output_properties(inst);
	if (rc)
		goto error;

	rc = msm_vidc_set_v4l2_properties(inst);
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

int msm_venc_try_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_pix_format_mplane *pixmp = &f->fmt.pix_mp;
	u32 pix_fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	memset(pixmp->reserved, 0, sizeof(pixmp->reserved));

	if (f->type == INPUT_MPLANE) {
		pix_fmt = v4l2_colorformat_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		if (!pix_fmt) {
			i_vpr_h(inst, "%s: unsupported format, set default params\n", __func__);
			f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VIDC_NV12C;
			f->fmt.pix_mp.width = VIDEO_Y_STRIDE_PIX(f->fmt.pix_mp.pixelformat,
								 DEFAULT_WIDTH);
			f->fmt.pix_mp.height = VIDEO_Y_SCANLINES(f->fmt.pix_mp.pixelformat,
								 DEFAULT_HEIGHT);
		}
	} else if (f->type == OUTPUT_MPLANE) {
		pix_fmt = v4l2_codec_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		if (!pix_fmt) {
			i_vpr_h(inst, "%s: unsupported codec, set default params\n", __func__);
			f->fmt.pix_mp.width = DEFAULT_WIDTH;
			f->fmt.pix_mp.height = DEFAULT_HEIGHT;
			f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
			pix_fmt = v4l2_colorformat_to_driver(f->fmt.pix_mp.pixelformat, __func__);
		}
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, f->type);
		return -EINVAL;
	}

	if (pixmp->field == V4L2_FIELD_ANY)
		pixmp->field = V4L2_FIELD_NONE;
	pixmp->num_planes = 1;

	return rc;
}

int msm_venc_s_fmt_output(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_format *fmt;
	struct msm_vidc_core *core;
	u32 codec_align;
	u32 width, height;

	if (!inst || !inst->core || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	fmt = &inst->fmts[OUTPUT_PORT];
	if (fmt->fmt.pix_mp.pixelformat != f->fmt.pix_mp.pixelformat) {
		rc = msm_venc_codec_change(inst, f->fmt.pix_mp.pixelformat);
		if (rc)
			return rc;
	}
	fmt->type = OUTPUT_MPLANE;

	codec_align = (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC ||
		f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEIC) ? 32 : 16;
	/* use rotated width height if rotation is enabled */
	width = inst->compose.width;
	height = inst->compose.height;
	if (is_rotation_90_or_270(inst)) {
		width = inst->compose.height;
		height = inst->compose.width;
	}
	/* width, height is readonly for client */
	fmt->fmt.pix_mp.width = ALIGN(width, codec_align);
	fmt->fmt.pix_mp.height = ALIGN(height, codec_align);
	/* use grid dimension for image session */
	if (is_image_session(inst))
		fmt->fmt.pix_mp.width = fmt->fmt.pix_mp.height = HEIC_GRID_DIMENSION;
	fmt->fmt.pix_mp.num_planes = 1;
	fmt->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	/* video hw supports conversion to V4L2_COLORSPACE_REC709 only */
	if (f->fmt.pix_mp.colorspace != V4L2_COLORSPACE_DEFAULT &&
	    f->fmt.pix_mp.colorspace != V4L2_COLORSPACE_REC709)
		f->fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
	fmt->fmt.pix_mp.colorspace = f->fmt.pix_mp.colorspace;
	fmt->fmt.pix_mp.xfer_func = f->fmt.pix_mp.xfer_func;
	fmt->fmt.pix_mp.ycbcr_enc = f->fmt.pix_mp.ycbcr_enc;
	fmt->fmt.pix_mp.quantization = f->fmt.pix_mp.quantization;
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
	memcpy(f, fmt, sizeof(struct v4l2_format));

	/* reset metadata buffer size with updated resolution*/
	msm_vidc_update_meta_port_settings(inst);

	i_vpr_h(inst,
		"%s: type: OUTPUT, codec %s width %d height %d size %u min_count %d extra_count %d\n",
		__func__, v4l2_pixelfmt_name(fmt->fmt.pix_mp.pixelformat),
		fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height,
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
		inst->buffers.output.min_count,
		inst->buffers.output.extra_count);

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

	memcpy(f, fmt, sizeof(struct v4l2_format));

	i_vpr_h(inst, "%s: type: OUTPUT_META, size %u min_count %d extra_count %d\n",
		__func__, fmt->fmt.meta.buffersize,
		inst->buffers.output_meta.min_count,
		inst->buffers.output_meta.extra_count);

	return rc;
}

static int msm_venc_s_fmt_input(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct v4l2_format *fmt;
	struct msm_vidc_core *core;
	u32 pix_fmt, width, height, size, bytesperline,
		crop_width, crop_height;

	if (!inst || !inst->core || !inst->capabilities) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	pix_fmt = v4l2_colorformat_to_driver(f->fmt.pix_mp.pixelformat, __func__);
	msm_vidc_update_cap_value(inst, PIX_FMTS, pix_fmt, __func__);

	if (is_rgba_colorformat(pix_fmt)) {
		width = VIDEO_RGB_STRIDE_PIX(f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width);
		height = VIDEO_RGB_SCANLINES(f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.height);
		crop_width = VIDEO_RGB_STRIDE_PIX(f->fmt.pix_mp.pixelformat, inst->crop.width);
		crop_height = VIDEO_RGB_SCANLINES(f->fmt.pix_mp.pixelformat, inst->crop.height);
		bytesperline =
			VIDEO_RGB_STRIDE_BYTES(f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width);
	} else if (is_image_session(inst)) {
		width = ALIGN(f->fmt.pix_mp.width, HEIC_GRID_DIMENSION);
		height = ALIGN(f->fmt.pix_mp.height, HEIC_GRID_DIMENSION);
		crop_width = ALIGN(inst->crop.width, HEIC_GRID_DIMENSION);
		crop_height = ALIGN(inst->crop.height, HEIC_GRID_DIMENSION);
		bytesperline = width * (is_10bit_colorformat(pix_fmt) ? 2 : 1);
	} else {
		width = VIDEO_Y_STRIDE_PIX(f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width);
		height = VIDEO_Y_SCANLINES(f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.height);
		crop_width = VIDEO_Y_STRIDE_PIX(f->fmt.pix_mp.pixelformat, inst->crop.width);
		crop_height = VIDEO_Y_SCANLINES(f->fmt.pix_mp.pixelformat, inst->crop.height);
		bytesperline = VIDEO_Y_STRIDE_BYTES(f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width);
	}

	fmt = &inst->fmts[INPUT_PORT];
	fmt->type = INPUT_MPLANE;
	fmt->fmt.pix_mp.width = width;
	fmt->fmt.pix_mp.height = height;
	fmt->fmt.pix_mp.num_planes = 1;
	fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
	fmt->fmt.pix_mp.plane_fmt[0].bytesperline = bytesperline;
	if (is_image_session(inst))
		size = bytesperline * height * 3 / 2;
	else
		size = call_session_op(core, buffer_size, inst, MSM_VIDC_BUF_INPUT);
	fmt->fmt.pix_mp.plane_fmt[0].sizeimage = size;
	fmt->fmt.pix_mp.colorspace = f->fmt.pix_mp.colorspace;
	fmt->fmt.pix_mp.xfer_func = f->fmt.pix_mp.xfer_func;
	fmt->fmt.pix_mp.ycbcr_enc = f->fmt.pix_mp.ycbcr_enc;
	fmt->fmt.pix_mp.quantization = f->fmt.pix_mp.quantization;
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
	inst->buffers.input.size = size;

	if (fmt->fmt.pix_mp.width != crop_width ||
		fmt->fmt.pix_mp.height != crop_height) {
		struct v4l2_format *output_fmt;

		/* reset crop dimensions with updated resolution */
		inst->crop.top = inst->crop.left = 0;
		inst->crop.width = f->fmt.pix_mp.width;
		inst->crop.height = f->fmt.pix_mp.height;

		/* reset compose dimensions with updated resolution */
		inst->compose.top = inst->compose.left = 0;
		inst->compose.width = f->fmt.pix_mp.width;
		inst->compose.height = f->fmt.pix_mp.height;

		output_fmt = &inst->fmts[OUTPUT_PORT];
		rc = msm_venc_s_fmt_output(inst, output_fmt);
		if (rc)
			return rc;
	}
	memcpy(f, fmt, sizeof(struct v4l2_format));

	/* reset metadata buffer size with updated resolution*/
	msm_vidc_update_meta_port_settings(inst);

	i_vpr_h(inst,
		"%s: type: INPUT, format %s width %d height %d size %u min_count %d extra_count %d\n",
		__func__, v4l2_pixelfmt_name(fmt->fmt.pix_mp.pixelformat),
		fmt->fmt.pix_mp.width, fmt->fmt.pix_mp.height,
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
		inst->buffers.input.min_count,
		inst->buffers.input.extra_count);

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

	memcpy(f, fmt, sizeof(struct v4l2_format));

	i_vpr_h(inst, "%s: type: INPUT_META, size %u min_count %d extra_count %d\n",
		__func__, fmt->fmt.meta.buffersize,
		inst->buffers.input_meta.min_count,
		inst->buffers.input_meta.extra_count);

	return rc;
}

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
			goto exit;
	} else if (f->type == INPUT_META_PLANE) {
		rc = msm_venc_s_fmt_input_meta(inst, f);
		if (rc)
			goto exit;
	} else if (f->type == OUTPUT_MPLANE) {
		rc = msm_venc_s_fmt_output(inst, f);
		if (rc)
			goto exit;
	} else if (f->type == OUTPUT_META_PLANE) {
		rc = msm_venc_s_fmt_output_meta(inst, f);
		if (rc)
			goto exit;
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, f->type);
		rc = -EINVAL;
		goto exit;
	}

exit:
	if (rc)
		i_vpr_e(inst, "%s: failed\n", __func__);

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
	struct v4l2_format *output_fmt;

	if (!inst || !s) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (s->type != INPUT_MPLANE && s->type != V4L2_BUF_TYPE_VIDEO_OUTPUT) {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, s->type);
		return -EINVAL;
	}

	switch (s->target) {
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
		inst->compose.left = inst->crop.left;
		inst->compose.top = inst->crop.top;
		inst->compose.width = inst->crop.width;
		inst->compose.height = inst->crop.height;
		/* update output format based on new crop dimensions */
		output_fmt = &inst->fmts[OUTPUT_PORT];
		rc = msm_venc_s_fmt_output(inst, output_fmt);
		if (rc)
			return rc;
		break;
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

		if (is_scaling_enabled(inst)) {
			i_vpr_h(inst,
				"%s: scaling enabled, crop: l %d t %d w %d h %d compose: l %d t %d w %d h %d\n",
				__func__, inst->crop.left, inst->crop.top,
				inst->crop.width, inst->crop.height,
				inst->compose.left, inst->compose.top,
				inst->compose.width, inst->compose.height);
		}

		/* update output format based on new compose dimensions */
		output_fmt = &inst->fmts[OUTPUT_PORT];
		rc = msm_venc_s_fmt_output(inst, output_fmt);
		if (rc)
			return rc;
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
		max_rate = capability->cap[OPERATING_RATE].max >> 16;
		default_rate = capability->cap[OPERATING_RATE].value >> 16;
		s_parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
	} else {
		timeperframe = &s_parm->parm.capture.timeperframe;
		is_frame_rate = true;
		max_rate = capability->cap[FRAME_RATE].max >> 16;
		default_rate = capability->cap[FRAME_RATE].value >> 16;
		s_parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	}

	if (!timeperframe->denominator || !timeperframe->numerator) {
		i_vpr_e(inst, "%s: type %s, invalid rate, update with default\n",
			 __func__, v4l2_type_name(s_parm->type));
		if (!timeperframe->numerator)
			timeperframe->numerator = 1;
		if (!timeperframe->denominator)
			timeperframe->denominator = default_rate;
	}

	us_per_frame = timeperframe->numerator * (u64)USEC_PER_SEC;
	do_div(us_per_frame, timeperframe->denominator);

	if (!us_per_frame) {
		i_vpr_e(inst, "%s: us_per_frame is zero\n", __func__);
		rc = -EINVAL;
		goto exit;
	}

	input_rate = (u64)USEC_PER_SEC;
	do_div(input_rate, us_per_frame);

	i_vpr_h(inst, "%s: type %s, %s value %d\n",
		__func__, v4l2_type_name(s_parm->type),
		is_frame_rate ? "frame rate" : "operating rate", input_rate);

	q16_rate = (u32)input_rate << 16;
	msm_vidc_update_cap_value(inst, is_frame_rate ? FRAME_RATE : OPERATING_RATE,
		q16_rate, __func__);
	if (is_realtime_session(inst) &&
		((s_parm->type == INPUT_MPLANE && inst->vb2q[INPUT_PORT].streaming) ||
		(s_parm->type == OUTPUT_MPLANE && inst->vb2q[OUTPUT_PORT].streaming))) {
		rc = msm_vidc_check_core_mbps(inst);
		if (rc) {
			i_vpr_e(inst, "%s: unsupported load\n", __func__);
			goto reset_rate;
		}
		rc = input_rate > max_rate;
		if (rc) {
			i_vpr_e(inst, "%s: unsupported rate %u, max %u\n", __func__,
				input_rate, max_rate);
			rc = -ENOMEM;
			goto reset_rate;
		}
	}
	inst->priority_level = MSM_VIDC_PRIORITY_HIGH;

	if (is_frame_rate)
		capability->cap[FRAME_RATE].flags |= CAP_FLAG_CLIENT_SET;
	else
		capability->cap[OPERATING_RATE].flags |= CAP_FLAG_CLIENT_SET;
	/*
	 * In static case, frame rate is set via
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
				"%s: failed to set frame rate to fw\n", __func__);
			goto exit;
		}
		inst->auto_framerate = q16_rate;
	}

	return 0;

reset_rate:
	if (rc) {
		i_vpr_e(inst, "%s: setting rate %u failed, reset to %u\n", __func__,
			input_rate, default_rate);
		msm_vidc_update_cap_value(inst, is_frame_rate ? FRAME_RATE : OPERATING_RATE,
			default_rate << 16, __func__);
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
		s_parm->parm.output.capability = V4L2_CAP_TIMEPERFRAME;
	} else {
		timeperframe = &s_parm->parm.capture.timeperframe;
		timeperframe->numerator = 1;
		timeperframe->denominator =
			capability->cap[FRAME_RATE].value >> 16;
		s_parm->parm.capture.capability = V4L2_CAP_TIMEPERFRAME;
	}

	i_vpr_h(inst, "%s: type %s, num %u denom %u\n",
		__func__, v4l2_type_name(s_parm->type), timeperframe->numerator,
		timeperframe->denominator);
	return 0;
}

int msm_venc_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 array[32] = {0};
	u32 i = 0;

	if (!inst || !inst->core || !inst->capabilities || !f ||
	    f->index >= ARRAY_SIZE(array)) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == OUTPUT_MPLANE) {
		u32 codecs = core->capabilities[ENC_CODECS].value;
		u32 idx = 0;

		for (i = 0; i <= 31; i++) {
			if (codecs & BIT(i)) {
				if (idx >= ARRAY_SIZE(array))
					break;
				array[idx] = codecs & BIT(i);
				idx++;
			}
		}
		if (!array[f->index])
			return -EINVAL;
		f->pixelformat = v4l2_codec_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == INPUT_MPLANE) {
		u32 formats = inst->capabilities->cap[PIX_FMTS].step_or_mask;
		u32 idx = 0;

		for (i = 0; i <= 31; i++) {
			if (formats & BIT(i)) {
				if (idx >= ARRAY_SIZE(array))
					break;
				array[idx] = formats & BIT(i);
				idx++;
			}
		}
		if (!array[f->index])
			return -EINVAL;
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

	i_vpr_h(inst, "%s: index %d, %s: %s, flags %#x\n",
		__func__, f->index, f->description,
		v4l2_pixelfmt_name(f->pixelformat), f->flags);
	return rc;
}

int msm_venc_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *f;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	i_vpr_h(inst, "%s()\n", __func__);

	core = inst->core;

	if (core->capabilities[DCVS].value)
		inst->power.dcvs_mode = true;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_MPLANE;
	f->fmt.pix_mp.width = DEFAULT_WIDTH;
	f->fmt.pix_mp.height = DEFAULT_HEIGHT;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	f->fmt.pix_mp.field = V4L2_FIELD_NONE;
	f->fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
	f->fmt.pix_mp.xfer_func = V4L2_XFER_FUNC_DEFAULT;
	f->fmt.pix_mp.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	f->fmt.pix_mp.quantization = V4L2_QUANTIZATION_DEFAULT;
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

	inst->compose.left = inst->compose.top = 0;
	inst->compose.width = f->fmt.pix_mp.width;
	inst->compose.height = f->fmt.pix_mp.height;

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
	f->fmt.pix_mp.width = VIDEO_Y_STRIDE_PIX(f->fmt.pix_mp.pixelformat,
		DEFAULT_WIDTH);
	f->fmt.pix_mp.height = VIDEO_Y_SCANLINES(f->fmt.pix_mp.pixelformat,
		DEFAULT_HEIGHT);
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline =
		VIDEO_Y_STRIDE_BYTES(f->fmt.pix_mp.pixelformat,
		DEFAULT_WIDTH);
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_INPUT);
	f->fmt.pix_mp.field = V4L2_FIELD_NONE;
	f->fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;
	f->fmt.pix_mp.xfer_func = V4L2_XFER_FUNC_DEFAULT;
	f->fmt.pix_mp.ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	f->fmt.pix_mp.quantization = V4L2_QUANTIZATION_DEFAULT;
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

	inst->priority_level = MSM_VIDC_PRIORITY_LOW;

	inst->hfi_rc_type = HFI_RC_VBR_CFR;
	inst->hfi_layer_type = HFI_HIER_P_SLIDING_WINDOW;

	rc = msm_venc_codec_change(inst,
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat);
	if (rc)
		return rc;

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
	if (rc)
		return rc;

	return rc;
}
