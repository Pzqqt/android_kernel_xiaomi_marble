/*
 * Copyright (c) 2015-2016, 2018-2020 The Linux Foundation. All rights reserved.
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

#ifndef __IF_PCI_INTERNAL_H__
#define __IF_PCI_INTERNAL_H__

#ifdef DISABLE_L1SS_STATES
#define PCI_CFG_TO_DISABLE_L1SS_STATES(pdev, addr) \
{ \
	uint32_t lcr_val; \
	pfrm_read_config_dword(pdev, addr, &lcr_val); \
	pfrm_write_config_dword(pdev, addr, (lcr_val & ~0x0000000f)); \
}
#else
#define PCI_CFG_TO_DISABLE_L1SS_STATES(pdev, addr)
#endif

#ifdef QCA_WIFI_3_0
#define PCI_CLR_CAUSE0_REGISTER(sc) \
{ \
	uint32_t tmp_cause0; \
	tmp_cause0 = hif_read32_mb(sc, sc->mem + PCIE_INTR_CAUSE_ADDRESS); \
	hif_write32_mb(sc, sc->mem + PCIE_INTR_CLR_ADDRESS, \
		      PCIE_INTR_FIRMWARE_MASK | tmp_cause0); \
	hif_read32_mb(sc, sc->mem + PCIE_INTR_CLR_ADDRESS); \
	hif_write32_mb(sc, sc->mem + PCIE_INTR_CLR_ADDRESS, 0); \
	hif_read32_mb(sc, sc->mem + PCIE_INTR_CLR_ADDRESS); \
}
#else
#define PCI_CLR_CAUSE0_REGISTER(sc)
#endif
#endif /* __IF_PCI_INTERNAL_H__ */
