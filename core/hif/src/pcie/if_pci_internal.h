/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
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

#ifndef __IF_PCI_INTERNAL_H__
#define __IF_PCI_INTERNAL_H__
#ifdef CONFIG_CNSS
#define HIF_REGISTER_DRIVER(wlan_drv_id) \
	cnss_wlan_register_driver(wlan_drv_id)
#define HIF_UNREGISTER_DRIVER(wlan_drv_id) \
	cnss_wlan_unregister_driver(wlan_drv_id)
#else
#define HIF_REGISTER_DRIVER(wlan_drv_id) \
	pci_register_driver(wlan_drv_id)
#define HIF_UNREGISTER_DRIVER(wlan_drv_id) \
	pci_unregister_driver(wlan_drv_id)
#endif

#ifdef DISABLE_L1SS_STATES
#define PCI_CFG_TO_DISABLE_L1SS_STATES(pdev, addr) \
{ \
	uint32_t lcr_val; \
	pci_read_config_dword(pdev, addr, &lcr_val); \
	pci_write_config_dword(pdev, addr, (lcr_val & ~0x0000000f)); \
}
#else
#define PCI_CFG_TO_DISABLE_L1SS_STATES(pdev, addr)
#endif

#if defined(CONFIG_CNSS) && !defined(QCA_WIFI_3_0)
#define GET_VIRT_RAMDUMP_MEM(ol_sc) \
{ \
	ol_sc->ramdump_base = cnss_get_virt_ramdump_mem(&ol_sc->ramdump_size); \
	if (ol_sc->ramdump_base == NULL || !ol_sc->ramdump_size) \
		HIF_ERROR("%s: Failed to get RAM dump memory addr or size!", \
			__func__); \
}
#else
#define GET_VIRT_RAMDUMP_MEM(ol_sc)
#endif

#ifdef QCA_WIFI_3_0
#define PCI_CLR_CAUSE0_REGISTER(sc) \
{ \
	uint32_t tmp_cause0; \
	tmp_cause0 = hif_read32_mb(sc->mem + PCIE_INTR_CAUSE_ADDRESS); \
	hif_write32_mb(sc->mem + PCIE_INTR_CLR_ADDRESS, \
		      PCIE_INTR_FIRMWARE_MASK | tmp_cause0); \
	hif_read32_mb(sc->mem + PCIE_INTR_CLR_ADDRESS); \
	hif_write32_mb(sc->mem + PCIE_INTR_CLR_ADDRESS, 0); \
	hif_read32_mb(sc->mem + PCIE_INTR_CLR_ADDRESS); \
}

#define HIF_PCI_TARG_IS_AWAKE(scn, mem) \
{ \
	return 1; \
}
#else
#define PCI_CLR_CAUSE0_REGISTER(sc)

#define HIF_PCI_TARG_IS_AWAKE(scn, mem) \
{ \
	uint32_t val; \
	if (scn->recovery) \
		return false; \
	val = hif_read32_mb(mem + PCIE_LOCAL_BASE_ADDRESS \
		+ RTC_STATE_ADDRESS); \
	return RTC_STATE_V_GET(val) == RTC_STATE_V_ON; \
}
#endif

#if !defined(REMOVE_PKT_LOG) && !defined(QVIT)
#define PKT_LOG_MOD_INIT(ol_sc) \
{ \
	ol_txrx_pdev_handle pdev_txrx_handle; \
	pdev_txrx_handle = cds_get_context(CDF_MODULE_ID_TXRX); \
	if (cds_get_conparam() != CDF_GLOBAL_FTM_MODE && \
	    !WLAN_IS_EPPING_ENABLED(cds_get_conparam())) { \
		ol_pl_sethandle(&pdev_txrx_handle->pl_dev, ol_sc); \
		if (pktlogmod_init(ol_sc)) \
			HIF_ERROR("%s: pktlogmod_init failed", __func__); \
	} \
}
#else
#define PKT_LOG_MOD_INIT(ol_sc)
#endif
#endif /* __IF_PCI_INTERNAL_H__ */
