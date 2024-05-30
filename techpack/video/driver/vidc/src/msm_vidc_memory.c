// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2022, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/dma-mapping.h>
#include <linux/qcom-dma-mapping.h>
#include <linux/mem-buf.h>
#include <soc/qcom/secure_buffer.h>

#include "msm_vidc_memory.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_dt.h"
#include "msm_vidc_core.h"
#include "msm_vidc_events.h"

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

struct dma_buf *msm_vidc_memory_get_dmabuf(struct msm_vidc_inst *inst, int fd)
{
	struct msm_memory_dmabuf *buf = NULL;
	struct dma_buf *dmabuf = NULL;
	bool found = false;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return NULL;
	}

	/* get local dmabuf ref for tracking */
	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		d_vpr_e("Failed to get dmabuf for %d, error %ld\n",
				fd, PTR_ERR(dmabuf));
		return NULL;
	}

	/* track dmabuf - inc refcount if already present */
	list_for_each_entry(buf, &inst->dmabuf_tracker, list) {
		if (buf->dmabuf == dmabuf) {
			buf->refcount++;
			found = true;
			break;
		}
	}
	if (found) {
		/* put local dmabuf ref */
		dma_buf_put(dmabuf);
		return dmabuf;
	}

	/* get tracker instance from pool */
	buf = msm_memory_alloc(inst, MSM_MEM_POOL_DMABUF);
	if (!buf) {
		i_vpr_e(inst, "%s: dmabuf alloc failed\n", __func__);
		dma_buf_put(dmabuf);
		return NULL;
	}
	/* hold dmabuf strong ref in tracker */
	buf->dmabuf = dmabuf;
	buf->refcount = 1;
	INIT_LIST_HEAD(&buf->list);

	/* add new dmabuf entry to tracker */
	list_add_tail(&buf->list, &inst->dmabuf_tracker);

	return dmabuf;
}

void msm_vidc_memory_put_dmabuf(struct msm_vidc_inst *inst, struct dma_buf *dmabuf)
{
	struct msm_memory_dmabuf *buf = NULL;
	bool found = false;

	if (!inst || !dmabuf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	/* track dmabuf - dec refcount if already present */
	list_for_each_entry(buf, &inst->dmabuf_tracker, list) {
		if (buf->dmabuf == dmabuf) {
			buf->refcount--;
			found = true;
			break;
		}
	}
	if (!found) {
		i_vpr_e(inst, "%s: invalid dmabuf %#x\n", __func__, dmabuf);
		return;
	}

	/* non-zero refcount - do nothing */
	if (buf->refcount)
		return;

	/* remove dmabuf entry from tracker */
	list_del(&buf->list);

	/* release dmabuf strong ref from tracker */
	dma_buf_put(buf->dmabuf);

	/* put tracker instance back to pool */
	msm_memory_free(inst, buf);
}

void msm_vidc_memory_put_dmabuf_completely(struct msm_vidc_inst *inst,
	struct msm_memory_dmabuf *buf)
{
	if (!inst || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return;
	}

	while (buf->refcount) {
		buf->refcount--;
		if (!buf->refcount) {
			/* remove dmabuf entry from tracker */
			list_del(&buf->list);

			/* release dmabuf strong ref from tracker */
			dma_buf_put(buf->dmabuf);

			/* put tracker instance back to pool */
			msm_memory_free(inst, buf);
			break;
		}
	}
}

static bool is_non_secure_buffer(struct dma_buf *dmabuf)
{
	return mem_buf_dma_buf_exclusive_owner(dmabuf);
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

	/* reject non-secure mapping request for a secure buffer(or vice versa) */
	if (map->region == MSM_VIDC_NON_SECURE || map->region == MSM_VIDC_NON_SECURE_PIXEL) {
		if (!is_non_secure_buffer(map->dmabuf)) {
			d_vpr_e("%s: secure buffer mapping to non-secure region %d not allowed\n",
				__func__, map->region);
			return -EINVAL;
		}
	} else {
		if (is_non_secure_buffer(map->dmabuf)) {
			d_vpr_e("%s: non-secure buffer mapping to secure region %d not allowed\n",
				__func__, map->region);
			return -EINVAL;
		}
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

int msm_vidc_vmem_alloc(unsigned long size, void **mem, const char *msg)
{
	int rc = 0;

	if (*mem) {
		d_vpr_e("%s: error: double alloc\n", msg);
		rc = -EINVAL;
	}

	*mem = vzalloc(size);
	if (!*mem) {
		d_vpr_e("allocation failed for %s\n", msg);
		rc = -ENOMEM;
	}

	return rc;
}

void msm_vidc_vmem_free(void **addr)
{
	if (addr && *addr) {
		vfree(*addr);
		*addr = NULL;
	}
}

int msm_vidc_memory_alloc(struct msm_vidc_core *core, struct msm_vidc_alloc *mem)
{
	int rc = 0;
	int size = 0;
	struct dma_heap *heap;
	char *heap_name = NULL;
	struct mem_buf_lend_kernel_arg lend_arg;
	int vmids[1];
	int perms[1];

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
			heap_name = "qcom,system";
			break;
		default:
			d_vpr_e("invalid secure region : %#x\n", mem->region);
			return -EINVAL;
		}
	} else {
		if (core->is_non_coherent)
			heap_name = "qcom,system-uncached";
		else
			heap_name = "qcom,system";
	}

	heap = dma_heap_find(heap_name);
	if (!heap) {
		d_vpr_e("%s: No heap named %s\n", __func__, heap_name);
		rc = -ENOMEM;
		goto error;
	}
	mem->dmabuf = dma_heap_buffer_alloc(heap, size, 0, 0);
	if (IS_ERR_OR_NULL(mem->dmabuf)) {
		d_vpr_e("%s: dma heap %s alloc failed\n", __func__, heap_name);
		mem->dmabuf = NULL;
		rc = -ENOMEM;
		goto error;
	}

	if (mem->secure && mem->type == MSM_VIDC_BUF_BIN)
	{
		vmids[0] = VMID_CP_BITSTREAM;
		perms[0] = PERM_READ | PERM_WRITE;

		lend_arg.nr_acl_entries = ARRAY_SIZE(vmids);
		lend_arg.vmids = vmids;
		lend_arg.perms = perms;

		rc = mem_buf_lend(mem->dmabuf, &lend_arg);
		if (rc) {
			d_vpr_e("%s: BIN dmabuf %pK LEND failed, rc %d heap %s\n",
				__func__, mem->dmabuf, rc, heap_name);
			goto error;
		}
	}

	if (mem->map_kernel) {
		dma_buf_begin_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL);

#if (KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
		mem->kvaddr = dma_buf_vmap(mem->dmabuf);
		if (!mem->kvaddr) {
			d_vpr_e("%s: kernel map failed\n", __func__);
			rc = -EIO;
			goto error;
		}
#else
		rc = dma_buf_vmap(mem->dmabuf, &mem->dmabuf_map);
		if (rc) {
			d_vpr_e("%s: kernel map failed\n", __func__);
			rc = -EIO;
			goto error;
		}
		mem->kvaddr = mem->dmabuf_map.vaddr;
#endif
	}

	d_vpr_h(
		"%s: dmabuf %pK, size %d, kvaddr %pK, buffer_type %s, secure %d, region %d\n",
		__func__, mem->dmabuf, mem->size, mem->kvaddr, buf_name(mem->type),
		mem->secure, mem->region);
	trace_msm_vidc_dma_buffer("ALLOC", mem->dmabuf, mem->size, mem->kvaddr,
		buf_name(mem->type), mem->secure, mem->region);

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

	trace_msm_vidc_dma_buffer("FREE", mem->dmabuf, mem->size, mem->kvaddr,
		buf_name(mem->type), mem->secure, mem->region);

	if (mem->kvaddr) {
#if (KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE)
		dma_buf_vunmap(mem->dmabuf, mem->kvaddr);
#else
		dma_buf_vunmap(mem->dmabuf, &mem->dmabuf_map);
#endif
		mem->kvaddr = NULL;
		dma_buf_end_cpu_access(mem->dmabuf, DMA_BIDIRECTIONAL);
	}

	if (mem->dmabuf) {
		dma_heap_buffer_free(mem->dmabuf);
		mem->dmabuf = NULL;
	}

	return rc;
};

void *msm_memory_alloc(struct msm_vidc_inst *inst, enum msm_memory_pool_type type)
{
	struct msm_memory_alloc_header *hdr = NULL;
	struct msm_memory_pool *pool;
	int rc = 0;

	if (!inst || type < 0 || type >= MSM_MEM_POOL_MAX) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return NULL;
	}
	pool = &inst->pool[type];

	if (!list_empty(&pool->free_pool)) {
		/* get 1st node from free pool */
		hdr = list_first_entry(&pool->free_pool,
			struct msm_memory_alloc_header, list);
		list_del_init(&hdr->list);

		/* reset existing data */
		memset((char *)hdr->buf, 0, pool->size);

		/* add to busy pool */
		list_add_tail(&hdr->list, &pool->busy_pool);

		/* set busy flag to true. This is to catch double free request */
		hdr->busy = true;

		return hdr->buf;
	}

	rc = msm_vidc_vmem_alloc(pool->size + sizeof(struct msm_memory_alloc_header),
			(void **)&hdr, __func__);
	if (rc)
		return NULL;

	INIT_LIST_HEAD(&hdr->list);
	hdr->type = type;
	hdr->busy = true;
	hdr->buf = (void *)(hdr + 1);
	list_add_tail(&hdr->list, &pool->busy_pool);

	return hdr->buf;
}

void msm_memory_free(struct msm_vidc_inst *inst, void *vidc_buf)
{
	struct msm_memory_alloc_header *hdr;
	struct msm_memory_pool *pool;

	if (!inst || !vidc_buf) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return;
	}
	hdr = (struct msm_memory_alloc_header *)vidc_buf - 1;

	/* sanitize buffer addr */
	if (hdr->buf != vidc_buf) {
		i_vpr_e(inst, "%s: invalid buf addr %#x\n", __func__, vidc_buf);
		return;
	}

	/* sanitize pool type */
	if (hdr->type < 0 || hdr->type >= MSM_MEM_POOL_MAX) {
		i_vpr_e(inst, "%s: invalid pool type %#x\n", __func__, hdr->type);
		return;
	}
	pool = &inst->pool[hdr->type];

	/* catch double-free request */
	if (!hdr->busy) {
		i_vpr_e(inst, "%s: double free request. type %s, addr %#x\n", __func__,
			pool->name, vidc_buf);
		return;
	}
	hdr->busy = false;

	/* remove from busy pool */
	list_del_init(&hdr->list);

	/* add to free pool */
	list_add_tail(&hdr->list, &pool->free_pool);
}

static void msm_vidc_destroy_pool_buffers(struct msm_vidc_inst *inst,
	enum msm_memory_pool_type type)
{
	struct msm_memory_alloc_header *hdr, *dummy;
	struct msm_memory_pool *pool;
	u32 fcount = 0, bcount = 0;

	if (!inst || type < 0 || type >= MSM_MEM_POOL_MAX) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return;
	}
	pool = &inst->pool[type];

	/* detect memleak: busy pool is expected to be empty here */
	if (!list_empty(&pool->busy_pool))
		i_vpr_e(inst, "%s: destroy request on active buffer. type %s\n",
			__func__, pool->name);

	/* destroy all free buffers */
	list_for_each_entry_safe(hdr, dummy, &pool->free_pool, list) {
		list_del(&hdr->list);
		msm_vidc_vmem_free((void **)&hdr);
		fcount++;
	}

	/* destroy all busy buffers */
	list_for_each_entry_safe(hdr, dummy, &pool->busy_pool, list) {
		list_del(&hdr->list);
		msm_vidc_vmem_free((void **)&hdr);
		bcount++;
	}

	i_vpr_h(inst, "%s: type: %23s, count: free %2u, busy %2u\n",
		__func__, pool->name, fcount, bcount);
}

void msm_memory_pools_deinit(struct msm_vidc_inst *inst)
{
	u32 i = 0;

	if (!inst) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return;
	}

	/* destroy all buffers from all pool types */
	for (i = 0; i < MSM_MEM_POOL_MAX; i++)
		msm_vidc_destroy_pool_buffers(inst, i);
}

struct msm_vidc_type_size_name {
	enum msm_memory_pool_type type;
	u32                       size;
	char                     *name;
};

static struct msm_vidc_type_size_name buftype_size_name_arr[] = {
	{MSM_MEM_POOL_BUFFER,     sizeof(struct msm_vidc_buffer),     "MSM_MEM_POOL_BUFFER"     },
	{MSM_MEM_POOL_MAP,        sizeof(struct msm_vidc_map),        "MSM_MEM_POOL_MAP"        },
	{MSM_MEM_POOL_ALLOC,      sizeof(struct msm_vidc_alloc),      "MSM_MEM_POOL_ALLOC"      },
	{MSM_MEM_POOL_TIMESTAMP,  sizeof(struct msm_vidc_timestamp),  "MSM_MEM_POOL_TIMESTAMP"  },
	{MSM_MEM_POOL_DMABUF,     sizeof(struct msm_memory_dmabuf),   "MSM_MEM_POOL_DMABUF"     },
};

int msm_memory_pools_init(struct msm_vidc_inst *inst)
{
	u32 i;

	if (!inst) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	if (ARRAY_SIZE(buftype_size_name_arr) != MSM_MEM_POOL_MAX) {
		i_vpr_e(inst, "%s: num elements mismatch %lu %u\n", __func__,
			ARRAY_SIZE(buftype_size_name_arr), MSM_MEM_POOL_MAX);
		return -EINVAL;
	}

	for (i = 0; i < MSM_MEM_POOL_MAX; i++) {
		if (i != buftype_size_name_arr[i].type) {
			i_vpr_e(inst, "%s: type mismatch %u %u\n", __func__,
				i, buftype_size_name_arr[i].type);
			return -EINVAL;
		}
		inst->pool[i].size = buftype_size_name_arr[i].size;
		inst->pool[i].name = buftype_size_name_arr[i].name;
		INIT_LIST_HEAD(&inst->pool[i].free_pool);
		INIT_LIST_HEAD(&inst->pool[i].busy_pool);
	}

	return 0;
}


int msm_memory_cache_operations(struct msm_vidc_inst *inst,
	struct dma_buf *dbuf, enum msm_memory_cache_type cache_type,
	u32 offset, u32 size)
{
	struct msm_vidc_core *core;
	int rc = 0;

	if (!inst || !dbuf) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	/* skip cache ops for "dma-coherent" enabled chipsets */
	if (!core->is_non_coherent)
		return 0;

	if (!inst) {
		d_vpr_e("%s: invalid parameters\n", __func__);
		return -EINVAL;
	}

	if (!dbuf) {
		i_vpr_e(inst, "%s: invalid params\n", __func__);
		return -EINVAL;
	}

	switch (cache_type) {
	case MSM_MEM_CACHE_CLEAN_INVALIDATE:
		rc = dma_buf_begin_cpu_access_partial(dbuf, DMA_TO_DEVICE,
				offset, size);
		if (rc)
			break;
		rc = dma_buf_end_cpu_access_partial(dbuf, DMA_FROM_DEVICE,
				offset, size);
		break;
	case MSM_MEM_CACHE_INVALIDATE:
		rc = dma_buf_begin_cpu_access_partial(dbuf, DMA_FROM_DEVICE,
				offset, size);
		if (rc)
			break;
		rc = dma_buf_end_cpu_access_partial(dbuf, DMA_FROM_DEVICE,
				offset, size);
		break;
	default:
		i_vpr_e(inst, "%s: cache (%d) operation not supported\n",
			__func__, cache_type);
		rc = -EINVAL;
		break;
	}

	return rc;
}

/*
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
