// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/iommu.h>
#include <linux/workqueue.h>
#include <media/msm_vidc_utils.h>
#include <media/msm_media_info.h>

#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_memory.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"

void print_vidc_buffer(struct msm_vidc_inst *inst, struct msm_vidc_buffer *b)
{
}

int msm_vidc_get_port_from_v4l2_type(u32 type)
{
	int port;

	if (type == INPUT_PLANE) {
		port = INPUT_PORT;
	} else if (type == INPUT_META_PLANE) {
		port = INPUT_META_PORT;
	} else if (type == OUTPUT_PLANE) {
		port = OUTPUT_PORT;
	} else if (type == OUTPUT_META_PLANE) {
		port = OUTPUT_META_PORT;
	} else {
		d_vpr_e("%s: invalid type %d\n", __func__, type);
		port = -EINVAL;
	}

	return port;
}

u32 msm_vidc_get_buffer_region(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	u32 region = MSM_VIDC_NON_SECURE;

	if (!is_secure_session(inst))
		return region;

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
	case MSM_VIDC_BUF_SCRATCH:
		region = MSM_VIDC_SECURE_BITSTREAM;
		break;
	case MSM_VIDC_BUF_SCRATCH_1:
		region = MSM_VIDC_SECURE_NONPIXEL;
		break;
	case MSM_VIDC_BUF_SCRATCH_2:
		region = MSM_VIDC_SECURE_PIXEL;
		break;
	case MSM_VIDC_BUF_PERSIST:
		if (is_encode_session(inst))
			region = MSM_VIDC_SECURE_NONPIXEL;
		else
			region = MSM_VIDC_SECURE_BITSTREAM;
		break;
	case MSM_VIDC_BUF_PERSIST_1:
		region = MSM_VIDC_SECURE_NONPIXEL;
		break;
	default:
		s_vpr_e(inst->sid, "%s: invalid buffer type %d\n",
			__func__, buffer_type);
	}
	return region;
}

struct msm_vidc_buffer_info *msm_vidc_get_buffer_info(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
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
	case MSM_VIDC_BUF_SCRATCH:
		return &inst->buffers.scratch;
	case MSM_VIDC_BUF_SCRATCH_1:
		return &inst->buffers.scratch_1;
	case MSM_VIDC_BUF_SCRATCH_2:
		return &inst->buffers.scratch_2;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->buffers.persist;
	case MSM_VIDC_BUF_PERSIST_1:
		return &inst->buffers.persist_1;
	default:
		s_vpr_e(inst->sid, "%s: invalid buffer type %d\n",
			__func__, buffer_type);
		return NULL;
	}
}

struct msm_vidc_map_info *msm_vidc_get_map_info(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		return &inst->maps.input;
	case MSM_VIDC_BUF_INPUT_META:
		return &inst->maps.input_meta;
	case MSM_VIDC_BUF_OUTPUT:
		return &inst->maps.output;
	case MSM_VIDC_BUF_OUTPUT_META:
		return &inst->maps.output_meta;
	case MSM_VIDC_BUF_SCRATCH:
		return &inst->maps.scratch;
	case MSM_VIDC_BUF_SCRATCH_1:
		return &inst->maps.scratch_1;
	case MSM_VIDC_BUF_SCRATCH_2:
		return &inst->maps.scratch_2;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->maps.persist;
	case MSM_VIDC_BUF_PERSIST_1:
		return &inst->maps.persist_1;
	default:
		s_vpr_e(inst->sid, "%s: invalid buffer type %d\n",
			__func__, buffer_type);
		return NULL;
	}
}

struct msm_vidc_alloc_info *msm_vidc_get_alloc_info(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_SCRATCH:
		return &inst->allocations.scratch;
	case MSM_VIDC_BUF_SCRATCH_1:
		return &inst->allocations.scratch_1;
	case MSM_VIDC_BUF_SCRATCH_2:
		return &inst->allocations.scratch_2;
	case MSM_VIDC_BUF_PERSIST:
		return &inst->allocations.persist;
	case MSM_VIDC_BUF_PERSIST_1:
		return &inst->allocations.persist_1;
	default:
		s_vpr_e(inst->sid, "%s: invalid buffer type %d\n",
			__func__, buffer_type);
		return NULL;
	}
}

int msm_vidc_change_inst_state(struct msm_vidc_inst *inst,
		enum msm_vidc_inst_state request_state)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!request_state) {
		d_vpr_e("%s: invalid request state\n", __func__);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_ERROR) {
		s_vpr_h(inst->sid,
			"inst is in bad state, can not change state to %d\n",
			request_state);
		return 0;
	}

	s_vpr_h(inst->sid, "state changed from %d to %d\n",
		   inst->state, request_state);
	inst->state = request_state;
	return 0;
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
		s_vpr_h(inst->sid, "g_min: output buffers %d\n", ctrl->val);
		break;
	case V4L2_CID_MIN_BUFFERS_FOR_OUTPUT:
		ctrl->val = inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
		s_vpr_h(inst->sid, "g_min: input buffers %d\n", ctrl->val);
		break;
	default:
		break;
	}

	return rc;
}

u32 msm_vidc_convert_color_fmt(u32 v4l2_fmt)
{
	switch (v4l2_fmt) {
	case V4L2_PIX_FMT_NV12:
		return COLOR_FMT_NV12;
	case V4L2_PIX_FMT_NV21:
		return COLOR_FMT_NV21;
	case V4L2_PIX_FMT_NV12_512:
		return COLOR_FMT_NV12_512;
	case V4L2_PIX_FMT_SDE_Y_CBCR_H2V2_P010_VENUS:
		return COLOR_FMT_P010;
	case V4L2_PIX_FMT_NV12_UBWC:
		return COLOR_FMT_NV12_UBWC;
	case V4L2_PIX_FMT_NV12_TP10_UBWC:
		return COLOR_FMT_NV12_BPP10_UBWC;
	case V4L2_PIX_FMT_RGBA8888_UBWC:
		return COLOR_FMT_RGBA8888_UBWC;
	default:
		d_vpr_e(
			"Invalid v4l2 color fmt FMT : %x, Set default(NV12)",
			v4l2_fmt);
		return COLOR_FMT_NV12;
	}
}

int msm_vidc_create_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffer_info *buffer_info;
	struct msm_vidc_alloc_info *alloc_info;
	struct msm_vidc_map_info *map_info;
	int i;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		s_vpr_e(inst->sid, "%s: buffer type %#d is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffer_info = msm_vidc_get_buffer_info(inst, buffer_type);
	if (!buffer_info)
		return -EINVAL;
	alloc_info = msm_vidc_get_alloc_info(inst, buffer_type);
	if (!alloc_info)
		return -EINVAL;
	map_info = msm_vidc_get_map_info(inst, buffer_type);
	if (!alloc_info)
		return -EINVAL;

	for (i = 0; i < buffer_info->min_count; i++) {
		struct msm_vidc_buffer *buffer;
		struct msm_vidc_alloc *alloc;
		struct msm_vidc_map *map;

		if (!buffer_info->size) {
			d_vpr_e("%s: invalid buffer %#x\n", __func__, buffer_type);
			return -EINVAL;
		}
		buffer = kzalloc(sizeof(struct msm_vidc_buffer), GFP_KERNEL);
		if (!buffer) {
			s_vpr_e(inst->sid, "%s: msm_vidc_buffer alloc failed\n", __func__);
			return -ENOMEM;
		}
		INIT_LIST_HEAD(&buffer->list);
		buffer->valid = true;
		buffer->type = buffer_type;
		buffer->index = i;
		buffer->buffer_size = buffer_info->size;
		list_add_tail(&buffer->list, &buffer_info->list);

		alloc = kzalloc(sizeof(struct msm_vidc_alloc), GFP_KERNEL);
		if (!alloc) {
			s_vpr_e(inst->sid, "%s: msm_vidc_alloc alloc failed\n", __func__);
			return -ENOMEM;
		}
		INIT_LIST_HEAD(&alloc->list);
		alloc->buffer_type = buffer_type;
		alloc->region      = msm_vidc_get_buffer_region(inst, buffer_type);
		alloc->size        = buffer->buffer_size;
		rc = msm_vidc_memory_alloc(inst->core, alloc);
		if (rc)
			return -ENOMEM;
		list_add_tail(&alloc->list, &alloc_info->list);

		map = kzalloc(sizeof(struct msm_vidc_map), GFP_KERNEL);
		if (!map) {
			s_vpr_e(inst->sid, "%s: msm_vidc_map alloc failed\n", __func__);
			return -ENOMEM;
		}
		INIT_LIST_HEAD(&map->list);
		map->buffer_type  = alloc->buffer_type;
		map->region       = alloc->region;
		map->dmabuf       = alloc->dmabuf;
		rc = msm_vidc_memory_map(inst->core, map);
		if (rc)
			return -ENOMEM;
		list_add_tail(&map->list, &map_info->list);

		s_vpr_e(inst->sid, "%s: created buffer_type %d, size %d\n",
			__func__, buffer_type, buffer_info->size);
	}

	return 0;
}

int msm_vidc_queue_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffer_info *buffer_info;
	struct msm_vidc_buffer *buffer, *dummy;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		s_vpr_e(inst->sid, "%s: buffer type %#d is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffer_info = msm_vidc_get_buffer_info(inst, buffer_type);
	if (!buffer_info)
		return -EINVAL;

	list_for_each_entry_safe(buffer, dummy, &buffer_info->list, list) {
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

		s_vpr_e(inst->sid, "%s: queued buffer_type %d, size %d\n",
			__func__, buffer_type, buffer_info->size);
	}

	return 0;
}

int msm_vidc_release_internal_buffers(struct msm_vidc_inst *inst,
		enum msm_vidc_buffer_type buffer_type)
{
	int rc = 0;
	struct msm_vidc_buffer_info *buffer_info;
	struct msm_vidc_buffer *buffer, *dummy;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (!is_internal_buffer(buffer_type)) {
		s_vpr_e(inst->sid, "%s: buffer type %#d is not internal\n",
			__func__, buffer_type);
		return 0;
	}

	buffer_info = msm_vidc_get_buffer_info(inst, buffer_type);
	if (!buffer_info)
		return -EINVAL;

	list_for_each_entry_safe(buffer, dummy, &buffer_info->list, list) {
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

		s_vpr_e(inst->sid, "%s: released buffer_type %d, size %d\n",
			__func__, buffer_type, buffer_info->size);
	}

	return 0;
}

int msm_vidc_setup_event_queue(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int index;
	struct msm_vidc_core *core;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	// TODO: check decode is index = 0 and encode is index 1
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

static int vb2q_init(struct msm_vidc_inst *inst,
	struct vb2_queue *q, enum v4l2_buf_type type)
{
	struct msm_vidc_core *core;

	if (!inst || !q || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	q->type = type;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	q->ops = core->vb2_ops;
	q->mem_ops = core->vb2_mem_ops;
	q->drv_priv = inst;
	q->allow_zero_bytesused = 1;
	q->copy_timestamp = 1;
	return vb2_queue_init(q);
}

int msm_vidc_vb2_queue_init(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = vb2q_init(inst, &inst->vb2q[INPUT_PORT], INPUT_PLANE);
	if (rc)
		return rc;

	rc = vb2q_init(inst, &inst->vb2q[OUTPUT_PORT], OUTPUT_PLANE);
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

	mutex_lock(&core->lock);
	list_for_each_entry(i, &core->instances, list)
		count++;

	if (count < MAX_SUPPORTED_INSTANCES) {
		list_add_tail(&inst->list, &core->instances);
	} else {
		d_vpr_e("%s: total sessions %d exceeded max limit %d\n",
			__func__, count, MAX_SUPPORTED_INSTANCES);
		rc = -EINVAL;
	}
	mutex_unlock(&core->lock);

	/* assign session_id */
	inst->session_id = count + 1;
	inst->sid = inst->session_id;

	return rc;
}

int msm_vidc_session_open(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = venus_hfi_session_open(inst);
	if (rc)
		return rc;

	inst->session_created = true;
	return 0;
}

int msm_vidc_core_init(struct msm_vidc_core *core)
{
	int rc;

	d_vpr_h("%s()\n", __func__);
	if (!core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&core->lock);
	if (core->state == MSM_VIDC_CORE_ERROR) {
		d_vpr_e("%s: core invalid state\n", __func__);
		rc = -EINVAL;
		goto unlock;
	}
	if (core->state == MSM_VIDC_CORE_INIT) {
		rc = 0;
		goto unlock;
	}

	rc = venus_hfi_core_init(core);
	if (rc) {
		d_vpr_e("%s: core init failed\n", __func__);
		core->state = MSM_VIDC_CORE_DEINIT;
		goto unlock;
	}

	core->state = MSM_VIDC_CORE_INIT;
	core->smmu_fault_handled = false;
	core->ssr.trigger = false;

unlock:
	mutex_unlock(&core->lock);
	return rc;
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
}

void msm_vidc_batch_handler(struct work_struct *work)
{
}
