/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
#include "pld_sdio.h"
#include "targaddrs.h"
#include "sdio_api.h"
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

struct hif_sdio_softc *scn;
struct hif_softc *ol_sc;
static atomic_t hif_sdio_load_state;
/* Wait queue for MC thread */
wait_queue_head_t sync_wait_queue;

/**
 * hif_sdio_probe() - configure sdio device
 * @context: sdio device context
 * @hif_handle: pointer to hif handle
 *
 * Return: 0 for success and non-zero for failure
 */
static A_STATUS hif_sdio_probe(void *context, void *hif_handle)
{
	int ret = 0;
	struct HIF_DEVICE_OS_DEVICE_INFO os_dev_info;
	struct sdio_func *func = NULL;
	const struct sdio_device_id *id;
	uint32_t target_type;

	HIF_ENTER();
	scn = (struct hif_sdio_softc *)qdf_mem_malloc(sizeof(*scn));
	if (!scn) {
		ret = -ENOMEM;
		goto err_alloc;
	}

	scn->hif_handle = hif_handle;
	hif_configure_device(hif_handle, HIF_DEVICE_GET_OS_DEVICE,
			     &os_dev_info,
			     sizeof(os_dev_info));

	scn->aps_osdev.device = os_dev_info.os_dev;
	scn->aps_osdev.bc.bc_bustype = QDF_BUS_TYPE_SDIO;
	spin_lock_init(&scn->target_lock);
	ol_sc = qdf_mem_malloc(sizeof(*ol_sc));
	if (!ol_sc) {
		ret = -ENOMEM;
		goto err_attach;
	}
	OS_MEMZERO(ol_sc, sizeof(*ol_sc));

	{
		/*
		 * Attach Target register table. This is needed early on
		 * even before BMI since PCI and HIF initialization
		 * directly access Target registers.
		 *
		 * TBDXXX: targetdef should not be global -- should be stored
		 * in per-device struct so that we can support multiple
		 * different Target types with a single Host driver.
		 * The whole notion of an "hif type" -- (not as in the hif
		 * module, but generic "Host Interface Type") is bizarre.
		 * At first, one one expect it to be things like SDIO, USB, PCI.
		 * But instead, it's an actual platform type. Inexplicably, the
		 * values used for HIF platform types are *different* from the
		 * values used for Target Types.
		 */

#if defined(CONFIG_AR9888_SUPPORT)
		hif_register_tbl_attach(ol_sc, HIF_TYPE_AR9888);
		target_register_tbl_attach(ol_sc, TARGET_TYPE_AR9888);
		target_type = TARGET_TYPE_AR9888;
#elif defined(CONFIG_AR6320_SUPPORT)
		id = ((struct hif_sdio_dev *) hif_handle)->id;
		if (((id->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
				MANUFACTURER_ID_QCA9377_BASE) ||
			((id->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
				MANUFACTURER_ID_QCA9379_BASE)) {
			hif_register_tbl_attach(ol_sc, HIF_TYPE_AR6320V2);
			target_register_tbl_attach(ol_sc, TARGET_TYPE_AR6320V2);
		} else if ((id->device & MANUFACTURER_ID_AR6K_BASE_MASK) ==
				MANUFACTURER_ID_AR6320_BASE) {
			int ar6kid = id->device & MANUFACTURER_ID_AR6K_REV_MASK;

			if (ar6kid >= 1) {
				/* v2 or higher silicon */
				hif_register_tbl_attach(ol_sc,
					HIF_TYPE_AR6320V2);
				target_register_tbl_attach(ol_sc,
					  TARGET_TYPE_AR6320V2);
			} else {
				/* legacy v1 silicon */
				hif_register_tbl_attach(ol_sc,
					HIF_TYPE_AR6320);
				target_register_tbl_attach(ol_sc,
					  TARGET_TYPE_AR6320);
			}
		}
		target_type = TARGET_TYPE_AR6320;

#endif
	}
	func = ((struct hif_sdio_dev *) hif_handle)->func;
	scn->targetdef =  ol_sc->targetdef;
	scn->hostdef =  ol_sc->hostdef;
	scn->aps_osdev.bdev = func;
	ol_sc->bus_type = scn->aps_osdev.bc.bc_bustype;
	scn->ol_sc = *ol_sc;
	ol_sc->target_info.target_type = target_type;

	scn->ramdump_base = pld_hif_sdio_get_virt_ramdump_mem(
					scn->aps_osdev.device,
					&scn->ramdump_size);
	if (scn->ramdump_base == NULL || !scn->ramdump_size) {
		QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to get RAM dump memory address or size!\n",
			__func__);
	} else {
		QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO,
			"%s: ramdump base 0x%pK size %d\n", __func__,
			scn->ramdump_base, (int)scn->ramdump_size);
	}

	if (athdiag_procfs_init(scn) != 0) {
		QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR,
			  "%s athdiag_procfs_init failed", __func__);
		ret = QDF_STATUS_E_FAILURE;
		goto err_attach1;
	}

	atomic_set(&hif_sdio_load_state, true);
	wake_up_interruptible(&sync_wait_queue);

	return 0;

err_attach1:
	if (scn->ramdump_base)
		pld_hif_sdio_release_ramdump_mem(scn->ramdump_base);
	qdf_mem_free(ol_sc);
err_attach:
	qdf_mem_free(scn);
	scn = NULL;
err_alloc:
	return ret;
}

/**
 * hif_sdio_remove() - remove sdio device
 * @conext: sdio device context
 * @hif_handle: pointer to sdio function
 *
 * Return: 0 for success and non-zero for failure
 */
static A_STATUS hif_sdio_remove(void *context, void *hif_handle)
{
	HIF_ENTER();

	if (!scn) {
		QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR,
			  "Global SDIO context is NULL");
		return A_ERROR;
	}

	atomic_set(&hif_sdio_load_state, false);
	athdiag_procfs_remove();

#ifndef TARGET_DUMP_FOR_NON_QC_PLATFORM
	iounmap(scn->ramdump_base);
#endif

	if (ol_sc) {
		qdf_mem_free(ol_sc);
		ol_sc = NULL;
	}

	if (scn) {
		qdf_mem_free(scn);
		scn = NULL;
	}

	HIF_EXIT();

	return 0;
}

/**
 * hif_sdio_suspend() - sdio suspend routine
 * @context: sdio device context
 *
 * Return: 0 for success and non-zero for failure
 */
static A_STATUS hif_sdio_suspend(void *context)
{
	return 0;
}

/**
 * hif_sdio_resume() - sdio resume routine
 * @context: sdio device context
 *
 * Return: 0 for success and non-zero for failure
 */
static A_STATUS hif_sdio_resume(void *context)
{
	return 0;
}

/**
 * hif_sdio_power_change() - change power state of sdio bus
 * @conext: sdio device context
 * @config: power state configurartion
 *
 * Return: 0 for success and non-zero for failure
 */
static A_STATUS hif_sdio_power_change(void *context, uint32_t config)
{
	return 0;
}

/*
 * Module glue.
 */
#include <linux/version.h>
static char *version = "HIF (Atheros/multi-bss)";
static char *dev_info = "ath_hif_sdio";

/**
 * init_ath_hif_sdio() - initialize hif sdio callbacks
 * @param: none
 *
 * Return: 0 for success and non-zero for failure
 */
static int init_ath_hif_sdio(void)
{
	QDF_STATUS status;
	struct osdrv_callbacks osdrv_callbacks;

	HIF_ENTER();
	qdf_mem_zero(&osdrv_callbacks, sizeof(osdrv_callbacks));
	osdrv_callbacks.device_inserted_handler = hif_sdio_probe;
	osdrv_callbacks.device_removed_handler = hif_sdio_remove;
	osdrv_callbacks.device_suspend_handler = hif_sdio_suspend;
	osdrv_callbacks.device_resume_handler = hif_sdio_resume;
	osdrv_callbacks.device_power_change_handler = hif_sdio_power_change;

	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_INFO, "%s %d", __func__,
		  __LINE__);
	status = hif_init(&osdrv_callbacks);
	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
			  "%s hif_init failed!", __func__);
		return -ENODEV;
	}
	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR,
		 "%s: %s\n", dev_info, version);

	return 0;
}

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

	hif_device_suspend(dev);
	return 0;
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

	hif_device_resume(dev);
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
	if (ol_sc) {
		qdf_mem_free(ol_sc);
		ol_sc = NULL;
	}

	if (scn) {
		qdf_mem_free(scn);
		scn = NULL;
	}
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
	QDF_STATUS status;

	hif_sc->bus_type = bus_type;
	status = init_ath_hif_sdio();

	return status;
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
QDF_STATUS hif_sdio_enable_bus(struct hif_softc *hif_sc,
		struct device *dev, void *bdev, const struct hif_bus_id *bid,
		enum hif_enable_type type)
{
	int ret = 0;
	const struct sdio_device_id *id = (const struct sdio_device_id *)bid;
	struct hif_sdio_softc *sc = HIF_GET_SDIO_SOFTC(hif_sc);

	init_waitqueue_head(&sync_wait_queue);
	if (hif_sdio_device_inserted(dev, id)) {
		HIF_ERROR("wlan: %s hif_sdio_device_inserted failed", __func__);
		return QDF_STATUS_E_NOMEM;
	}

	wait_event_interruptible_timeout(sync_wait_queue,
			  atomic_read(&hif_sdio_load_state) == true,
			  HIF_SDIO_LOAD_TIMEOUT);
	hif_sc->hostdef = ol_sc->hostdef;
	hif_sc->targetdef = ol_sc->targetdef;
	hif_sc->bus_type = ol_sc->bus_type;
	hif_sc->target_info.target_type = ol_sc->target_info.target_type;

	sc->hif_handle = scn->hif_handle;
	sc->aps_osdev.device = scn->aps_osdev.device;
	sc->aps_osdev.bc.bc_bustype = scn->aps_osdev.bc.bc_bustype;
	sc->target_lock = scn->target_lock;
	sc->targetdef = scn->targetdef;
	sc->hostdef = scn->hostdef;
	sc->aps_osdev.bdev = scn->aps_osdev.bdev;
	sc->ramdump_size = scn->ramdump_size;
	sc->ramdump_base = scn->ramdump_base;

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
	struct hif_sdio_softc *sc = HIF_GET_SDIO_SOFTC(hif_sc);
	struct sdio_func *func = sc->aps_osdev.bdev;

	hif_sdio_device_removed(func);
}

/**
 * hif_sdio_get_config_item - sdio configure bus
 * @hif_sc: hif context
 * @opcode: configuration type
 * @config: configuration value to set
 * @config_len: configuration length
 *
 * Return: QDF_STATUS_SUCCESS for sucess
 */
QDF_STATUS hif_sdio_get_config_item(struct hif_softc *hif_sc,
		     int opcode, void *config, uint32_t config_len)
{
	struct hif_sdio_softc *sc = HIF_GET_SDIO_SOFTC(hif_sc);
	struct hif_sdio_dev *hif_device = sc->hif_handle;

	return hif_configure_device(hif_device,
				opcode, config, config_len);
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

	hif_device->swap_mailbox = true;
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
