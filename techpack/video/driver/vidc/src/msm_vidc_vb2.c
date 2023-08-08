// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_vb2.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_power.h"
#include "msm_vdec.h"
#include "msm_venc.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_control.h"

struct vb2_queue *msm_vidc_get_vb2q(struct msm_vidc_inst *inst,
	u32 type, const char *func)
{
	struct vb2_queue *q = NULL;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", func);
		return NULL;
	}
	if (type == INPUT_MPLANE) {
		q = &inst->vb2q[INPUT_PORT];
	} else if (type == OUTPUT_MPLANE) {
		q = &inst->vb2q[OUTPUT_PORT];
	} else if (type == INPUT_META_PLANE) {
		q = &inst->vb2q[INPUT_META_PORT];
	} else if (type == OUTPUT_META_PLANE) {
		q = &inst->vb2q[OUTPUT_META_PORT];
	} else {
		i_vpr_e(inst, "%s: invalid buffer type %d\n",
			__func__, type);
	}
	return q;
}

#if (KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
void *msm_vb2_get_userptr(struct device *dev, unsigned long vaddr,
			unsigned long size, enum dma_data_direction dma_dir)
{
	return (void *)0xdeadbeef;
}
void* msm_vb2_attach_dmabuf(struct device* dev, struct dma_buf* dbuf,
	unsigned long size, enum dma_data_direction dma_dir)
{
	return (void*)0xdeadbeef;
}
#else
void *msm_vb2_get_userptr(struct vb2_buffer *vb, struct device *dev,
			unsigned long vaddr, unsigned long size)
{
	return (void *)0xdeadbeef;
}
void *msm_vb2_attach_dmabuf(struct vb2_buffer *vb, struct device *dev,
	struct dma_buf *dbuf, unsigned long size)
{
	return (void *)0xdeadbeef;
}
#endif
void msm_vb2_put_userptr(void *buf_priv)
{
}
void msm_vb2_detach_dmabuf(void* buf_priv)
{
}

int msm_vb2_map_dmabuf(void* buf_priv)
{
	return 0;
}

void msm_vb2_unmap_dmabuf(void* buf_priv)
{
}

int msm_vidc_queue_setup(struct vb2_queue *q,
		unsigned int *num_buffers, unsigned int *num_planes,
		unsigned int sizes[], struct device *alloc_devs[])
{
	int rc = 0;
	struct msm_vidc_inst *inst;
	int port;

	if (!q || !num_buffers || !num_planes
		|| !sizes || !q->drv_priv) {
		d_vpr_e("%s: invalid params, q = %pK, %pK, %pK\n",
			__func__, q, num_buffers, num_planes);
		return -EINVAL;
	}
	inst = q->drv_priv;
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params %pK\n", __func__, inst);
		return -EINVAL;
	}

	if (inst->state == MSM_VIDC_START) {
		i_vpr_e(inst, "%s: invalid state %d\n", __func__, inst->state);
		return -EINVAL;
	}

	port = v4l2_type_to_driver_port(inst, q->type, __func__);
	if (port < 0)
		return -EINVAL;

	if (port == INPUT_PORT) {
		*num_planes = 1;
		if (*num_buffers < inst->buffers.input.min_count +
			inst->buffers.input.extra_count)
			*num_buffers = inst->buffers.input.min_count +
				inst->buffers.input.extra_count;
		inst->buffers.input.actual_count = *num_buffers;

	} else if (port == INPUT_META_PORT) {
		*num_planes = 1;
		if (*num_buffers < inst->buffers.input_meta.min_count +
			inst->buffers.input_meta.extra_count)
			*num_buffers = inst->buffers.input_meta.min_count +
				inst->buffers.input_meta.extra_count;
		inst->buffers.input_meta.actual_count = *num_buffers;

	} else if (port == OUTPUT_PORT) {
		*num_planes = 1;
		if (*num_buffers < inst->buffers.output.min_count +
			inst->buffers.output.extra_count)
			*num_buffers = inst->buffers.output.min_count +
				inst->buffers.output.extra_count;
		inst->buffers.output.actual_count = *num_buffers;

	} else if (port == OUTPUT_META_PORT) {
		*num_planes = 1;
		if (*num_buffers < inst->buffers.output_meta.min_count +
			inst->buffers.output_meta.extra_count)
			*num_buffers = inst->buffers.output_meta.min_count +
				inst->buffers.output_meta.extra_count;
		inst->buffers.output_meta.actual_count = *num_buffers;
	}

	if (port == INPUT_PORT || port == OUTPUT_PORT) {
		sizes[0] = inst->fmts[port].fmt.pix_mp.plane_fmt[0].sizeimage;
	} else if (port == OUTPUT_META_PORT) {
		sizes[0] = inst->fmts[port].fmt.meta.buffersize;
	} else if (port == INPUT_META_PORT) {
		if (inst->capabilities->cap[SUPER_FRAME].value)
			sizes[0] = inst->capabilities->cap[SUPER_FRAME].value *
				inst->fmts[port].fmt.meta.buffersize;
		else
			sizes[0] = inst->fmts[port].fmt.meta.buffersize;
	}

	i_vpr_h(inst,
		"queue_setup: type %s num_buffers %d sizes[0] %d\n",
		v4l2_type_name(q->type), *num_buffers, sizes[0]);
	return rc;
}

int msm_vidc_start_streaming(struct vb2_queue *q, unsigned int count)
{
	int rc = 0;
	struct msm_vidc_inst *inst;
	enum msm_vidc_buffer_type buf_type;

	if (!q || !q->drv_priv) {
		d_vpr_e("%s: invalid input, q = %pK\n", __func__, q);
		return -EINVAL;
	}
	inst = q->drv_priv;
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	if (q->type == INPUT_META_PLANE || q->type == OUTPUT_META_PLANE) {
		i_vpr_h(inst, "%s: nothing to start on %s\n",
			__func__, v4l2_type_name(q->type));
		return 0;
	}
	if (!is_decode_session(inst) && !is_encode_session(inst)) {
		i_vpr_e(inst, "%s: invalid session %d\n",
			__func__, inst->domain);
		return -EINVAL;
	}
	i_vpr_h(inst, "Streamon: %s\n", v4l2_type_name(q->type));

	if (!inst->once_per_session_set) {
		inst->once_per_session_set = true;
		rc = msm_vidc_session_set_codec(inst);
		if (rc)
			return rc;

		rc = msm_vidc_session_set_secure_mode(inst);
		if (rc)
			return rc;

		if (is_encode_session(inst)) {
			rc = msm_vidc_alloc_and_queue_session_internal_buffers(inst,
				MSM_VIDC_BUF_ARP);
			if (rc)
				goto error;
		} else if(is_decode_session(inst)) {
			rc = msm_vidc_session_set_default_header(inst);
			if (rc)
				return rc;

			rc = msm_vidc_alloc_and_queue_session_internal_buffers(inst,
				MSM_VIDC_BUF_PERSIST);
			if (rc)
				goto error;
		}
	}

	if (is_decode_session(inst))
		inst->decode_batch.enable = msm_vidc_allow_decode_batch(inst);

	msm_vidc_allow_dcvs(inst);
	msm_vidc_power_data_reset(inst);

	if (q->type == INPUT_MPLANE) {
		if (is_decode_session(inst))
			rc = msm_vdec_streamon_input(inst);
		else if (is_encode_session(inst))
			rc = msm_venc_streamon_input(inst);
		else
			goto error;
	} else if (q->type == OUTPUT_MPLANE) {
		if (is_decode_session(inst))
			rc = msm_vdec_streamon_output(inst);
		else if (is_encode_session(inst))
			rc = msm_venc_streamon_output(inst);
		else
			goto error;
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, q->type);
		goto error;
	}
	if (rc)
		goto error;

	/* print final buffer counts & size details */
	msm_vidc_print_buffer_info(inst);

	buf_type = v4l2_type_to_driver(q->type, __func__);
	if (!buf_type)
		goto error;

	/* queue pending buffers */
	rc = msm_vidc_queue_deferred_buffers(inst, buf_type);
	if (rc)
		goto error;

	/* initialize statistics timer(one time) */
	if (!inst->stats.time_ms)
		inst->stats.time_ms = ktime_get_ns() / 1000 / 1000;

	/* schedule to print buffer statistics */
	rc = schedule_stats_work(inst);
	if (rc)
		goto error;

	if ((q->type == INPUT_MPLANE && inst->vb2q[OUTPUT_PORT].streaming) ||
		(q->type == OUTPUT_MPLANE && inst->vb2q[INPUT_PORT].streaming)) {
		rc = msm_vidc_get_properties(inst);
		if (rc)
			goto error;
	}

	i_vpr_h(inst, "Streamon: %s successful\n", v4l2_type_name(q->type));

	return rc;

error:
	i_vpr_e(inst, "Streamon: %s failed\n", v4l2_type_name(q->type));
	return rc;
}

void msm_vidc_stop_streaming(struct vb2_queue *q)
{
	int rc = 0;
	struct msm_vidc_inst *inst;

	if (!q || !q->drv_priv) {
		d_vpr_e("%s: invalid input, q = %pK\n", __func__, q);
		return;
	}
	inst = q->drv_priv;
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	if (q->type == INPUT_META_PLANE || q->type == OUTPUT_META_PLANE) {
		i_vpr_h(inst, "%s: nothing to stop on %s\n",
			__func__, v4l2_type_name(q->type));
		return;
	}
	if (!is_decode_session(inst) && !is_encode_session(inst)) {
		i_vpr_e(inst, "%s: invalid session %d\n",
			__func__, inst->domain);
		return;
	}
	i_vpr_h(inst, "Streamoff: %s\n", v4l2_type_name(q->type));

	if (q->type == INPUT_MPLANE) {
		if (is_decode_session(inst))
			rc = msm_vdec_streamoff_input(inst);
		else if (is_encode_session(inst))
			rc = msm_venc_streamoff_input(inst);
	} else if (q->type == OUTPUT_MPLANE) {
		if (is_decode_session(inst))
			rc = msm_vdec_streamoff_output(inst);
		else if (is_encode_session(inst))
			rc = msm_venc_streamoff_output(inst);
	} else {
		i_vpr_e(inst, "%s: invalid type %d\n", __func__, q->type);
		goto error;
	}
	if (rc)
		goto error;

	/* Input port streamoff - flush timestamps list*/
	if (q->type == INPUT_MPLANE)
		msm_vidc_flush_ts(inst);

	i_vpr_h(inst, "Streamoff: %s successful\n", v4l2_type_name(q->type));
	return;

error:
	i_vpr_e(inst, "Streamoff: %s failed\n", v4l2_type_name(q->type));
	return;
}

void msm_vidc_buf_queue(struct vb2_buffer *vb2)
{
	int rc = 0;
	struct msm_vidc_inst *inst;

	inst = vb2_get_drv_priv(vb2->vb2_queue);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	if (is_decode_session(inst))
		rc = msm_vdec_qbuf(inst, vb2);
	else if (is_encode_session(inst))
		rc = msm_venc_qbuf(inst, vb2);
	else
		rc = -EINVAL;

	if (rc) {
		print_vb2_buffer("failed vb2-qbuf", inst, vb2);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		vb2_buffer_done(vb2, VB2_BUF_STATE_ERROR);
	}
}

void msm_vidc_buf_cleanup(struct vb2_buffer *vb)
{
}
