/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2013-2015, 2017-2021, The Linux Foundation. All rights reserved.
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
int msm_audio_get_phy_addr(int fd, dma_addr_t *paddr, size_t *pa_len);
void msm_audio_ion_crash_handler(void);
#endif /* _LINUX_MSM_AUDIO_ION_H */
