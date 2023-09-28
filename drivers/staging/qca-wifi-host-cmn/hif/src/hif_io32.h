/*
 * Copyright (c) 2015-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "pld_common.h"
/* Device memory is 32MB but bar size is only 1MB.
 * Register remapping logic is used to access 32MB device memory.
 * 0-512KB : Fixed address, 512KB-1MB : remapped address.
 * Use PCIE_REMAP_1M_BAR_CTRL register to set window
 * for pcie based wifi chipsets.
 */
#define MAX_UNWINDOWED_ADDRESS 0x80000
#if defined(QCA_WIFI_QCA6390) || defined(QCA_WIFI_QCA6490) || \
	defined(QCA_WIFI_QCN9000) || defined(QCA_WIFI_QCA6750) || \
	defined(QCA_WIFI_QCN9224) || defined(QCA_WIFI_KIWI)
#define WINDOW_ENABLE_BIT 0x40000000
#else
#define WINDOW_ENABLE_BIT 0x80000000
#endif
#define WINDOW_REG_ADDRESS 0x310C
#define WINDOW_SHIFT 19
#define WINDOW_VALUE_MASK 0x3F
#define WINDOW_START MAX_UNWINDOWED_ADDRESS
#define WINDOW_RANGE_MASK 0x7FFFF

#if defined(HIF_REG_WINDOW_SUPPORT) && defined(HIF_PCI)

static inline
void hif_write32_mb_reg_window(void *sc,
			       void __iomem *addr, uint32_t value);
static inline
uint32_t hif_read32_mb_reg_window(void *sc,
				  void __iomem *addr);
#define hif_read32_mb(scn, addr) \
	hif_read32_mb_reg_window((void *)scn, \
				 (void __iomem *)addr)
#define hif_write32_mb(scn, addr, value) \
	hif_write32_mb_reg_window((void *)scn, \
				  (void __iomem *)addr, value)

#else
#define hif_read32_mb(scn, addr)         ioread32((void __iomem *)addr)
#define hif_write32_mb(scn, addr, value) \
	iowrite32((u32)(value), (void __iomem *)(addr))
#endif

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
#endif
#ifdef HIF_IPCI
#include "hif_io32_ipci.h"
#endif

#ifdef HIF_IPCI
/**
 * hif_target_access_allowed(): Check if target access is allowed
 *
 * @scn: HIF handler
 *
 * Return: True if access is allowed else False
 */
static inline
bool hif_target_access_allowed(struct hif_softc *scn)
{
	return !(scn->recovery);
}

#define TARGET_ACCESS_ALLOWED(scn) \
	hif_target_access_allowed(scn)
#else
#define TARGET_ACCESS_ALLOWED(scn) (1)
#endif

#if defined(HIF_REG_WINDOW_SUPPORT) && defined(HIF_PCI)

#include "qdf_lock.h"
#include "qdf_util.h"

/**
 * hif_reg_write_result_check() - check register writing result
 * @sc: hif pcie context
 * @offset: register offset to read
 * @exp_val: the expected value of register
 *
 * Return: none
 */
static inline void hif_reg_write_result_check(struct hif_pci_softc *sc,
					      uint32_t offset,
					      uint32_t exp_val)
{
	uint32_t value;

	value = qdf_ioread32(sc->mem + offset);
	if (exp_val != value) {
		hif_err("Reg write failed. write val 0x%x read val 0x%x offset 0x%x",
			exp_val,
			value,
			offset);
	}
}

#ifdef PCIE_REG_WINDOW_LOCAL_NO_CACHE
/**
 * hif_select_window_confirm(): Update the register window
 * @sc: HIF pci handle
 * @offset: reg offset to read from or write to
 *
 * Calculate the window using the offset provided and update
 * the window reg value accordingly for windowed read/write reg
 * access.
 * Read back to make sure the window is written to the register.
 * Return: None
 */
static inline
void hif_select_window_confirm(struct hif_pci_softc *sc, uint32_t offset)
{
	uint32_t window = (offset >> WINDOW_SHIFT) & WINDOW_VALUE_MASK;

	qdf_iowrite32(sc->mem + WINDOW_REG_ADDRESS,
		      WINDOW_ENABLE_BIT | window);
	sc->register_window = window;
	hif_reg_write_result_check(sc, WINDOW_REG_ADDRESS,
				   WINDOW_ENABLE_BIT | window);
}
#else /* PCIE_REG_WINDOW_LOCAL_NO_CACHE */

static inline
void hif_select_window_confirm(struct hif_pci_softc *sc, uint32_t offset)
{
	uint32_t window = (offset >> WINDOW_SHIFT) & WINDOW_VALUE_MASK;

	if (window != sc->register_window) {
		qdf_iowrite32(sc->mem + WINDOW_REG_ADDRESS,
			      WINDOW_ENABLE_BIT | window);
		sc->register_window = window;
		hif_reg_write_result_check(sc, WINDOW_REG_ADDRESS,
					   WINDOW_ENABLE_BIT | window);
	}
}
#endif /* PCIE_REG_WINDOW_LOCAL_NO_CACHE */

#ifdef WINDOW_REG_PLD_LOCK_ENABLE
/**
 * hif_lock_reg_access() - Lock window register access spinlock
 * @sc: HIF handle
 * @flags: variable pointer to save CPU states
 *
 * Lock register window spinlock
 *
 * Return: void
 */
static inline void hif_lock_reg_access(struct hif_pci_softc *sc,
				       unsigned long *flags)
{
	pld_lock_reg_window(sc->dev, flags);
}

/**
 * hif_unlock_reg_access() - Unlock window register access spinlock
 * @sc: HIF handle
 * @flags: variable pointer to save CPU states
 *
 * Unlock register window spinlock
 *
 * Return: void
 */
static inline void hif_unlock_reg_access(struct hif_pci_softc *sc,
					 unsigned long *flags)
{
	pld_unlock_reg_window(sc->dev, flags);
}
#else
static inline void hif_lock_reg_access(struct hif_pci_softc *sc,
				       unsigned long *flags)
{
	qdf_spin_lock_irqsave(&sc->register_access_lock);
}

static inline void hif_unlock_reg_access(struct hif_pci_softc *sc,
					 unsigned long *flags)
{
	qdf_spin_unlock_irqrestore(&sc->register_access_lock);
}
#endif

/**
 * note1: WINDOW_RANGE_MASK = (1 << WINDOW_SHIFT) -1
 * note2: 1 << WINDOW_SHIFT = MAX_UNWINDOWED_ADDRESS
 * note3: WINDOW_VALUE_MASK = big enough that trying to write past that window
 *				would be a bug
 */
static inline void hif_write32_mb_reg_window(void *scn,
					     void __iomem *addr, uint32_t value)
{
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	uint32_t offset = addr - sc->mem;
	unsigned long flags;

	if (!sc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		qdf_iowrite32(addr, value);
	} else {
		hif_lock_reg_access(sc, &flags);
		hif_select_window_confirm(sc, offset);
		qdf_iowrite32(sc->mem + WINDOW_START +
			  (offset & WINDOW_RANGE_MASK), value);
		hif_unlock_reg_access(sc, &flags);
	}
}

static inline uint32_t hif_read32_mb_reg_window(void *scn, void __iomem *addr)
{
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	uint32_t ret;
	uint32_t offset = addr - sc->mem;
	unsigned long flags;

	if (!sc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		return qdf_ioread32(addr);
	}
	hif_lock_reg_access(sc, &flags);
	hif_select_window_confirm(sc, offset);
	ret = qdf_ioread32(sc->mem + WINDOW_START +
		       (offset & WINDOW_RANGE_MASK));
	hif_unlock_reg_access(sc, &flags);

	return ret;
}
#endif

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
	hif_read32_mb(scn, scn->mem + (offset))
#define A_TARGET_WRITE(scn, offset, value) \
	hif_write32_mb(scn, (scn->mem) + (offset), value)
#endif

void hif_irq_enable(struct hif_softc *scn, int irq_id);
void hif_irq_disable(struct hif_softc *scn, int irq_id);

#endif /* __HIF_IO32_H__ */
