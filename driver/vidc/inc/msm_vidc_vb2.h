/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_VB2_H_
#define _MSM_VIDC_VB2_H_

#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>
#include "msm_vidc_inst.h"

struct vb2_queue *msm_vidc_get_vb2q(struct msm_vidc_inst *inst,
	u32 type, const char *func);

/* vb2_mem_ops */
void *msm_vb2_get_userptr(struct device *dev, unsigned long vaddr,
			unsigned long size, enum dma_data_direction dma_dir);
void msm_vb2_put_userptr(void *buf_priv);
void* msm_vb2_attach_dmabuf(struct device* dev, struct dma_buf* dbuf,
	unsigned long size, enum dma_data_direction dma_dir);
void msm_vb2_detach_dmabuf(void* buf_priv);
int msm_vb2_map_dmabuf(void* buf_priv);
void msm_vb2_unmap_dmabuf(void* buf_priv);

/* vb2_ops */
int msm_vidc_queue_setup(struct vb2_queue *q,
		unsigned int *num_buffers, unsigned int *num_planes,
		unsigned int sizes[], struct device *alloc_devs[]);
int msm_vidc_start_streaming(struct vb2_queue *q, unsigned int count);
void msm_vidc_stop_streaming(struct vb2_queue *q);
void msm_vidc_buf_queue(struct vb2_buffer *vb2);
void msm_vidc_buf_cleanup(struct vb2_buffer *vb);
#endif // _MSM_VIDC_VB2_H_