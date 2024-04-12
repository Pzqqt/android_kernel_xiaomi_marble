// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "msm_vidc_fence.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"

extern struct msm_vidc_core *g_core;

static const char *msm_vidc_dma_fence_get_driver_name(struct dma_fence *df)
{
	struct msm_vidc_fence *fence;

	if (df) {
		fence = container_of(df, struct msm_vidc_fence, dma_fence);
		return fence->name;
	}
	return "msm_vidc_dma_fence_get_driver_name: invalid fence";
}

static const char *msm_vidc_dma_fence_get_timeline_name(struct dma_fence *df)
{
	struct msm_vidc_fence *fence;

	if (df) {
		fence = container_of(df, struct msm_vidc_fence, dma_fence);
		return fence->name;
	}
	return "msm_vidc_dma_fence_get_timeline_name: invalid fence";
}

static void msm_vidc_dma_fence_release(struct dma_fence *df)
{
	struct msm_vidc_fence *fence;

	if (df) {
		fence = container_of(df, struct msm_vidc_fence, dma_fence);
		d_vpr_l("%s: name %s\n", __func__, fence->name);
		msm_vidc_vmem_free((void **)&fence);
	} else {
		d_vpr_e("%s: invalid fence\n", __func__);
	}
}

static const struct dma_fence_ops msm_vidc_dma_fence_ops = {
	.get_driver_name = msm_vidc_dma_fence_get_driver_name,
	.get_timeline_name = msm_vidc_dma_fence_get_timeline_name,
	.release = msm_vidc_dma_fence_release,
};

struct msm_vidc_fence *msm_vidc_fence_create(struct msm_vidc_inst *inst)
{
	struct msm_vidc_fence *fence = NULL;
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	rc = msm_vidc_vmem_alloc(sizeof(*fence), (void **)&fence, __func__);
	if (rc)
		return NULL;

	fence->fd = INVALID_FD;
	spin_lock_init(&fence->lock);
	dma_fence_init(&fence->dma_fence, &msm_vidc_dma_fence_ops,
		&fence->lock, inst->fence_context.ctx_num,
		++inst->fence_context.seq_num);
	snprintf(fence->name, sizeof(fence->name), "%s: %llu",
		inst->fence_context.name, inst->fence_context.seq_num);

	/* reset seqno to avoid going beyond INT_MAX */
	if (inst->fence_context.seq_num >= INT_MAX)
		inst->fence_context.seq_num = 0;

	INIT_LIST_HEAD(&fence->list);
	list_add_tail(&fence->list, &inst->fence_list);
	i_vpr_l(inst, "%s: created %s\n", __func__, fence->name);

	return fence;
}

int msm_vidc_create_fence_fd(struct msm_vidc_inst *inst,
	struct msm_vidc_fence *fence)
{
	int rc = 0;

	if (!inst || !fence) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	fence->fd = get_unused_fd_flags(0);
	if (fence->fd < 0) {
		i_vpr_e(inst, "%s: getting fd (%d) failed\n", __func__,
			fence->fd);
		rc = -EINVAL;
		goto err_fd;
	}
	fence->sync_file = sync_file_create(&fence->dma_fence);
	if (!fence->sync_file) {
		i_vpr_e(inst, "%s: sync_file_create failed\n", __func__);
		rc = -EINVAL;
		goto err_sync_file;
	}
	fd_install(fence->fd, fence->sync_file->file);

	i_vpr_l(inst, "%s: created fd %d for fence %s\n", __func__,
		fence->fd, fence->name);

	return 0;

err_sync_file:
	put_unused_fd(fence->fd);
err_fd:
	return rc;
}

struct msm_vidc_fence *msm_vidc_get_fence_from_id(
	struct msm_vidc_inst *inst, u32 fence_id)
{
	struct msm_vidc_fence *fence, *dummy_fence;
	bool found = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	list_for_each_entry_safe(fence, dummy_fence, &inst->fence_list, list) {
		if (fence->dma_fence.seqno == (u64)fence_id) {
			found = true;
			break;
		}
	}

	if (!found)
		return NULL;

	return fence;
}

int msm_vidc_fence_signal(struct msm_vidc_inst *inst, u32 fence_id)
{
	int rc = 0;
	struct msm_vidc_fence *fence;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	fence = msm_vidc_get_fence_from_id(inst, fence_id);
	if (!fence) {
		i_vpr_e(inst, "%s: no fence available to signal with id: %u\n",
			__func__, fence_id);
		rc = -EINVAL;
		goto exit;
	}

	i_vpr_l(inst, "%s: fence %s\n", __func__, fence->name);
	list_del_init(&fence->list);
	dma_fence_signal(&fence->dma_fence);
	dma_fence_put(&fence->dma_fence);

exit:
	return rc;
}

void msm_vidc_fence_destroy(struct msm_vidc_inst *inst, u32 fence_id)
{
	struct msm_vidc_fence *fence;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	fence = msm_vidc_get_fence_from_id(inst, fence_id);
	if (!fence) {
		return;
	}

	i_vpr_e(inst, "%s: fence %s\n", __func__, fence->name);
	list_del_init(&fence->list);
	dma_fence_set_error(&fence->dma_fence, -EINVAL);
	dma_fence_signal(&fence->dma_fence);
	dma_fence_put(&fence->dma_fence);
}

int msm_vidc_fence_init(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	inst->fence_context.ctx_num = dma_fence_context_alloc(1);
	snprintf(inst->fence_context.name, sizeof(inst->fence_context.name),
		"msm_vidc_fence: %s: %llu", inst->debug_str,
		inst->fence_context.ctx_num);
	i_vpr_h(inst, "%s: %s\n", __func__, inst->fence_context.name);

	return rc;
}

void msm_vidc_fence_deinit(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}
	i_vpr_h(inst, "%s: %s\n", __func__, inst->fence_context.name);
	inst->fence_context.ctx_num = 0;
	snprintf(inst->fence_context.name, sizeof(inst->fence_context.name),
		"%s", "");
}
