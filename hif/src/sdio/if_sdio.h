/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef __IF_SDIO_H__
#define __IF_SDIO_H__

#include <linux/version.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <osdep.h>
#include <ol_if_athvar.h>
#include <athdefs.h>
#include "a_osapi.h"
#include "hif_internal.h"


#define AR6320_HEADERS_DEF

#define ATH_DBG_DEFAULT   0

#define RAMDUMP_ADDR     0x8F000000
#define RAMDUMP_SIZE     0x700000

struct hif_sdio_softc {
	struct hif_softc ol_sc;
	struct device *dev;
	struct _NIC_DEV aps_osdev;
	struct tasklet_struct intr_tq;  /* tasklet */

	int irq;
	/*
	 * Guard changes to Target HW state and to software
	 * structures that track hardware state.
	 */
	spinlock_t target_lock;
	void *hif_handle;
	void *ramdump_base;
	unsigned long ramdump_address;
	unsigned long ramdump_size;
	struct targetdef_s *targetdef;
	struct hostdef_s *hostdef;
};

#if defined(CONFIG_ATH_PROCFS_DIAG_SUPPORT)
int athdiag_procfs_init(void *scn);
void athdiag_procfs_remove(void);
#else
static inline int athdiag_procfs_init(void *scn)
{
	return 0;
}

static inline void athdiag_procfs_remove(void)
{
	return;
}
#endif

#ifndef REMOVE_PKT_LOG
extern int pktlogmod_init(void *context);
extern void pktlogmod_exit(void *context);
#endif

#define DMA_MAPPING_ERROR(dev, addr) dma_mapping_error((dev), (addr))

int ath_sdio_probe(void *context, void *hif_handle);
void ath_sdio_remove(void *context, void *hif_handle);
int ath_sdio_suspend(void *context);
int ath_sdio_resume(void *context);

/*These functions are exposed to HDD*/
void hif_init_qdf_ctx(qdf_device_t qdf_dev, void *ol_sc);
void hif_deinit_qdf_ctx(void *ol_sc);

int hif_sdio_device_inserted(struct device *dev,
		const struct sdio_device_id *id);
void hif_sdio_stop(struct hif_softc *hif_ctx);
void hif_sdio_shutdown(struct hif_softc *hif_ctx);
void hif_sdio_device_removed(struct sdio_func *func);
int hif_device_suspend(struct device *dev);
int hif_device_resume(struct device *dev);
void hif_register_tbl_attach(struct hif_softc *scn,
						u32 hif_type);
void target_register_tbl_attach(struct hif_softc *scn,
						u32 target_type);
#endif /* __IF_SDIO_H__ */
