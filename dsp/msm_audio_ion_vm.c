// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2013-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/dma-buf.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/export.h>
#include <linux/ion_kernel.h>
#include <ipc/apr.h>
#include <asm/dma-iommu.h>
#include <dsp/msm_audio_ion.h>
#include <soc/qcom/secure_buffer.h>
#include <linux/habmm.h>

#define MSM_AUDIO_ION_PROBED (1 << 0)

#define MSM_AUDIO_ION_PHYS_ADDR(alloc_data) \
	alloc_data->table->sgl->dma_address

#define MSM_AUDIO_SMMU_VM_CMD_MAP 0x00000001
#define MSM_AUDIO_SMMU_VM_CMD_UNMAP 0x00000002
#define MSM_AUDIO_SMMU_VM_HAB_MINOR_ID 1

enum msm_audio_mem_type{
	MSM_AUDIO_MEM_TYPE_ION,
	MSM_AUDIO_MEM_TYPE_DMA,
};

struct msm_audio_ion_private {
	bool smmu_enabled;
	struct device *cb_dev;
	u8 device_status;
	struct list_head alloc_list;
	struct mutex list_mutex;
};

struct msm_audio_alloc_data {
	size_t len;
	void *vaddr;
	void *handle;
	struct dma_buf_attachment *attach;
	struct sg_table *table;
	struct list_head list;
	dma_addr_t *paddr;
	enum msm_audio_mem_type type;
	u32 export_id;
};

struct msm_audio_smmu_vm_map_cmd {
	int cmd_id;
	u32 export_id;
	u32 buf_size;
};

struct msm_audio_smmu_vm_map_cmd_rsp {
	int status;
	u64 addr;
};

struct msm_audio_smmu_vm_unmap_cmd {
	int cmd_id;
	u32 export_id;
};

struct msm_audio_smmu_vm_unmap_cmd_rsp {
	int status;
};

static struct msm_audio_ion_private msm_audio_ion_data = {0,};
static u32 msm_audio_ion_hab_handle;

static void msm_audio_ion_add_allocation(
	struct msm_audio_ion_private *msm_audio_ion_data,
	struct msm_audio_alloc_data *alloc_data)
{
	/*
	 * Since these APIs can be invoked by multiple
	 * clients, there is need to make sure the list
	 * of allocations is always protected
	 */
	mutex_lock(&(msm_audio_ion_data->list_mutex));
	list_add_tail(&(alloc_data->list),
		      &(msm_audio_ion_data->alloc_list));
	mutex_unlock(&(msm_audio_ion_data->list_mutex));
}

static int msm_audio_dma_buf_map(void *handle, void *vaddr,
					  dma_addr_t *paddr,
					  size_t *len)
{
	struct msm_audio_alloc_data *alloc_data;

	/* Data required per buffer mapping */
	alloc_data = kzalloc(sizeof(*alloc_data), GFP_KERNEL);
	if (!alloc_data)
		return -ENOMEM;

	alloc_data->handle = handle;
	alloc_data->len = *len;
	alloc_data->vaddr = vaddr;
	alloc_data->paddr = paddr;
	alloc_data->type = MSM_AUDIO_MEM_TYPE_DMA;

	msm_audio_ion_add_allocation(&msm_audio_ion_data,
				     alloc_data);

	return 0;
}

static int msm_audio_ion_dma_buf_map(struct dma_buf *dma_buf,
				 dma_addr_t *addr, size_t *len)
{

	struct msm_audio_alloc_data *alloc_data;
	struct device *cb_dev;
	unsigned long ionflag = 0;
	int rc = 0;

	cb_dev = msm_audio_ion_data.cb_dev;

	/* Data required per buffer mapping */
	alloc_data = kzalloc(sizeof(*alloc_data), GFP_KERNEL);
	if (!alloc_data)
		return -ENOMEM;

	alloc_data->handle = (void*)dma_buf;
	alloc_data->len = dma_buf->size;
	alloc_data->type = MSM_AUDIO_MEM_TYPE_ION;
	*len = dma_buf->size;

	/* Attach the dma_buf to context bank device */
	alloc_data->attach = dma_buf_attach(dma_buf, cb_dev);
	if (IS_ERR(alloc_data->attach)) {
		rc = PTR_ERR(alloc_data->attach);
		dev_err(cb_dev,
			"%s: Fail to attach dma_buf to CB, rc = %d\n",
			__func__, rc);
		goto free_alloc_data;
	}

	/* For uncached buffers, avoid cache maintanance */
	rc = dma_buf_get_flags(dma_buf, &ionflag);
	if (rc) {
		dev_err(cb_dev, "%s: dma_buf_get_flags failed: %d\n",
			__func__, rc);
		goto detach_dma_buf;
	}

	if (!(ionflag & ION_FLAG_CACHED))
		alloc_data->attach->dma_map_attrs |= DMA_ATTR_SKIP_CPU_SYNC;

	/*
	 * Get the scatter-gather list.
	 * There is no info as this is a write buffer or
	 * read buffer, hence the request is bi-directional
	 * to accommodate both read and write mappings.
	 */
	alloc_data->table = dma_buf_map_attachment(alloc_data->attach,
				DMA_BIDIRECTIONAL);
	if (IS_ERR(alloc_data->table)) {
		rc = PTR_ERR(alloc_data->table);
		dev_err(cb_dev,
			"%s: Fail to map attachment, rc = %d\n",
			__func__, rc);
		goto detach_dma_buf;
	}

	/* physical address from mapping */
	*addr = MSM_AUDIO_ION_PHYS_ADDR(alloc_data);
	alloc_data->paddr = addr;
	msm_audio_ion_add_allocation(&msm_audio_ion_data,
				     alloc_data);
	return rc;

detach_dma_buf:
	dma_buf_detach(dma_buf, alloc_data->attach);
free_alloc_data:
	kfree(alloc_data);

	return rc;
}

static int msm_audio_ion_unmap_kernel(void *vaddr, void *handle)
{
	int rc = 0;
	struct device *cb_dev = msm_audio_ion_data.cb_dev;

	if (!vaddr) {
		dev_err(cb_dev,
			"%s: cannot find allocation for handle %pK\n",
			__func__, handle);
		rc = -EINVAL;
		goto err;
	}

	dma_buf_vunmap((struct dma_buf*)handle, vaddr);

	rc = dma_buf_end_cpu_access((struct dma_buf*)handle, DMA_BIDIRECTIONAL);
	if (rc) {
		dev_err(cb_dev, "%s: kmap dma_buf_end_cpu_access fail\n",
			__func__);
		goto err;
	}

err:
	return rc;
}

static int msm_audio_dma_buf_unmap(void *handle)
{
	int rc = 0;
	struct msm_audio_alloc_data *alloc_data = NULL;
	struct list_head *ptr, *next;
	struct device *cb_dev = msm_audio_ion_data.cb_dev;
	bool found = false;

	/*
	 * Though list_for_each_safe is delete safe, lock
	 * should be explicitly acquired to avoid race condition
	 * on adding elements to the list.
	 */
	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_safe(ptr, next, &(msm_audio_ion_data.alloc_list)) {

		alloc_data = list_entry(ptr, struct msm_audio_alloc_data, list);
		if(alloc_data->type == MSM_AUDIO_MEM_TYPE_ION) {
			if (alloc_data->handle == handle) {
				rc = msm_audio_ion_unmap_kernel(
							alloc_data->vaddr,
							handle);
			if(rc) {
				pr_err("%s: Unable to unmap ion mem rc: %d\n",
				       __func__, rc);
				mutex_unlock(&(msm_audio_ion_data.list_mutex));
				return rc;
			}

				found = true;
				dma_buf_unmap_attachment(alloc_data->attach,
							 alloc_data->table,
							 DMA_BIDIRECTIONAL);

				dma_buf_detach((struct dma_buf*)
						alloc_data->handle,
					       alloc_data->attach);

				dma_buf_put((struct dma_buf*)
					    alloc_data->handle);

				list_del(&(alloc_data->list));
				kfree(alloc_data);
				break;
			}
		} else {
			alloc_data = list_entry(ptr,
						struct msm_audio_alloc_data,
						list);

			if (alloc_data->handle == handle) {
				found = true;

				dma_free_coherent(cb_dev, alloc_data->len,
						  alloc_data->vaddr,
						  *(alloc_data->paddr));

				list_del(&(alloc_data->list));
				kfree(alloc_data);
				break;
			}
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!found) {
		dev_err(cb_dev,
			"%s: cannot find allocation, handle %pK",
			__func__, handle);
		rc = -EINVAL;
	}

	return rc;
}

static int msm_audio_ion_smmu_map(void *handle,
		dma_addr_t *paddr, size_t *len)
{
	int rc;
	u32 export_id;
	u32 cmd_rsp_size;
	bool found = false;
	bool exported = false;
	struct msm_audio_smmu_vm_map_cmd smmu_map_cmd;
	struct msm_audio_smmu_vm_map_cmd_rsp cmd_rsp;
	struct msm_audio_alloc_data *alloc_data = NULL;
	unsigned long delay = jiffies + (HZ / 2);

	*len = ((struct dma_buf*)handle)->size;

	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry(alloc_data, &(msm_audio_ion_data.alloc_list),
			    list) {
		if (alloc_data->handle == handle) {
			found = true;

			/* Export the buffer to physical VM */
			rc = habmm_export(msm_audio_ion_hab_handle, handle, *len,
				&export_id, HABMM_EXPIMP_FLAGS_DMABUF);
			if (rc) {
				pr_err("%s: habmm_export failed handle = %pK, len = %zd, rc = %d\n",
					__func__, handle, *len, rc);
				goto err;
			}

			exported = true;
			smmu_map_cmd.cmd_id = MSM_AUDIO_SMMU_VM_CMD_MAP;
			smmu_map_cmd.export_id = export_id;
			smmu_map_cmd.buf_size = *len;

			rc = habmm_socket_send(msm_audio_ion_hab_handle,
				(void *)&smmu_map_cmd, sizeof(smmu_map_cmd), 0);
			if (rc) {
				pr_err("%s: habmm_socket_send failed %d\n",
					__func__, rc);
				goto err;
			}

			do {
				cmd_rsp_size = sizeof(cmd_rsp);
				rc = habmm_socket_recv(msm_audio_ion_hab_handle,
					(void *)&cmd_rsp,
					&cmd_rsp_size,
					0xFFFFFFFF,
					0);
			} while (time_before(jiffies, delay) && (rc == -EINTR) &&
					(cmd_rsp_size == 0));
			if (rc) {
				pr_err("%s: habmm_socket_recv failed %d\n",
					__func__, rc);
				goto err;
			}

			if (cmd_rsp_size != sizeof(cmd_rsp)) {
				pr_err("%s: invalid size for cmd rsp %u, expected %zu\n",
					__func__, cmd_rsp_size, sizeof(cmd_rsp));
				rc = -EIO;
				goto err;
			}

			if (cmd_rsp.status) {
				pr_err("%s: SMMU map command failed %d\n",
					__func__, cmd_rsp.status);
				rc = cmd_rsp.status;
				goto err;
			}

			*paddr = (dma_addr_t)cmd_rsp.addr;
			alloc_data->export_id = export_id;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!found) {
		pr_err("%s: cannot find allocation, handle %pK\n", __func__, handle);
		return -EINVAL;
	}

	return 0;

err:
	if (exported)
		(void)habmm_unexport(msm_audio_ion_hab_handle, export_id, 0);

	mutex_unlock(&(msm_audio_ion_data.list_mutex));
	return rc;
}

static int msm_audio_ion_smmu_unmap(void *handle)
{
	int rc;
	bool found = false;
	u32 cmd_rsp_size;
	struct msm_audio_smmu_vm_unmap_cmd smmu_unmap_cmd;
	struct msm_audio_smmu_vm_unmap_cmd_rsp cmd_rsp;
	struct msm_audio_alloc_data *alloc_data, *next;
	unsigned long delay = jiffies + (HZ / 2);

	/*
	 * Though list_for_each_entry_safe is delete safe, lock
	 * should be explicitly acquired to avoid race condition
	 * on adding elements to the list.
	 */
	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry_safe(alloc_data, next,
		&(msm_audio_ion_data.alloc_list), list) {

		if (alloc_data->handle == handle) {
			found = true;
			smmu_unmap_cmd.cmd_id = MSM_AUDIO_SMMU_VM_CMD_UNMAP;
			smmu_unmap_cmd.export_id = alloc_data->export_id;

			rc = habmm_socket_send(msm_audio_ion_hab_handle,
				(void *)&smmu_unmap_cmd,
				sizeof(smmu_unmap_cmd), 0);
			if (rc) {
				pr_err("%s: habmm_socket_send failed %d\n",
					__func__, rc);
				goto err;
			}

			do {
				cmd_rsp_size = sizeof(cmd_rsp);
				rc = habmm_socket_recv(msm_audio_ion_hab_handle,
					(void *)&cmd_rsp,
					&cmd_rsp_size,
					0xFFFFFFFF,
					0);
			} while (time_before(jiffies, delay) &&
					(rc == -EINTR) && (cmd_rsp_size == 0));
			if (rc) {
				pr_err("%s: habmm_socket_recv failed %d\n",
					__func__, rc);
				goto err;
			}

			if (cmd_rsp_size != sizeof(cmd_rsp)) {
				pr_err("%s: invalid size for cmd rsp %u\n",
					__func__, cmd_rsp_size);
				rc = -EIO;
				goto err;
			}

			if (cmd_rsp.status) {
				pr_err("%s: SMMU unmap command failed %d\n",
					__func__, cmd_rsp.status);
				rc = cmd_rsp.status;
				goto err;
			}

			rc = habmm_unexport(msm_audio_ion_hab_handle,
				alloc_data->export_id, 0xFFFFFFFF);
			if (rc) {
				pr_err("%s: habmm_unexport failed export_id = %d, rc = %d\n",
					__func__, alloc_data->export_id, rc);
			}

			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!found) {
		pr_err("%s: cannot find allocation, handle %pK\n", __func__, handle);
		rc = -EINVAL;
	}

	return rc;

err:
	if (found) {
		(void)habmm_unexport(msm_audio_ion_hab_handle,
			alloc_data->export_id, 0xFFFFFFFF);
		list_del(&(alloc_data->list));
		kfree(alloc_data);
	}

	mutex_unlock(&(msm_audio_ion_data.list_mutex));
	return rc;
}

static int msm_audio_ion_get_phys(struct dma_buf *dma_buf,
				  dma_addr_t *addr, size_t *len)
{
	int rc = 0;

	rc = msm_audio_ion_dma_buf_map(dma_buf, addr, len);
	if (rc) {
		pr_err("%s: failed to map DMA buf, err = %d\n",
			__func__, rc);
		goto err;
	}

	pr_debug("phys=%pK, len=%zd, rc=%d\n", addr, *len, rc);
err:
	return rc;
}

static void *msm_audio_ion_map_kernel(void *handle)
{
	int rc = 0;
	void *addr = NULL;
	struct msm_audio_alloc_data *alloc_data = NULL;

	rc = dma_buf_begin_cpu_access((struct dma_buf*)handle,
				      DMA_BIDIRECTIONAL);
	if (rc) {
		pr_err("%s: kmap dma_buf_begin_cpu_access fail\n", __func__);
		goto exit;
	}

	addr = dma_buf_vmap((struct dma_buf*)handle);
	if (!addr) {
		pr_err("%s: kernel mapping of dma_buf failed\n",
		       __func__);
		goto exit;
	}

	/*
	 * TBD: remove the below section once new API
	 * for mapping kernel virtual address is available.
	 */
	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry(alloc_data, &(msm_audio_ion_data.alloc_list),
			    list) {
		if (alloc_data->handle == handle) {
			alloc_data->vaddr = addr;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

exit:
	return addr;
}

static int msm_audio_ion_map_buf(void *handle, dma_addr_t *paddr,
				 size_t *plen, void **vaddr)
{
	int rc = 0;

	rc = msm_audio_ion_get_phys((struct dma_buf*) handle, paddr, plen);
	if (rc) {
		pr_err("%s: ION Get Physical for AUDIO failed, rc = %d\n",
				__func__, rc);
		dma_buf_put(dma_buf);
		goto err;
	}

	*vaddr = msm_audio_ion_map_kernel(handle);
	if (IS_ERR_OR_NULL(*vaddr)) {
		pr_err("%s: ION memory mapping for AUDIO failed\n", __func__);
		rc = -ENOMEM;
		msm_audio_dma_buf_unmap(dma_buf);
		goto err;
	}

	if (msm_audio_ion_data.smmu_enabled) {
		rc = msm_audio_ion_smmu_map(handle, paddr, plen);
		if (rc) {
			pr_err("%s: failed to do smmu map, err = %d\n",
				__func__, rc);
			msm_audio_dma_buf_unmap((struct dma_buf *) handle);
			goto err;
		}
	}
err:
	return rc;
}

/**
 * msm_audio_ion_alloc -
 *        Allocs ION memory for given client name
 *
 * @handle: generic handle to the memory allocation
 *          dma_buf for the system heap memory. vaddr for audio heap memory.
 * @bufsz: buffer size
 * @paddr: Physical address to be assigned with allocated region
 * @plen: length of allocated region to be assigned
 * vaddr: virtual address to be assigned
 *
 * Returns 0 on success or error on failure
 */
int msm_audio_ion_alloc(void **handle, size_t bufsz,
			dma_addr_t *paddr, size_t *plen, void **vaddr)
{
	int rc = -EINVAL;
	unsigned long err_ion_ptr = 0;

	if (!(msm_audio_ion_data.device_status & MSM_AUDIO_ION_PROBED)) {
		pr_debug("%s:probe is not done, deferred\n", __func__);
		return -EPROBE_DEFER;
	}
	if (!handle || !paddr || !vaddr || !bufsz || !plen) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	if (msm_audio_ion_data.smmu_enabled == true) {
		pr_debug("%s: system heap is used\n", __func__);
		*handle = ion_alloc(bufsz, ION_HEAP(ION_SYSTEM_HEAP_ID), 0);
	} else {
		pr_debug("%s: audio heap is used\n", __func__);
		*vaddr = *handle = dma_alloc_coherent(
						      msm_audio_ion_data.cb_dev,
						      bufsz, paddr, GFP_KERNEL);
		if(*vaddr != NULL) {
			pr_err("%s: vaddr = %pK, size=%zd\n", __func__, *vaddr,
			       bufsz);
			rc = 0;
		}
	}
	if (IS_ERR_OR_NULL((void *)(*handle))) {
		if (IS_ERR((void *)(*handle)))
			err_ion_ptr = PTR_ERR((int *)(*handle));
		pr_err("%s: ION alloc fail err ptr=%ld, smmu_enabled=%d\n",
		       __func__, err_ion_ptr, msm_audio_ion_data.smmu_enabled);
		rc = -ENOMEM;
		goto err;
	}
	if (msm_audio_ion_data.smmu_enabled) {
		rc = msm_audio_ion_map_buf(*handle, paddr, plen, vaddr);
		if (rc) {
			pr_err("%s: failed to map ION buf, rc = %d\n", __func__,
			       rc);
		}
	} else {
		rc = msm_audio_dma_buf_map(*handle, *vaddr, paddr,
						    &bufsz);
		if (rc) {
			pr_err("%s: failed to map ION buf, rc = %d\n", __func__,
				rc);
			dma_free_coherent(msm_audio_ion_data.cb_dev,
					  bufsz, vaddr, *paddr);
		}
	}
	pr_debug("%s: mapped address = %pK, size=%zd\n", __func__,
		*vaddr, bufsz);

	memset(*vaddr, 0, bufsz);

err:
	return rc;
}
EXPORT_SYMBOL(msm_audio_ion_alloc);

int msm_audio_ion_phys_free(void *handle,
			   dma_addr_t *paddr,
			   size_t *pa_len,
			   u8 assign_type,
			   int id,
			   int key)
{
	handle = NULL;
	return 0;
}
EXPORT_SYMBOL(msm_audio_ion_phys_free);

int msm_audio_ion_phys_assign(void **handle, int fd,
		dma_addr_t *paddr, size_t *pa_len, u8 assign_type, int id)
{
	*handle = NULL;
	return 0;
}
EXPORT_SYMBOL(msm_audio_ion_phys_assign);

bool msm_audio_is_hypervisor_supported(void)
{
	return false;
}
EXPORT_SYMBOL(msm_audio_is_hypervisor_supported);
/**
 * msm_audio_ion_import-
 *        Import ION buffer with given file descriptor
 *
 * @handle: generic handle to the memory allocation
 *          dma_buf for the system heap memory. vaddr for audio heap memory.
 * @fd: file descriptor for the ION memory
 * @ionflag: flags associated with ION buffer
 * @bufsz: buffer size
 * @paddr: Physical address to be assigned with allocated region
 * @plen: length of allocated region to be assigned
 * vaddr: virtual address to be assigned
 *
 * Returns 0 on success or error on failure
 */
int msm_audio_ion_import(void **handle, int fd,
			unsigned long *ionflag, size_t bufsz,
			dma_addr_t *paddr, size_t *plen, void **vaddr)
{
	int rc = 0;

	if (!(msm_audio_ion_data.device_status & MSM_AUDIO_ION_PROBED)) {
		pr_debug("%s: probe is not done, deferred\n", __func__);
		return -EPROBE_DEFER;
	}

	if (!handle || !paddr || !vaddr || !plen) {
		pr_err("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	/* bufsz should be 0 and fd shouldn't be 0 as of now */
	*handle = dma_buf_get(fd);
	pr_debug("%s: handle =%pK, fd=%d\n", __func__, *handle, fd);
	if (IS_ERR_OR_NULL((void *)(*handle))) {
		pr_err("%s: dma_buf_get failed\n", __func__);
		rc = -EINVAL;
		goto err;
	}

	if (ionflag != NULL) {
		rc = dma_buf_get_flags((struct dma_buf*)*handle, ionflag);
		if (rc) {
			pr_err("%s: could not get flags for the dma_buf\n",
				__func__);
			goto err_ion_flag;
		}
	}

	rc = msm_audio_ion_map_buf(*handle, paddr, plen, vaddr);
	if (rc) {
		pr_err("%s: failed to map ION buf, rc = %d\n", __func__, rc);
		goto err;
	}
	pr_debug("%s: mapped address = %pK, size=%zd\n", __func__,
		*vaddr, bufsz);

	return 0;

err_ion_flag:
	dma_buf_put((struct dma_buf*) *handle);
err:
	*handle = NULL;
	return rc;
}
EXPORT_SYMBOL(msm_audio_ion_import);

/**
 * msm_audio_ion_free -
 *        fress ION memory for given client and handle
 *
 * @handle: generic handle to the memory allocation
 *          dma_buf for the system heap memory. vaddr for audio heap memory.
 *
 * Returns 0 on success or error on failure
 */
int msm_audio_ion_free(void *handle)
{
	int ret = 0;

	if (!handle) {
		pr_err("%s: handle invalid\n", __func__);
		return -EINVAL;
	}

	if (msm_audio_ion_data.smmu_enabled) {
		ret = msm_audio_ion_smmu_unmap(handle);
		if (ret)
			pr_err("%s: smmu unmap failed with ret %d\n",
				__func__, ret);
	}

	msm_audio_dma_buf_unmap(handle);

	return 0;
}
EXPORT_SYMBOL(msm_audio_ion_free);

/**
 * msm_audio_ion_mmap -
 *       Audio ION memory map
 *
 * @abuff: audio buf pointer
 * @vma: virtual mem area
 *
 * Returns 0 on success or error on failure
 */
int msm_audio_ion_mmap(struct audio_buffer *abuff,
		       struct vm_area_struct *vma)
{
	struct msm_audio_alloc_data *alloc_data = NULL;
	struct sg_table *table;
	unsigned long addr = vma->vm_start;
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	struct scatterlist *sg;
	unsigned int i;
	struct page *page;
	int ret = 0;
	bool found = false;
	struct device *cb_dev = msm_audio_ion_data.cb_dev;

	mutex_lock(&(msm_audio_ion_data.list_mutex));
	list_for_each_entry(alloc_data, &(msm_audio_ion_data.alloc_list),
			    list) {
		if (alloc_data->handle == abuff->mem_handle) {
			found = true;
			table = alloc_data->table;
			break;
		}
	}
	mutex_unlock(&(msm_audio_ion_data.list_mutex));

	if (!found) {
		dev_err(cb_dev,
			"%s: cannot find allocation, dma_buf %pK",
			__func__, abuff->mem_handle);
		return -EINVAL;
	}
	/* uncached */
	vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	/* We need to check if a page is associated with this sg list because:
	 * If the allocation came from a carveout we currently don't have
	 * pages associated with carved out memory. This might change in the
	 * future and we can remove this check and the else statement.
	 */
	page = sg_page(table->sgl);
	if (page) {
		pr_debug("%s: page is NOT null\n", __func__);
		for_each_sg(table->sgl, sg, table->nents, i) {
			unsigned long remainder = vma->vm_end - addr;
			unsigned long len = sg->length;

			page = sg_page(sg);

			if (offset >= len) {
				offset -= len;
				continue;
			} else if (offset) {
				page += offset / PAGE_SIZE;
				len -= offset;
				offset = 0;
			}
			len = min(len, remainder);
			pr_debug("vma=%pK, addr=%x len=%ld vm_start=%x vm_end=%x vm_page_prot=%lu\n",
				vma, (unsigned int)addr, len,
				(unsigned int)vma->vm_start,
				(unsigned int)vma->vm_end,
				(unsigned long)pgprot_val(vma->vm_page_prot));
			remap_pfn_range(vma, addr, page_to_pfn(page), len,
					vma->vm_page_prot);
			addr += len;
			if (addr >= vma->vm_end)
				return 0;
		}
	} else {
		pr_debug("%s: page is NULL\n", __func__);
		ret = -EINVAL;
	}

	return ret;
}
EXPORT_SYMBOL(msm_audio_ion_mmap);

/**
 * msm_audio_populate_upper_32_bits -
 *        retrieve upper 32bits of 64bit address
 *
 * @pa: 64bit physical address
 *
 */
u32 msm_audio_populate_upper_32_bits(dma_addr_t pa)
{
	return upper_32_bits(pa);
}
EXPORT_SYMBOL(msm_audio_populate_upper_32_bits);

static const struct of_device_id msm_audio_ion_dt_match[] = {
	{ .compatible = "qcom,msm-audio-ion" },
	{ }
};
MODULE_DEVICE_TABLE(of, msm_audio_ion_dt_match);

static int msm_audio_ion_probe(struct platform_device *pdev)
{
	int rc = 0;
	const char *msm_audio_ion_dt = "qcom,smmu-enabled";
	bool smmu_enabled;
	struct device *dev = &pdev->dev;

	if (dev->of_node == NULL) {
		dev_err(dev,
			"%s: device tree is not found\n",
			__func__);
		msm_audio_ion_data.smmu_enabled = 0;
		return 0;
	}

	smmu_enabled = of_property_read_bool(dev->of_node,
					     msm_audio_ion_dt);
	msm_audio_ion_data.smmu_enabled = smmu_enabled;

	if (!smmu_enabled) {
		dev_dbg(dev, "%s: SMMU is Disabled\n", __func__);
		goto exit;
	}

	rc = habmm_socket_open(&msm_audio_ion_hab_handle,
		HAB_MMID_CREATE(MM_AUD_3,
			MSM_AUDIO_SMMU_VM_HAB_MINOR_ID),
		0xFFFFFFFF,
		HABMM_SOCKET_OPEN_FLAGS_SINGLE_BE_SINGLE_FE);
	if (rc) {
		dev_err(dev, "%s: habmm_socket_open failed %d\n",
			__func__, rc);
		return rc;
	}

	dev_info(dev, "%s: msm_audio_ion_hab_handle %x\n",
		__func__, msm_audio_ion_hab_handle);

exit:
	if (!rc)
		msm_audio_ion_data.device_status |= MSM_AUDIO_ION_PROBED;

	msm_audio_ion_data.cb_dev = dev;
	INIT_LIST_HEAD(&msm_audio_ion_data.alloc_list);
	mutex_init(&(msm_audio_ion_data.list_mutex));

	return rc;
}

static int msm_audio_ion_remove(struct platform_device *pdev)
{
	if (msm_audio_ion_data.smmu_enabled) {
		if (msm_audio_ion_hab_handle)
			habmm_socket_close(msm_audio_ion_hab_handle);
	}
	msm_audio_ion_data.smmu_enabled = 0;
	msm_audio_ion_data.device_status = 0;
	mutex_destroy(&(msm_audio_ion_data.list_mutex));
	return 0;
}

static struct platform_driver msm_audio_ion_driver = {
	.driver = {
		.name = "msm-audio-ion",
		.owner = THIS_MODULE,
		.of_match_table = msm_audio_ion_dt_match,
	},
	.probe = msm_audio_ion_probe,
	.remove = msm_audio_ion_remove,
};

int __init msm_audio_ion_init(void)
{
	return platform_driver_register(&msm_audio_ion_driver);
}

void msm_audio_ion_exit(void)
{
	platform_driver_unregister(&msm_audio_ion_driver);
}

MODULE_DESCRIPTION("MSM Audio ION VM module");
MODULE_LICENSE("GPL v2");
