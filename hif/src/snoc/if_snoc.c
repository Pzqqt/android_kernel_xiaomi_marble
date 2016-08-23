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
#include "snoc_api.h"
#include <soc/qcom/icnss.h>
#include "pld_common.h"
#include "qdf_util.h"

/**
 * hif_disable_isr(): disable isr
 *
 * This function disables isr and kills tasklets
 *
 * @hif_ctx: struct hif_softc
 *
 * Return: void
 */
void hif_snoc_disable_isr(struct hif_softc *scn)
{
	hif_nointrs(scn);
	ce_tasklet_kill(scn);
	qdf_atomic_set(&scn->active_tasklet_cnt, 0);
}

/**
 * hif_dump_registers(): dump bus debug registers
 * @scn: struct hif_opaque_softc
 *
 * This function dumps hif bus debug registers
  *
 * Return: 0 for success or error code
 */
int hif_snoc_dump_registers(struct hif_softc *hif_ctx)
{
	int status;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	status = hif_dump_ce_registers(scn);
	if (status)
		HIF_ERROR("%s: Dump CE Registers Failed", __func__);

	return 0;
}

void hif_snoc_display_stats(struct hif_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);

	if (hif_state == NULL) {
		HIF_ERROR("%s, hif_ctx null", __func__);
		return;
	}
	hif_display_ce_stats(hif_state);
}

void hif_snoc_clear_stats(struct hif_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);

	if (hif_state == NULL) {
		HIF_ERROR("%s, hif_ctx null", __func__);
		return;
	}
	hif_clear_ce_stats(hif_state);
}

/**
 * hif_snoc_close(): hif_bus_close
 *
 * Return: n/a
 */
void hif_snoc_close(struct hif_softc *scn)
{
	hif_ce_close(scn);
}

/**
 * hif_bus_open(): hif_bus_open
 * @hif_ctx: hif context
 * @bus_type: bus type
 *
 * Return: n/a
 */
QDF_STATUS hif_snoc_open(struct hif_softc *hif_ctx, enum qdf_bus_type bus_type)
{
	return hif_ce_open(hif_ctx);
}

/**
 * hif_snoc_get_soc_info() - populates scn with hw info
 *
 * fills in the virtual and physical base address as well as
 * soc version info.
 *
 * return 0 or QDF_STATUS_E_FAILURE
 */
static QDF_STATUS hif_snoc_get_soc_info(struct hif_softc *scn)
{
	int ret;
	struct pld_soc_info soc_info;

	qdf_mem_zero(&soc_info, sizeof(soc_info));

	ret = pld_get_soc_info(scn->qdf_dev->dev, &soc_info);
	if (ret < 0) {
		HIF_ERROR("%s: pld_get_soc_info error = %d", __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	scn->mem = soc_info.v_addr;
	scn->mem_pa = soc_info.p_addr;
	return QDF_STATUS_SUCCESS;
}

/**
 * hif_bus_configure() - configure the snoc bus
 * @scn: pointer to the hif context.
 *
 * return: 0 for success. nonzero for failure.
 */
int hif_snoc_bus_configure(struct hif_softc *scn)
{
	int ret;

	ret = hif_snoc_get_soc_info(scn);
	if (ret)
		return ret;

	hif_ce_prepare_config(scn);

	ret = hif_wlan_enable(scn);
	if (ret) {
		HIF_ERROR("%s: hif_wlan_enable error = %d",
				__func__, ret);
		return ret;
	}

	ret = hif_config_ce(scn);
	if (ret)
		hif_wlan_disable(scn);
	return ret;
}

/**
 * hif_snoc_get_target_type(): Get the target type
 *
 * This function is used to query the target type.
 *
 * @ol_sc: hif_softc struct pointer
 * @dev: device pointer
 * @bdev: bus dev pointer
 * @bid: bus id pointer
 * @hif_type: HIF type such as HIF_TYPE_QCA6180
 * @target_type: target type such as TARGET_TYPE_QCA6180
 *
 * Return: 0 for success
 */
static inline int hif_snoc_get_target_type(struct hif_softc *ol_sc,
	struct device *dev, void *bdev, const hif_bus_id *bid,
	uint32_t *hif_type, uint32_t *target_type)
{
	/* TODO: need to use HW version. Hard code for now */
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
 * Return: QDF_STATUS
 */
QDF_STATUS hif_snoc_enable_bus(struct hif_softc *ol_sc,
			  struct device *dev, void *bdev,
			  const hif_bus_id *bid,
			  enum hif_enable_type type)
{
	int ret;
	int hif_type;
	int target_type;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0)
	ret = dma_set_mask_and_coherent(dev, DMA_BIT_MASK(37));
#else
	ret = dma_set_coherent_mask(dev, DMA_BIT_MASK(37));
#endif
	if (ret) {
		HIF_ERROR("%s: failed to set dma mask error = %d",
				__func__, ret);
		return ret;
	}

	ret = qdf_device_init_wakeup(ol_sc->qdf_dev, true);
	if (ret == -EEXIST)
		HIF_WARN("%s: device_init_wakeup already done",
				__func__);
	else if (ret) {
		HIF_ERROR("%s: device_init_wakeup: err= %d",
				__func__, ret);
		return ret;
	}

	if (!ol_sc) {
		HIF_ERROR("%s: hif_ctx is NULL", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	ret = hif_snoc_get_target_type(ol_sc, dev, bdev, bid,
			&hif_type, &target_type);
	if (ret < 0) {
		HIF_ERROR("%s: invalid device id/revision_id", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ol_sc->target_info.target_type = target_type;

	hif_register_tbl_attach(ol_sc, hif_type);
	hif_target_register_tbl_attach(ol_sc, target_type);

	HIF_TRACE("%s: X - hif_type = 0x%x, target_type = 0x%x",
		  __func__, hif_type, target_type);

	return QDF_STATUS_SUCCESS;
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
void hif_snoc_disable_bus(struct hif_softc *scn)
{
	int ret = qdf_device_init_wakeup(scn->qdf_dev, false);

	if (ret)
		HIF_ERROR("%s: device_init_wakeup: err %d", __func__, ret);
}

/**
 * hif_nointrs(): disable IRQ
 *
 * This function stops interrupt(s)
 *
 * @scn: struct hif_softc
 *
 * Return: none
 */
void hif_snoc_nointrs(struct hif_softc *scn)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	if (scn->request_irq_done) {
		ce_unregister_irq(hif_state, 0xfff);
		scn->request_irq_done = false;
	}
}

/**
 * ce_irq_enable() - enable copy engine IRQ
 * @scn: struct hif_softc
 * @ce_id: ce_id
 *
 * Return: N/A
 */
void hif_snoc_irq_enable(struct hif_softc *scn,
		int ce_id)
{
	ce_enable_irq_in_individual_register(scn, ce_id);
}

/**
 * ce_irq_disable() - disable copy engine IRQ
 * @scn: struct hif_softc
 * @ce_id: ce_id
 *
 * Return: N/A
 */
void hif_snoc_irq_disable(struct hif_softc *scn, int ce_id)
{
	ce_disable_irq_in_individual_register(scn, ce_id);
}

/*
 * hif_snoc_setup_wakeup_sources() - enable/disable irq wake on correct irqs
 * @hif_softc: hif context
 *
 * Firmware will send a wakeup request to the HTC_CTRL_RSVD_SVC when waking up
 * the host driver. Ensure that the copy complete interrupt from this copy
 * engine can wake up the apps processor.
 *
 * Return: 0 for success
 */
QDF_STATUS hif_snoc_setup_wakeup_sources(struct hif_softc *scn, bool enable)
{
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);
	uint8_t ul_pipe, dl_pipe;
	int ul_is_polled, dl_is_polled;
	int irq_to_wake_on;

	QDF_STATUS status;
	int ret;

	status = hif_map_service_to_pipe(hif_hdl, HTC_CTRL_RSVD_SVC,
					 &ul_pipe, &dl_pipe,
					 &ul_is_polled, &dl_is_polled);
	if (status) {
		HIF_ERROR("%s: pipe_mapping failure", __func__);
		return status;
	}

	irq_to_wake_on = icnss_get_irq(dl_pipe);
	if (irq_to_wake_on < 0) {
		HIF_ERROR("%s: failed to map ce to irq", __func__);
		return QDF_STATUS_E_RESOURCES;
	}

	if (enable)
		ret = enable_irq_wake(irq_to_wake_on);
	else
		ret = disable_irq_wake(irq_to_wake_on);

	if (ret) {
		HIF_ERROR("%s: Fail to setup wake IRQ!", __func__);
		return QDF_STATUS_E_RESOURCES;
	}

	HIF_ERROR("%s: expecting wake from ce %d, irq %d",
		  __func__, dl_pipe, irq_to_wake_on);
	return QDF_STATUS_SUCCESS;
}

/**
 * __hif_snoc_irq_state_vote() - enable/disable all the ce interrupts
 * @scn: hif context pointer
 * @enble: true to enable the interrupts false to disable
 *
 * takes care of unwinding partial successes.
 *
 * Return: QDF_STATUS_SUCCESS on success
 */

static QDF_STATUS __hif_snoc_irq_state_vote(struct hif_softc *scn, bool enable)
{
	int num_ce = scn->ce_count;
	int ce_id = 0;
	int irq;

	while (ce_id < num_ce) {
		irq = icnss_get_irq(ce_id);
		if (irq < 0)
			goto error;

		if (enable)
			enable_irq(irq);
		else
			disable_irq(irq);

		ce_id++;
	}

	return QDF_STATUS_SUCCESS;

error:
	HIF_ERROR("%s: failed to map ce to irq", __func__);

	while (--ce_id >= 0) {
		irq = icnss_get_irq(ce_id);
		if (irq < 0)
			continue;

		if (enable)
			disable_irq(irq);
		else
			enable_irq(irq);
	}
	return QDF_STATUS_E_FAULT;
}

/**
 * hif_snoc_enable_irqs() - enable the ce irqs
 * @scn: hif context
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS hif_snoc_enable_irqs(struct hif_softc *scn)
{
	return __hif_snoc_irq_state_vote(scn, true);
}

/**
 * hif_snoc_enable_irqs() - enable the ce irqs
 * @scn: hif context
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static QDF_STATUS hif_snoc_disable_irqs(struct hif_softc *scn)
{
	return __hif_snoc_irq_state_vote(scn, false);
}

/**
 * hif_snoc_bus_suspend() - prepare to suspend the bus
 * @scn: hif context
 *
 * Setup wakeup interrupt configuration.
 * Disable CE interrupts (wakeup interrupt will still wake apps)
 * Drain tasklets. - make sure that we don't suspend while processing
 * the wakeup message.
 *
 * Return: 0 on success.
 */
int hif_snoc_bus_suspend(struct hif_softc *scn)
{
	if (hif_snoc_setup_wakeup_sources(scn, true) != QDF_STATUS_SUCCESS)
		goto error;

	if (hif_snoc_disable_irqs(scn) != QDF_STATUS_SUCCESS)
		goto wakeup_sources;

	if (hif_drain_tasklets(scn) != 0)
		goto enable_irqs;
	return 0;

enable_irqs:
	if (hif_snoc_enable_irqs(scn) != QDF_STATUS_SUCCESS)
		QDF_BUG(0);
wakeup_sources:
	if (hif_snoc_setup_wakeup_sources(scn, false) != QDF_STATUS_SUCCESS)
		QDF_BUG(0);
error:
	return -EFAULT;

}

/**
 * hif_snoc_bus_resume() - snoc bus resume function
 * @scn: hif context
 *
 * Clear wakeup interrupt configuration.
 * Reenable ce interrupts
 *
 * Return: 0 on success
 */
int hif_snoc_bus_resume(struct hif_softc *scn)
{
	if (hif_snoc_setup_wakeup_sources(scn, false) != QDF_STATUS_SUCCESS)
		QDF_BUG(0);

	if (hif_snoc_enable_irqs(scn) != QDF_STATUS_SUCCESS)
		QDF_BUG(0);

	return 0;
}
