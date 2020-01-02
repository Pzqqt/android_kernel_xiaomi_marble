/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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
 * DOC: if_ahb.c
 *
 * c file for ahb specific implementations.
 */

#include "hif.h"
#include "target_type.h"
#include "hif_main.h"
#include "hif_debug.h"
#include "hif_io32.h"
#include "ce_main.h"
#include "ce_api.h"
#include "ce_tasklet.h"
#include "if_ahb.h"
#include "if_pci.h"
#include "ahb_api.h"
#include "pci_api.h"
#include "hif_napi.h"
#include "qal_vbus_dev.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
#define IRQF_DISABLED 0x00000020
#endif

#define HIF_IC_CE0_IRQ_OFFSET 4
#define HIF_IC_MAX_IRQ 52

static uint8_t ic_irqnum[HIF_IC_MAX_IRQ];
/* integrated chip irq names */
const char *ic_irqname[HIF_IC_MAX_IRQ] = {
"misc-pulse1",
"misc-latch",
"sw-exception",
"watchdog",
"ce0",
"ce1",
"ce2",
"ce3",
"ce4",
"ce5",
"ce6",
"ce7",
"ce8",
"ce9",
"ce10",
"ce11",
"host2wbm-desc-feed",
"host2reo-re-injection",
"host2reo-command",
"host2rxdma-monitor-ring3",
"host2rxdma-monitor-ring2",
"host2rxdma-monitor-ring1",
"reo2ost-exception",
"wbm2host-rx-release",
"reo2host-status",
"reo2host-destination-ring4",
"reo2host-destination-ring3",
"reo2host-destination-ring2",
"reo2host-destination-ring1",
"rxdma2host-monitor-destination-mac3",
"rxdma2host-monitor-destination-mac2",
"rxdma2host-monitor-destination-mac1",
"ppdu-end-interrupts-mac3",
"ppdu-end-interrupts-mac2",
"ppdu-end-interrupts-mac1",
"rxdma2host-monitor-status-ring-mac3",
"rxdma2host-monitor-status-ring-mac2",
"rxdma2host-monitor-status-ring-mac1",
"host2rxdma-host-buf-ring-mac3",
"host2rxdma-host-buf-ring-mac2",
"host2rxdma-host-buf-ring-mac1",
"rxdma2host-destination-ring-mac3",
"rxdma2host-destination-ring-mac2",
"rxdma2host-destination-ring-mac1",
"host2tcl-input-ring4",
"host2tcl-input-ring3",
"host2tcl-input-ring2",
"host2tcl-input-ring1",
"wbm2host-tx-completions-ring3",
"wbm2host-tx-completions-ring2",
"wbm2host-tx-completions-ring1",
"tcl2host-status-ring",
};

/** hif_ahb_get_irq_name() - get irqname
 * This function gives irqnumber to irqname
 * mapping.
 *
 * @irq_no: irq number
 *
 * Return: irq name
 */
const char *hif_ahb_get_irq_name(int irq_no)
{
	return ic_irqname[irq_no];
}

/**
 * hif_disable_isr() - disable isr
 *
 * This function disables isr and kills tasklets
 *
 * @hif_ctx: struct hif_softc
 *
 * Return: void
 */
void hif_ahb_disable_isr(struct hif_softc *scn)
{
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);

	hif_exec_kill(&scn->osc);
	hif_nointrs(scn);
	ce_tasklet_kill(scn);
	tasklet_kill(&sc->intr_tq);
	qdf_atomic_set(&scn->active_tasklet_cnt, 0);
	qdf_atomic_set(&scn->active_grp_tasklet_cnt, 0);
}

/**
 * hif_dump_registers() - dump bus debug registers
 * @scn: struct hif_opaque_softc
 *
 * This function dumps hif bus debug registers
 *
 * Return: 0 for success or error code
 */
int hif_ahb_dump_registers(struct hif_softc *hif_ctx)
{
	int status;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	status = hif_dump_ce_registers(scn);
	if (status)
		HIF_ERROR("%s: Dump CE Registers Failed status %d", __func__,
							status);

	return 0;
}

/**
 * hif_ahb_close() - hif_bus_close
 * @scn: pointer to the hif context.
 *
 * This is a callback function for hif_bus_close.
 *
 *
 * Return: n/a
 */
void hif_ahb_close(struct hif_softc *scn)
{
	hif_ce_close(scn);
}

/**
 * hif_bus_open() - hif_ahb open
 * @hif_ctx: hif context
 * @bus_type: bus type
 *
 * This is a callback function for hif_bus_open.
 *
 * Return: n/a
 */
QDF_STATUS hif_ahb_open(struct hif_softc *hif_ctx, enum qdf_bus_type bus_type)
{

	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(hif_ctx);

	qdf_spinlock_create(&sc->irq_lock);
	return hif_ce_open(hif_ctx);
}

/**
 * hif_bus_configure() - Configure the bus
 * @scn: pointer to the hif context.
 *
 * This function configure the ahb bus
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_ahb_bus_configure(struct hif_softc *scn)
{
	return hif_pci_bus_configure(scn);
}

/**
 * hif_configure_msi_ahb - Configure MSI interrupts
 * @sc : pointer to the hif context
 *
 * return: 0 for success. nonzero for failure.
 */

int hif_configure_msi_ahb(struct hif_pci_softc *sc)
{
	return 0;
}

/**
 * hif_ahb_configure_legacy_irq() - Configure Legacy IRQ
 * @sc: pointer to the hif context.
 *
 * This function registers the irq handler and enables legacy interrupts
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_ahb_configure_legacy_irq(struct hif_pci_softc *sc)
{
	int ret = 0;
	struct hif_softc *scn = HIF_GET_SOFTC(sc);
	struct platform_device *pdev = (struct platform_device *)sc->pdev;
	int irq = 0;

	/* do not support MSI or MSI IRQ failed */
	tasklet_init(&sc->intr_tq, wlan_tasklet, (unsigned long)sc);
	qal_vbus_get_irq((struct qdf_pfm_hndl *)pdev, "legacy", &irq);
	if (irq < 0) {
		dev_err(&pdev->dev, "Unable to get irq\n");
		ret = -1;
		goto end;
	}
	ret = request_irq(irq, hif_pci_legacy_ce_interrupt_handler,
				IRQF_DISABLED, "wlan_ahb", sc);
	if (ret) {
		dev_err(&pdev->dev, "ath_request_irq failed\n");
		ret = -1;
		goto end;
	}
	sc->irq = irq;

	/* Use Legacy PCI Interrupts */
	hif_write32_mb(sc, sc->mem + (SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS),
			PCIE_INTR_FIRMWARE_MASK | PCIE_INTR_CE_MASK_ALL);
	/* read once to flush */
	hif_read32_mb(sc, sc->mem + (SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS));

end:
	return ret;
}

int hif_ahb_configure_irq(struct hif_pci_softc *sc)
{
	int ret = 0;
	struct hif_softc *scn = HIF_GET_SOFTC(sc);
	struct platform_device *pdev = (struct platform_device *)sc->pdev;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct CE_attr *host_ce_conf = hif_state->host_ce_config;
	int irq = 0;
	int i;

	/* configure per CE interrupts */
	for (i = 0; i < scn->ce_count; i++) {
		if (host_ce_conf[i].flags & CE_ATTR_DISABLE_INTR)
			continue;
		ret = pfrm_get_irq(&pdev->dev, (struct qdf_pfm_hndl *)pdev,
				   ic_irqname[HIF_IC_CE0_IRQ_OFFSET + i],
				   HIF_IC_CE0_IRQ_OFFSET + i, &irq);
		if (ret) {
			dev_err(&pdev->dev, "get irq failed\n");
			ret = -1;
			goto end;
		}

		ic_irqnum[HIF_IC_CE0_IRQ_OFFSET + i] = irq;
		ret = pfrm_request_irq(&pdev->dev, irq,
				       hif_ahb_interrupt_handler,
				       IRQF_TRIGGER_RISING,
				       ic_irqname[HIF_IC_CE0_IRQ_OFFSET + i],
				       &hif_state->tasklets[i]);
		if (ret) {
			dev_err(&pdev->dev, "ath_request_irq failed\n");
			ret = -1;
			goto end;
		}
		hif_ahb_irq_enable(scn, i);
	}

end:
	return ret;
}

int hif_ahb_configure_grp_irq(struct hif_softc *scn,
			      struct hif_exec_context *hif_ext_group)
{
	int ret = 0;
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	struct platform_device *pdev = (struct platform_device *)sc->pdev;
	int irq = 0;
	int j;

	/* configure external interrupts */
	hif_ext_group->irq_enable = &hif_ahb_exec_grp_irq_enable;
	hif_ext_group->irq_disable = &hif_ahb_exec_grp_irq_disable;
	hif_ext_group->irq_name = &hif_ahb_get_irq_name;
	hif_ext_group->work_complete = &hif_dummy_grp_done;

	qdf_spin_lock_irqsave(&hif_ext_group->irq_lock);

	for (j = 0; j < hif_ext_group->numirq; j++) {
		ret = pfrm_get_irq(&pdev->dev, (struct qdf_pfm_hndl *)pdev,
				   ic_irqname[hif_ext_group->irq[j]],
				   hif_ext_group->irq[j], &irq);
		if (ret) {
			dev_err(&pdev->dev, "get irq failed\n");
			ret = -1;
			goto end;
		}
		ic_irqnum[hif_ext_group->irq[j]] = irq;
		irq_set_status_flags(irq, IRQ_DISABLE_UNLAZY);
		ret = pfrm_request_irq(scn->qdf_dev->dev,
				       irq, hif_ext_group_interrupt_handler,
				       IRQF_TRIGGER_RISING,
				       ic_irqname[hif_ext_group->irq[j]],
				       hif_ext_group);
		if (ret) {
			dev_err(&pdev->dev, "ath_request_irq failed\n");
			ret = -1;
			goto end;
		}
		hif_ext_group->os_irq[j] = irq;
	}
	qdf_spin_unlock_irqrestore(&hif_ext_group->irq_lock);

	qdf_spin_lock_irqsave(&hif_ext_group->irq_lock);
	hif_ext_group->irq_requested = true;

end:
	qdf_spin_unlock_irqrestore(&hif_ext_group->irq_lock);
	return ret;
}

void hif_ahb_deconfigure_grp_irq(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;
	int i, j;
	int irq = 0;

	/* configure external interrupts */
	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		hif_ext_group = hif_state->hif_ext_group[i];
		if (hif_ext_group->irq_requested == true) {
			qdf_spin_lock_irqsave(&hif_ext_group->irq_lock);
			hif_ext_group->irq_requested = false;
			for (j = 0; j < hif_ext_group->numirq; j++) {
				irq = hif_ext_group->os_irq[j];
				hif_ext_group->irq_enabled = false;
				irq_clear_status_flags(irq,
						       IRQ_DISABLE_UNLAZY);
			}
			qdf_spin_unlock_irqrestore(&hif_ext_group->irq_lock);

			/* Avoid holding the irq_lock while freeing the irq
			 * as the same lock is being held by the irq handler
			 * while disabling the irq. This causes a deadlock
			 * between free_irq and irq_handler.
			 */
			for (j = 0; j < hif_ext_group->numirq; j++) {
				irq = hif_ext_group->os_irq[j];
				pfrm_free_irq(scn->qdf_dev->dev,
					      irq, hif_ext_group);
			}
		}
	}
}

irqreturn_t hif_ahb_interrupt_handler(int irq, void *context)
{
	struct ce_tasklet_entry *tasklet_entry = context;
	return ce_dispatch_interrupt(tasklet_entry->ce_id, tasklet_entry);
}

/**
 * hif_target_sync() : ensure the target is ready
 * @scn: hif control structure
 *
 * Informs fw that we plan to use legacy interupts so that
 * it can begin booting. Ensures that the fw finishes booting
 * before continuing. Should be called before trying to write
 * to the targets other registers for the first time.
 *
 * Return: none
 */
int hif_target_sync_ahb(struct hif_softc *scn)
{
	int val = 0;
	int limit = 0;

	while (limit < 50) {
		hif_write32_mb(scn, scn->mem +
			(SOC_CORE_BASE_ADDRESS | PCIE_INTR_ENABLE_ADDRESS),
			PCIE_INTR_FIRMWARE_MASK | PCIE_INTR_CE_MASK_ALL);
		qdf_mdelay(10);
		val = hif_read32_mb(scn, scn->mem +
			(SOC_CORE_BASE_ADDRESS | PCIE_INTR_ENABLE_ADDRESS));
		if (val == 0)
			break;
		limit++;
	}
	hif_write32_mb(scn, scn->mem +
		(SOC_CORE_BASE_ADDRESS | PCIE_INTR_ENABLE_ADDRESS),
		PCIE_INTR_FIRMWARE_MASK | PCIE_INTR_CE_MASK_ALL);
	hif_write32_mb(scn, scn->mem + FW_INDICATOR_ADDRESS, FW_IND_HOST_READY);
	if (HAS_FW_INDICATOR) {
		int wait_limit = 500;
		int fw_ind = 0;

		while (1) {
			fw_ind = hif_read32_mb(scn, scn->mem +
					FW_INDICATOR_ADDRESS);
			if (fw_ind & FW_IND_INITIALIZED)
				break;
			if (wait_limit-- < 0)
				break;
			hif_write32_mb(scn, scn->mem + (SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS),
				PCIE_INTR_FIRMWARE_MASK);
			qdf_mdelay(10);
		}
		if (wait_limit < 0) {
			HIF_TRACE("%s: FW signal timed out", __func__);
			return -EIO;
		}
		HIF_TRACE("%s: Got FW signal, retries = %x", __func__,
							500-wait_limit);
	}

	return 0;
}

/**
 * hif_disable_bus() - Disable the bus
 * @scn : pointer to the hif context
 *
 * This function disables the bus and helds the target in reset state
 *
 * Return: none
 */
void hif_ahb_disable_bus(struct hif_softc *scn)
{
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	void __iomem *mem;
	struct platform_device *pdev = (struct platform_device *)sc->pdev;
	struct resource *memres = NULL;
	int mem_pa_size = 0;
	struct hif_target_info *tgt_info = NULL;
	struct qdf_vbus_resource *vmres = NULL;
	QDF_STATUS status;

	tgt_info = &scn->target_info;
	/*Disable WIFI clock input*/
	if (sc->mem) {
		status = pfrm_platform_get_resource(
				scn->qdf_dev->dev,
				(struct qdf_pfm_hndl *)pdev, &vmres,
				IORESOURCE_MEM, 0);
		if (QDF_IS_STATUS_ERROR(status)) {
			HIF_INFO("%s: Failed to get IORESOURCE_MEM\n",
				 __func__);
			return;
		}
		memres = (struct resource *)vmres;
		if (memres)
			mem_pa_size = memres->end - memres->start + 1;

		/* Should not be executed on 8074 platform */
		if ((tgt_info->target_type != TARGET_TYPE_QCA8074) &&
		    (tgt_info->target_type != TARGET_TYPE_QCA8074V2) &&
		    (tgt_info->target_type != TARGET_TYPE_QCA6018)) {
			hif_ahb_clk_enable_disable(&pdev->dev, 0);

			hif_ahb_device_reset(scn);
		}
		mem = (void __iomem *)sc->mem;
		if (mem) {
			pfrm_devm_iounmap(&pdev->dev, mem);
			pfrm_devm_release_mem_region(&pdev->dev, scn->mem_pa,
						     mem_pa_size);
			sc->mem = NULL;
		}
	}
	scn->mem = NULL;
}

/**
 * hif_enable_bus() - Enable the bus
 * @dev: dev
 * @bdev: bus dev
 * @bid: bus id
 * @type: bus type
 *
 * This function enables the radio bus by enabling necessary
 * clocks and waits for the target to get ready to proceed futher
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_ahb_enable_bus(struct hif_softc *ol_sc,
		struct device *dev, void *bdev,
		const struct hif_bus_id *bid,
		enum hif_enable_type type)
{
	int ret = 0;
	int hif_type;
	int target_type;
	const struct platform_device_id *id = (struct platform_device_id *)bid;
	struct platform_device *pdev = bdev;
	struct hif_target_info *tgt_info = NULL;
	struct resource *memres = NULL;
	void __iomem *mem = NULL;
	uint32_t revision_id = 0;
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(ol_sc);
	QDF_STATUS status;
	struct qdf_vbus_resource *vmres = NULL;

	sc->pdev = (struct pci_dev *)pdev;
	sc->dev = &pdev->dev;
	sc->devid = id->driver_data;

	ret = hif_get_device_type(id->driver_data, revision_id,
			&hif_type, &target_type);
	if (ret < 0) {
		HIF_ERROR("%s: invalid device  ret %d id %d revision_id %d",
			__func__, ret, (int)id->driver_data, revision_id);
		return QDF_STATUS_E_FAILURE;
	}

	status = pfrm_platform_get_resource(&pdev->dev,
					    (struct qdf_pfm_hndl *)pdev,
					    &vmres,
					    IORESOURCE_MEM, 0);
	if (QDF_IS_STATUS_ERROR(status)) {
		HIF_INFO("%s: Failed to get IORESOURCE_MEM\n", __func__);
		return -EIO;
	}
	memres = (struct resource *)vmres;
	if (!memres) {
		HIF_INFO("%s: Failed to get IORESOURCE_MEM\n", __func__);
		return -EIO;
	}

	ret = pfrm_dma_set_mask(dev, 32);
	if (ret) {
		HIF_INFO("ath: 32-bit DMA not available\n");
		goto err_cleanup1;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	ret = pfrm_dma_set_mask_and_coherent(dev, 32);
#else
	ret = pfrm_dma_set_coherent_mask(dev, 32);
#endif
	if (ret) {
		HIF_ERROR("%s: failed to set dma mask error = %d",
				__func__, ret);
		return ret;
	}

	/* Arrange for access to Target SoC registers. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	status = pfrm_devm_ioremap_resource(dev,
					    (struct qdf_vbus_resource *)memres,
					    &mem);
#else
	status = pfrm_devm_request_and_ioremap(
					dev,
					(struct qdf_vbus_resource *)memres,
					&mem);
#endif
	if (QDF_IS_STATUS_ERROR(status)) {
		HIF_INFO("ath: ioremap error\n");
		ret = PTR_ERR(mem);
		goto err_cleanup1;
	}

	sc->mem = mem;
	ol_sc->mem = mem;
	ol_sc->mem_pa = memres->start;

	tgt_info = hif_get_target_info_handle((struct hif_opaque_softc *)ol_sc);

	tgt_info->target_type = target_type;
	hif_register_tbl_attach(ol_sc, hif_type);
	hif_target_register_tbl_attach(ol_sc, target_type);

	/* QCA_WIFI_QCA8074_VP:Should not be executed on 8074 VP platform */
	if ((tgt_info->target_type != TARGET_TYPE_QCA8074) &&
	    (tgt_info->target_type != TARGET_TYPE_QCA8074V2) &&
	    (tgt_info->target_type != TARGET_TYPE_QCA6018)) {
		if (hif_ahb_enable_radio(sc, pdev, id) != 0) {
			HIF_INFO("error in enabling soc\n");
			return -EIO;
		}

		if (hif_target_sync_ahb(ol_sc) < 0) {
			ret = -EIO;
			goto err_target_sync;
		}
	}
	HIF_TRACE("%s: X - hif_type = 0x%x, target_type = 0x%x",
			__func__, hif_type, target_type);

	return QDF_STATUS_SUCCESS;
err_target_sync:
	/* QCA_WIFI_QCA8074_VP:Should not be executed on 8074 VP platform */
	if ((tgt_info->target_type != TARGET_TYPE_QCA8074) &&
	    (tgt_info->target_type != TARGET_TYPE_QCA8074V2) &&
	    (tgt_info->target_type != TARGET_TYPE_QCA6018)) {
		HIF_INFO("Error: Disabling target\n");
		hif_ahb_disable_bus(ol_sc);
	}
err_cleanup1:
	return ret;
}


/**
 * hif_reset_soc() - reset soc
 *
 * @hif_ctx: HIF context
 *
 * This function resets soc and helds the
 * target in reset state
 *
 * Return: void
 */
/* Function to reset SoC */
void hif_ahb_reset_soc(struct hif_softc *hif_ctx)
{
	hif_ahb_device_reset(hif_ctx);
}


/**
 * hif_nointrs() - disable IRQ
 *
 * @scn: struct hif_softc
 *
 * This function stops interrupt(s)
 *
 * Return: none
 */
void hif_ahb_nointrs(struct hif_softc *scn)
{
	int i;
	struct hif_pci_softc *sc = HIF_GET_PCI_SOFTC(scn);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct CE_attr *host_ce_conf = hif_state->host_ce_config;

	ce_unregister_irq(hif_state, CE_ALL_BITMAP);

	if (scn->request_irq_done == false)
		return;

	if (sc->num_msi_intrs > 0) {
		/* MSI interrupt(s) */
		for (i = 0; i < sc->num_msi_intrs; i++) {
			pfrm_free_irq(scn->qdf_dev->dev, sc->irq + i, sc);
		}
		sc->num_msi_intrs = 0;
	} else {
		if (!scn->per_ce_irq) {
			pfrm_free_irq(scn->qdf_dev->dev, sc->irq, sc);
		} else {
			for (i = 0; i < scn->ce_count; i++) {
				if (host_ce_conf[i].flags
						& CE_ATTR_DISABLE_INTR)
					continue;

				pfrm_free_irq(
					scn->qdf_dev->dev,
					ic_irqnum[HIF_IC_CE0_IRQ_OFFSET + i],
					&hif_state->tasklets[i]);
			}
			hif_ahb_deconfigure_grp_irq(scn);
		}
	}
	scn->request_irq_done = false;

}

/**
 * ce_irq_enable() - enable copy engine IRQ
 * @scn: struct hif_softc
 * @ce_id: ce_id
 *
 * This function enables the interrupt for the radio.
 *
 * Return: N/A
 */
void hif_ahb_irq_enable(struct hif_softc *scn, int ce_id)
{
	uint32_t regval;
	uint32_t reg_offset = 0;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct CE_pipe_config *target_ce_conf = &hif_state->target_ce_config[ce_id];
	struct hif_target_info *tgt_info = &scn->target_info;

	if (scn->per_ce_irq) {
		if (target_ce_conf->pipedir & PIPEDIR_OUT) {
			reg_offset = HOST_IE_ADDRESS;
			qdf_spin_lock_irqsave(&hif_state->irq_reg_lock);
			regval = hif_read32_mb(scn, scn->mem + reg_offset);
			regval |= HOST_IE_REG1_CE_BIT(ce_id);
			hif_write32_mb(scn, scn->mem + reg_offset, regval);
			qdf_spin_unlock_irqrestore(&hif_state->irq_reg_lock);
		}
		if (target_ce_conf->pipedir & PIPEDIR_IN) {
			reg_offset = HOST_IE_ADDRESS_2;
			qdf_spin_lock_irqsave(&hif_state->irq_reg_lock);
			regval = hif_read32_mb(scn, scn->mem + reg_offset);
			regval |= HOST_IE_REG2_CE_BIT(ce_id);
			hif_write32_mb(scn, scn->mem + reg_offset, regval);
			if (tgt_info->target_type == TARGET_TYPE_QCA8074 ||
			    tgt_info->target_type == TARGET_TYPE_QCA8074V2 ||
			    tgt_info->target_type == TARGET_TYPE_QCA6018) {
				/* Enable destination ring interrupts for
				 * 8074, 8074V2 and 6018
				 */
				regval = hif_read32_mb(scn, scn->mem +
					HOST_IE_ADDRESS_3);
				regval |= HOST_IE_REG3_CE_BIT(ce_id);

				hif_write32_mb(scn, scn->mem +
					       HOST_IE_ADDRESS_3, regval);
			}
			qdf_spin_unlock_irqrestore(&hif_state->irq_reg_lock);
		}
	} else {
		hif_pci_irq_enable(scn, ce_id);
	}
}

/**
 * ce_irq_disable() - disable copy engine IRQ
 * @scn: struct hif_softc
 * @ce_id: ce_id
 *
 * Return: N/A
 */
void hif_ahb_irq_disable(struct hif_softc *scn, int ce_id)
{
	uint32_t regval;
	uint32_t reg_offset = 0;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct CE_pipe_config *target_ce_conf = &hif_state->target_ce_config[ce_id];
	struct hif_target_info *tgt_info = &scn->target_info;

	if (scn->per_ce_irq) {
		if (target_ce_conf->pipedir & PIPEDIR_OUT) {
			reg_offset = HOST_IE_ADDRESS;
			qdf_spin_lock_irqsave(&hif_state->irq_reg_lock);
			regval = hif_read32_mb(scn, scn->mem + reg_offset);
			regval &= ~HOST_IE_REG1_CE_BIT(ce_id);
			hif_write32_mb(scn, scn->mem + reg_offset, regval);
			qdf_spin_unlock_irqrestore(&hif_state->irq_reg_lock);
		}
		if (target_ce_conf->pipedir & PIPEDIR_IN) {
			reg_offset = HOST_IE_ADDRESS_2;
			qdf_spin_lock_irqsave(&hif_state->irq_reg_lock);
			regval = hif_read32_mb(scn, scn->mem + reg_offset);
			regval &= ~HOST_IE_REG2_CE_BIT(ce_id);
			hif_write32_mb(scn, scn->mem + reg_offset, regval);
			if (tgt_info->target_type == TARGET_TYPE_QCA8074 ||
			    tgt_info->target_type == TARGET_TYPE_QCA8074V2 ||
			    tgt_info->target_type == TARGET_TYPE_QCA6018) {
				/* Disable destination ring interrupts for
				 * 8074, 8074V2 and 6018
				 */
				regval = hif_read32_mb(scn, scn->mem +
					HOST_IE_ADDRESS_3);
				regval &= ~HOST_IE_REG3_CE_BIT(ce_id);

				hif_write32_mb(scn, scn->mem +
					       HOST_IE_ADDRESS_3, regval);
			}
			qdf_spin_unlock_irqrestore(&hif_state->irq_reg_lock);
		}
	}
}

void hif_ahb_exec_grp_irq_disable(struct hif_exec_context *hif_ext_group)
{
	int i;

	qdf_spin_lock_irqsave(&hif_ext_group->irq_lock);
	if (hif_ext_group->irq_enabled) {
		for (i = 0; i < hif_ext_group->numirq; i++) {
			disable_irq_nosync(hif_ext_group->os_irq[i]);
		}
		hif_ext_group->irq_enabled = false;
	}
	qdf_spin_unlock_irqrestore(&hif_ext_group->irq_lock);
}

void hif_ahb_exec_grp_irq_enable(struct hif_exec_context *hif_ext_group)
{
	int i;

	qdf_spin_lock_irqsave(&hif_ext_group->irq_lock);
	if (hif_ext_group->irq_requested && !hif_ext_group->irq_enabled) {
		for (i = 0; i < hif_ext_group->numirq; i++) {
			enable_irq(hif_ext_group->os_irq[i]);
		}
		hif_ext_group->irq_enabled = true;
	}
	qdf_spin_unlock_irqrestore(&hif_ext_group->irq_lock);
}

/**
 * hif_ahb_needs_bmi() - return true if the soc needs bmi through the driver
 * @scn: hif context
 *
 * Return: true if soc needs driver bmi otherwise false
 */
bool hif_ahb_needs_bmi(struct hif_softc *scn)
{
	return !ce_srng_based(scn);
}

void hif_ahb_display_stats(struct hif_softc *scn)
{
	if (!scn) {
		HIF_ERROR("%s, hif_scn null", __func__);
		return;
	}
	hif_display_ce_stats(scn);
}

void hif_ahb_clear_stats(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);

	if (!hif_state) {
		HIF_ERROR("%s, hif_state null", __func__);
		return;
	}
	hif_clear_ce_stats(hif_state);
}
