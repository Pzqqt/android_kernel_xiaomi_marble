// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/v4l2_vidc_extensions.h>
#include <media/msm_media_info.h>

#include "msm_vdec.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"
#include "hfi_packet.h"

u32 msm_vdec_subscribe_for_port_settings_change[] = {
	HFI_PROP_ALIGN_RESOLUTION,
	HFI_PROP_CROP_COORDINATE_TOP_LEFT,
	HFI_PROP_CROP_RESOLUTION,
	HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
	HFI_PROP_CABAC_SESSION,
	HFI_PROP_CODED_FRAMES,
	HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
	HFI_PROP_PIC_ORDER_CNT_TYPE,
	HFI_PROP_SIGNAL_COLOR_INFO,
};

u32 msm_vdec_subscribe_for_properties[] = {
	HFI_PROP_TAG_NOT_PROPAGATED_TO_OUTPUT,
};

u32 msm_vdec_subscribe_for_metadata[] = {
	HFI_PROP_BUFFER_TAG,
};

u32 msm_vdec_deliver_as_metadata[] = {
	HFI_PROP_BUFFER_TAG,
};

static int msm_vdec_codec_change(struct msm_vidc_inst *inst, u32 codec)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);

	inst->codec = v4l2_codec_to_driver(codec, __func__);
	rc = msm_vidc_get_inst_capability(inst);
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 resolution;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	resolution = inst->fmts[port].fmt.pix_mp.width << 16 |
		inst->fmts[port].fmt.pix_mp.height;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_ALIGN_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&resolution,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_crop_top_left(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 crop_top_left;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	crop_top_left = 0;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CROP_COORDINATE_TOP_LEFT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&crop_top_left,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_crop_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 crop;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	/* output buffer resolution is nothing but crop */
	crop = inst->fmts[OUTPUT_PORT].fmt.pix_mp.width << 16 |
		inst->fmts[OUTPUT_PORT].fmt.pix_mp.height;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CROP_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&crop,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_bit_depth(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 colorformat;
	u32 bitdepth = 8 << 16 | 8;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	colorformat = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat;
	if (colorformat == V4L2_PIX_FMT_NV12_P010 ||
	    colorformat == V4L2_PIX_FMT_NV12_TP10_UBWC)
		bitdepth = 10 << 16 | 10;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LUMA_CHROMA_BIT_DEPTH,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&bitdepth,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_cabac(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 cabac = 0;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_CABAC_SESSION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&cabac,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_coded_frames(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 coded_frames;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	/* (mb_adaptive_frame_field_flag << 1) | frame_mbs_only_flag */
	coded_frames = 1;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_CODED_FRAMES,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&coded_frames,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_min_output_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 min_output;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	min_output = inst->buffers.output.min_count;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&min_output,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_picture_order_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 poc = 1;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIC_ORDER_CNT_TYPE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&poc,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_colorspace(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 colorspace;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	colorspace = inst->fmts[OUTPUT_PORT].fmt.pix_mp.colorspace;
	rc = venus_hfi_session_property(inst,
			HFI_PROP_SIGNAL_COLOR_INFO,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&colorspace,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_colorformat(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 pixelformat;
	enum msm_vidc_colorformat_type colorformat;
	u32 hfi_colorformat;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	pixelformat = inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat;
	if (pixelformat != V4L2_PIX_FMT_NV12_UBWC &&
	    pixelformat != V4L2_PIX_FMT_NV12_TP10_UBWC) {
		s_vpr_e(inst->sid, "%s: invalid pixelformat %#x\n",
			__func__, pixelformat);
		return -EINVAL;
	}
	colorformat = v4l2_colorformat_to_driver(pixelformat, __func__);
	hfi_colorformat = get_hfi_colorformat(inst, colorformat);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_COLOR_FORMAT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&hfi_colorformat,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_stage(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core = inst->core;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = call_session_op(core, decide_work_mode, inst);
	if (rc) {
		s_vpr_e(inst->sid, "%s: decide_work_mode failed %d\n",
			__func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_STAGE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&inst->stage,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_pipe(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core = inst->core;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	rc = call_session_op(core, decide_work_route, inst);
	if (rc) {
		s_vpr_e(inst->sid, "%s: decide_work_route failed %d\n",
			__func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIPE,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&inst->pipe,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_vdec_set_input_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change);
	     i++) {
		switch (msm_vdec_subscribe_for_port_settings_change[i]) {
		case HFI_PROP_ALIGN_RESOLUTION:
			rc = msm_vdec_set_resolution(inst, INPUT_PORT);
			break;
		case HFI_PROP_CROP_COORDINATE_TOP_LEFT:
			rc = msm_vdec_set_crop_top_left(inst, INPUT_PORT);
			break;
		case HFI_PROP_CROP_RESOLUTION:
			rc = msm_vdec_set_crop_resolution(inst, INPUT_PORT);
			break;
		case HFI_PROP_LUMA_CHROMA_BIT_DEPTH:
			rc = msm_vdec_set_bit_depth(inst, INPUT_PORT);
			break;
		case HFI_PROP_CABAC_SESSION:
			rc = msm_vdec_set_cabac(inst, INPUT_PORT);
			break;
		case HFI_PROP_CODED_FRAMES:
			rc = msm_vdec_set_coded_frames(inst, INPUT_PORT);
			break;
		case HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT:
			rc = msm_vdec_set_min_output_count(inst, INPUT_PORT);
			break;
		case HFI_PROP_PIC_ORDER_CNT_TYPE:
			rc = msm_vdec_set_picture_order_count(inst, INPUT_PORT);
			break;
		case HFI_PROP_SIGNAL_COLOR_INFO:
			rc = msm_vdec_set_colorspace(inst, INPUT_PORT);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_vdec_subscribe_for_port_settings_change[i]);
			rc = -EINVAL;
			break;
		}
	}

	return rc;
}

static int msm_vdec_set_output_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vdec_set_colorformat(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_stage(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_set_pipe(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change);
	     i++) {
		switch (msm_vdec_subscribe_for_port_settings_change[i]) {
		case HFI_PROP_ALIGN_RESOLUTION:
			rc = msm_vdec_set_resolution(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_CROP_COORDINATE_TOP_LEFT:
			rc = msm_vdec_set_crop_top_left(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_CROP_RESOLUTION:
			rc = msm_vdec_set_crop_resolution(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_LUMA_CHROMA_BIT_DEPTH:
			rc = msm_vdec_set_bit_depth(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_CABAC_SESSION:
			rc = msm_vdec_set_cabac(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_CODED_FRAMES:
			rc = msm_vdec_set_coded_frames(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT:
			rc = msm_vdec_set_min_output_count(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_PIC_ORDER_CNT_TYPE:
			rc = msm_vdec_set_picture_order_count(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_SIGNAL_COLOR_INFO:
			rc = msm_vdec_set_colorspace(inst, OUTPUT_PORT);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_vdec_subscribe_for_port_settings_change[i]);
			rc = -EINVAL;
			break;
		}
	}

	return rc;
}

static int msm_vdec_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	inst->buffers.bin.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_BIN);
	inst->buffers.comv.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_COMV);
	inst->buffers.non_comv.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_NON_COMV);
	inst->buffers.line.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_LINE);
	inst->buffers.persist.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_PERSIST);

	inst->buffers.bin.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_BIN);
	inst->buffers.comv.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_COMV);
	inst->buffers.non_comv.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_NON_COMV);
	inst->buffers.line.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_LINE);
	inst->buffers.persist.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_PERSIST);

	s_vpr_h(inst->sid, "internal buffer: min     size\n");
	s_vpr_h(inst->sid, "bin  buffer: %d      %d\n",
		inst->buffers.bin.min_count,
		inst->buffers.bin.size);
	s_vpr_h(inst->sid, "comv  buffer: %d      %d\n",
		inst->buffers.comv.min_count,
		inst->buffers.comv.size);
	s_vpr_h(inst->sid, "non_comv  buffer: %d      %d\n",
		inst->buffers.non_comv.min_count,
		inst->buffers.non_comv.size);
	s_vpr_h(inst->sid, "line buffer: %d      %d\n",
		inst->buffers.line.min_count,
		inst->buffers.line.size);
	s_vpr_h(inst->sid, "persist buffer: %d      %d\n",
		inst->buffers.persist.min_count,
		inst->buffers.persist.size);

	return rc;
}

static int msm_vdec_create_input_internal_buffers(struct msm_vidc_inst *inst)
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
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc;

	return 0;
}

static int msm_vdec_queue_input_internal_buffers(struct msm_vidc_inst *inst)
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
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc;

	return 0;
}

/*
static int msm_vdec_release_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_SCRATCH_1);
	if (rc)
		return rc;
	rc = msm_vidc_release_internal_buffers(inst, MSM_VIDC_BUF_PERSIST_1);
	if (rc)
		return rc;

	return 0;
}
*/

static int msm_vdec_port_settings_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_PORT_SETTINGS_CHANGE;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change);
	     i++)
		payload[i + 1] = msm_vdec_subscribe_for_port_settings_change[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_subscribe_for_port_settings_change) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_vdec_property_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_PROPERTY;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_properties); i++)
		payload[i + 1] = msm_vdec_subscribe_for_properties[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_subscribe_for_properties) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_vdec_metadata_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_subscribe_for_metadata); i++)
		payload[i + 1] = msm_vdec_subscribe_for_metadata[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_subscribe_for_metadata) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_vdec_metadata_delivery(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(msm_vdec_deliver_as_metadata); i++)
		payload[i + 1] = msm_vdec_deliver_as_metadata[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_vdec_deliver_as_metadata) + 1) *
			sizeof(u32));

	return rc;
}

int msm_vdec_stop_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_stop(inst, INPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_start_input(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	s_vpr_h(inst->sid, "%s()\n", __func__);

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;
	//rc = msm_vidc_check_scaling_supported(inst);
	if (rc)
		goto error;

	rc = msm_vdec_set_input_properties(inst);
	if (rc)
		goto error;

	/* Decide bse vpp delay after work mode */
	//msm_vidc_set_bse_vpp_delay(inst);

	rc = msm_vdec_get_input_internal_buffers(inst);
	if (rc)
		goto error;
	/* check for memory after all buffers calculation */
	//rc = msm_vidc_check_memory_supported(inst);
	if (rc)
		goto error;

	//msm_vidc_update_dcvs(inst);
	//msm_vidc_update_batching(inst);
	//msm_vidc_scale_power(inst);

	rc = msm_vdec_create_input_internal_buffers(inst);
	rc = 0; // TODO
	if (rc)
		goto error;
	rc = msm_vdec_queue_input_internal_buffers(inst);
	rc = 0; // TODO
	if (rc)
		goto error;

	rc = msm_vdec_port_settings_subscription(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_property_subscription(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_metadata_delivery(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = venus_hfi_start(inst, INPUT_PORT);
	if (rc)
		goto error;

	s_vpr_h(inst->sid, "%s: done\n", __func__);
	return 0;

error:
	s_vpr_e(inst->sid, "%s: failed\n", __func__);
	msm_vdec_stop_input(inst);
	return rc;
}

int msm_vdec_stop_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_stop(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_vdec_start_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vdec_set_output_properties(inst);
	if (rc)
		goto error;

	rc = msm_vdec_port_settings_subscription(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vdec_metadata_subscription(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	rc = venus_hfi_start(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	d_vpr_h("%s: done\n", __func__);
	return 0;

error:
	msm_vdec_stop_output(inst);
	return rc;
}

static int msm_vdec_qbuf_batch(struct msm_vidc_inst *inst,
	struct vb2_buffer *vb2)
{
	int rc = 0;

	if (!inst || !vb2) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	d_vpr_h("%s()\n", __func__);

	return rc;
}

int msm_vdec_qbuf(struct msm_vidc_inst *inst, struct vb2_buffer *vb2)
{
	int rc = 0;

	if (inst->decode_batch.enable)
		rc = msm_vdec_qbuf_batch(inst, vb2);
	else
		rc = msm_vidc_queue_buffer(inst, vb2);

	return rc;
}

int msm_vdec_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (inst->state == MSM_VIDC_START) {
		d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
		return -EINVAL;
	}

	if (f->type == INPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		if (inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat !=
			f->fmt.pix_mp.pixelformat) {
			s_vpr_e(inst->sid,
				"%s: codec changed from %#x to %#x\n", __func__,
				inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat,
				f->fmt.pix_mp.pixelformat);
			rc = msm_vdec_codec_change(inst, f->fmt.pix_mp.pixelformat);
			if (rc)
				goto err_invalid_fmt;
		}
		fmt = &inst->fmts[INPUT_PORT];
		fmt->type = INPUT_MPLANE;
		fmt->fmt.pix_mp.width = ALIGN(f->fmt.pix_mp.width, 16);
		fmt->fmt.pix_mp.height = ALIGN(f->fmt.pix_mp.height, 16);
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
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

		/* update crop dimensions */
		inst->crop.x = inst->crop.y = 0;
		inst->crop.width = f->fmt.pix_mp.width;
		inst->crop.height = f->fmt.pix_mp.height;

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;
		//update_log_ctxt(inst->sid, inst->session_type,
		//	mplane->pixelformat);
		s_vpr_h(inst->sid,
			"%s: input: codec %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width,
			f->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.input.min_count,
			inst->buffers.input.extra_count);

		//msm_vidc_update_dcvs(inst);
		//msm_vidc_update_batching(inst);

	} else if (f->type == INPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_INPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[INPUT_META_PORT];
		fmt->type = INPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
		fmt->fmt.meta.buffersize = call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_INPUT_META);
		inst->buffers.input_meta.min_count =
				inst->buffers.input.min_count;
		inst->buffers.input_meta.extra_count =
				inst->buffers.input.extra_count;
		inst->buffers.input_meta.actual_count =
				inst->buffers.input.actual_count;
		inst->buffers.input_meta.size = fmt->fmt.meta.buffersize;
		s_vpr_h(inst->sid,
			"%s: input meta: size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.input_meta.min_count,
			inst->buffers.input_meta.extra_count);
	} else if (f->type == OUTPUT_MPLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[OUTPUT_PORT];
		fmt->type = OUTPUT_MPLANE;
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
		fmt->fmt.pix_mp.width = VENUS_Y_STRIDE(
			v4l2_colorformat_to_media(
			fmt->fmt.pix_mp.pixelformat, __func__),
			f->fmt.pix_mp.width);
		fmt->fmt.pix_mp.height = VENUS_Y_SCANLINES(
			v4l2_colorformat_to_media(
			fmt->fmt.pix_mp.pixelformat, __func__),
			f->fmt.pix_mp.height);
		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline =
			fmt->fmt.pix_mp.width;
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
			goto err_invalid_fmt;
		s_vpr_h(inst->sid,
			"%s: output: format %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.width,
			fmt->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.output.min_count,
			inst->buffers.output.extra_count);
	} else if (f->type == OUTPUT_META_PLANE) {
		if (inst->state == MSM_VIDC_START_OUTPUT) {
			d_vpr_e("%s: invalid state %d\n", __func__, inst->state);
			return -EINVAL;
		}
		fmt = &inst->fmts[OUTPUT_META_PORT];
		fmt->type = OUTPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
		fmt->fmt.meta.buffersize = call_session_op(core, buffer_size,
				inst, MSM_VIDC_BUF_OUTPUT_META);
		inst->buffers.output_meta.min_count =
				inst->buffers.output.min_count;
		inst->buffers.output_meta.extra_count =
				inst->buffers.output.extra_count;
		inst->buffers.output_meta.actual_count =
				inst->buffers.output.actual_count;
		inst->buffers.output_meta.size = fmt->fmt.meta.buffersize;
		s_vpr_h(inst->sid,
			"%s: output meta: size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.output_meta.min_count,
			inst->buffers.output_meta.extra_count);
	} else {
		s_vpr_e(inst->sid, "%s: invalid type %d\n", __func__, f->type);
		goto err_invalid_fmt;
	}
	memcpy(f, fmt, sizeof(struct v4l2_format));

err_invalid_fmt:
	return rc;
}

int msm_vdec_g_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	int port;

	d_vpr_h("%s()\n", __func__);
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

int msm_vdec_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
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

	if (f->type == INPUT_MPLANE) {
		u32 codecs = core->capabilities[DEC_CODECS].value;

		while (codecs) {
			if (idx > 31)
				break;
			if (codecs & BIT(i)) {
				array[idx] = codecs & BIT(i);
				idx++;
			}
			i++;
			codecs >>= 1;
		}
		f->pixelformat = v4l2_codec_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == OUTPUT_MPLANE) {
		u32 formats = inst->capabilities->cap[PIX_FMTS].step_or_mask;

		while (formats) {
			if (idx > 31)
				break;
			if (formats & BIT(i)) {
				array[idx] = formats & BIT(i);
				idx++;
			}
			i++;
			formats >>= 1;
		}
		f->pixelformat = v4l2_colorformat_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		strlcpy(f->description, "colorformat", sizeof(f->description));
	} else if (f->type == INPUT_META_PLANE || f->type == OUTPUT_META_PLANE) {
		if (!f->index) {
			f->pixelformat = V4L2_PIX_FMT_VIDC_META;
			strlcpy(f->description, "metadata", sizeof(f->description));
		} else {
			return -EINVAL;
		}
	}
	memset(f->reserved, 0, sizeof(f->reserved));

	s_vpr_h(inst->sid, "%s: index %d, %s : %#x, flags %#x\n",
		__func__, f->index, f->description, f->pixelformat, f->flags);
	return rc;
}

int msm_vdec_inst_init(struct msm_vidc_inst *inst)
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

	INIT_DELAYED_WORK(&inst->decode_batch.work, msm_vidc_batch_handler);

	f = &inst->fmts[INPUT_PORT];
	f->type = INPUT_MPLANE;
	f->fmt.pix_mp.width = DEFAULT_WIDTH;
	f->fmt.pix_mp.height = DEFAULT_HEIGHT;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
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

	inst->crop.x = inst->crop.y = 0;
	inst->crop.width = f->fmt.pix_mp.width;
	inst->crop.height = f->fmt.pix_mp.height;

	f = &inst->fmts[INPUT_META_PORT];
	f->type = INPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_INPUT_META);
	inst->buffers.input_meta.min_count = inst->buffers.input.min_count;
	inst->buffers.input_meta.extra_count = inst->buffers.input.extra_count;
	inst->buffers.input_meta.actual_count = inst->buffers.input.actual_count;
	inst->buffers.input_meta.size = f->fmt.meta.buffersize;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_MPLANE;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12_UBWC;
	f->fmt.pix_mp.width = VENUS_Y_STRIDE(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_WIDTH);
	f->fmt.pix_mp.height = VENUS_Y_SCANLINES(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_HEIGHT);
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = f->fmt.pix_mp.width;
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

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_PIX_FMT_VIDC_META;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_OUTPUT_META);
	inst->buffers.output_meta.min_count = inst->buffers.output.min_count;
	inst->buffers.output_meta.extra_count = inst->buffers.output.extra_count;
	inst->buffers.output_meta.actual_count = inst->buffers.output.actual_count;
	inst->buffers.output_meta.size = f->fmt.meta.buffersize;

	inst->prop.frame_rate = DEFAULT_FPS << 16;
	inst->prop.operating_rate = DEFAULT_FPS << 16;
	inst->stage = MSM_VIDC_STAGE_2;
	inst->pipe = MSM_VIDC_PIPE_4;

	rc = msm_vdec_codec_change(inst,
			inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat);

	return rc;
}

