// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "msm_vidc_vb2.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_debug.h"

void *msm_vb2_get_userptr(struct device *dev, unsigned long vaddr,
			unsigned long size, enum dma_data_direction dma_dir)
{
	return (void *)0xdeadbeef;
}

void msm_vb2_put_userptr(void *buf_priv)
{
}

int msm_vidc_queue_setup(struct vb2_queue *q,
		unsigned int *num_buffers, unsigned int *num_planes,
		unsigned int sizes[], struct device *alloc_devs[])
{
	return 0;
}

int msm_vidc_start_streaming(struct vb2_queue *q, unsigned int count)
{
	return 0;
}

void msm_vidc_stop_streaming(struct vb2_queue *q)
{
}

void msm_vidc_buf_queue(struct vb2_buffer *vb2)
{
}

void msm_vidc_buf_cleanup(struct vb2_buffer *vb)
{
}