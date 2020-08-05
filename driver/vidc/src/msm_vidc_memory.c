// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/dma-buf.h>
#include <linux/msm_ion.h>
#include <linux/ion.h>

#include "msm_vidc_memory.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_core.h"


static int get_ion_secure_flag(enum msm_vidc_buffer_region region)
{
	u32 ion_flag = 0;

	switch (region) {
	case MSM_VIDC_SECURE_PIXEL:
		ion_flag = ION_FLAG_CP_PIXEL;
		break;
	case MSM_VIDC_SECURE_NONPIXEL:
		ion_flag = ION_FLAG_CP_NON_PIXEL;
		break;
	case MSM_VIDC_SECURE_BITSTREAM:
		ion_flag = ION_FLAG_CP_BITSTREAM;
		break;
	default:
		d_vpr_e("invalid secure region : %#x\n", region);
	}

	return ion_flag;
}

struct context_bank_info *get_context_bank(struct msm_vidc_core *core,
		enum msm_vidc_buffer_region region)
{
	char *name;
	struct context_bank_info *cb = NULL, *match = NULL;

	switch (region) {
	case MSM_VIDC_NON_SECURE:
		name = "venus_ns";
		break;
	case MSM_VIDC_SECURE_PIXEL:
		name = "venus_sec_pixel";
		break;
	case MSM_VIDC_SECURE_NONPIXEL:
		name = "venus_sec_non_pixel";
		break;
	case MSM_VIDC_SECURE_BITSTREAM:
		name = "venus_sec_bitstream";
		break;
	default:
		d_vpr_e("invalid region : %#x\n", region);
		return NULL;
	}

	list_for_each_entry(cb, &core->dt->context_banks, list) {
		if (!strcmp(cb->name, name)) {
			match = cb;
			break;
		}
	}
	if (!match)
		d_vpr_e("cb not found for region %#x\n", region);

	return match;
}

struct dma_buf *msm_vidc_memory_get_dmabuf(int fd)
{
	struct dma_buf *dmabuf;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		d_vpr_e("Failed to get dmabuf for %d, error %ld\n",
				fd, PTR_ERR(dmabuf));
		dmabuf = NULL;
	}

	return dmabuf;
}

void msm_vidc_memory_put_dmabuf(void *dmabuf)
{
	if (!dmabuf) {
		d_vpr_e("%s: NULL dmabuf\n", __func__);
		return;
	}

	dma_buf_put((struct dma_buf *)dmabuf);
}

int msm_vidc_memory_map(struct msm_vidc_core *core, struct msm_vidc_map *map)
{
	int rc = 0;
	struct dma_buf_attachment *attach = NULL;
	struct sg_table *table = NULL;
	struct context_bank_info *cb = NULL;

	if (!core || !map) {
		d_vpr_e("%s: invalid params\n",	__func__);
		return -EINVAL;
	}

	if (map->refcount) {
		map->refcount++;
		return 0;
	}

	cb = get_context_bank(core, map->region);
	if (!cb) {
		d_vpr_e("%s: Failed to get context bank device\n",
			 __func__);
		rc = -EIO;
		goto error_cb;
	}

	/* Prepare a dma buf for dma on the given device */
	attach = dma_buf_attach(map->dmabuf, cb->dev);
	if (IS_ERR_OR_NULL(attach)) {
		rc = PTR_ERR(attach) ? PTR_ERR(attach) : -ENOMEM;
		d_vpr_e("Failed to attach dmabuf\n");
		goto error_attach;
	}

	/*
	 * Get the scatterlist for the given attachment
	 * Mapping of sg is taken care by map attachment
	 */
	attach->dma_map_attrs = DMA_ATTR_DELAYED_UNMAP;
	/*
	 * We do not need dma_map function to perform cache operations
	 * on the whole buffer size and hence pass skip sync flag.
	 * We do the required cache operations separately for the
	 * required buffer size
	 */
	attach->dma_map_attrs |= DMA_ATTR_SKIP_CPU_SYNC;
	if (core->dt->sys_cache_present)
		attach->dma_map_attrs |=
			DMA_ATTR_IOMMU_USE_UPSTREAM_HINT;

	table = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR_OR_NULL(table)) {
		rc = PTR_ERR(table) ? PTR_ERR(table) : -ENOMEM;
		d_vpr_e("Failed to map table\n");
		goto error_table;
	}
	if (!table->sgl) {
		d_vpr_e("sgl is NULL\n");
		rc = -ENOMEM;
		goto error_sg;
	}

	map->device_addr = table->sgl->dma_address;
	map->table = table;
	map->attach = attach;
	map->refcount++;
	return 0;

error_sg:
	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
error_table:
	dma_buf_detach(map->dmabuf, attach);
error_attach:
error_cb:
	return rc;
}

int msm_vidc_memory_unmap(struct msm_vidc_core *core, struct msm_vidc_map *map)
{
	int rc = 0;

	if (!core || !map) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (map->refcount) {
		map->refcount--;
	} else {
		d_vpr_e("unmap called while refcount is zero already\n");
		return -EINVAL;
	}

	if (map->refcount)
		goto exit;

	dma_buf_unmap_attachment(map->attach, map->table, DMA_BIDIRECTIONAL);
	dma_buf_detach(map->dmabuf, map->attach);

	map->device_addr = 0x0;
	map->dmabuf = NULL;
	map->attach = NULL;
	map->table = NULL;

exit:
	return rc;
}

int msm_vidc_memory_alloc(struct msm_vidc_core *core, struct msm_vidc_alloc *mem)
{
	int rc = 0;
	int ion_flags = 0;
	int ion_secure_flag = 0;
	unsigned long heap_mask = 0;
	int size = 0;

	if (!mem) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	size = ALIGN(mem->size, SZ_4K);

	if (mem->cached)
		ion_flags |= ION_FLAG_CACHED;

	if (mem->secure) {
		ion_secure_flag = get_ion_secure_flag(mem->region);
		ion_flags |= ION_FLAG_SECURE | ion_secure_flag;
		heap_mask = ION_HEAP(ION_SECURE_HEAP_ID);
	} else {
		heap_mask = ION_HEAP(ION_SYSTEM_HEAP_ID);
	}

	mem->dmabuf = ion_alloc(size, heap_mask, ion_flags);
	if (IS_ERR_OR_NULL(mem->dmabuf)) {
		d_vpr_e("%s: ion alloc failed\n", __func__);
		mem->dmabuf = NULL;
		rc = -ENOMEM;
		goto error;
	}

	if (mem->map_kernel) {
		dma_buf_begin_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL);
		mem->kvaddr = dma_buf_vmap(mem->dmabuf);
		if (!mem->kvaddr) {
			d_vpr_e("%s: kernel map failed\n", __func__);
			rc = -EIO;
			goto error;
		}
	}

	d_vpr_h(
		"%s: dmabuf = %pK, size = %d, kvaddr = %pK, buffer_type = %#x\n",
		__func__, mem->dmabuf, mem->size,
		mem->kvaddr, mem->buffer_type);
	return 0;

error:
	msm_vidc_memory_free(core, mem);
	return rc;
}

int msm_vidc_memory_free(struct msm_vidc_core *core, struct msm_vidc_alloc *mem)
{
	int rc = 0;

	if (!mem || !mem->dmabuf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	d_vpr_h(
		"%s: dmabuf = %pK, size = %d, kvaddr = %pK, buffer_type = %#x\n",
		__func__, mem->dmabuf, mem->size,
		mem->kvaddr, mem->buffer_type);

	if (mem->kvaddr) {
		dma_buf_vunmap(mem->dmabuf, mem->kvaddr);
		mem->kvaddr = NULL;
		dma_buf_end_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL);
	}

	if (mem->dmabuf) {
		dma_buf_put(mem->dmabuf);
		mem->dmabuf = NULL;
	}

	return rc;
};
/*
int msm_memory_cache_operations(struct dma_buf *dbuf,
	enum smem_cache_ops cache_op, unsigned long offset,
	unsigned long size, u32 sid)
{
	int rc = 0;
	unsigned long flags = 0;

	if (!dbuf) {
		s_vpr_e(sid, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = dma_buf_get_flags(dbuf, &flags);
	if (rc) {
		s_vpr_e(sid, "%s: dma_buf_get_flags failed, err %d\n",
			__func__, rc);
		return rc;
	} else if (!(flags & ION_FLAG_CACHED)) {
		return rc;
	}

	switch (cache_op) {
	case SMEM_CACHE_CLEAN:
	case SMEM_CACHE_CLEAN_INVALIDATE:
		rc = dma_buf_begin_cpu_access_partial(dbuf, DMA_TO_DEVICE,
				offset, size);
		if (rc)
			break;
		rc = dma_buf_end_cpu_access_partial(dbuf, DMA_TO_DEVICE,
				offset, size);
		break;
	case SMEM_CACHE_INVALIDATE:
		rc = dma_buf_begin_cpu_access_partial(dbuf, DMA_TO_DEVICE,
				offset, size);
		if (rc)
			break;
		rc = dma_buf_end_cpu_access_partial(dbuf, DMA_FROM_DEVICE,
				offset, size);
		break;
	default:
		s_vpr_e(sid, "%s: cache (%d) operation not supported\n",
			__func__, cache_op);
		rc = -EINVAL;
		break;
	}

	return rc;
}

int msm_smem_memory_prefetch(struct msm_vidc_inst *inst)
{
	int i, rc = 0;
	struct memory_regions *vidc_regions = NULL;
	struct ion_prefetch_region ion_region[MEMORY_REGIONS_MAX];

	if (!inst) {
		d_vpr_e("%s: invalid parameters\n", __func__);
		return -EINVAL;
	}

	vidc_regions = &inst->regions;
	if (vidc_regions->num_regions > MEMORY_REGIONS_MAX) {
		s_vpr_e(inst->sid, "%s: invalid num_regions %d, max %d\n",
			__func__, vidc_regions->num_regions,
			MEMORY_REGIONS_MAX);
		return -EINVAL;
	}

	memset(ion_region, 0, sizeof(ion_region));
	for (i = 0; i < vidc_regions->num_regions; i++) {
		ion_region[i].size = vidc_regions->region[i].size;
		ion_region[i].vmid = vidc_regions->region[i].vmid;
	}

	rc = msm_ion_heap_prefetch(ION_SECURE_HEAP_ID, ion_region,
		vidc_regions->num_regions);
	if (rc)
		s_vpr_e(inst->sid, "%s: prefetch failed, ret: %d\n",
			__func__, rc);
	else
		s_vpr_l(inst->sid, "%s: prefetch succeeded\n", __func__);

	return rc;
}

int msm_smem_memory_drain(struct msm_vidc_inst *inst)
{
	int i, rc = 0;
	struct memory_regions *vidc_regions = NULL;
	struct ion_prefetch_region ion_region[MEMORY_REGIONS_MAX];

	if (!inst) {
		d_vpr_e("%s: invalid parameters\n", __func__);
		return -EINVAL;
	}

	vidc_regions = &inst->regions;
	if (vidc_regions->num_regions > MEMORY_REGIONS_MAX) {
		s_vpr_e(inst->sid, "%s: invalid num_regions %d, max %d\n",
			__func__, vidc_regions->num_regions,
			MEMORY_REGIONS_MAX);
		return -EINVAL;
	}

	memset(ion_region, 0, sizeof(ion_region));
	for (i = 0; i < vidc_regions->num_regions; i++) {
		ion_region[i].size = vidc_regions->region[i].size;
		ion_region[i].vmid = vidc_regions->region[i].vmid;
	}

	rc = msm_ion_heap_drain(ION_SECURE_HEAP_ID, ion_region,
		vidc_regions->num_regions);
	if (rc)
		s_vpr_e(inst->sid, "%s: drain failed, ret: %d\n", __func__, rc);
	else
		s_vpr_l(inst->sid, "%s: drain succeeded\n", __func__);

	return rc;
}
*/
