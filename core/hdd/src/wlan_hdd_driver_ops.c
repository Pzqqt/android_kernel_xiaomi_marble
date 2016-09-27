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

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

#include <linux/platform_device.h>
#include <linux/pci.h>
#include "cds_api.h"
#include "qdf_status.h"
#include "qdf_lock.h"
#include "cds_sched.h"
#include "osdep.h"
#include "hif.h"
#include "htc.h"
#include "epping_main.h"
#include "wlan_hdd_main.h"
#include "wlan_hdd_power.h"
#include "wlan_logging_sock_svc.h"
#include "wma_api.h"
#include "wlan_hdd_napi.h"
#include "cds_concurrency.h"
#include "qwlan_version.h"
#include "bmi.h"
#include "cdp_txrx_bus.h"
#include "pld_common.h"

#ifdef MODULE
#define WLAN_MODULE_NAME  module_name(THIS_MODULE)
#else
#define WLAN_MODULE_NAME  "wlan"
#endif

#define DISABLE_KRAIT_IDLE_PS_VAL      1

/*
 * In BMI Phase we are only sending small chunk (256 bytes) of the FW image at
 * a time, and wait for the completion interrupt to start the next transfer.
 * During this phase, the KRAIT is entering IDLE/StandAlone(SA) Power Save(PS).
 * The delay incurred for resuming from IDLE/SA PS is huge during driver load.
 * So prevent APPS IDLE/SA PS durint driver load for reducing interrupt latency.
 */

static inline void hdd_request_pm_qos(struct device *dev, int val)
{
	pld_request_pm_qos(dev, val);
}

static inline void hdd_remove_pm_qos(struct device *dev)
{
	pld_remove_pm_qos(dev);
}

/**
 * hdd_set_recovery_in_progress() - API to set recovery in progress
 * @data: Context
 * @val: Value to set
 *
 * Return: None
 */
static void hdd_set_recovery_in_progress(void *data, uint8_t val)
{
	cds_set_recovery_in_progress(val);
}

/**
 * hdd_is_driver_unloading() - API to query if driver is unloading
 * @data: Private Data
 *
 * Return: True/False
 */
static bool hdd_is_driver_unloading(void *data)
{
	return cds_is_driver_unloading();
}

/**
 * hdd_is_load_or_unload_in_progress() - API to query if driver is
 * loading/unloading
 * @data: Private Data
 *
 * Return: bool
 */
static bool hdd_is_load_or_unload_in_progress(void *data)
{
	return cds_is_load_or_unload_in_progress();
}

/**
 * hdd_is_recovery_in_progress() - API to query if recovery in progress
 * @data: Private Data
 *
 * Return: bool
 */
static bool hdd_is_recovery_in_progress(void *data)
{
	return cds_is_driver_recovering();
}

/**
 * hdd_hif_init_driver_state_callbacks() - API to initialize HIF callbacks
 * @data: Private Data
 * @cbk: HIF Driver State callbacks
 *
 * HIF should be independent of CDS calls. Pass CDS Callbacks to HIF, HIF will
 * call the callbacks.
 *
 * Return: void
 */
static void hdd_hif_init_driver_state_callbacks(void *data,
			struct hif_driver_state_callbacks *cbk)
{
	cbk->context = data;
	cbk->set_recovery_in_progress = hdd_set_recovery_in_progress;
	cbk->is_recovery_in_progress = hdd_is_recovery_in_progress;
	cbk->is_load_unload_in_progress = hdd_is_load_or_unload_in_progress;
	cbk->is_driver_unloading = hdd_is_driver_unloading;
}

/**
 * hdd_init_cds_hif_context() - API to set CDS HIF Context
 * @hif: HIF Context
 *
 * Return: success/failure
 */
static int hdd_init_cds_hif_context(void *hif)
{
	QDF_STATUS status;

	status = cds_set_context(QDF_MODULE_ID_HIF, hif);

	if (status)
		return -ENOENT;

	return 0;
}

/**
 * hdd_deinit_cds_hif_context() - API to clear CDS HIF COntext
 *
 * Return: None
 */
static void hdd_deinit_cds_hif_context(void)
{
	QDF_STATUS status;

	status = cds_set_context(QDF_MODULE_ID_HIF, NULL);

	if (status)
		hdd_err("Failed to reset CDS HIF Context");

	return;
}

/**
 * to_bus_type() - Map PLD bus type to low level bus type
 * @bus_type: PLD bus type
 *
 * Map PLD bus type to low level bus type.
 *
 * Return: low level bus type.
 */
static enum qdf_bus_type to_bus_type(enum pld_bus_type bus_type)
{
	switch (bus_type) {
	case PLD_BUS_TYPE_PCIE:
		return QDF_BUS_TYPE_PCI;
	case PLD_BUS_TYPE_SNOC:
		return QDF_BUS_TYPE_SNOC;
	case PLD_BUS_TYPE_SDIO:
		return QDF_BUS_TYPE_SDIO;
	case PLD_BUS_TYPE_USB:
		return QDF_BUS_TYPE_USB;
	default:
		return QDF_BUS_TYPE_NONE;
	}
}

/**
 * hdd_hif_open() - HIF open helper
 * @dev: wlan device structure
 * @bdev: bus device structure
 * @bid: bus identifier for shared busses
 * @bus_type: underlying bus type
 * @reinit: true if we are reinitializing the driver during recovery phase
 *
 * This function brings-up HIF layer during load/recovery phase.
 *
 * Return: 0 on success and errno on failure.
 */
int hdd_hif_open(struct device *dev, void *bdev, const hif_bus_id *bid,
			enum qdf_bus_type bus_type, bool reinit)
{
	QDF_STATUS status;
	int ret = 0;
	struct hif_opaque_softc *hif_ctx;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	struct hif_driver_state_callbacks cbk;
	uint32_t mode = cds_get_conparam();
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (!hdd_ctx) {
		hdd_err("hdd_ctx error");
		return -EFAULT;
	}

	hdd_hif_init_driver_state_callbacks(dev, &cbk);

	hif_ctx = hif_open(qdf_ctx, mode, bus_type, &cbk);
	if (!hif_ctx) {
		hdd_err("hif_open error");
		return -ENOMEM;
	}

	ret = hdd_init_cds_hif_context(hif_ctx);
	if (ret) {
		hdd_err("Failed to set global HIF CDS Context err:%d", ret);
		goto err_hif_close;
	}

	status = hif_enable(hif_ctx, dev, bdev, bid, bus_type,
			    (reinit == true) ?  HIF_ENABLE_TYPE_REINIT :
			    HIF_ENABLE_TYPE_PROBE);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("hif_enable error = %d, reinit = %d",
			status, reinit);
		ret = qdf_status_to_os_return(status);
		goto err_hif_close;
	} else {
		ret = hdd_napi_create();
		hdd_info("hdd_napi_create returned: %d", ret);
		if (ret == 0)
			hdd_warn("NAPI: no instances are created");
		else if (ret < 0) {
			hdd_err("NAPI creation error, rc: 0x%x, reinit = %d",
				ret, reinit);
			ret = -EFAULT;
			goto err_hif_close;
		} else {
			hdd_napi_event(NAPI_EVT_INI_FILE,
				(void *)hdd_ctx->napi_enable);
		}
	}

	hif_enable_power_management(hif_ctx, cds_is_packet_log_enabled());

	return 0;

err_hif_close:
	hdd_deinit_cds_hif_context();
	hif_close(hif_ctx);
	return ret;
}

/**
 * hdd_hif_close() - HIF close helper
 * @hif_ctx:	HIF context
 *
 * Helper function to close HIF
 */
void hdd_hif_close(void *hif_ctx)
{
	if (hif_ctx == NULL)
		return;

	hif_disable_power_management(hif_ctx);

	hif_disable(hif_ctx, HIF_DISABLE_TYPE_REMOVE);

	hdd_napi_destroy(true);

	hdd_deinit_cds_hif_context();
	hif_close(hif_ctx);
}

/**
 * hdd_init_qdf_ctx() - API to initialize global QDF Device structure
 * @dev: Device Pointer
 * @bdev: Bus Device pointer
 * @bus_type: Underlying bus type
 * @bid: Bus id passed by platform driver
 *
 * Return: void
 */
void hdd_init_qdf_ctx(struct device *dev, void *bdev,
		      enum qdf_bus_type bus_type, const struct hif_bus_id *bid)
{
	qdf_device_t qdf_dev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_dev) {
		hdd_err("Invalid QDF device");
		return;
	}

	qdf_dev->dev = dev;
	qdf_dev->drv_hdl = bdev;
	qdf_dev->bus_type = bus_type;
	qdf_dev->bid = bid;
}

/**
 * wlan_hdd_probe() - handles probe request
 *
 * This function is called to probe the wlan driver
 *
 * @dev: wlan device structure
 * @bdev: bus device structure
 * @bid: bus identifier for shared busses
 * @bus_type: underlying bus type
 * @reinit: true if we are reinitiallizing the driver after a subsystem restart
 *
 * Return: 0 on successfull probe
 */
static int wlan_hdd_probe(struct device *dev, void *bdev, const hif_bus_id *bid,
	enum qdf_bus_type bus_type, bool reinit)
{
	int ret = 0;

	pr_info("%s: %sprobing driver v%s\n", WLAN_MODULE_NAME,
		reinit ? "re-" : "", QWLAN_VERSIONSTR);

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT);

	/*
	* The Krait is going to Idle/Stand Alone Power Save
	* more aggressively which is resulting in the longer driver load time.
	* The Fix is to not allow Krait to enter Idle Power Save during driver
	* load.
	*/
	hdd_request_pm_qos(dev, DISABLE_KRAIT_IDLE_PS_VAL);

	if (reinit) {
		cds_set_recovery_in_progress(true);
	} else {
		ret = hdd_init();

		if (ret)
			goto out;
		cds_set_load_in_progress(true);
	}

	hdd_init_qdf_ctx(dev, bdev, bus_type, (const struct hif_bus_id *)bid);

	if (reinit)
		ret = hdd_wlan_re_init();
	else
		ret = hdd_wlan_startup(dev);

	if (ret)
		goto err_hdd_deinit;


	if (reinit) {
		cds_set_recovery_in_progress(false);
	} else {
		cds_set_load_in_progress(false);
		cds_set_driver_loaded(true);
	}

	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT);
	hdd_remove_pm_qos(dev);

	return 0;


err_hdd_deinit:
	if (reinit)
		cds_set_recovery_in_progress(false);
	else
		cds_set_load_in_progress(false);
	hdd_deinit();
out:
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT);
	hdd_remove_pm_qos(dev);
	return ret;
}

static inline void hdd_pld_driver_unloading(struct device *dev)
{
	pld_set_driver_status(dev, PLD_LOAD_UNLOAD);
}

/**
 * wlan_hdd_remove() - wlan_hdd_remove
 *
 * This function is called by the platform driver to remove the
 * driver
 *
 * Return: void
 */
static void wlan_hdd_remove(struct device *dev)
{
	pr_info("%s: Removing driver v%s\n", WLAN_MODULE_NAME,
		QWLAN_VERSIONSTR);


	cds_set_driver_loaded(false);
	cds_set_unload_in_progress(true);

	if (!cds_wait_for_external_threads_completion(__func__))
		hdd_err("External threads are still active attempting driver unload anyway");

	hdd_pld_driver_unloading(dev);

	if (QDF_IS_EPPING_ENABLED(cds_get_conparam())) {
		epping_disable();
		epping_close();
	} else {
		__hdd_wlan_exit();
	}

	hdd_deinit();

	pr_info("%s: Driver De-initialized\n", WLAN_MODULE_NAME);
}

/**
 * wlan_hdd_shutdown() - wlan_hdd_shutdown
 *
 * This is routine is called by platform driver to shutdown the
 * driver
 *
 * Return: void
 */
static void wlan_hdd_shutdown(void)
{
	void *hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);

	if (cds_is_load_or_unload_in_progress()) {
		hdd_err("Load/unload in progress, ignore SSR shutdown");
		return;
	}
	/* this is for cases, where shutdown invoked from platform */
	cds_set_recovery_in_progress(true);

	if (!cds_wait_for_external_threads_completion(__func__))
		hdd_err("Host is not ready for SSR, attempting anyway");

	if (!QDF_IS_EPPING_ENABLED(cds_get_conparam())) {
		hif_disable_isr(hif_ctx);
		hdd_wlan_shutdown();
	}
}

/**
 * wlan_hdd_crash_shutdown() - wlan_hdd_crash_shutdown
 *
 * HDD crash shutdown funtion: This function is called by
 * platfrom driver's crash shutdown routine
 *
 * Return: void
 */
void wlan_hdd_crash_shutdown(void)
{
	hif_crash_shutdown(cds_get_context(QDF_MODULE_ID_HIF));
}

/**
 * wlan_hdd_notify_handler() - wlan_hdd_notify_handler
 *
 * This function is called by the platform driver to notify the
 * COEX
 *
 * @state: state
 *
 * Return: void
 */
void wlan_hdd_notify_handler(int state)
{
	if (!QDF_IS_EPPING_ENABLED(cds_get_conparam())) {
		int ret = 0;
		ret = hdd_wlan_notify_modem_power_state(state);
		if (ret < 0)
			hdd_err("Fail to send notify");
	}
}

/**
 * __wlan_hdd_bus_suspend() - handles platform supsend
 * @state: suspend message from the kernel
 *
 * Does precondtion validation. Ensures that a subsystem restart isn't in
 * progress.  Ensures that no load or unload is in progress.
 * Calls ol_txrx_bus_suspend to ensure the layer is ready for a bus suspend.
 * Calls wma_suspend to configure offloads.
 * Calls hif_suspend to suspend the bus.
 *
 * Return: 0 for success, -EFAULT for null pointers,
 *     -EBUSY or -EAGAIN if another opperation is in progress and
 *     wlan will not be ready to suspend in time.
 */
static int __wlan_hdd_bus_suspend(pm_message_t state)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int err = wlan_hdd_validate_context(hdd_ctx);
	int status;

	hdd_info("event %d", state.event);

	if (err)
		goto done;

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_info("Driver Module closed return success");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (NULL == hif_ctx) {
		err = -EINVAL;
		goto done;
	}

	err = qdf_status_to_os_return(ol_txrx_bus_suspend());
	if (err)
		goto done;

	err = wma_bus_suspend();
	if (err)
		goto resume_oltxrx;

	err = hif_bus_suspend(hif_ctx);
	if (err)
		goto resume_wma;

	hdd_info("suspend done");
	return 0;

resume_wma:
	status = wma_bus_resume();
	QDF_BUG(!status);
resume_oltxrx:
	status = ol_txrx_bus_resume();
	QDF_BUG(!status);
done:
	hdd_err("suspend failed, status = %d", err);
	return err;
}

/**
 * wlan_hdd_bus_suspend() - suspend the wlan bus
 *
 * This function is called by the platform driver to suspend the
 * wlan bus
 *
 * @state: state
 *
 * Return: QDF_STATUS
 */
int wlan_hdd_bus_suspend(pm_message_t state)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_bus_suspend(state);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_bus_suspend_noirq() - handle .suspend_noirq callback
 *
 * This function is called by the platform driver to complete the
 * bus suspend callback when device interrupts are disabled by kernel.
 * Call HIF and WMA suspend_noirq callbacks to make sure there is no
 * wake up pending from FW before allowing suspend.
 *
 * Return: 0 for success and -EBUSY if FW is requesting wake up
 */
int __wlan_hdd_bus_suspend_noirq(void)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int err;
	int status;

	err = wlan_hdd_validate_context(hdd_ctx);
	if (err) {
		hdd_err("Invalid HDD context: %d", err);
		return err;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_info("Driver Module closed return success");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (NULL == hif_ctx) {
		err = -EINVAL;
		goto done;
	}

	err = hif_bus_suspend_noirq(hif_ctx);
	if (err)
		goto done;

	err = wma_is_target_wake_up_received();
	if (err)
		goto resume_hif_noirq;

	hdd_info("suspend_noirq done");
	return 0;

resume_hif_noirq:
	status = hif_bus_resume_noirq(hif_ctx);
	QDF_BUG(!status);
done:
	if (err == -EAGAIN)
		hdd_err("Firmware attempting wakeup, try again");
	else
		hdd_err("suspend_noirq failed, status = %d", err);
	return err;
}

/**
 * wlan_hdd_bus_suspend_noirq() - handle .suspend_noirq callback
 *
 * This function is called by the platform driver to complete the
 * bus suspend callback when device interrupts are disabled by kernel.
 * Call HIF and WMA suspend_noirq callbacks to make sure there is no
 * wake up pending from FW before allowing suspend.
 *
 * Return: 0 for success and -EBUSY if FW is requesting wake up
 */
int wlan_hdd_bus_suspend_noirq(void)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_bus_suspend_noirq();
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_bus_resume() - handles platform resume
 *
 * Does precondtion validation. Ensures that a subsystem restart isn't in
 * progress.  Ensures that no load or unload is in progress.  Ensures that
 * it has valid pointers for the required contexts.
 * Calls into hif to resume the bus opperation.
 * Calls into wma to handshake with firmware and notify it that the bus is up.
 * Calls into ol_txrx for symetry.
 * Failures are treated as catastrophic.
 *
 * return: error code or 0 for success
 */
static int __wlan_hdd_bus_resume(void)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int status = wlan_hdd_validate_context(hdd_ctx);

	if (status)
		return status;

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_info("Driver Module closed return success");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (NULL == hif_ctx)
		return -EINVAL;

	status = hif_bus_resume(hif_ctx);
	QDF_BUG(!status);

	status = wma_bus_resume();
	QDF_BUG(!status);

	status = ol_txrx_bus_resume();
	QDF_BUG(!status);

	hdd_info("resume done");
	return status;
}

/**
 * wlan_hdd_bus_resume(): wake up the bus
 *
 * This function is called by the platform driver to resume wlan
 * bus
 *
 * Return: void
 */
int wlan_hdd_bus_resume(void)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_bus_resume();
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_bus_resume_noirq(): handle bus resume no irq
 *
 * This function is called by the platform driver to do bus
 * resume no IRQ before calling resume callback. Call WMA and HIF
 * layers to complete the resume_noirq.
 *
 * Return: 0 for success and negative error code for failure
 */
int __wlan_hdd_bus_resume_noirq(void)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int status = wlan_hdd_validate_context(hdd_ctx);

	if (status) {
		hdd_err("Invalid HDD context: %d", status);
		return status;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_info("Driver Module closed return success");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (NULL == hif_ctx)
		return -EINVAL;

	status = wma_clear_target_wake_up();
	QDF_BUG(!status);

	status = hif_bus_resume_noirq(hif_ctx);
	QDF_BUG(!status);

	hdd_info("resume_noirq done");
	return status;
}

/**
 * wlan_hdd_bus_resume_noirq(): handle bus resume no irq
 *
 * This function is called by the platform driver to do bus
 * resume no IRQ before calling resume callback. Call WMA and HIF
 * layers to complete the resume_noirq.
 *
 * Return: 0 for success and negative error code for failure
 */
int wlan_hdd_bus_resume_noirq(void)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_bus_resume_noirq();
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_bus_reset_resume() - resume wlan bus after reset
 *
 * This function is called to tell the driver that the device has been resumed
 * and it has also been reset. The driver should redo any necessary
 * initialization. It is mainly used by the USB bus
 *
 * Return: int 0 for success, non zero for failure
 */
static int wlan_hdd_bus_reset_resume(void)
{
	int ret;
	struct hif_opaque_softc *scn = NULL;

	scn = cds_get_context(QDF_MODULE_ID_HIF);
	if (!scn) {
		hdd_err("Failed to get HIF context");
		return -EFAULT;
	}

	cds_ssr_protect(__func__);
	ret = hif_bus_reset_resume(scn);
	cds_ssr_unprotect(__func__);
	return ret;
}

#ifdef FEATURE_RUNTIME_PM
/**
 * __wlan_hdd_runtime_suspend() - suspend the wlan bus without apps suspend
 *
 * Each layer is responsible for its own suspend actions.  wma_runtime_suspend
 * takes care of the parts of the 802.11 suspend that we want to do for runtime
 * suspend.
 *
 * Return: 0 or errno
 */
static int __wlan_hdd_runtime_suspend(struct device *dev)
{
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	void *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	void *htc_ctx = cds_get_context(QDF_MODULE_ID_HTC);
	int status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		goto process_failure;

	status = hif_pre_runtime_suspend(hif_ctx);
	if (status)
		goto process_failure;

	status = qdf_status_to_os_return(ol_txrx_runtime_suspend(txrx_pdev));
	if (status)
		goto process_failure;

	status = htc_runtime_suspend(htc_ctx);
	if (status)
		goto resume_txrx;

	status = wma_runtime_suspend();
	if (status)
		goto resume_htc;

	status = hif_runtime_suspend(hif_ctx);
	if (status)
		goto resume_wma;

	status = pld_auto_suspend(dev);
	if (status)
		goto resume_hif;

	hif_process_runtime_suspend_success(hif_ctx);
	return status;

resume_hif:
	QDF_BUG(!hif_runtime_resume(hif_ctx));
resume_wma:
	QDF_BUG(!wma_runtime_resume());
resume_htc:
	QDF_BUG(!htc_runtime_resume(htc_ctx));
resume_txrx:
	QDF_BUG(!qdf_status_to_os_return(ol_txrx_runtime_resume(txrx_pdev)));
process_failure:
	hif_process_runtime_suspend_failure(hif_ctx);
	return status;
}


/**
 * wlan_hdd_runtime_suspend() - suspend the wlan bus without apps suspend
 *
 * This function is called by the platform driver to suspend the
 * wlan bus separately from system suspend
 *
 * Return: 0 or errno
 */
static int wlan_hdd_runtime_suspend(struct device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_runtime_suspend(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_runtime_resume() - resume the wlan bus from runtime suspend
 *
 * Sets the runtime pm state and coordinates resume between hif wma and
 * ol_txrx.
 *
 * Return: success since failure is a bug
 */
static int __wlan_hdd_runtime_resume(struct device *dev)
{
	void *hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	void *htc_ctx = cds_get_context(QDF_MODULE_ID_HTC);
	void *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	hif_pre_runtime_resume(hif_ctx);
	QDF_BUG(!pld_auto_resume(dev));
	QDF_BUG(!hif_runtime_resume(hif_ctx));
	QDF_BUG(!wma_runtime_resume());
	QDF_BUG(!htc_runtime_resume(htc_ctx));
	QDF_BUG(!qdf_status_to_os_return(ol_txrx_runtime_resume(txrx_pdev)));
	hif_process_runtime_resume_success(hif_ctx);
	return 0;
}

/**
 * wlan_hdd_runtime_resume() - resume the wlan bus from runtime suspend
 *
 * This function is called by the platform driver to resume the
 * wlan bus separately from system suspend
 *
 * Return: success since failure is a bug
 */
static int wlan_hdd_runtime_resume(struct device *dev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_runtime_resume(dev);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

/**
 * wlan_hdd_pld_probe() - probe function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 * @bdev: bus device structure
 * @id: bus identifier for shared busses
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_probe(struct device *dev,
		   enum pld_bus_type pld_bus_type,
		   void *bdev, void *id)
{
	enum qdf_bus_type bus_type;

	bus_type = to_bus_type(pld_bus_type);
	if (bus_type == QDF_BUS_TYPE_NONE) {
		hdd_err("Invalid bus type %d->%d",
			pld_bus_type, bus_type);
		return -EINVAL;
	}

	return wlan_hdd_probe(dev, bdev, id, bus_type, false);
}

/**
 * wlan_hdd_pld_remove() - remove function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: void
 */
static void wlan_hdd_pld_remove(struct device *dev,
		     enum pld_bus_type bus_type)
{
	wlan_hdd_remove(dev);
}

/**
 * wlan_hdd_pld_shutdown() - shutdown function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: void
 */
static void wlan_hdd_pld_shutdown(struct device *dev,
		       enum pld_bus_type bus_type)
{
	wlan_hdd_shutdown();
}

/**
 * wlan_hdd_pld_reinit() - reinit function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 * @bdev: bus device structure
 * @id: bus identifier for shared busses
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_reinit(struct device *dev,
		    enum pld_bus_type pld_bus_type,
		    void *bdev, void *id)
{
	enum qdf_bus_type bus_type;

	bus_type = to_bus_type(pld_bus_type);
	if (bus_type == QDF_BUS_TYPE_NONE) {
		hdd_err("Invalid bus type %d->%d",
			pld_bus_type, bus_type);
		return -EINVAL;
	}

	return wlan_hdd_probe(dev, bdev, id, bus_type, true);
}

/**
 * wlan_hdd_pld_crash_shutdown() - crash_shutdown function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: void
 */
static void wlan_hdd_pld_crash_shutdown(struct device *dev,
			     enum pld_bus_type bus_type)
{
	wlan_hdd_crash_shutdown();
}

/**
 * wlan_hdd_pld_suspend() - suspend function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 * @state: PM state
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_suspend(struct device *dev,
		     enum pld_bus_type bus_type,
		     pm_message_t state)

{
	return wlan_hdd_bus_suspend(state);
}

/**
 * wlan_hdd_pld_resume() - resume function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_resume(struct device *dev,
		    enum pld_bus_type bus_type)
{
	return wlan_hdd_bus_resume();
}


/**
 * wlan_hdd_pld_suspend_noirq() - handle suspend no irq
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Complete the actions started by suspend().  Carry out any
 * additional operations required for suspending the device that might be
 * racing with its driver's interrupt handler, which is guaranteed not to
 * run while suspend_noirq() is being executed. Make sure to resume device
 * if FW has sent initial wake up message and expecting APPS to wake up.
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_suspend_noirq(struct device *dev,
		     enum pld_bus_type bus_type)
{
	return wlan_hdd_bus_suspend_noirq();
}

/**
 * wlan_hdd_pld_resume_noirq() - handle resume no irq
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Prepare for the execution of resume() by carrying out any
 * operations required for resuming the device that might be racing with
 * its driver's interrupt handler, which is guaranteed not to run while
 * resume_noirq() is being executed. Make sure to clear target initial
 * wake up request such that next suspend can happen cleanly.
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_resume_noirq(struct device *dev,
		    enum pld_bus_type bus_type)
{
	return wlan_hdd_bus_resume_noirq();
}

/**
 * wlan_hdd_pld_reset_resume() - reset resume function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_reset_resume(struct device *dev,
		    enum pld_bus_type bus_type)
{
	return wlan_hdd_bus_reset_resume();
}

/**
 * wlan_hdd_pld_notify_handler() - notify_handler function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 * @state: Modem power state
 *
 * Return: void
 */
static void wlan_hdd_pld_notify_handler(struct device *dev,
			     enum pld_bus_type bus_type,
			     int state)
{
	wlan_hdd_notify_handler(state);
}

#ifdef FEATURE_RUNTIME_PM
/**
 * wlan_hdd_pld_runtime_suspend() - runtime suspend function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_runtime_suspend(struct device *dev,
					enum pld_bus_type bus_type)
{
	return wlan_hdd_runtime_suspend(dev);
}

/**
 * wlan_hdd_pld_runtime_resume() - runtime resume function registered to PLD
 * @dev: device
 * @pld_bus_type: PLD bus type
 *
 * Return: 0 on success
 */
static int wlan_hdd_pld_runtime_resume(struct device *dev,
				       enum pld_bus_type bus_type)
{
	return wlan_hdd_runtime_resume(dev);
}
#endif

struct pld_driver_ops wlan_drv_ops = {
	.probe      = wlan_hdd_pld_probe,
	.remove     = wlan_hdd_pld_remove,
	.shutdown   = wlan_hdd_pld_shutdown,
	.reinit     = wlan_hdd_pld_reinit,
	.crash_shutdown = wlan_hdd_pld_crash_shutdown,
	.suspend    = wlan_hdd_pld_suspend,
	.resume     = wlan_hdd_pld_resume,
	.suspend_noirq = wlan_hdd_pld_suspend_noirq,
	.resume_noirq  = wlan_hdd_pld_resume_noirq,
	.reset_resume = wlan_hdd_pld_reset_resume,
	.modem_status = wlan_hdd_pld_notify_handler,
#ifdef FEATURE_RUNTIME_PM
	.runtime_suspend = wlan_hdd_pld_runtime_suspend,
	.runtime_resume = wlan_hdd_pld_runtime_resume,
#endif
};

/**
 * wlan_hdd_register_driver() - wlan_hdd_register_driver
 *
 * Return: int
 */
int wlan_hdd_register_driver(void)
{
	return pld_register_driver(&wlan_drv_ops);
}

/**
 * wlan_hdd_unregister_driver() - wlan_hdd_unregister_driver
 *
 * Return: void
 */
void wlan_hdd_unregister_driver(void)
{
	pld_unregister_driver();
}
