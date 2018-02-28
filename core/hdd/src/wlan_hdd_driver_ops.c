/*
 * Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
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
#include "wlan_policy_mgr_api.h"
#include "qwlan_version.h"
#include "bmi.h"
#include "cdp_txrx_bus.h"
#include "cdp_txrx_misc.h"
#include "pld_common.h"
#include "wlan_hdd_driver_ops.h"

#ifdef MODULE
#define WLAN_MODULE_NAME  module_name(THIS_MODULE)
#else
#define WLAN_MODULE_NAME  "wlan"
#endif

#define DISABLE_KRAIT_IDLE_PS_VAL      1

#define SSR_MAX_FAIL_CNT 3
static uint8_t re_init_fail_cnt, probe_fail_cnt;

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
 * hdd_is_target_ready() - API to query if target is in ready state
 * @data: Private Data
 *
 * Return: bool
 */
static bool hdd_is_target_ready(void *data)
{
	return cds_is_target_ready();
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
	cbk->is_target_ready = hdd_is_target_ready;
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

int hdd_hif_open(struct device *dev, void *bdev, const struct hif_bus_id *bid,
			enum qdf_bus_type bus_type, bool reinit)
{
	QDF_STATUS status;
	int ret = 0;
	struct hif_opaque_softc *hif_ctx;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	struct hif_driver_state_callbacks cbk;
	uint32_t mode = cds_get_conparam();
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

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
		hdd_err("Failed to set global HIF CDS Context err: %d", ret);
		goto err_hif_close;
	}

	status = hif_enable(hif_ctx, dev, bdev, bid, bus_type,
			    (reinit == true) ?  HIF_ENABLE_TYPE_REINIT :
			    HIF_ENABLE_TYPE_PROBE);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		hdd_err("hif_enable failed status: %d, reinit: %d",
			status, reinit);

		ret = qdf_status_to_os_return(status);
		goto err_hif_close;
	} else {
		cds_set_target_ready(true);
		ret = hdd_napi_create();
		hdd_debug("hdd_napi_create returned: %d", ret);
		if (ret == 0)
			hdd_warn("NAPI: no instances are created");
		else if (ret < 0) {
			hdd_err("NAPI creation error, rc: 0x%x, reinit: %d",
				ret, reinit);
			ret = -EFAULT;
			goto err_hif_close;
		} else {
			hdd_napi_event(NAPI_EVT_INI_FILE,
				(void *)hdd_ctx->napi_enable);
		}
	}

	pmo_ucfg_psoc_set_hif_handle(hdd_ctx->hdd_psoc, hif_ctx);

	return 0;

err_hif_close:
	hdd_deinit_cds_hif_context();
	hif_close(hif_ctx);
	return ret;
}

void hdd_hif_close(struct hdd_context *hdd_ctx, void *hif_ctx)
{
	if (!hdd_ctx) {
		hdd_err("hdd_ctx error");
		return;
	}

	if (hif_ctx == NULL)
		return;

	hif_disable(hif_ctx, HIF_DISABLE_TYPE_REMOVE);

	hdd_napi_destroy(true);

	hdd_deinit_cds_hif_context();
	hif_close(hif_ctx);

	pmo_ucfg_psoc_set_hif_handle(hdd_ctx->hdd_psoc, NULL);
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
static void hdd_init_qdf_ctx(struct device *dev, void *bdev,
			     enum qdf_bus_type bus_type,
			     const struct hif_bus_id *bid)
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
 * check_for_probe_defer() - API to check return value
 * @ret: Return Value
 *
 * Return: return -EPROBE_DEFER to platform driver if return value
 * is -ENOMEM. Platform driver will try to re-probe.
 */
#ifdef MODULE
static int check_for_probe_defer(int ret)
{
	return ret;
}
#else
static int check_for_probe_defer(int ret)
{
	if (ret == -ENOMEM)
		return -EPROBE_DEFER;
	return ret;
}
#endif

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
static int wlan_hdd_probe(struct device *dev, void *bdev,
			  const struct hif_bus_id *bid,
			  enum qdf_bus_type bus_type, bool reinit)
{
	int ret = 0;

	pr_info("%s: %sprobing driver v%s\n", WLAN_MODULE_NAME,
		reinit ? "re-" : "", QWLAN_VERSIONSTR);

	mutex_lock(&hdd_init_deinit_lock);
	cds_set_driver_in_bad_state(false);
	if (!reinit)
		hdd_start_driver_ops_timer(eHDD_DRV_OP_PROBE);
	else
		hdd_start_driver_ops_timer(eHDD_DRV_OP_REINIT);

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT);

	/*
	 * The Krait is going to Idle/Stand Alone Power Save more
	 * aggressively which is resulting in the longer driver load
	 * time. The Fix is to not allow Krait to enter Idle Power
	 * Save during driver load.
	 */
	hdd_request_pm_qos(dev, DISABLE_KRAIT_IDLE_PS_VAL);

	if (reinit)
		cds_set_recovery_in_progress(true);
	else
		cds_set_load_in_progress(true);

	hdd_init_qdf_ctx(dev, bdev, bus_type, (const struct hif_bus_id *)bid);

	if (reinit) {
		ret = hdd_wlan_re_init();
		if (ret)
			re_init_fail_cnt++;
	} else {
		ret = hdd_wlan_startup(dev);
		if (ret)
			probe_fail_cnt++;
	}

	if (ret)
		goto err_hdd_deinit;


	if (reinit) {
		cds_set_recovery_in_progress(false);
	} else {
		cds_set_load_in_progress(false);
		cds_set_driver_loaded(true);
		hdd_start_complete(0);
	}

	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT);
	hdd_remove_pm_qos(dev);
	cds_set_fw_down(false);
	probe_fail_cnt = 0;
	re_init_fail_cnt = 0;
	hdd_stop_driver_ops_timer();
	mutex_unlock(&hdd_init_deinit_lock);
	return 0;


err_hdd_deinit:
	pr_err("probe/reinit failure counts %hhu/%hhu",
		probe_fail_cnt, re_init_fail_cnt);
	if (probe_fail_cnt >= SSR_MAX_FAIL_CNT ||
	    re_init_fail_cnt >= SSR_MAX_FAIL_CNT)
		QDF_BUG(0);

	if (reinit) {
		cds_set_driver_in_bad_state(true);
		cds_set_recovery_in_progress(false);
	} else
		cds_set_load_in_progress(false);

	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_DRIVER_INIT);
	hdd_remove_pm_qos(dev);

	cds_set_fw_down(false);
	hdd_stop_driver_ops_timer();
	mutex_unlock(&hdd_init_deinit_lock);
	return check_for_probe_defer(ret);
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
		hdd_warn("External threads are still active attempting driver unload anyway");

	if (QDF_IS_EPPING_ENABLED(cds_get_conparam())) {
		epping_disable();
		epping_close();
	} else {
		__hdd_wlan_exit();
	}

	cds_set_driver_in_bad_state(false);
	cds_set_unload_in_progress(false);

	pr_info("%s: Driver De-initialized\n", WLAN_MODULE_NAME);
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * hdd_wlan_ssr_shutdown_event()- send ssr shutdown state
 *
 * This Function send send ssr shutdown state diag event
 *
 * Return: void.
 */
static void hdd_wlan_ssr_shutdown_event(void)
{
	WLAN_HOST_DIAG_EVENT_DEF(ssr_shutdown,
					struct host_event_wlan_ssr_shutdown);
	qdf_mem_zero(&ssr_shutdown, sizeof(ssr_shutdown));
	ssr_shutdown.status = SSR_SUB_SYSTEM_SHUTDOWN;
	WLAN_HOST_DIAG_EVENT_REPORT(&ssr_shutdown,
					EVENT_WLAN_SSR_SHUTDOWN_SUBSYSTEM);
}
#else
static inline void hdd_wlan_ssr_shutdown_event(void)
{

};
#endif

/**
 * hdd_send_hang_reason() - Send hang reason to the userspace
 *
 * Return: None
 */
static void hdd_send_hang_reason(void)
{
	enum qdf_hang_reason reason = QDF_REASON_UNSPECIFIED;
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	cds_get_recovery_reason(&reason);
	cds_reset_recovery_reason();
	wlan_hdd_send_hang_reason_event(hdd_ctx, reason);
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

	if (hdd_get_conparam() == QDF_GLOBAL_FTM_MODE) {
		hdd_err("Crash recovery is not allowed in FTM mode");
		QDF_BUG(0);
		return;
	}

	if (!hif_ctx) {
		hdd_err("Failed to get HIF context, ignore SSR shutdown");
		return;
	}
	/* mask the host controller interrupts */
	hif_mask_interrupt_call(hif_ctx);

	if (cds_is_load_or_unload_in_progress()) {
		hdd_err("Load/unload in progress, ignore SSR shutdown");
		return;
	}
	/* this is for cases, where shutdown invoked from platform */
	cds_set_recovery_in_progress(true);
	hdd_wlan_ssr_shutdown_event();
	hdd_send_hang_reason();

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
static void wlan_hdd_crash_shutdown(void)
{
	QDF_STATUS ret;
	WMA_HANDLE wma_handle = cds_get_context(QDF_MODULE_ID_WMA);

	if (!wma_handle) {
		hdd_err("wma_handle is null");
		return;
	}

	/*
	 * When kernel panic happen, if WiFi FW is still active
	 * it may cause NOC errors/memory corruption, to avoid
	 * this, inject a fw crash first.
	 * send crash_inject to FW directly, because we are now
	 * in an atomic context, and preempt has been disabled,
	 * MCThread won't be scheduled at the moment, at the same
	 * time, TargetFailure event wont't be received after inject
	 * crash due to the same reason.
	 */
	ret = wma_crash_inject(wma_handle, RECOVERY_SIM_ASSERT, 0);
	if (QDF_IS_STATUS_ERROR(ret)) {
		hdd_err("Failed to send crash inject:%d", ret);
		return;
	}

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
static void wlan_hdd_notify_handler(int state)
{
	if (!QDF_IS_EPPING_ENABLED(cds_get_conparam())) {
		int ret;

		ret = hdd_wlan_notify_modem_power_state(state);
		if (ret < 0)
			hdd_err("Fail to send notify");
	}
}

static int hdd_to_pmo_interface_pause(enum wow_interface_pause hdd_pause,
				      enum pmo_wow_interface_pause *pmo_pause)
{
	switch (hdd_pause) {
	case WOW_INTERFACE_PAUSE_DEFAULT:
		*pmo_pause = PMO_WOW_INTERFACE_PAUSE_DEFAULT;
		break;
	case WOW_INTERFACE_PAUSE_ENABLE:
		*pmo_pause = PMO_WOW_INTERFACE_PAUSE_ENABLE;
		break;
	case WOW_INTERFACE_PAUSE_DISABLE:
		*pmo_pause = PMO_WOW_INTERFACE_PAUSE_DISABLE;
		break;
	default:
		hdd_err("Invalid interface pause: %d", hdd_pause);
		return -EINVAL;
	}

	return 0;
}

static int hdd_to_pmo_resume_trigger(enum wow_resume_trigger hdd_trigger,
				     enum pmo_wow_resume_trigger *pmo_trigger)
{
	switch (hdd_trigger) {
	case WOW_RESUME_TRIGGER_DEFAULT:
		*pmo_trigger = PMO_WOW_RESUME_TRIGGER_DEFAULT;
		break;
	case WOW_RESUME_TRIGGER_HTC_WAKEUP:
		*pmo_trigger = PMO_WOW_RESUME_TRIGGER_HTC_WAKEUP;
		break;
	case WOW_RESUME_TRIGGER_GPIO:
		*pmo_trigger = PMO_WOW_RESUME_TRIGGER_GPIO;
		break;
	default:
		hdd_err("Invalid resume trigger: %d", hdd_trigger);
		return -EINVAL;
	}

	return 0;
}

static int
hdd_to_pmo_wow_enable_params(struct wow_enable_params *in_params,
			     struct pmo_wow_enable_params *out_params)
{
	int err;

	/* unit-test suspend */
	out_params->is_unit_test = in_params->is_unit_test;

	/* interface pause */
	err = hdd_to_pmo_interface_pause(in_params->interface_pause,
					 &out_params->interface_pause);
	if (err)
		return err;

	/* resume trigger */
	err = hdd_to_pmo_resume_trigger(in_params->resume_trigger,
					&out_params->resume_trigger);
	if (err)
		return err;

	return 0;
}

/**
 * __wlan_hdd_bus_suspend() - handles platform supsend
 * @wow_params: collection of wow enable override parameters
 *
 * Does precondtion validation. Ensures that a subsystem restart isn't in
 * progress. Ensures that no load or unload is in progress. Does:
 *	data path suspend
 *	component (pmo) suspend
 *	hif (bus) suspend
 *
 * Return: 0 for success, -EFAULT for null pointers,
 *     -EBUSY or -EAGAIN if another opperation is in progress and
 *     wlan will not be ready to suspend in time.
 */
static int __wlan_hdd_bus_suspend(struct wow_enable_params wow_params)
{
	int err;
	QDF_STATUS status;
	struct hdd_context *hdd_ctx;
	void *hif_ctx;
	void *dp_soc;
	void *dp_pdev;
	struct pmo_wow_enable_params pmo_params;

	hdd_info("starting bus suspend");

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	err = wlan_hdd_validate_context(hdd_ctx);
	if (err) {
		hdd_err("Invalid hdd context: %d", err);
		return err;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver Module closed; skipping suspend");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("Failed to get hif context");
		return -EINVAL;
	}

	err = hdd_to_pmo_wow_enable_params(&wow_params, &pmo_params);
	if (err) {
		hdd_err("Invalid WoW enable parameters: %d", err);
		return err;
	}

	dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	dp_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	err = qdf_status_to_os_return(cdp_bus_suspend(dp_soc, dp_pdev));
	if (err) {
		hdd_err("Failed cdp bus suspend: %d", err);
		return err;
	}

	err = hif_bus_early_suspend(hif_ctx);
	if (err) {
		hdd_err("Failed hif bus early suspend");
		goto resume_cdp;
	}

	status = pmo_ucfg_psoc_bus_suspend_req(hdd_ctx->hdd_psoc,
					       QDF_SYSTEM_SUSPEND,
					       &pmo_params);
	err = qdf_status_to_os_return(status);
	if (err) {
		hdd_err("Failed pmo bus suspend: %d", status);
		goto late_hif_resume;
	}

	err = hif_bus_suspend(hif_ctx);
	if (err) {
		hdd_err("Failed hif bus suspend: %d", err);
		goto resume_pmo;
	}

	hdd_info("bus suspend succeeded");
	return 0;

resume_pmo:
	status = pmo_ucfg_psoc_bus_resume_req(hdd_ctx->hdd_psoc,
					      QDF_SYSTEM_SUSPEND);
	QDF_BUG(QDF_IS_STATUS_SUCCESS(status));

late_hif_resume:
	status = hif_bus_late_resume(hif_ctx);
	QDF_BUG(QDF_IS_STATUS_SUCCESS(status));

resume_cdp:
	status = cdp_bus_resume(dp_soc, dp_pdev);
	QDF_BUG(QDF_IS_STATUS_SUCCESS(status));

	return err;
}

int wlan_hdd_bus_suspend(void)
{
	int ret;
	struct wow_enable_params default_params = {0};

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_bus_suspend(default_params);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_SUSPEND_RESUME_TEST
int wlan_hdd_unit_test_bus_suspend(struct wow_enable_params wow_params)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_bus_suspend(wow_params);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

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
static int __wlan_hdd_bus_suspend_noirq(void)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int errno;
	uint32_t pending_events;

	hdd_info("start bus_suspend_noirq");
	errno = wlan_hdd_validate_context(hdd_ctx);
	if (errno) {
		hdd_err("Invalid HDD context: errno %d", errno);
		return errno;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver module closed; skip bus-noirq suspend");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (!hif_ctx) {
		hdd_err("hif_ctx is null");
		return -EINVAL;
	}

	errno = hif_bus_suspend_noirq(hif_ctx);
	if (errno)
		goto done;

	errno = pmo_ucfg_psoc_is_target_wake_up_received(hdd_ctx->hdd_psoc);
	if (errno == -EAGAIN) {
		hdd_err("Firmware attempting wakeup, try again");
		wlan_hdd_inc_suspend_stats(hdd_ctx,
					   SUSPEND_FAIL_INITIAL_WAKEUP);
	}
	if (errno)
		goto resume_hif_noirq;

	pending_events = wma_critical_events_in_flight();
	if (pending_events) {
		hdd_err("%d critical event(s) in flight; try again",
			pending_events);
		errno = -EAGAIN;
		goto resume_hif_noirq;
	}

	hdd_ctx->suspend_resume_stats.suspends++;

	hdd_info("bus_suspend_noirq done");
	return 0;

resume_hif_noirq:
	QDF_BUG(!hif_bus_resume_noirq(hif_ctx));

done:
	hdd_err("suspend_noirq failed, status: %d", errno);

	return errno;
}

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
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int status;
	QDF_STATUS qdf_status;
	void *dp_soc;
	void *dp_pdev;

	if (cds_is_driver_recovering())
		return 0;

	hdd_info("starting bus resume");

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status) {
		hdd_err("Invalid hdd context");
		return status;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver Module closed; return success");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (NULL == hif_ctx) {
		hdd_err("Failed to get hif context");
		return -EINVAL;
	}

	status = hif_bus_resume(hif_ctx);
	if (status) {
		hdd_err("Failed hif bus resume");
		goto out;
	}

	qdf_status = pmo_ucfg_psoc_bus_resume_req(hdd_ctx->hdd_psoc,
			QDF_SYSTEM_SUSPEND);
	status = qdf_status_to_os_return(qdf_status);
	if (status) {
		hdd_err("Failed pmo bus resume");
		goto out;
	}

	status = hif_bus_late_resume(hif_ctx);
	if (status) {
		hdd_err("Failed hif bus late resume");
		goto out;
	}

	dp_soc = cds_get_context(QDF_MODULE_ID_SOC);
	dp_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	qdf_status = cdp_bus_resume(dp_soc, dp_pdev);
	status = qdf_status_to_os_return(qdf_status);
	if (status) {
		hdd_err("Failed cdp bus resume");
		goto out;
	}

	hdd_info("bus resume succeeded");
	return 0;

out:
	if (cds_is_driver_recovering() || cds_is_driver_in_bad_state() ||
		cds_is_fw_down())
		return 0;

	QDF_BUG(false);

	return status;
}

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
static int __wlan_hdd_bus_resume_noirq(void)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	void *hif_ctx;
	int status;
	QDF_STATUS qdf_status;

	hdd_info("starting bus_resume_noirq");
	if (cds_is_driver_recovering())
		return 0;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status) {
		hdd_err("Invalid HDD context: %d", status);
		return status;
	}

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver Module closed return success");
		return 0;
	}

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (NULL == hif_ctx)
		return -EINVAL;

	qdf_status = pmo_ucfg_psoc_clear_target_wake_up(hdd_ctx->hdd_psoc);
	QDF_BUG(!qdf_status);

	status = hif_bus_resume_noirq(hif_ctx);
	QDF_BUG(!status);

	hdd_info("bus_resume_noirq done");
	return status;
}

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
 * hdd_pld_runtime_suspend_cb() - Runtime suspend callback from PMO
 *
 * Return: 0 on success or error value otherwise
 */
static int hdd_pld_runtime_suspend_cb(void)
{
	qdf_device_t qdf_dev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_dev) {
		hdd_err("Invalid context");
		return -EINVAL;
	}

	return pld_auto_suspend(qdf_dev->dev);
}

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
	int err;
	QDF_STATUS status;
	struct hdd_context *hdd_ctx;

	hdd_debug("Starting runtime suspend");

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	err = wlan_hdd_validate_context(hdd_ctx);
	if (err)
		return err;

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver module closed skipping runtime suspend");
		return 0;
	}

	if (ucfg_scan_get_pdev_status(hdd_ctx->hdd_pdev) !=
	    SCAN_NOT_IN_PROGRESS) {
		hdd_debug("Scan in progress, ignore runtime suspend");
		return -EBUSY;
	}

	status = pmo_ucfg_psoc_bus_runtime_suspend(hdd_ctx->hdd_psoc,
						   hdd_pld_runtime_suspend_cb);
	err = qdf_status_to_os_return(status);

	hdd_debug("Runtime suspend done result: %d", err);

	return err;
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
 * hdd_pld_runtime_resume_cb() - Runtime resume callback from PMO
 *
 * Return: 0 on success or error value otherwise
 */
static int hdd_pld_runtime_resume_cb(void)
{
	qdf_device_t qdf_dev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_dev) {
		hdd_err("Invalid context");
		return -EINVAL;
	}

	return pld_auto_resume(qdf_dev->dev);
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
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	QDF_STATUS status;

	hdd_debug("Starting runtime resume");

	if (wlan_hdd_validate_context(hdd_ctx))
		return 0;

	if (hdd_ctx->driver_status != DRIVER_MODULES_ENABLED) {
		hdd_debug("Driver module closed skipping runtime resume");
		return 0;
	}

	status = pmo_ucfg_psoc_bus_runtime_resume(hdd_ctx->hdd_psoc,
						  hdd_pld_runtime_resume_cb);
	if (status != QDF_STATUS_SUCCESS)
		hdd_err("PMO Runtime resume failed: %d", status);

	hdd_debug("Runtime resume done");

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
	ENTER();
	mutex_lock(&hdd_init_deinit_lock);
	hdd_start_driver_ops_timer(eHDD_DRV_OP_REMOVE);

	wlan_hdd_remove(dev);

	hdd_stop_driver_ops_timer();
	mutex_unlock(&hdd_init_deinit_lock);
	EXIT();
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
	ENTER();
	mutex_lock(&hdd_init_deinit_lock);
	hdd_start_driver_ops_timer(eHDD_DRV_OP_SHUTDOWN);

	wlan_hdd_shutdown();

	hdd_stop_driver_ops_timer();
	mutex_unlock(&hdd_init_deinit_lock);
	EXIT();
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
	return wlan_hdd_bus_suspend();
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

static void wlan_hdd_purge_notifier(void)
{
	struct hdd_context *hdd_ctx;

	ENTER();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		hdd_err("hdd context is NULL return!!");
		return;
	}

	qdf_cancel_delayed_work(&hdd_ctx->iface_idle_work);

	mutex_lock(&hdd_ctx->iface_change_lock);
	cds_shutdown_notifier_call();
	cds_shutdown_notifier_purge();
	mutex_unlock(&hdd_ctx->iface_change_lock);
	EXIT();
}

/**
 * wlan_hdd_set_the_pld_uevent() - set the pld event
 * @uevent: uevent status
 *
 * Return: void
 */
static void wlan_hdd_set_the_pld_uevent(struct pld_uevent_data *uevent)
{
	switch (uevent->uevent) {
	case PLD_RECOVERY:
		cds_set_recovery_in_progress(true);
		break;
	case PLD_FW_DOWN:
		cds_set_fw_state(CDS_FW_STATE_DOWN);
		break;
	}
}

/**
 * wlan_hdd_handle_the_pld_uevent() - handle the pld event
 * @uevent: uevent status
 *
 * Return: void
 */
static void wlan_hdd_handle_the_pld_uevent(struct pld_uevent_data *uevent)
{
	enum cds_driver_state driver_state;
	struct hdd_context *hdd_ctx;

	driver_state = cds_get_driver_state();

	if (driver_state == CDS_DRIVER_STATE_UNINITIALIZED)
		return;

	if (cds_is_driver_loading())
		return;

	switch (uevent->uevent) {
	case PLD_RECOVERY:
		hdd_pld_ipa_uc_shutdown_pipes();
		break;
	case PLD_FW_DOWN:
		qdf_complete_wait_events();
		cds_set_target_ready(false);
		hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
		if (hdd_ctx != NULL)
			wlan_cfg80211_cleanup_scan_queue(
					hdd_ctx->hdd_pdev, NULL);
		break;
	default:
		break;
	}
}

/**
 * wlan_hdd_pld_uevent() - update driver status
 * @dev: device
 * @uevent: uevent status
 *
 * Return: void
 */
static void wlan_hdd_pld_uevent(struct device *dev,
				struct pld_uevent_data *uevent)
{
	ENTER();
	hdd_info("pld event %d", uevent->uevent);

	mutex_lock(&hdd_init_deinit_lock);
	wlan_hdd_set_the_pld_uevent(uevent);
	wlan_hdd_handle_the_pld_uevent(uevent);
	mutex_unlock(&hdd_init_deinit_lock);

	wlan_hdd_purge_notifier();
	EXIT();
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
	.uevent = wlan_hdd_pld_uevent,
#ifdef FEATURE_RUNTIME_PM
	.runtime_suspend = wlan_hdd_pld_runtime_suspend,
	.runtime_resume = wlan_hdd_pld_runtime_resume,
#endif
};

int wlan_hdd_register_driver(void)
{
	return pld_register_driver(&wlan_drv_ops);
}

void wlan_hdd_unregister_driver(void)
{
	pld_unregister_driver();
}
