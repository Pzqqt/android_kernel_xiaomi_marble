/*
 * Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
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
#include "ce_api.h"
#include "ce_internal.h"
#include "snoc_api.h"
#include "pld_common.h"
#include "qdf_util.h"
#ifdef IPA_OFFLOAD
#include <uapi/linux/msm_ipa.h>
#endif
#include "target_type.h"

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
	hif_exec_kill(&scn->osc);
	hif_nointrs(scn);
	ce_tasklet_kill(scn);
	qdf_atomic_set(&scn->active_tasklet_cnt, 0);
	qdf_atomic_set(&scn->active_grp_tasklet_cnt, 0);
}

/**
 * hif_dump_registers(): dump bus debug registers
 * @hif_ctx: struct hif_opaque_softc
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

	scn->target_info.soc_version = soc_info.soc_id;
	scn->target_info.target_version = soc_info.soc_id;
	scn->target_info.target_revision = 0;
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
	uint8_t wake_ce_id;

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
		goto wlan_disable;

	ret = hif_get_wake_ce_id(scn, &wake_ce_id);
	if (ret)
		goto unconfig_ce;

	scn->wake_irq = pld_get_irq(scn->qdf_dev->dev, wake_ce_id);

	HIF_INFO(FL("expecting wake from ce %d, irq %d"),
		 wake_ce_id, scn->wake_irq);

	return 0;

unconfig_ce:
	hif_unconfig_ce(scn);

wlan_disable:
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
	struct device *dev, void *bdev, const struct hif_bus_id *bid,
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

#ifdef IPA_OFFLOAD
static int hif_set_dma_coherent_mask(qdf_device_t osdev)
{
	uint8_t addr_bits;

	if (false == hif_get_ipa_present())
		return qdf_set_dma_coherent_mask(osdev->dev,
					DMA_COHERENT_MASK_IPA_VER_3_AND_ABOVE);

	if (hif_get_ipa_hw_type() < IPA_HW_v3_0)
		addr_bits = DMA_COHERENT_MASK_BELOW_IPA_VER_3;
	else
		addr_bits = DMA_COHERENT_MASK_IPA_VER_3_AND_ABOVE;

	return qdf_set_dma_coherent_mask(osdev->dev, addr_bits);
}
#else
static int hif_set_dma_coherent_mask(qdf_device_t osdev)
{
	return qdf_set_dma_coherent_mask(osdev->dev, 37);
}
#endif

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
			  const struct hif_bus_id *bid,
			  enum hif_enable_type type)
{
	int ret;
	int hif_type;
	int target_type;

	if (!ol_sc) {
		HIF_ERROR("%s: hif_ctx is NULL", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	ret = hif_set_dma_coherent_mask(ol_sc->qdf_dev);
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

	ret = hif_snoc_get_target_type(ol_sc, dev, bdev, bid,
			&hif_type, &target_type);
	if (ret < 0) {
		HIF_ERROR("%s: invalid device id/revision_id", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	ol_sc->target_info.target_type = target_type;

	hif_register_tbl_attach(ol_sc, hif_type);
	hif_target_register_tbl_attach(ol_sc, target_type);

	/* the bus should remain on durring suspend for snoc */
	hif_vote_link_up(GET_HIF_OPAQUE_HDL(ol_sc));

	HIF_DBG("%s: X - hif_type = 0x%x, target_type = 0x%x",
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
	int ret;

	hif_vote_link_down(GET_HIF_OPAQUE_HDL(scn));

	ret = qdf_device_init_wakeup(scn->qdf_dev, false);
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

	ce_unregister_irq(hif_state, CE_ALL_BITMAP);
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
static
QDF_STATUS hif_snoc_setup_wakeup_sources(struct hif_softc *scn, bool enable)
{
	int ret;

	if (enable)
		ret = enable_irq_wake(scn->wake_irq);
	else
		ret = disable_irq_wake(scn->wake_irq);

	if (ret) {
		HIF_ERROR("%s: Fail to setup wake IRQ!", __func__);
		return QDF_STATUS_E_RESOURCES;
	}

	return QDF_STATUS_SUCCESS;
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
		return -EFAULT;
	return 0;
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

	return 0;
}

/**
 * hif_snoc_bus_suspend_noirq() - ensure there are no pending transactions
 * @scn: hif context
 *
 * Ensure that if we received the wakeup message before the irq
 * was disabled that the message is pocessed before suspending.
 *
 * Return: -EBUSY if we fail to flush the tasklets.
 */
int hif_snoc_bus_suspend_noirq(struct hif_softc *scn)
{
	if (hif_drain_tasklets(scn) != 0)
		return -EBUSY;
	return 0;
}

int hif_snoc_map_ce_to_irq(struct hif_softc *scn, int ce_id)
{
	return pld_get_irq(scn->qdf_dev->dev, ce_id);
}

/**
 * hif_is_target_register_access_allowed(): Check target register access allow
 * @scn: HIF Context
 *
 * This function help to check whether target register access is allowed or not
 *
 * Return: true if target access is allowed else false
 */
bool hif_is_target_register_access_allowed(struct hif_softc *scn)
{
	if (hif_is_recovery_in_progress(scn))
		return hif_is_target_ready(scn);
	else
		return true;
}

/**
 * hif_snoc_needs_bmi() - return true if the soc needs bmi through the driver
 * @scn: hif context
 *
 * Return: true if soc needs driver bmi otherwise false
 */
bool hif_snoc_needs_bmi(struct hif_softc *scn)
{
	return false;
}
