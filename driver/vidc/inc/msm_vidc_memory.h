/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021,, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_MEMORY_H_
#define _MSM_VIDC_MEMORY_H_

#include "msm_vidc_internal.h"
#include "msm_vidc_core.h"

int msm_vidc_memory_alloc(struct msm_vidc_core *core,
	struct msm_vidc_alloc *alloc);
int msm_vidc_memory_free(struct msm_vidc_core *core,
	struct msm_vidc_alloc *alloc);
int msm_vidc_memory_map(struct msm_vidc_core *core,
	struct msm_vidc_map *map);
int msm_vidc_memory_unmap(struct msm_vidc_core *core,
	struct msm_vidc_map *map);
int msm_vidc_memory_unmap_completely(struct msm_vidc_core *core,
	struct msm_vidc_map *map);
struct dma_buf *msm_vidc_memory_get_dmabuf(int fd);
void msm_vidc_memory_put_dmabuf(void *dmabuf);

#endif // _MSM_VIDC_MEMORY_H_