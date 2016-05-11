/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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


/* this file dispatches functions to bus specific definitions */
#include "hif_debug.h"
#include "hif.h"
#include "hif_main.h"
#include "multibus.h"
#include "dummy.h"
#if defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB)
#include "ce_main.h"
#endif
#include "htc_services.h"
#include "a_types.h"
#include "dummy.h"

/**
 * hif_intialize_default_ops() - intializes default operations values
 *
 * bus specific features should assign their dummy implementations here.
 */
static void hif_intialize_default_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;

	/* must be filled in by hif_bus_open */
	bus_ops->hif_bus_close = NULL;
	/* dummy implementations */
	bus_ops->hif_display_stats =
		&hif_dummy_display_stats;
	bus_ops->hif_clear_stats =
		&hif_dummy_clear_stats;
	bus_ops->hif_set_bundle_mode = hif_dummy_set_bundle_mode;
	bus_ops->hif_bus_reset_resume = hif_dummy_bus_reset_resume;
}

#define NUM_OPS (sizeof(struct hif_bus_ops) / sizeof(void *))

/**
 * hif_verify_basic_ops() - ensure required bus apis are defined
 *
 * all bus operations must be defined to avoid crashes
 * itterate over the structure and ensure all function pointers
 * are non null.
 *
 * Return: QDF_STATUS_SUCCESS if all the operations are defined
 */
static QDF_STATUS hif_verify_basic_ops(struct hif_softc *hif_sc)
{
	struct hif_bus_ops *bus_ops = &hif_sc->bus_ops;
	void **ops_array = (void *)bus_ops;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int i;

	for (i = 0; i < NUM_OPS; i++) {
		if (!ops_array[i]) {
			HIF_ERROR("%s: function %d is null", __func__, i);
			status = QDF_STATUS_E_NOSUPPORT;
		}
	}
	return status;
}

/**
 * hif_bus_get_context_size - API to return size of the bus specific structure
 *
 * Return: sizeof of hif_pci_softc
 */
int hif_bus_get_context_size(enum qdf_bus_type bus_type)
{
	switch (bus_type) {
	case QDF_BUS_TYPE_PCI:
		return hif_pci_get_context_size();
	case QDF_BUS_TYPE_AHB:
		return hif_ahb_get_context_size();
	case QDF_BUS_TYPE_SNOC:
		return hif_snoc_get_context_size();
	case QDF_BUS_TYPE_SDIO:
		return hif_sdio_get_context_size();
	case QDF_BUS_TYPE_USB:
		return hif_usb_get_context_size();
	default:
		return 0;
	}
}

/**
 * hif_bus_open() - initialize the bus_ops and call the bus specific open
 * hif_sc: hif_context
 * bus_type: type of bus being enumerated
 *
 * Return: QDF_STATUS_SUCCESS or error
 */
QDF_STATUS hif_bus_open(struct hif_softc *hif_sc,
			enum qdf_bus_type bus_type)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;

	hif_intialize_default_ops(hif_sc);

	switch (bus_type) {
	case QDF_BUS_TYPE_PCI:
		status = hif_initialize_pci_ops(hif_sc);
		break;
	case QDF_BUS_TYPE_SNOC:
		status = hif_initialize_snoc_ops(&hif_sc->bus_ops);
		break;
	case QDF_BUS_TYPE_AHB:
		status = hif_initialize_ahb_ops(&hif_sc->bus_ops);
		break;
	case QDF_BUS_TYPE_SDIO:
		status = hif_initialize_sdio_ops(hif_sc);
	case QDF_BUS_TYPE_USB:
		status = hif_initialize_usb_ops(&hif_sc->bus_ops);
		break;
	default:
		status = QDF_STATUS_E_NOSUPPORT;
		break;
	}

	if (status != QDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: %d not supported", __func__, bus_type);
		return status;
	}

	status = hif_verify_basic_ops(hif_sc);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	return hif_sc->bus_ops.hif_bus_open(hif_sc, bus_type);
}

/**
 * hif_bus_close() - close the bus
 * @hif_sc: hif_context
 */
void hif_bus_close(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_bus_close(hif_sc);
}

/**
 * hif_bus_prevent_linkdown() - prevent linkdown
 * @hif_ctx: hif context
 * @flag: true = keep bus alive false = let bus go to sleep
 *
 * Keeps the bus awake durring suspend.
 */
void hif_bus_prevent_linkdown(struct hif_softc *hif_sc, bool flag)
{
	hif_sc->bus_ops.hif_bus_prevent_linkdown(hif_sc, flag);
}


void hif_reset_soc(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	hif_sc->bus_ops.hif_reset_soc(hif_sc);
}

int hif_bus_suspend(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	return hif_sc->bus_ops.hif_bus_suspend(hif_sc);
}

int hif_bus_resume(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	return hif_sc->bus_ops.hif_bus_resume(hif_sc);
}

int hif_target_sleep_state_adjust(struct hif_softc *hif_sc,
			      bool sleep_ok, bool wait_for_it)
{
	return hif_sc->bus_ops.hif_target_sleep_state_adjust(hif_sc,
			sleep_ok, wait_for_it);
}

void hif_disable_isr(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	hif_sc->bus_ops.hif_disable_isr(hif_sc);
}

void hif_nointrs(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_nointrs(hif_sc);
}

QDF_STATUS hif_enable_bus(struct hif_softc *hif_sc, struct device *dev,
			  void *bdev, const hif_bus_id *bid,
			  enum hif_enable_type type)
{
	return hif_sc->bus_ops.hif_enable_bus(hif_sc, dev, bdev, bid, type);
}

void hif_disable_bus(struct hif_softc *hif_sc)
{
	hif_sc->bus_ops.hif_disable_bus(hif_sc);
}

int hif_bus_configure(struct hif_softc *hif_sc)
{
	return hif_sc->bus_ops.hif_bus_configure(hif_sc);
}

QDF_STATUS hif_get_config_item(struct hif_opaque_softc *hif_ctx,
		     int opcode, void *config, uint32_t config_len)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	return hif_sc->bus_ops.hif_get_config_item(hif_sc, opcode, config,
						 config_len);
}

void hif_set_mailbox_swap(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	hif_sc->bus_ops.hif_set_mailbox_swap(hif_sc);
}

void hif_claim_device(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	hif_sc->bus_ops.hif_claim_device(hif_sc);
}

void hif_shutdown_device(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	hif_sc->bus_ops.hif_shutdown_device(hif_sc);
}

void hif_stop(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_ctx);
	hif_sc->bus_ops.hif_stop(hif_sc);
}

void hif_cancel_deferred_target_sleep(struct hif_softc *hif_sc)
{
	return hif_sc->bus_ops.hif_cancel_deferred_target_sleep(hif_sc);
}

void hif_irq_enable(struct hif_softc *hif_sc, int irq_id)
{
	hif_sc->bus_ops.hif_irq_enable(hif_sc, irq_id);
}

void hif_irq_disable(struct hif_softc *hif_sc, int irq_id)
{
	hif_sc->bus_ops.hif_irq_disable(hif_sc, irq_id);
}

int hif_dump_registers(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	return hif_sc->bus_ops.hif_dump_registers(hif_sc);
}

void hif_dump_target_memory(struct hif_opaque_softc *hif_hdl,
			    void *ramdump_base,
			    uint32_t address, uint32_t size)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	hif_sc->bus_ops.hif_dump_target_memory(hif_sc, ramdump_base,
					       address, size);
}

void hif_ipa_get_ce_resource(struct hif_opaque_softc *hif_hdl,
			     qdf_dma_addr_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	hif_sc->bus_ops.hif_ipa_get_ce_resource(hif_sc, ce_sr_base_paddr,
						ce_sr_ring_size, ce_reg_paddr);
}

void hif_mask_interrupt_call(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	hif_sc->bus_ops.hif_mask_interrupt_call(hif_sc);
}

void hif_display_bus_stats(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	hif_sc->bus_ops.hif_display_stats(hif_sc);
}

void hif_clear_bus_stats(struct hif_opaque_softc *scn)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);

	hif_sc->bus_ops.hif_clear_stats(hif_sc);
}

/**
 * hif_enable_power_management() - enable power management after driver load
 * @hif_hdl: opaque pointer to the hif context
 * is_packet_log_enabled: true if packet log is enabled
 *
 * Driver load and firmware download are done in a high performance mode.
 * Enable power management after the driver is loaded.
 * packet log can require fewer power management features to be enabled.
 */
void hif_enable_power_management(struct hif_opaque_softc *hif_hdl,
				 bool is_packet_log_enabled)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	hif_sc->bus_ops.hif_enable_power_management(hif_sc,
				    is_packet_log_enabled);
}

/**
 * hif_disable_power_management() - reset the bus power management
 * @hif_hdl: opaque pointer to the hif context
 *
 * return the power management of the bus to its default state.
 * This isn't necessarily a complete reversal of its counterpart.
 * This should be called when unloading the driver.
 */
void hif_disable_power_management(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(hif_hdl);
	hif_sc->bus_ops.hif_disable_power_management(hif_sc);
}

/**
 * hif_set_bundle_mode() - enable bundling and set default rx bundle cnt
 * @scn: pointer to hif_opaque_softc structure
 * @enabled: flag to enable/disable bundling
 * @rx_bundle_cnt: bundle count to be used for RX
 *
 * Return: none
 */
void hif_set_bundle_mode(struct hif_opaque_softc *scn, bool enabled,
				int rx_bundle_cnt)
{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);
	hif_sc->bus_ops.hif_set_bundle_mode(hif_sc, enabled, rx_bundle_cnt);
}

/**
 * hif_bus_reset_resume() - resume the bus after reset
 * @scn: struct hif_opaque_softc
 *
 * This function is called to tell the driver that USB device has been resumed
 * and it has also been reset. The driver should redo any necessary
 * initialization. This function resets WLAN SOC.
 *
 * Return: int 0 for success, non zero for failure
 */
int hif_bus_reset_resume(struct hif_opaque_softc *scn)

{
	struct hif_softc *hif_sc = HIF_GET_SOFTC(scn);
	return hif_sc->bus_ops.hif_bus_reset_resume(hif_sc);
}
