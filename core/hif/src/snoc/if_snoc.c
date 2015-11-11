/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

/**
 * DOC: if_snoc.c
 *
 * c file for snoc specif implementations.
 */

#include "hif.h"
#include "hif_main.h"
#include "hif_debug.h"
#include "hif_io32.h"
#include "ce_main.h"
#include "ce_tasklet.h"

/**
 * hif_bus_prevent_linkdown(): prevent linkdown
 *
 * Dummy function for busses and platforms that do not support
 * link down.  This may need to be replaced with a wakelock.
 *
 * This is duplicated here because CONFIG_CNSS can be defined
 * even though it is not used for the snoc bus.
 */
void hif_bus_prevent_linkdown(struct ol_softc *scn, bool flag)
{
	HIF_ERROR("wlan: %s pcie power collapse ignored",
			(flag ? "disable" : "enable"));
}

/**
 * hif_targ_is_awake(): check if target is awake
 *
 * This function returns true if the target is awake
 *
 * @scn: struct ol_softc
 * @mem: mapped mem base
 *
 * Return: bool
 */
bool hif_targ_is_awake(struct ol_softc *scn, void *__iomem *mem)
{
	return true;
}

/**
 * hif_reset_soc(): reset soc
 *
 * this function resets soc
 *
 * @hif_ctx: HIF context
 *
 * Return: void
 */
/* Function to reset SoC */
void hif_reset_soc(void *hif_ctx)
{
}

/**
 * hif_disable_isr(): disable isr
 *
 * This function disables isr and kills tasklets
 *
 * @hif_ctx: struct ol_softc
 *
 * Return: void
 */
void hif_disable_isr(void *hif_ctx)
{
	struct ol_softc *scn = (struct ol_softc *)hif_ctx;

	hif_nointrs(scn);
	ce_tasklet_kill(scn->hif_hdl);
	cdf_atomic_set(&scn->active_tasklet_cnt, 0);
}

/**
 * dump_ce_debug_register(): dump CE debug registers
 *
 * This function dumps CE debug registers
 *
 * @scn: struct ol_softc
 *
 * Return: void
 */
void dump_ce_debug_register(struct ol_softc *scn)
{
	return;
}

/**
 * hif_runtime_pm_set_state_inprogress() - dummy function
 *
 * currently runtime pm only supported in pci
 */
void hif_runtime_pm_set_state_inprogress(void)
{
}

/**
 * hif_runtime_pm_set_state_on() - dummy function
 *
 * currently runtime pm only supported in pci
 */
void hif_runtime_pm_set_state_on(void)
{
}

/**
 * hif_runtime_pm_set_state_suspended() - dummy function
 *
 * currently runtime pm only supported in pci
 */
void hif_runtime_pm_set_state_on(void)
{
}

/**
 * hif_runtime_pm_set_state_suspended() - dummy function
 *
 * currently runtime pm only supported in pci
 */
void hif_runtime_pm_set_state_suspended(void)
{
}

/**
 * hif_bus_suspend() - suspend the bus
 *
 * This function suspends the bus, but snoc doesn't need to suspend.
 * Therefore do nothing.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_bus_suspend(void)
{
	return 0;
}

/**
 * hif_bus_resume() - hif resume API
 *
 * This function resumes the bus. but snoc doesn't need to resume.
 * Therefore do nothing.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_bus_resume(void)
{
	return 0;
}

static int hif_runtime_suspend(void)
{
	return hif_bus_suspend();
}

static int hif_runtime_resume(void)
{
	return hif_bus_resume();
}


/**
 * hif_enable_power_gating(): enable HW power gating
 *
 * Return: n/a
 */
void hif_enable_power_gating(void *hif_ctx)
{
}

/**
 * hif_disable_aspm(): hif_disable_aspm
 *
 * Return: n/a
 */
void hif_disable_aspm(void)
{
}

/**
 * hif_bus_close(): hif_bus_close
 *
 * Return: n/a
 */
void hif_bus_close(struct ol_softc *scn)
{
}

/**
 * hif_bus_open(): hif_bus_open
 * @scn: scn
 * @bus_type: bus type
 *
 * Return: n/a
 */
CDF_STATUS hif_bus_open(struct ol_softc *scn, enum ath_hal_bus_type bus_type)
{
	return CDF_STATUS_SUCCESS;
}

/**
 * hif_get_target_type(): Get the target type
 *
 * This function is used to query the target type.
 *
 * @ol_sc: ol_softc struct pointer
 * @dev: device pointer
 * @bdev: bus dev pointer
 * @bid: bus id pointer
 * @hif_type: HIF type such as HIF_TYPE_QCA6180
 * @target_type: target type such as TARGET_TYPE_QCA6180
 *
 * Return: 0 for success
 */
int hif_get_target_type(struct ol_softc *ol_sc, struct device *dev,
	void *bdev, const hif_bus_id *bid, uint32_t *hif_type,
	uint32_t *target_type)
{
	/* TODO: need to use CNSS's HW version. Hard code for now */
#ifdef QCA_WIFI_3_0_ADRASTEA
	*hif_type = HIF_TYPE_ADRASTEA;
	*target_type = TARGET_TYPE_ADRASTEA;
#else
	*hif_type = 0;
	*target_type = 0;
#endif
	return 0;
}

/**
 * hif_enable_bus(): hif_enable_bus
 * @dev: dev
 * @bdev: bus dev
 * @bid: bus id
 * @type: bus type
 *
 * Return: CDF_STATUS
 */
CDF_STATUS hif_enable_bus(struct ol_softc *ol_sc,
			  struct device *dev, void *bdev,
			  const hif_bus_id *bid,
			  enum hif_enable_type type)
{
	int ret;
	int hif_type;
	int target_type;
	ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(32));
	if (ret) {
		HIF_ERROR("%s: failed to set dma mask error = %d",
				__func__, ret);
		return ret;
	}

	if (!ol_sc) {
		HIF_ERROR("%s: hif_ctx is NULL", __func__);
		return CDF_STATUS_E_NOMEM;
	}

	ol_sc->aps_osdev.device = dev;
	ol_sc->aps_osdev.bc.bc_handle = (void *)ol_sc->mem;
	ol_sc->aps_osdev.bc.bc_bustype = type;

	ret = hif_get_target_type(ol_sc, dev, bdev, bid,
			&hif_type, &target_type);
	if (ret < 0) {
		HIF_ERROR("%s: invalid device id/revision_id", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	hif_register_tbl_attach(ol_sc, hif_type);
	target_register_tbl_attach(ol_sc, target_type);

	HIF_TRACE("%s: X - hif_type = 0x%x, target_type = 0x%x",
		  __func__, hif_type, target_type);

	ret = hif_init_cdf_ctx(ol_sc);
	if (ret != 0) {
		HIF_ERROR("%s: cannot init CDF", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	return CDF_STATUS_SUCCESS;
}

/**
 * hif_disable_bus(): hif_disable_bus
 *
 * This function disables the bus
 *
 * @bdev: bus dev
 *
 * Return: none
 */
void hif_disable_bus(void *bdev)
{
}

/**
 * hif_nointrs(): disable IRQ
 *
 * This function stops interrupt(s)
 *
 * @scn: struct ol_softc
 *
 * Return: none
 */
void hif_nointrs(struct ol_softc *scn)
{
	if (scn->request_irq_done) {
		ce_unregister_irq(scn->hif_hdl, 0xfff);
		scn->request_irq_done = false;
	}
}
