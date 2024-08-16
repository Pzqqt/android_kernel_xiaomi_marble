/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022,2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: i_qdf_mem.h
 * Linux-specific definitions for QDF memory API's
 */

#ifndef __I_QDF_MEM_H
#define __I_QDF_MEM_H

#ifdef __KERNEL__
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 17)
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#endif
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/vmalloc.h>
#include <linux/pci.h> /* pci_alloc_consistent */
#include <linux/cache.h> /* L1_CACHE_BYTES */

#define __qdf_cache_line_sz L1_CACHE_BYTES
#include "queue.h"

#else
/*
 * Provide dummy defs for kernel data types, functions, and enums
 * used in this header file.
 */
#define GFP_KERNEL 0
#define GFP_ATOMIC 0
#define kzalloc(size, flags) NULL
#define vmalloc(size)        NULL
#define kfree(buf)
#define vfree(buf)
#define pci_alloc_consistent(dev, size, paddr) NULL
#define __qdf_mempool_t void*
#define QDF_RET_IP NULL
#endif /* __KERNEL__ */
#include <qdf_status.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)) && defined(MSM_PLATFORM)
#include <linux/qcom-iommu-util.h>
#endif

#if IS_ENABLED(CONFIG_ARM_SMMU)
#include <pld_common.h>
#ifdef ENABLE_SMMU_S1_TRANSLATION
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
#include <asm/dma-iommu.h>
#endif
#endif
#include <linux/iommu.h>
#endif

#ifdef __KERNEL__
typedef struct mempool_elem {
	STAILQ_ENTRY(mempool_elem) mempool_entry;
} mempool_elem_t;

/**
 * typedef __qdf_mempool_ctxt_t - Memory pool context
 * @pool_id: pool identifier
 * @flags: flags
 * @elem_size: size of each pool element in bytes
 * @pool_mem: pool_addr address of the pool created
 * @mem_size: Total size of the pool in bytes
 * @free_list: free pool list
 * @lock: spinlock object
 * @max_elem: Maximum number of elements in tha pool
 * @free_cnt: Number of free elements available
 */
typedef struct __qdf_mempool_ctxt {
	int pool_id;
	u_int32_t flags;
	size_t elem_size;
	void *pool_mem;
	u_int32_t mem_size;

	STAILQ_HEAD(, mempool_elem) free_list;
	spinlock_t lock;
	u_int32_t max_elem;
	u_int32_t free_cnt;
} __qdf_mempool_ctxt_t;

#endif /* __KERNEL__ */

#define __page_size ((size_t)PAGE_SIZE)
#define __qdf_align(a, mask) ALIGN(a, mask)

#ifdef DISABLE_MEMDEBUG_PANIC
#define QDF_MEMDEBUG_PANIC(reason_fmt, args...) \
	do { \
		/* no-op */ \
	} while (false)
#else
#define QDF_MEMDEBUG_PANIC(reason_fmt, args...) \
	QDF_DEBUG_PANIC(reason_fmt, ## args)
#endif

/* typedef for dma_data_direction */
typedef enum dma_data_direction __dma_data_direction;

/**
 * __qdf_dma_dir_to_os() - Convert DMA data direction to OS specific enum
 * @dir: QDF DMA data direction
 *
 * Return:
 * enum dma_data_direction
 */
static inline
enum dma_data_direction __qdf_dma_dir_to_os(qdf_dma_dir_t qdf_dir)
{
	switch (qdf_dir) {
	case QDF_DMA_BIDIRECTIONAL:
		return DMA_BIDIRECTIONAL;
	case QDF_DMA_TO_DEVICE:
		return DMA_TO_DEVICE;
	case QDF_DMA_FROM_DEVICE:
		return DMA_FROM_DEVICE;
	default:
		return DMA_NONE;
	}
}


/**
 * __qdf_mem_map_nbytes_single - Map memory for DMA
 * @osdev: pomter OS device context
 * @buf: pointer to memory to be dma mapped
 * @dir: DMA map direction
 * @nbytes: number of bytes to be mapped.
 * @phy_addr: ponter to recive physical address.
 *
 * Return: success/failure
 */
static inline uint32_t __qdf_mem_map_nbytes_single(qdf_device_t osdev,
						  void *buf, qdf_dma_dir_t dir,
						  int nbytes,
						  qdf_dma_addr_t *phy_addr)
{
	/* assume that the OS only provides a single fragment */
	*phy_addr = dma_map_single(osdev->dev, buf, nbytes,
					__qdf_dma_dir_to_os(dir));
	return dma_mapping_error(osdev->dev, *phy_addr) ?
	QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
static inline void __qdf_mem_dma_cache_sync(qdf_device_t osdev,
					    qdf_dma_addr_t buf,
					    qdf_dma_dir_t dir,
					    int nbytes)
{
	dma_cache_sync(osdev->dev, buf, nbytes, __qdf_dma_dir_to_os(dir));
}
#else
static inline void __qdf_mem_dma_cache_sync(qdf_device_t osdev,
					    qdf_dma_addr_t buf,
					    qdf_dma_dir_t dir,
					    int nbytes)
{
	dma_sync_single_for_cpu(osdev->dev, buf, nbytes,
				__qdf_dma_dir_to_os(dir));
}
#endif

/**
 * __qdf_mem_unmap_nbytes_single() - un_map memory for DMA
 *
 * @osdev: pomter OS device context
 * @phy_addr: physical address of memory to be dma unmapped
 * @dir: DMA unmap direction
 * @nbytes: number of bytes to be unmapped.
 *
 * Return - none
 */
static inline void __qdf_mem_unmap_nbytes_single(qdf_device_t osdev,
						 qdf_dma_addr_t phy_addr,
						 qdf_dma_dir_t dir, int nbytes)
{
	dma_unmap_single(osdev->dev, phy_addr, nbytes,
				__qdf_dma_dir_to_os(dir));
}
#ifdef __KERNEL__

typedef __qdf_mempool_ctxt_t *__qdf_mempool_t;

int __qdf_mempool_init(qdf_device_t osdev, __qdf_mempool_t *pool, int pool_cnt,
		       size_t pool_entry_size, u_int32_t flags);
void __qdf_mempool_destroy(qdf_device_t osdev, __qdf_mempool_t pool);
void *__qdf_mempool_alloc(qdf_device_t osdev, __qdf_mempool_t pool);
void __qdf_mempool_free(qdf_device_t osdev, __qdf_mempool_t pool, void *buf);
#define QDF_RET_IP ((void *)_RET_IP_)

#define __qdf_mempool_elem_size(_pool) ((_pool)->elem_size)
#endif

/**
 * __qdf_mem_smmu_s1_enabled() - Return SMMU stage 1 translation enable status
 * @osdev parent device instance
 *
 * Return: true if smmu s1 enabled, false if smmu s1 is bypassed
 */
static inline bool __qdf_mem_smmu_s1_enabled(qdf_device_t osdev)
{
	return osdev->smmu_s1_enabled;
}

#if IS_ENABLED(CONFIG_ARM_SMMU) && defined(ENABLE_SMMU_S1_TRANSLATION)
/*
 * typedef __qdf_iommu_domain_t: abstraction for struct iommu_domain
 */
typedef struct iommu_domain __qdf_iommu_domain_t;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
#if IS_ENABLED(CONFIG_QCOM_IOMMU_UTIL)
/**
 * __qdf_iommu_attr_to_os() - Convert qdf iommu attribute to OS mapping
 *			      configurations bitmap
 * @attr: QDF iommu attribute
 *
 * Return: IOMMU mapping configuration bitmaps
 */
static inline int __qdf_iommu_attr_to_os(enum qdf_iommu_attr attr)
{
	switch (attr) {
	case QDF_DOMAIN_ATTR_S1_BYPASS:
		return QCOM_IOMMU_MAPPING_CONF_S1_BYPASS;
	case QDF_DOMAIN_ATTR_ATOMIC:
		return QCOM_IOMMU_MAPPING_CONF_ATOMIC;
	case QDF_DOMAIN_ATTR_FAST:
		return QCOM_IOMMU_MAPPING_CONF_FAST;
	default:
		return -EINVAL;
	}
}

/**
 * __qdf_iommu_domain_get_attr() - API to get iommu domain attributes
 *
 * @domain: iommu domain
 * @attr: iommu attribute
 * @data: data pointer
 *
 * Return: 0 for success, and negative values otherwise
 */
static inline int
__qdf_iommu_domain_get_attr(__qdf_iommu_domain_t *domain,
			    enum qdf_iommu_attr attr, void *data)
{
	int mapping_config;
	int mapping_bitmap;
	int *value;

	mapping_bitmap = __qdf_iommu_attr_to_os(attr);
	if (mapping_bitmap < 0)
		return -EINVAL;

	mapping_config = qcom_iommu_get_mappings_configuration(domain);
	if (mapping_config < 0)
		return -EINVAL;

	value = data;
	*value = (mapping_config & mapping_bitmap) ? 1 : 0;

	return 0;
}
#else /* !CONFIG_QCOM_IOMMU_UTIL */
static inline int
__qdf_iommu_domain_get_attr(__qdf_iommu_domain_t *domain,
			    enum qdf_iommu_attr attr, void *data)
{
	return -ENOTSUPP;
}
#endif /* CONFIG_QCOM_IOMMU_UTIL */
#else
/**
 * __qdf_iommu_attr_to_os() - Convert qdf iommu attribute to OS specific enum
 * @attr: QDF iommu attribute
 *
 * Return: enum iommu_attr
 */
static inline
enum iommu_attr __qdf_iommu_attr_to_os(enum qdf_iommu_attr attr)
{
	switch (attr) {
	case QDF_DOMAIN_ATTR_GEOMETRY:
		return DOMAIN_ATTR_GEOMETRY;
	case QDF_DOMAIN_ATTR_PAGING:
		return DOMAIN_ATTR_PAGING;
	case QDF_DOMAIN_ATTR_WINDOWS:
		return DOMAIN_ATTR_WINDOWS;
	case QDF_DOMAIN_ATTR_FSL_PAMU_STASH:
		return DOMAIN_ATTR_FSL_PAMU_STASH;
	case QDF_DOMAIN_ATTR_FSL_PAMU_ENABLE:
		return DOMAIN_ATTR_FSL_PAMU_ENABLE;
	case QDF_DOMAIN_ATTR_FSL_PAMUV1:
		return DOMAIN_ATTR_FSL_PAMUV1;
	case QDF_DOMAIN_ATTR_NESTING:
		return DOMAIN_ATTR_NESTING;
	case QDF_DOMAIN_ATTR_DMA_USE_FLUSH_QUEUE:
		return DOMAIN_ATTR_DMA_USE_FLUSH_QUEUE;
	case QDF_DOMAIN_ATTR_CONTEXT_BANK:
		return DOMAIN_ATTR_CONTEXT_BANK;
	case QDF_DOMAIN_ATTR_NON_FATAL_FAULTS:
		return DOMAIN_ATTR_NON_FATAL_FAULTS;
	case QDF_DOMAIN_ATTR_S1_BYPASS:
		return DOMAIN_ATTR_S1_BYPASS;
	case QDF_DOMAIN_ATTR_ATOMIC:
		return DOMAIN_ATTR_ATOMIC;
	case QDF_DOMAIN_ATTR_SECURE_VMID:
		return DOMAIN_ATTR_SECURE_VMID;
	case QDF_DOMAIN_ATTR_FAST:
		return DOMAIN_ATTR_FAST;
	case QDF_DOMAIN_ATTR_PGTBL_INFO:
		return DOMAIN_ATTR_PGTBL_INFO;
	case QDF_DOMAIN_ATTR_USE_UPSTREAM_HINT:
		return DOMAIN_ATTR_USE_UPSTREAM_HINT;
	case QDF_DOMAIN_ATTR_EARLY_MAP:
		return DOMAIN_ATTR_EARLY_MAP;
	case QDF_DOMAIN_ATTR_PAGE_TABLE_IS_COHERENT:
		return DOMAIN_ATTR_PAGE_TABLE_IS_COHERENT;
	case QDF_DOMAIN_ATTR_PAGE_TABLE_FORCE_COHERENT:
		return DOMAIN_ATTR_PAGE_TABLE_FORCE_COHERENT;
	case QDF_DOMAIN_ATTR_USE_LLC_NWA:
		return DOMAIN_ATTR_USE_LLC_NWA;
	case QDF_DOMAIN_ATTR_SPLIT_TABLES:
		return DOMAIN_ATTR_SPLIT_TABLES;
	case QDF_DOMAIN_ATTR_FAULT_MODEL_NO_CFRE:
		return DOMAIN_ATTR_FAULT_MODEL_NO_CFRE;
	case QDF_DOMAIN_ATTR_FAULT_MODEL_NO_STALL:
		return DOMAIN_ATTR_FAULT_MODEL_NO_STALL;
	case QDF_DOMAIN_ATTR_FAULT_MODEL_HUPCF:
		return DOMAIN_ATTR_FAULT_MODEL_HUPCF;
	default:
		return DOMAIN_ATTR_EXTENDED_MAX;
	}
}

/**
 * __qdf_iommu_domain_get_attr() - API to get iommu domain attributes
 *
 * @domain: iommu domain
 * @attr: iommu attribute
 * @data: data pointer
 *
 * Return: iommu domain attr
 */
static inline int
__qdf_iommu_domain_get_attr(__qdf_iommu_domain_t *domain,
			    enum qdf_iommu_attr attr, void *data)
{
	return iommu_domain_get_attr(domain, __qdf_iommu_attr_to_os(attr),
				     data);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
/**
 * __qdf_dev_get_domain() - get iommu domain from osdev
 * @osdev: parent device instance
 *
 * Return: iommu domain
 */
static inline struct iommu_domain *
__qdf_dev_get_domain(qdf_device_t osdev)
{
	return osdev->domain;
}
#else
static inline struct iommu_domain *
__qdf_dev_get_domain(qdf_device_t osdev)
{
	if (osdev->iommu_mapping)
		return osdev->iommu_mapping->domain;

	return NULL;
}
#endif

/**
 * __qdf_mem_paddr_from_dmaaddr() - get actual physical address from dma_addr
 * @osdev: parent device instance
 * @dma_addr: dma_addr
 *
 * Get actual physical address from dma_addr based on SMMU enablement status.
 * IF SMMU Stage 1 translation is enabled, DMA APIs return IO virtual address
 * (IOVA) otherwise returns physical address. So get SMMU physical address
 * mapping from IOVA.
 *
 * Return: dmaable physical address
 */
static inline unsigned long
__qdf_mem_paddr_from_dmaaddr(qdf_device_t osdev,
			     qdf_dma_addr_t dma_addr)
{
	struct iommu_domain *domain;

	if (__qdf_mem_smmu_s1_enabled(osdev)) {
		domain = __qdf_dev_get_domain(osdev);
		if (domain)
			return iommu_iova_to_phys(domain, dma_addr);
	}

	return dma_addr;
}
#else
static inline unsigned long
__qdf_mem_paddr_from_dmaaddr(qdf_device_t osdev,
			     qdf_dma_addr_t dma_addr)
{
	return dma_addr;
}
#endif

/**
 * __qdf_os_mem_dma_get_sgtable() - Returns DMA memory scatter gather table
 * @dev: device instace
 * @sgt: scatter gather table pointer
 * @cpu_addr: HLOS virtual address
 * @dma_addr: dma/iova
 * @size: allocated memory size
 *
 * Return: physical address
 */
static inline int
__qdf_os_mem_dma_get_sgtable(struct device *dev, void *sgt, void *cpu_addr,
			     qdf_dma_addr_t dma_addr, size_t size)
{
	return dma_get_sgtable(dev, (struct sg_table *)sgt, cpu_addr, dma_addr,
				size);
}

/**
 * __qdf_os_mem_free_sgtable() - Free a previously allocated sg table
 * @sgt: the mapped sg table header
 *
 * Return: None
 */
static inline void
__qdf_os_mem_free_sgtable(struct sg_table *sgt)
{
	sg_free_table(sgt);
}

/**
 * __qdf_dma_get_sgtable_dma_addr()-Assigns DMA address to scatterlist elements
 * @sgt: scatter gather table pointer
 *
 * Return: None
 */
static inline void
__qdf_dma_get_sgtable_dma_addr(struct sg_table *sgt)
{
	struct scatterlist *sg;
	int i;

	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		if (!sg)
			break;

		sg->dma_address = sg_phys(sg);
	}
}

/**
 * __qdf_mem_get_dma_addr() - Return dma addr based on SMMU translation status
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Based on smmu stage 1 translation enablement status, return corresponding dma
 * address from qdf_mem_info_t. If stage 1 translation enabled, return
 * IO virtual address otherwise return physical address.
 *
 * Return: dma address
 */
static inline qdf_dma_addr_t __qdf_mem_get_dma_addr(qdf_device_t osdev,
						    qdf_mem_info_t *mem_info)
{
	if (__qdf_mem_smmu_s1_enabled(osdev))
		return (qdf_dma_addr_t)mem_info->iova;
	else
		return (qdf_dma_addr_t)mem_info->pa;
}

/**
 * __qdf_mem_get_dma_addr_ptr() - Return DMA address storage pointer
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Based on smmu stage 1 translation enablement status, return corresponding
 * dma address pointer from qdf_mem_info_t structure. If stage 1 translation
 * enabled, return pointer to IO virtual address otherwise return pointer to
 * physical address
 *
 * Return: dma address storage pointer
 */
static inline qdf_dma_addr_t *
__qdf_mem_get_dma_addr_ptr(qdf_device_t osdev,
			   qdf_mem_info_t *mem_info)
{
	if (__qdf_mem_smmu_s1_enabled(osdev))
		return (qdf_dma_addr_t *)(&mem_info->iova);
	else
		return (qdf_dma_addr_t *)(&mem_info->pa);
}

/**
 * __qdf_update_mem_map_table() - Update DMA memory map info
 * @osdev: Parent device instance
 * @mem_info: Pointer to shared memory information
 * @dma_addr: dma address
 * @mem_size: memory size allocated
 *
 * Store DMA shared memory information
 *
 * Return: none
 */
static inline void __qdf_update_mem_map_table(qdf_device_t osdev,
					      qdf_mem_info_t *mem_info,
					      qdf_dma_addr_t dma_addr,
					      uint32_t mem_size)
{
	mem_info->pa = __qdf_mem_paddr_from_dmaaddr(osdev, dma_addr);
	mem_info->iova = dma_addr;
	mem_info->size = mem_size;
}

/**
 * __qdf_mem_get_dma_size() - Return DMA memory size
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Return: DMA memory size
 */
static inline uint32_t
__qdf_mem_get_dma_size(qdf_device_t osdev,
		       qdf_mem_info_t *mem_info)
{
	return mem_info->size;
}

/**
 * __qdf_mem_set_dma_size() - Set DMA memory size
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 * @mem_size: memory size allocated
 *
 * Return: none
 */
static inline void
__qdf_mem_set_dma_size(qdf_device_t osdev,
		       qdf_mem_info_t *mem_info,
		       uint32_t mem_size)
{
	mem_info->size = mem_size;
}

/**
 * __qdf_mem_get_dma_size() - Return DMA physical address
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 *
 * Return: DMA physical address
 */
static inline qdf_dma_addr_t
__qdf_mem_get_dma_pa(qdf_device_t osdev,
		     qdf_mem_info_t *mem_info)
{
	return mem_info->pa;
}

/**
 * __qdf_mem_set_dma_size() - Set DMA physical address
 * @osdev: parent device instance
 * @mem_info: Pointer to allocated memory information
 * @dma_pa: DMA phsical address
 *
 * Return: none
 */
static inline void
__qdf_mem_set_dma_pa(qdf_device_t osdev,
		     qdf_mem_info_t *mem_info,
		     qdf_dma_addr_t dma_pa)
{
	mem_info->pa = dma_pa;
}

/**
 * __qdf_mem_alloc_consistent() - allocates consistent qdf memory
 * @osdev: OS device handle
 * @dev: Pointer to device handle
 * @size: Size to be allocated
 * @paddr: Physical address
 * @func: Function name of the call site
 * @line: line numbe rof the call site
 *
 * Return: pointer of allocated memory or null if memory alloc fails
 */
void *__qdf_mem_alloc_consistent(qdf_device_t osdev, void *dev,
				 qdf_size_t size, qdf_dma_addr_t *paddr,
				 const char *func, uint32_t line);

/**
 * __qdf_mem_malloc() - allocates QDF memory
 * @size: Number of bytes of memory to allocate.
 *
 * @func: Function name of the call site
 * @line: line numbe rof the call site
 *
 * This function will dynamicallly allocate the specified number of bytes of
 * memory.
 *
 * Return:
 * Upon successful allocate, returns a non-NULL pointer to the allocated
 * memory.  If this function is unable to allocate the amount of memory
 * specified (for any reason) it returns NULL.
 */
void *__qdf_mem_malloc(qdf_size_t size, const char *func, uint32_t line);

/**
 * __qdf_mem_free() - free QDF memory
 * @ptr: Pointer to the starting address of the memory to be freed.
 *
 * This function will free the memory pointed to by 'ptr'.
 * Return: None
 */
void __qdf_mem_free(void *ptr);

/**
 * __qdf_mem_valloc() - QDF virtual memory allocation API
 * @size: Number of bytes of virtual memory to allocate.
 * @func: Caller function name
 * @line: Line number
 *
 * Return: A valid memory location on success, or NULL on failure
 */
void *__qdf_mem_valloc(size_t size, const char *func, uint32_t line);

/**
 * __qdf_mem_vfree() - QDF API to free virtual memory
 * @ptr: Pointer to the virtual memory to free
 *
 * Return: None
 */
void __qdf_mem_vfree(void *ptr);

#ifdef QCA_WIFI_MODULE_PARAMS_FROM_INI
/**
 * __qdf_untracked_mem_malloc() - allocates non-QDF memory
 * @size: Number of bytes of memory to allocate.
 *
 * @func: Function name of the call site
 * @line: line number of the call site
 *
 * This function will dynamically allocate the specified number of bytes of
 * memory. Memory allocated is not tracked by qdf memory debug framework.
 *
 * Return:
 * Upon successful allocation, returns a non-NULL pointer to the allocated
 * memory.  If this function is unable to allocate the amount of memory
 * specified (for any reason) it returns NULL.
 */
void *__qdf_untracked_mem_malloc(qdf_size_t size, const char *func,
				 uint32_t line);

/**
 * __qdf_untracked_mem_free() - free non-QDF memory
 * @ptr: Pointer to the starting address of the memory to be freed.
 *
 * This function will free the memory pointed to by 'ptr'.
 * Return: None
 */

void __qdf_untracked_mem_free(void *ptr);
#endif

/**
 * __qdf_mem_free_consistent() - free consistent qdf memory
 * @osdev: OS device handle
 * @dev: Pointer to device handle
 * @size: Size to be allocated
 * @vaddr: virtual address
 * @paddr: Physical address
 * @memctx: Pointer to DMA context
 *
 * Return: none
 */
void __qdf_mem_free_consistent(qdf_device_t osdev, void *dev,
			       qdf_size_t size, void *vaddr,
			       qdf_dma_addr_t paddr, qdf_dma_context_t memctx);

#endif /* __I_QDF_MEM_H */
