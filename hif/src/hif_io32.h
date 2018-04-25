/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_IO32_H__
#define __HIF_IO32_H__

#include <linux/io.h>
#include "hif.h"
#include "hif_main.h"

#define hif_read32_mb(addr)         ioread32((void __iomem *)addr)
#define hif_write32_mb(addr, value) \
	iowrite32((u32)(value), (void __iomem *)(addr))

#define Q_TARGET_ACCESS_BEGIN(scn) \
	hif_target_sleep_state_adjust(scn, false, true)
#define Q_TARGET_ACCESS_END(scn) \
	hif_target_sleep_state_adjust(scn, true, false)
#define TARGET_REGISTER_ACCESS_ALLOWED(scn)\
		hif_is_target_register_access_allowed(scn)

/*
 * A_TARGET_ACCESS_LIKELY will not wait for the target to wake up before
 * continuing execution.  Because A_TARGET_ACCESS_LIKELY does not guarantee
 * that the target is awake before continuing, Q_TARGET_ACCESS macros must
 * protect the actual target access.  Since Q_TARGET_ACCESS protect the actual
 * target access, A_TARGET_ACCESS_LIKELY hints are optional.
 *
 * To ignore "LIKELY" hints, set CONFIG_TARGET_ACCESS_LIKELY to 0
 * (slightly worse performance, less power)
 *
 * To use "LIKELY" hints, set CONFIG_TARGET_ACCESS_LIKELY to 1
 * (slightly better performance, more power)
 *
 * note: if a bus doesn't use hif_target_sleep_state_adjust, this will have
 * no impact.
 */
#define CONFIG_TARGET_ACCESS_LIKELY 0
#if CONFIG_TARGET_ACCESS_LIKELY
#define A_TARGET_ACCESS_LIKELY(scn) \
	hif_target_sleep_state_adjust(scn, false, false)
#define A_TARGET_ACCESS_UNLIKELY(scn) \
	hif_target_sleep_state_adjust(scn, true, false)
#else                           /* CONFIG_ATH_PCIE_ACCESS_LIKELY */
#define A_TARGET_ACCESS_LIKELY(scn) \
	do { \
		unsigned long unused = (unsigned long)(scn); \
		unused = unused; \
	} while (0)

#define A_TARGET_ACCESS_UNLIKELY(scn) \
	do { \
		unsigned long unused = (unsigned long)(scn); \
		unused = unused; \
	} while (0)
#endif /* CONFIG_ATH_PCIE_ACCESS_LIKELY */


#ifdef HIF_PCI
#include "hif_io32_pci.h"
#endif
#ifdef HIF_SNOC
#include "hif_io32_snoc.h"
#endif /* HIF_PCI */

#ifdef CONFIG_IO_MEM_ACCESS_DEBUG
uint32_t hif_target_read_checked(struct hif_softc *scn,
					uint32_t offset);
void hif_target_write_checked(struct hif_softc *scn, uint32_t offset,
				     uint32_t value);

#define A_TARGET_READ(scn, offset) \
	hif_target_read_checked(scn, (offset))
#define A_TARGET_WRITE(scn, offset, value) \
	hif_target_write_checked(scn, (offset), (value))
#else                           /* CONFIG_ATH_PCIE_ACCESS_DEBUG */
#define A_TARGET_READ(scn, offset) \
	hif_read32_mb(scn->mem + (offset))
#define A_TARGET_WRITE(scn, offset, value) \
	hif_write32_mb((scn->mem) + (offset), value)
#endif

void hif_irq_enable(struct hif_softc *scn, int irq_id);
void hif_irq_disable(struct hif_softc *scn, int irq_id);

#endif /* __HIF_IO32_H__ */
