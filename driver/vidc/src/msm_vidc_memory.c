// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/dma-mapping.h>
#include <linux/qcom-dma-mapping.h>
#include <linux/msm_ion.h>
#include <linux/ion.h>

#include "msm_vidc_memory.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_core.h"

struct msm_vidc_buf_region_name {
	enum msm_vidc_buffer_region region;
	char *name;
};

struct context_bank_info *get_context_bank(struct msm_vidc_core *core,
		enum msm_vidc_buffer_region region)
{
	const char *name;
	struct context_bank_info *cb = NULL, *match = NULL;
	static const struct msm_vidc_buf_region_name buf_region_name[] = {
		{MSM_VIDC_REGION_NONE,          "none"                 },
		{MSM_VIDC_NON_SECURE,           "venus_ns"             },
		{MSM_VIDC_NON_SECURE_PIXEL,     "venus_ns_pixel"       },
		{MSM_VIDC_SECURE_PIXEL,         "venus_sec_pixel"      },
		{MSM_VIDC_SECURE_NONPIXEL,      "venus_sec_non_pixel"  },
		{MSM_VIDC_SECURE_BITSTREAM,     "venus_sec_bitstream"  },
	};

	if (!region || region > ARRAY_SIZE(buf_region_name))
		goto exit;

	if (buf_region_name[region].region != region)
		goto exit;

	name = buf_region_name[region].name;

	list_for_each_entry(cb, &core->dt->context_banks, list) {
		if (!strcmp(cb->name, name)) {
			match = cb;
			break;
		}
	}
	if (!match)
		d_vpr_e("cb not found for region %#x\n", region);

	return match;

exit:
	d_vpr_e("Invalid region %#x\n", region);
	return NULL;
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
		goto exit;
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

	if (!map->skip_delayed_unmap) {
		/*
		 * Get the scatterlist for the given attachment
		 * Mapping of sg is taken care by map attachment
		 */
		attach->dma_map_attrs |= DMA_ATTR_DELAYED_UNMAP;
	}

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

exit:
	d_vpr_l(
		"%s: type %11s, device_addr %#x, refcount %d, region %d\n",
		__func__, buf_name(map->type), map->device_addr, map->refcount, map->region);
	return 0;
error_sg:
	dma_buf_unmap_attachment(attach, table, DMA_BIDIRECTIONAL);
error_table:
	dma_buf_detach(map->dmabuf, attach);
error_attach:
error_cb:
	return rc;
}

int msm_vidc_memory_unmap(struct msm_vidc_core *core,
	struct msm_vidc_map *map)
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

	d_vpr_l(
		"%s: type %11s, device_addr %#x, refcount %d, region %d\n",
		__func__, buf_name(map->type), map->device_addr, map->refcount, map->region);

	if (map->refcount)
		goto exit;

	dma_buf_unmap_attachment(map->attach, map->table, DMA_BIDIRECTIONAL);
	dma_buf_detach(map->dmabuf, map->attach);

	map->device_addr = 0x0;
	map->attach = NULL;
	map->table = NULL;

exit:
	return rc;
}

int msm_vidc_memory_alloc(struct msm_vidc_core *core, struct msm_vidc_alloc *mem)
{
	int rc = 0;
	int size = 0;
	struct dma_heap *heap;
	char *heap_name = NULL;

	if (!mem) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	size = ALIGN(mem->size, SZ_4K);

	if (mem->secure) {
		switch (mem->region) {
		case MSM_VIDC_SECURE_PIXEL:
			heap_name = "qcom,secure-pixel";
			break;
		case MSM_VIDC_SECURE_NONPIXEL:
			heap_name = "qcom,secure-non-pixel";
			break;
		case MSM_VIDC_SECURE_BITSTREAM:
		default:
			d_vpr_e("invalid secure region : %#x\n", mem->region);
			return -EINVAL;
		}
	} else {
		heap_name = "qcom,system";
	}

	heap = dma_heap_find(heap_name);
	mem->dmabuf = dma_heap_buffer_alloc(heap, size, 0, 0);
	if (IS_ERR_OR_NULL(mem->dmabuf)) {
		d_vpr_e("%s: dma heap %s alloc failed\n", __func__, heap_name);
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
		"%s: dmabuf %pK, size %d, kvaddr %pK, buffer_type %s, secure %d, region %d\n",
		__func__, mem->dmabuf, mem->size, mem->kvaddr, buf_name(mem->type),
		mem->secure, mem->region);
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
		"%s: dmabuf %pK, size %d, kvaddr %pK, buffer_type %s, secure %d, region %d\n",
		__func__, mem->dmabuf, mem->size, mem->kvaddr, buf_name(mem->type),
		mem->secure, mem->region);

	if (mem->kvaddr) {
		dma_buf_vunmap(mem->dmabuf, mem->kvaddr);
		mem->kvaddr = NULL;
		dma_buf_end_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL);
	}

	if (mem->dmabuf) {
		dma_heap_buffer_free(mem->dmabuf);
		mem->dmabuf = NULL;
	}

	return rc;
};
/*
int msm_memory_cache_operations(struct msm_vidc_inst *inst,
	struct dma_buf *dbuf, enum smem_cache_ops cache_op,
	unsigned long offset, unsigned long size, u32 sid)
{
	int rc = 0;
	unsigned long flags = 0;

	if (!inst) {
		d_vpr_e("%s: invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (!dbuf) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = dma_buf_get_flags(dbuf, &flags);
	if (rc) {
		i_vpr_e(inst, "%s: dma_buf_get_flags failed, err %d\n",
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
		i_vpr_e(inst, "%s: cache (%d) operation not supported\n",
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
		i_vpr_e(inst, "%s: invalid num_regions %d, max %d\n",
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
		i_vpr_e(inst, "%s: prefetch failed, ret: %d\n",
			__func__, rc);
	else
		i_vpr_l(inst, "%s: prefetch succeeded\n", __func__);

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
		i_vpr_e(inst, "%s: invalid num_regions %d, max %d\n",
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
		i_vpr_e(inst, "%s: drain failed, ret: %d\n", __func__, rc);
	else
		i_vpr_l(inst, "%s: drain succeeded\n", __func__);

	return rc;
}
*/
