/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

#include <osdep.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sd.h>
#include <linux/wait.h>
#include <qdf_mem.h>
#include "if_sdio.h"
#include <qdf_trace.h>
#include <cds_api.h>
#include "regtable_sdio.h"
#include <hif_debug.h>
#include "target_type.h"
#include "epping_main.h"
#include "targaddrs.h"
#include "sdio_api.h"
#include <hif_sdio_dev.h>
#ifndef REMOVE_PKT_LOG
#include "ol_txrx_types.h"
#include "pktlog_ac_api.h"
#include "pktlog_ac.h"
#endif
#ifndef ATH_BUS_PM
#ifdef CONFIG_PM
#define ATH_BUS_PM
#endif /* CONFIG_PM */
#endif /* ATH_BUS_PM */

#ifndef REMOVE_PKT_LOG
struct ol_pl_os_dep_funcs *g_ol_pl_os_dep_funcs;
#endif
#define HIF_SDIO_LOAD_TIMEOUT 1000

/**
 * hif_sdio_bus_suspend() - suspend the bus
 *
 * This function suspends the bus, but sdio doesn't need to suspend.
 * Therefore do nothing.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_sdio_bus_suspend(struct hif_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;
	struct device *dev = &hif_device->func->dev;

	return hif_device_suspend(hif_ctx, dev);
}


/**
 * hif_sdio_bus_resume() - hif resume API
 *
 * This function resumes the bus. but sdio doesn't need to resume.
 * Therefore do nothing.
 *
 * Return: 0 for success and non-zero for failure
 */
int hif_sdio_bus_resume(struct hif_softc *hif_ctx)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;
	struct device *dev = &hif_device->func->dev;

	hif_device_resume(hif_ctx, dev);
	return 0;
}

/**
 * hif_enable_power_gating() - enable HW power gating
 *
 * Return: n/a
 */
void hif_enable_power_gating(void *hif_ctx)
{
}

/**
 * hif_sdio_close() - hif_bus_close
 *
 * Return: None
 */
void hif_sdio_close(struct hif_softc *hif_sc)
{
}

/**
 * hif_sdio_open() - hif_bus_open
 * @hif_sc: hif context
 * @bus_type: bus type
 *
 * Return: QDF status
 */
QDF_STATUS hif_sdio_open(struct hif_softc *hif_sc,
				   enum qdf_bus_type bus_type)
{
	hif_sc->bus_type = bus_type;

	return QDF_STATUS_SUCCESS;
}

void hif_get_target_revision(struct hif_softc *ol_sc)
{
	struct hif_softc *ol_sc_local = (struct hif_softc *)ol_sc;
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(ol_sc_local);
	uint32_t chip_id = 0;
	QDF_STATUS rv;

	rv = hif_diag_read_access(hif_hdl,
			(CHIP_ID_ADDRESS | RTC_SOC_BASE_ADDRESS), &chip_id);
	if (rv != QDF_STATUS_SUCCESS) {
		HIF_ERROR("%s[%d]: get chip id fail\n", __func__, __LINE__);
	} else {
		ol_sc_local->target_info.target_revision =
			CHIP_ID_REVISION_GET(chip_id);
	}
}

/**
 * hif_sdio_enable_bus() - hif_enable_bus
 * @hif_sc: hif context
 * @dev: dev
 * @bdev: bus dev
 * @bid: bus id
 * @type: bus type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_sdio_enable_bus(struct hif_softc *ol_sc, struct device *dev,
			       void *bdev, const struct hif_bus_id *bid,
			       enum hif_enable_type type)
{
	int ret = 0;
	const struct sdio_device_id *id = (const struct sdio_device_id *)bid;

	if (hif_sdio_device_inserted(ol_sc, dev, id)) {
		HIF_ERROR("wlan: %s hif_sdio_device_inserted failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	return ret;
}


/**
 * hif_sdio_disable_bus() - sdio disable bus
 * @hif_sc: hif softc pointer
 *
 * Return: none
 */
void hif_sdio_disable_bus(struct hif_softc *hif_sc)
{
	struct sdio_func *func = dev_to_sdio_func(hif_sc->qdf_dev->dev);

	hif_sdio_device_removed(func);
}

/**
 * hif_sdio_get_config_item - sdio configure bus
 * @hif_sc: hif context
 * @opcode: configuration type
 * @config: configuration value to set
 * @config_len: configuration length
 *
 * Return: QDF_STATUS_SUCCESS for success
 */
QDF_STATUS hif_sdio_get_config_item(struct hif_softc *hif_sc,
		     int opcode, void *config, uint32_t config_len)
{
	struct hif_sdio_softc *sc = HIF_GET_SDIO_SOFTC(hif_sc);
	struct hif_sdio_dev *hif_device = sc->hif_handle;

	return hif_configure_device(hif_sc, hif_device, opcode,
				    config, config_len);
}

/**
 * hif_sdio_set_mailbox_swap - set mailbox swap
 * @hif_sc: hif context
 *
 * Return: None
 */
void hif_sdio_set_mailbox_swap(struct hif_softc *hif_sc)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_sc);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	hif_dev_set_mailbox_swap(hif_device);
}

/**
 * hif_sdio_claim_device - set mailbox swap
 * @hif_sc: hif context
 *
 * Return: None
 */
void hif_sdio_claim_device(struct hif_softc *hif_sc)
{
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_sc);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	hif_device->claimed_ctx = hif_sc;
}

/**
 * hif_sdio_mask_interrupt_call() - disbale hif device irq
 * @scn: pointr to softc structure
 *
 * Return: None
 */
void hif_sdio_mask_interrupt_call(struct hif_softc *scn)
{
	struct hif_sdio_softc *hif_ctx = HIF_GET_SDIO_SOFTC(scn);
	struct hif_sdio_dev *hif_device = hif_ctx->hif_handle;

	hif_mask_interrupt(hif_device);
}

/**
 * hif_trigger_dump() - trigger various dump cmd
 * @scn: struct hif_opaque_softc
 * @cmd_id: dump command id
 * @start: start/stop dump
 *
 * Return: None
 */
void hif_trigger_dump(struct hif_opaque_softc *scn, uint8_t cmd_id, bool start)
{
}

/**
 * hif_check_fw_reg() - check fw selfrecovery indication
 * @hif_ctx: hif_opaque_softc
 *
 * Return: int
 */
int hif_check_fw_reg(struct hif_opaque_softc *hif_ctx)
{
	int ret = 1;
	uint32_t fw_indication = 0;
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);

	if (hif_diag_read_access(hif_ctx, FW_INDICATOR_ADDRESS,
				 &fw_indication) != QDF_STATUS_SUCCESS) {
		HIF_ERROR("%s Get fw indication failed\n", __func__);
		return 1;
	}
	HIF_INFO("%s: fw indication is 0x%x def 0x%x.\n", __func__,
		fw_indication, FW_IND_HELPER);
	if (fw_indication & FW_IND_HELPER)
		ret = 0;

	return ret;
}

/**
 * hif_wlan_disable() - call the platform driver to disable wlan
 * @scn: scn
 *
 * Return: void
 */
void hif_wlan_disable(struct hif_softc *scn)
{
}

/**
 * hif_sdio_needs_bmi() - return true if the soc needs bmi through the driver
 * @scn: hif context
 *
 * Return: true if soc needs driver bmi otherwise false
 */
bool hif_sdio_needs_bmi(struct hif_softc *scn)
{
	return true;
}
