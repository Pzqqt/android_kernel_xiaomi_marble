/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2013-2015, 2017-2020, The Linux Foundation. All rights reserved.
 */

#ifndef _LINUX_MSM_AUDIO_ION_H
#define _LINUX_MSM_AUDIO_ION_H
#include <sound/pcm.h>
#include <linux/msm_ion.h>
#include <linux/dma-mapping.h>

enum {
	HLOS_TO_ADSP = 1,
	ADSP_TO_HLOS,
};
#define VMID_CP_ADSP_SHARED 33
enum {
	MSM_AUDIO_ION_INV_CACHES = 0,
	MSM_AUDIO_ION_CLEAN_CACHES,
};

struct audio_buffer {
	dma_addr_t phys;
	void       *data;
	uint32_t   used;
	uint32_t   size;/* size of buffer */
	uint32_t   actual_size; /* actual number of bytes read by DSP */
	struct      dma_buf *dma_buf;
};
int msm_audio_ion_alloc(struct dma_buf **dma_buf, size_t bufsz,
			dma_addr_t *paddr, size_t *pa_len, void **vaddr);

int msm_audio_ion_import(struct dma_buf **dma_buf, int fd,
			unsigned long *ionflag, size_t bufsz,
			dma_addr_t *paddr, size_t *pa_len, void **vaddr);
int msm_audio_ion_free(struct dma_buf *dma_buf);
int msm_audio_ion_import_cma(struct dma_buf **dma_buf, int fd,
			     unsigned long *ionflag, size_t bufsz,
			     dma_addr_t *paddr, size_t *pa_len, void **vaddr);
int msm_audio_ion_free_cma(struct dma_buf *dma_buf);
int msm_audio_ion_mmap(struct audio_buffer *abuff, struct vm_area_struct *vma);
int msm_audio_ion_cache_operations(struct audio_buffer *abuff, int cache_op);

u32 msm_audio_populate_upper_32_bits(dma_addr_t pa);
int msm_audio_ion_get_smmu_info(struct device **cb_dev, u64 *smmu_sid);

int msm_audio_ion_dma_map(dma_addr_t *phys_addr, dma_addr_t *iova_base,
			u32 size, enum dma_data_direction dir);
int msm_audio_ion_phys_assign(void **mem_hdl, int fd, dma_addr_t *paddr,
			      size_t *pa_len, u8 assign_type, int id);
int msm_audio_ion_phys_free(void *mem_hdl, dma_addr_t *paddr,
			size_t *pa_len, u8 assign_type, int id, int key);
bool msm_audio_is_hypervisor_supported(void);
int msm_audio_get_phy_addr(int fd, dma_addr_t *paddr);
void msm_audio_ion_crash_handler(void);
#endif /* _LINUX_MSM_AUDIO_ION_H */
