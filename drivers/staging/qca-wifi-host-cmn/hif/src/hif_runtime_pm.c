/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#include "hif_io32.h"
#include "hif_runtime_pm.h"
#include "hif.h"
#include "target_type.h"
#include "hif_main.h"
#include "ce_main.h"
#include "ce_api.h"
#include "ce_internal.h"
#include "ce_reg.h"
#include "ce_bmi.h"
#include "regtable.h"
#include "hif_hw_version.h"
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include "qdf_status.h"
#include "qdf_atomic.h"
#include "pld_common.h"
#include "mp_dev.h"
#include "hif_debug.h"

#include "ce_tasklet.h"
#include "targaddrs.h"
#include "hif_exec.h"

#define CNSS_RUNTIME_FILE "cnss_runtime_pm"
#define CNSS_RUNTIME_FILE_PERM QDF_FILE_USR_READ

#ifdef FEATURE_RUNTIME_PM
#define PREVENT_LIST_STRING_LEN 200

/**
 * hif_pci_pm_runtime_enabled() - To check if Runtime PM is enabled
 * @scn: hif context
 *
 * This function will check if Runtime PM is enabled or not.
 *
 * Return: void
 */
static bool hif_pci_pm_runtime_enabled(struct hif_softc *scn)
{
	if (scn->hif_config.enable_runtime_pm)
		return true;

	return pm_runtime_enabled(hif_bus_get_dev(scn));
}

/**
 * hif_pm_runtime_state_to_string() - Mapping state into string
 * @state: runtime pm state
 *
 * This function will map the runtime pm state into corresponding
 * string for debug purpose.
 *
 * Return: pointer to the string
 */
static const char *hif_pm_runtime_state_to_string(uint32_t state)
{
	switch (state) {
	case HIF_PM_RUNTIME_STATE_NONE:
		return "INIT_STATE";
	case HIF_PM_RUNTIME_STATE_ON:
		return "ON";
	case HIF_PM_RUNTIME_STATE_RESUMING:
		return "RESUMING";
	case HIF_PM_RUNTIME_STATE_SUSPENDING:
		return "SUSPENDING";
	case HIF_PM_RUNTIME_STATE_SUSPENDED:
		return "SUSPENDED";
	default:
		return "INVALID STATE";
	}
}

#define HIF_PCI_RUNTIME_PM_STATS(_s, _rpm_ctx, _name) \
	seq_printf(_s, "%30s: %u\n", #_name, (_rpm_ctx)->pm_stats._name)
/**
 * hif_pci_runtime_pm_warn() - Runtime PM Debugging API
 * @hif_ctx: hif_softc context
 * @msg: log message
 *
 * log runtime pm stats when something seems off.
 *
 * Return: void
 */
static void hif_pci_runtime_pm_warn(struct hif_softc *scn,
				    const char *msg)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_pm_runtime_lock *ctx;
	int i;

	hif_nofl_debug("%s: usage_count: %d, pm_state: %s, prevent_suspend_cnt: %d",
		       msg, atomic_read(&dev->power.usage_count),
		       hif_pm_runtime_state_to_string(
				atomic_read(&rpm_ctx->pm_state)),
		       rpm_ctx->prevent_suspend_cnt);

	hif_nofl_debug("runtime_status: %d, runtime_error: %d, disable_depth: %d autosuspend_delay: %d",
		       dev->power.runtime_status,
		       dev->power.runtime_error,
		       dev->power.disable_depth,
		       dev->power.autosuspend_delay);

	hif_nofl_debug("runtime_get: %u, runtime_put: %u, request_resume: %u",
		       qdf_atomic_read(&rpm_ctx->pm_stats.runtime_get),
		       qdf_atomic_read(&rpm_ctx->pm_stats.runtime_put),
		       rpm_ctx->pm_stats.request_resume);

	hif_nofl_debug("get  put  get-timestamp put-timestamp :DBGID_NAME");
	for (i = 0; i < RTPM_ID_MAX; i++) {
		hif_nofl_debug("%-10d %-10d  0x%-10llx  0x%-10llx :%-30s",
			       qdf_atomic_read(
				       &rpm_ctx->pm_stats.runtime_get_dbgid[i]),
			       qdf_atomic_read(
				       &rpm_ctx->pm_stats.runtime_put_dbgid[i]),
			       rpm_ctx->pm_stats.runtime_get_timestamp_dbgid[i],
			       rpm_ctx->pm_stats.runtime_put_timestamp_dbgid[i],
			       rtpm_string_from_dbgid(i));
	}

	hif_nofl_debug("allow_suspend: %u, prevent_suspend: %u",
		       qdf_atomic_read(&rpm_ctx->pm_stats.allow_suspend),
		       qdf_atomic_read(&rpm_ctx->pm_stats.prevent_suspend));

	hif_nofl_debug("prevent_suspend_timeout: %u, allow_suspend_timeout: %u",
		       rpm_ctx->pm_stats.prevent_suspend_timeout,
		       rpm_ctx->pm_stats.allow_suspend_timeout);

	hif_nofl_debug("Suspended: %u, resumed: %u count",
		       rpm_ctx->pm_stats.suspended,
		       rpm_ctx->pm_stats.resumed);

	hif_nofl_debug("suspend_err: %u, runtime_get_err: %u",
		       rpm_ctx->pm_stats.suspend_err,
		       rpm_ctx->pm_stats.runtime_get_err);

	hif_nofl_debug("Active Wakeup Sources preventing Runtime Suspend: ");

	list_for_each_entry(ctx, &rpm_ctx->prevent_suspend_list, list) {
		hif_nofl_debug("source %s; timeout %d ms",
			       ctx->name, ctx->timeout);
	}

	if (qdf_is_fw_down()) {
		hif_err("fw is down");
		return;
	}

	QDF_DEBUG_PANIC("hif_pci_runtime_pm_warn");
}

/**
 * hif_pci_pm_runtime_debugfs_show(): show debug stats for runtimepm
 * @s: file to print to
 * @data: unused
 *
 * debugging tool added to the debug fs for displaying runtimepm stats
 *
 * Return: 0
 */
static int hif_pci_pm_runtime_debugfs_show(struct seq_file *s, void *data)
{
	struct hif_softc *scn = s->private;
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	struct device *dev = hif_bus_get_dev(scn);
	static const char * const autopm_state[] = {"NONE", "ON", "RESUMING",
		"SUSPENDING", "SUSPENDED"};
	unsigned int msecs_age;
	qdf_time_t usecs_age;
	int pm_state = atomic_read(&rpm_ctx->pm_state);
	unsigned long timer_expires;
	struct hif_pm_runtime_lock *ctx;
	int i;

	seq_printf(s, "%30s: %s\n", "Runtime PM state", autopm_state[pm_state]);
	seq_printf(s, "%30s: %d(%s)\n", "last_resume_rtpm_dbgid",
		   rpm_ctx->pm_stats.last_resume_rtpm_dbgid,
		   rtpm_string_from_dbgid(
			   rpm_ctx->pm_stats.last_resume_rtpm_dbgid));
	seq_printf(s, "%30s: %ps\n", "Last Busy Marker",
		   rpm_ctx->pm_stats.last_busy_marker);

	usecs_age = qdf_get_log_timestamp_usecs() -
		rpm_ctx->pm_stats.last_busy_timestamp;
	seq_printf(s, "%30s: %lu.%06lus\n", "Last Busy Timestamp",
		   rpm_ctx->pm_stats.last_busy_timestamp / 1000000,
		   rpm_ctx->pm_stats.last_busy_timestamp % 1000000);
	seq_printf(s, "%30s: %lu.%06lus\n", "Last Busy Since",
		   usecs_age / 1000000, usecs_age % 1000000);

	if (pm_state == HIF_PM_RUNTIME_STATE_SUSPENDED) {
		msecs_age = jiffies_to_msecs(jiffies -
					     rpm_ctx->pm_stats.suspend_jiffies);
		seq_printf(s, "%30s: %d.%03ds\n", "Suspended Since",
			   msecs_age / 1000, msecs_age % 1000);
	}

	seq_printf(s, "%30s: %d\n", "PM Usage count",
		   atomic_read(&dev->power.usage_count));

	seq_printf(s, "%30s: %u\n", "prevent_suspend_cnt",
		   rpm_ctx->prevent_suspend_cnt);

	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, suspended);
	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, suspend_err);
	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, resumed);

	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, request_resume);
	seq_printf(s, "%30s: %u\n", "prevent_suspend",
		   qdf_atomic_read(&rpm_ctx->pm_stats.prevent_suspend));
	seq_printf(s, "%30s: %u\n", "allow_suspend",
		   qdf_atomic_read(&rpm_ctx->pm_stats.allow_suspend));

	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, prevent_suspend_timeout);
	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, allow_suspend_timeout);
	HIF_PCI_RUNTIME_PM_STATS(s, rpm_ctx, runtime_get_err);

	seq_printf(s, "%30s: %u\n", "runtime_get",
		   qdf_atomic_read(&rpm_ctx->pm_stats.runtime_get));
	seq_printf(s, "%30s: %u\n", "runtime_put",
		   qdf_atomic_read(&rpm_ctx->pm_stats.runtime_put));
	seq_puts(s, "get  put  get-timestamp put-timestamp :DBGID_NAME\n");
	for (i = 0; i < RTPM_ID_MAX; i++) {
		seq_printf(s, "%-10d ",
			   qdf_atomic_read(
				 &rpm_ctx->pm_stats.runtime_get_dbgid[i]));
		seq_printf(s, "%-10d ",
			   qdf_atomic_read(
				 &rpm_ctx->pm_stats.runtime_put_dbgid[i]));
		seq_printf(s, "0x%-10llx ",
			   rpm_ctx->pm_stats.runtime_get_timestamp_dbgid[i]);
		seq_printf(s, "0x%-10llx ",
			   rpm_ctx->pm_stats.runtime_put_timestamp_dbgid[i]);
		seq_printf(s, ":%-30s\n", rtpm_string_from_dbgid(i));
	}

	timer_expires = rpm_ctx->runtime_timer_expires;
	if (timer_expires > 0) {
		msecs_age = jiffies_to_msecs(timer_expires - jiffies);
		seq_printf(s, "%30s: %d.%03ds\n", "Prevent suspend timeout",
			   msecs_age / 1000, msecs_age % 1000);
	}

	spin_lock_bh(&rpm_ctx->runtime_lock);
	if (list_empty(&rpm_ctx->prevent_suspend_list)) {
		spin_unlock_bh(&rpm_ctx->runtime_lock);
		return 0;
	}

	seq_printf(s, "%30s: ", "Active Wakeup_Sources");
	list_for_each_entry(ctx, &rpm_ctx->prevent_suspend_list, list) {
		seq_printf(s, "%s", ctx->name);
		if (ctx->timeout)
			seq_printf(s, "(%d ms)", ctx->timeout);
		seq_puts(s, " ");
	}
	seq_puts(s, "\n");
	spin_unlock_bh(&rpm_ctx->runtime_lock);

	return 0;
}

#undef HIF_PCI_RUNTIME_PM_STATS

/**
 * hif_pci_autopm_open() - open a debug fs file to access the runtime pm stats
 * @inode
 * @file
 *
 * Return: linux error code of single_open.
 */
static int hif_pci_runtime_pm_open(struct inode *inode, struct file *file)
{
	return single_open(file, hif_pci_pm_runtime_debugfs_show,
			inode->i_private);
}

static const struct file_operations hif_pci_runtime_pm_fops = {
	.owner          = THIS_MODULE,
	.open           = hif_pci_runtime_pm_open,
	.release        = single_release,
	.read           = seq_read,
	.llseek         = seq_lseek,
};

/**
 * hif_runtime_pm_debugfs_create() - creates runtimepm debugfs entry
 * @scn: hif context
 *
 * creates a debugfs entry to debug the runtime pm feature.
 */
static void hif_runtime_pm_debugfs_create(struct hif_softc *scn)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	rpm_ctx->pm_dentry = qdf_debugfs_create_entry(CNSS_RUNTIME_FILE,
						      CNSS_RUNTIME_FILE_PERM,
						      NULL,
						      scn,
						      &hif_pci_runtime_pm_fops);
}

/**
 * hif_runtime_pm_debugfs_remove() - removes runtimepm debugfs entry
 * @sc: pci context
 *
 * removes the debugfs entry to debug the runtime pm feature.
 */
static void hif_runtime_pm_debugfs_remove(struct hif_softc *scn)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	qdf_debugfs_remove_file(rpm_ctx->pm_dentry);
}

/**
 * hif_runtime_init() - Initialize Runtime PM
 * @dev: device structure
 * @delay: delay to be confgured for auto suspend
 *
 * This function will init all the Runtime PM config.
 *
 * Return: void
 */
static void hif_runtime_init(struct device *dev, int delay)
{
	pm_runtime_set_autosuspend_delay(dev, delay);
	pm_runtime_use_autosuspend(dev);
	pm_runtime_allow(dev);
	pm_runtime_mark_last_busy(dev);
	pm_runtime_put_noidle(dev);
	pm_suspend_ignore_children(dev, true);
}

/**
 * hif_runtime_exit() - Deinit/Exit Runtime PM
 * @dev: device structure
 *
 * This function will deinit all the Runtime PM config.
 *
 * Return: void
 */
static void hif_runtime_exit(struct device *dev)
{
	pm_runtime_get_noresume(dev);
	pm_runtime_set_active(dev);
	/* Symmetric call to make sure default usage count == 2 */
	pm_runtime_forbid(dev);
}

static void hif_pm_runtime_lock_timeout_fn(void *data);

/**
 * hif_pm_runtime_start(): start the runtime pm
 * @scn: hif context
 *
 * After this call, runtime pm will be active.
 */
void hif_pm_runtime_start(struct hif_softc *scn)
{
	uint32_t mode = hif_get_conparam(scn);
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (!scn->hif_config.enable_runtime_pm) {
		hif_info("RUNTIME PM is disabled in ini");
		return;
	}

	if (mode == QDF_GLOBAL_FTM_MODE || QDF_IS_EPPING_ENABLED(mode) ||
	    mode == QDF_GLOBAL_MONITOR_MODE) {
		hif_info("RUNTIME PM is disabled for FTM/EPPING mode");
		return;
	}

	qdf_timer_init(NULL, &rpm_ctx->runtime_timer,
		       hif_pm_runtime_lock_timeout_fn,
		       scn, QDF_TIMER_TYPE_WAKE_APPS);

	hif_info("Enabling RUNTIME PM, Delay: %d ms",
		 scn->hif_config.runtime_pm_delay);

	qdf_atomic_set(&rpm_ctx->pm_state, HIF_PM_RUNTIME_STATE_ON);
	hif_runtime_init(dev, scn->hif_config.runtime_pm_delay);
	hif_runtime_pm_debugfs_create(scn);
}

/**
 * hif_pm_runtime_stop(): stop runtime pm
 * @scn: hif context
 *
 * Turns off runtime pm and frees corresponding resources
 * that were acquired by hif_runtime_pm_start().
 */
void hif_pm_runtime_stop(struct hif_softc *scn)
{
	uint32_t mode = hif_get_conparam(scn);
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (!scn->hif_config.enable_runtime_pm)
		return;

	if (mode == QDF_GLOBAL_FTM_MODE || QDF_IS_EPPING_ENABLED(mode) ||
	    mode == QDF_GLOBAL_MONITOR_MODE)
		return;

	hif_runtime_exit(dev);

	hif_pm_runtime_sync_resume(GET_HIF_OPAQUE_HDL(scn), RTPM_ID_PM_STOP);

	qdf_atomic_set(&rpm_ctx->pm_state, HIF_PM_RUNTIME_STATE_NONE);

	hif_runtime_pm_debugfs_remove(scn);
	qdf_timer_free(&rpm_ctx->runtime_timer);
}

/**
 * hif_pm_runtime_open(): initialize runtime pm
 * @scn: hif ctx
 *
 * Early initialization
 */
void hif_pm_runtime_open(struct hif_softc *scn)
{
	int i;
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	spin_lock_init(&rpm_ctx->runtime_lock);
	qdf_spinlock_create(&rpm_ctx->runtime_suspend_lock);
	qdf_atomic_init(&rpm_ctx->pm_state);
	hif_runtime_lock_init(&rpm_ctx->prevent_linkdown_lock,
			      "prevent_linkdown_lock");
	qdf_atomic_set(&rpm_ctx->pm_state, HIF_PM_RUNTIME_STATE_NONE);
	qdf_atomic_init(&rpm_ctx->pm_stats.runtime_get);
	qdf_atomic_init(&rpm_ctx->pm_stats.runtime_put);
	qdf_atomic_init(&rpm_ctx->pm_stats.allow_suspend);
	qdf_atomic_init(&rpm_ctx->pm_stats.prevent_suspend);
	for (i = 0; i < RTPM_ID_MAX; i++) {
		qdf_atomic_init(&rpm_ctx->pm_stats.runtime_get_dbgid[i]);
		qdf_atomic_init(&rpm_ctx->pm_stats.runtime_put_dbgid[i]);
	}
	INIT_LIST_HEAD(&rpm_ctx->prevent_suspend_list);
}

/**
 * hif_check_for_get_put_out_of_sync() - Check if Get/Put is out of sync
 * @scn: hif context
 *
 * This function will check if get and put are out of sync or not.
 *
 * Return: void
 */
static void  hif_check_for_get_put_out_of_sync(struct hif_softc *scn)
{
	int32_t i;
	int32_t get_count, put_count;
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (qdf_is_fw_down())
		return;

	for (i = 0; i < RTPM_ID_MAX; i++) {
		get_count = qdf_atomic_read(
				&rpm_ctx->pm_stats.runtime_get_dbgid[i]);
		put_count = qdf_atomic_read(
				&rpm_ctx->pm_stats.runtime_put_dbgid[i]);
		if (get_count != put_count) {
			QDF_DEBUG_PANIC("%s get-put out of sync. get %d put %d",
					rtpm_string_from_dbgid(i),
					get_count, put_count);
		}
	}
}

/**
 * hif_pm_runtime_sanitize_on_exit(): sanitize runtime PM gets/puts from driver
 * @scn: hif context
 *
 * Ensure all gets/puts are in sync before exiting runtime PM feature.
 * Also make sure all runtime PM locks are deinitialized properly.
 *
 * Return: void
 */
static void hif_pm_runtime_sanitize_on_exit(struct hif_softc *scn)
{
	struct hif_pm_runtime_lock *ctx, *tmp;
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	hif_check_for_get_put_out_of_sync(scn);

	spin_lock_bh(&rpm_ctx->runtime_lock);
	list_for_each_entry_safe(ctx, tmp,
				 &rpm_ctx->prevent_suspend_list, list) {
		spin_unlock_bh(&rpm_ctx->runtime_lock);
		hif_runtime_lock_deinit(GET_HIF_OPAQUE_HDL(scn), ctx);
		spin_lock_bh(&rpm_ctx->runtime_lock);
	}
	spin_unlock_bh(&rpm_ctx->runtime_lock);
}

static int __hif_pm_runtime_allow_suspend(struct hif_softc *scn,
					  struct hif_pm_runtime_lock *lock);

/**
 * hif_pm_runtime_sanitize_on_ssr_exit() - Empty the suspend list on SSR
 * @scn: hif context
 *
 * API is used to empty the runtime pm prevent suspend list.
 *
 * Return: void
 */
static void hif_pm_runtime_sanitize_on_ssr_exit(struct hif_softc *scn)
{
	struct hif_pm_runtime_lock *ctx, *tmp;
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	spin_lock_bh(&rpm_ctx->runtime_lock);
	list_for_each_entry_safe(ctx, tmp,
				 &rpm_ctx->prevent_suspend_list, list) {
		__hif_pm_runtime_allow_suspend(scn, ctx);
	}
	spin_unlock_bh(&rpm_ctx->runtime_lock);
}

/**
 * hif_pm_runtime_close(): close runtime pm
 * @scn: hif ctx
 *
 * ensure runtime_pm is stopped before closing the driver
 */
void hif_pm_runtime_close(struct hif_softc *scn)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	struct hif_opaque_softc *hif_ctx = GET_HIF_OPAQUE_HDL(scn);

	/*
	 * Here cds hif context was already NULL,
	 * so calling hif_runtime_lock_deinit, instead of
	 * qdf_runtime_lock_deinit(&rpm_ctx->prevent_linkdown_lock);
	 */
	hif_runtime_lock_deinit(hif_ctx, rpm_ctx->prevent_linkdown_lock.lock);

	hif_is_recovery_in_progress(scn) ?
		hif_pm_runtime_sanitize_on_ssr_exit(scn) :
		hif_pm_runtime_sanitize_on_exit(scn);

	qdf_spinlock_destroy(&rpm_ctx->runtime_suspend_lock);
}

/**
 * hif_pm_runtime_sync_resume() - Invoke synchronous runtime resume.
 * @hif_ctx: hif context
 * @rtpm_dbgid: dbgid to trace who use it
 *
 * This function will invoke synchronous runtime resume.
 *
 * Return: status
 */
int hif_pm_runtime_sync_resume(struct hif_opaque_softc *hif_ctx,
			       wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;
	int pm_state;

	if (!scn)
		return -EINVAL;

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	pm_state = qdf_atomic_read(&rpm_ctx->pm_state);
	if (pm_state == HIF_PM_RUNTIME_STATE_SUSPENDED ||
	    pm_state == HIF_PM_RUNTIME_STATE_SUSPENDING)
		hif_info("request runtime PM resume, rtpm_dbgid(%d,%s)",
			 rtpm_dbgid,
			 rtpm_string_from_dbgid(rtpm_dbgid));

	rpm_ctx->pm_stats.request_resume++;
	rpm_ctx->pm_stats.last_resume_rtpm_dbgid = rtpm_dbgid;

	return pm_runtime_resume(hif_bus_get_dev(scn));
}

/**
 * hif_runtime_prevent_linkdown() - prevent or allow a runtime pm from occurring
 * @scn: hif context
 * @flag: prevent linkdown if true otherwise allow
 *
 * this api should only be called as part of bus prevent linkdown
 */
void hif_runtime_prevent_linkdown(struct hif_softc *scn, bool flag)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (flag)
		qdf_runtime_pm_prevent_suspend(&rpm_ctx->prevent_linkdown_lock);
	else
		qdf_runtime_pm_allow_suspend(&rpm_ctx->prevent_linkdown_lock);
}

/**
 * __hif_runtime_pm_set_state(): utility function
 * @state: state to set
 *
 * indexes into the runtime pm state and sets it.
 */
static void __hif_runtime_pm_set_state(struct hif_softc *scn,
				       enum hif_pm_runtime_state state)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (!rpm_ctx) {
		hif_err("HIF_CTX not initialized");
		return;
	}

	qdf_atomic_set(&rpm_ctx->pm_state, state);
}

/**
 * hif_runtime_pm_set_state_on():  adjust runtime pm state
 *
 * Notify hif that a the runtime pm state should be on
 */
static void hif_runtime_pm_set_state_on(struct hif_softc *scn)
{
	__hif_runtime_pm_set_state(scn, HIF_PM_RUNTIME_STATE_ON);
}

/**
 * hif_runtime_pm_set_state_resuming(): adjust runtime pm state
 *
 * Notify hif that a runtime pm resuming has started
 */
static void hif_runtime_pm_set_state_resuming(struct hif_softc *scn)
{
	__hif_runtime_pm_set_state(scn, HIF_PM_RUNTIME_STATE_RESUMING);
}

/**
 * hif_runtime_pm_set_state_suspending(): adjust runtime pm state
 *
 * Notify hif that a runtime pm suspend has started
 */
static void hif_runtime_pm_set_state_suspending(struct hif_softc *scn)
{
	__hif_runtime_pm_set_state(scn, HIF_PM_RUNTIME_STATE_SUSPENDING);
}

/**
 * hif_runtime_pm_set_state_suspended():  adjust runtime pm state
 *
 * Notify hif that a runtime suspend attempt has been completed successfully
 */
static void hif_runtime_pm_set_state_suspended(struct hif_softc *scn)
{
	__hif_runtime_pm_set_state(scn, HIF_PM_RUNTIME_STATE_SUSPENDED);
}

/**
 * hif_log_runtime_suspend_success() - log a successful runtime suspend
 */
static void hif_log_runtime_suspend_success(struct hif_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (!rpm_ctx)
		return;

	rpm_ctx->pm_stats.suspended++;
	rpm_ctx->pm_stats.suspend_jiffies = jiffies;
}

/**
 * hif_log_runtime_suspend_failure() - log a failed runtime suspend
 *
 * log a failed runtime suspend
 * mark last busy to prevent immediate runtime suspend
 */
static void hif_log_runtime_suspend_failure(void *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (!rpm_ctx)
		return;

	rpm_ctx->pm_stats.suspend_err++;
}

/**
 * hif_log_runtime_resume_success() - log a successful runtime resume
 *
 * log a successful runtime resume
 * mark last busy to prevent immediate runtime suspend
 */
static void hif_log_runtime_resume_success(void *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (!rpm_ctx)
		return;

	rpm_ctx->pm_stats.resumed++;
}

/**
 * hif_process_runtime_suspend_failure() - bookkeeping of suspend failure
 *
 * Record the failure.
 * mark last busy to delay a retry.
 * adjust the runtime_pm state.
 */
void hif_process_runtime_suspend_failure(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_log_runtime_suspend_failure(hif_ctx);
	hif_pm_runtime_mark_last_busy(hif_ctx);
	hif_runtime_pm_set_state_on(scn);
}

static bool hif_pm_runtime_is_suspend_allowed(struct hif_softc *scn)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	struct hif_pm_runtime_lock *ctx;
	uint32_t prevent_suspend_cnt;
	char *str_buf;
	bool is_suspend_allowed;
	int len = 0;

	if (!scn->hif_config.enable_runtime_pm)
		return false;

	str_buf = qdf_mem_malloc(PREVENT_LIST_STRING_LEN);
	if (!str_buf)
		return false;

	spin_lock_bh(&rpm_ctx->runtime_lock);
	prevent_suspend_cnt = rpm_ctx->prevent_suspend_cnt;
	is_suspend_allowed = (prevent_suspend_cnt == 0);
	if (!is_suspend_allowed) {
		list_for_each_entry(ctx, &rpm_ctx->prevent_suspend_list, list)
			len += qdf_scnprintf(str_buf + len,
				PREVENT_LIST_STRING_LEN - len,
				"%s ", ctx->name);
	}
	spin_unlock_bh(&rpm_ctx->runtime_lock);

	if (!is_suspend_allowed)
		hif_info("prevent_suspend_cnt %u, prevent_list: %s",
			 rpm_ctx->prevent_suspend_cnt, str_buf);

	qdf_mem_free(str_buf);

	return is_suspend_allowed;
}

void hif_print_runtime_pm_prevent_list(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_pm_runtime_is_suspend_allowed(scn);

	hif_info("Up_linkstate_vote %d", scn->linkstate_vote);
}

/**
 * hif_pre_runtime_suspend() - bookkeeping before beginning runtime suspend
 *
 * Makes sure that the pci link will be taken down by the suspend opperation.
 * If the hif layer is configured to leave the bus on, runtime suspend will
 * not save any power.
 *
 * Set the runtime suspend state to in progress.
 *
 * return -EINVAL if the bus won't go down.  otherwise return 0
 */
int hif_pre_runtime_suspend(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!hif_can_suspend_link(hif_ctx)) {
		hif_err("Runtime PM not supported for link up suspend");
		return -EINVAL;
	}

	hif_runtime_pm_set_state_suspending(scn);

	/* keep this after set suspending */
	if (!hif_pm_runtime_is_suspend_allowed(scn)) {
		hif_info("Runtime PM not allowed now");
		return -EINVAL;
	}

	return 0;
}

/**
 * hif_process_runtime_suspend_success() - bookkeeping of suspend success
 *
 * Record the success.
 * adjust the runtime_pm state
 */
void hif_process_runtime_suspend_success(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_runtime_pm_set_state_suspended(scn);
	hif_log_runtime_suspend_success(scn);
}

/**
 * hif_pre_runtime_resume() - bookkeeping before beginning runtime resume
 *
 * update the runtime pm state.
 */
void hif_pre_runtime_resume(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_pm_runtime_set_monitor_wake_intr(hif_ctx, 0);
	hif_runtime_pm_set_state_resuming(scn);
}

/**
 * hif_process_runtime_resume_success() - bookkeeping after a runtime resume
 *
 * record the success.
 * adjust the runtime_pm state
 */
void hif_process_runtime_resume_success(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_log_runtime_resume_success(hif_ctx);
	hif_pm_runtime_mark_last_busy(hif_ctx);
	hif_runtime_pm_set_state_on(scn);
}

/**
 * hif_runtime_suspend() - do the bus suspend part of a runtime suspend
 *
 * Return: 0 for success and non-zero error code for failure
 */
int hif_runtime_suspend(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	int errno;

	errno = hif_bus_suspend(hif_ctx);
	if (errno) {
		hif_err("Failed bus suspend: %d", errno);
		return errno;
	}

	hif_pm_runtime_set_monitor_wake_intr(hif_ctx, 1);

	errno = hif_bus_suspend_noirq(hif_ctx);
	if (errno) {
		hif_err("Failed bus suspend noirq: %d", errno);
		hif_pm_runtime_set_monitor_wake_intr(hif_ctx, 0);
		goto bus_resume;
	}

	qdf_atomic_set(&rpm_ctx->pm_dp_rx_busy, 0);

	return 0;

bus_resume:
	QDF_BUG(!hif_bus_resume(hif_ctx));

	return errno;
}

/**
 * hif_fastpath_resume() - resume fastpath for runtimepm
 *
 * ensure that the fastpath write index register is up to date
 * since runtime pm may cause ce_send_fast to skip the register
 * write.
 *
 * fastpath only applicable to legacy copy engine
 */
void hif_fastpath_resume(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct CE_state *ce_state;

	if (!scn)
		return;

	if (scn->fastpath_mode_on) {
		if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
			return;

		ce_state = scn->ce_id_to_state[CE_HTT_H2T_MSG];
		qdf_spin_lock_bh(&ce_state->ce_index_lock);

		/*war_ce_src_ring_write_idx_set */
		CE_SRC_RING_WRITE_IDX_SET(scn, ce_state->ctrl_addr,
					  ce_state->src_ring->write_index);
		qdf_spin_unlock_bh(&ce_state->ce_index_lock);
		Q_TARGET_ACCESS_END(scn);
	}
}

/**
 * hif_runtime_resume() - do the bus resume part of a runtime resume
 *
 *  Return: 0 for success and non-zero error code for failure
 */
int hif_runtime_resume(struct hif_opaque_softc *hif_ctx)
{
	int errno;

	QDF_BUG(!hif_bus_resume_noirq(hif_ctx));
	errno = hif_bus_resume(hif_ctx);
	if (errno)
		hif_err("Failed runtime resume: %d", errno);

	return errno;
}

/**
 * hif_pm_stats_runtime_get_record() - record runtime get statistics
 * @scn: hif context
 * @rtpm_dbgid: debug id to trace who use it
 *
 *
 * Return: void
 */
static void hif_pm_stats_runtime_get_record(struct hif_softc *scn,
					    wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (rtpm_dbgid >= RTPM_ID_MAX) {
		QDF_BUG(0);
		return;
	}
	qdf_atomic_inc(&rpm_ctx->pm_stats.runtime_get);
	qdf_atomic_inc(&rpm_ctx->pm_stats.runtime_get_dbgid[rtpm_dbgid]);
	rpm_ctx->pm_stats.runtime_get_timestamp_dbgid[rtpm_dbgid] =
						qdf_get_log_timestamp();
}

/**
 * hif_pm_stats_runtime_put_record() - record runtime put statistics
 * @scn: hif context
 * @rtpm_dbgid: dbg_id to trace who use it
 *
 *
 * Return: void
 */
static void hif_pm_stats_runtime_put_record(struct hif_softc *scn,
					    wlan_rtpm_dbgid rtpm_dbgid)
{
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	if (rtpm_dbgid >= RTPM_ID_MAX) {
		QDF_BUG(0);
		return;
	}

	if (atomic_read(&dev->power.usage_count) <= 0) {
		QDF_BUG(0);
		return;
	}

	qdf_atomic_inc(&rpm_ctx->pm_stats.runtime_put);
	qdf_atomic_inc(&rpm_ctx->pm_stats.runtime_put_dbgid[rtpm_dbgid]);
	rpm_ctx->pm_stats.runtime_put_timestamp_dbgid[rtpm_dbgid] =
						qdf_get_log_timestamp();
}

/**
 * hif_pm_runtime_get_sync() - do a get operation with sync resume
 * @hif_ctx: pointer of HIF context
 * @rtpm_dbgid: dbgid to trace who use it
 *
 * A get operation will prevent a runtime suspend until a corresponding
 * put is done. Unlike hif_pm_runtime_get(), this API will do a sync
 * resume instead of requesting a resume if it is runtime PM suspended
 * so it can only be called in non-atomic context.
 *
 * Return: 0 if it is runtime PM resumed otherwise an error code.
 */
int hif_pm_runtime_get_sync(struct hif_opaque_softc *hif_ctx,
			    wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	int pm_state;
	int ret;

	if (!rpm_ctx)
		return -EINVAL;

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	pm_state = qdf_atomic_read(&rpm_ctx->pm_state);
	if (pm_state == HIF_PM_RUNTIME_STATE_SUSPENDED ||
	    pm_state == HIF_PM_RUNTIME_STATE_SUSPENDING)
		hif_info_high("Runtime PM resume is requested by %ps",
			      (void *)_RET_IP_);

	hif_pm_stats_runtime_get_record(scn, rtpm_dbgid);
	ret = pm_runtime_get_sync(dev);

	/* Get can return 1 if the device is already active, just return
	 * success in that case.
	 */
	if (ret > 0)
		ret = 0;

	if (ret) {
		rpm_ctx->pm_stats.runtime_get_err++;
		hif_err("Runtime PM Get Sync error in pm_state: %d, ret: %d",
			qdf_atomic_read(&rpm_ctx->pm_state), ret);
		hif_pm_runtime_put(hif_ctx, rtpm_dbgid);
	}

	return ret;
}

/**
 * hif_pm_runtime_put_sync_suspend() - do a put operation with sync suspend
 * @hif_ctx: pointer of HIF context
 * @rtpm_dbgid: dbgid to trace who use it
 *
 * This API will do a runtime put operation followed by a sync suspend if usage
 * count is 0 so it can only be called in non-atomic context.
 *
 * Return: 0 for success otherwise an error code
 */
int hif_pm_runtime_put_sync_suspend(struct hif_opaque_softc *hif_ctx,
				    wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct device *dev;
	int usage_count;
	char *err = NULL;

	if (!scn)
		return -EINVAL;

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	dev = hif_bus_get_dev(scn);
	usage_count = atomic_read(&dev->power.usage_count);
	if (usage_count == 2 && !scn->hif_config.enable_runtime_pm)
		err = "Uexpected PUT when runtime PM is disabled";
	else if (usage_count == 0)
		err = "PUT without a GET Operation";

	if (err) {
		hif_pci_runtime_pm_warn(scn, err);
		return -EINVAL;
	}

	hif_pm_stats_runtime_put_record(scn, rtpm_dbgid);
	return pm_runtime_put_sync_suspend(dev);
}

/**
 * hif_pm_runtime_request_resume() - Invoke async runtime resume
 * @hif_ctx: hif context
 * @rtpm_dbgid: dbgid to trace who use it
 *
 * This function will invoke asynchronous runtime resume.
 *
 * Return: status
 */
int hif_pm_runtime_request_resume(struct hif_opaque_softc *hif_ctx,
				  wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;
	int pm_state;

	if (!scn)
		return -EINVAL;

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	pm_state = qdf_atomic_read(&rpm_ctx->pm_state);
	if (pm_state == HIF_PM_RUNTIME_STATE_SUSPENDED ||
	    pm_state == HIF_PM_RUNTIME_STATE_SUSPENDING)
		hif_info("request runtime PM resume, rtpm_dbgid(%d,%s)",
			 rtpm_dbgid,
			 rtpm_string_from_dbgid(rtpm_dbgid));

	rpm_ctx->pm_stats.request_resume++;
	rpm_ctx->pm_stats.last_resume_rtpm_dbgid = rtpm_dbgid;

	return hif_pm_request_resume(hif_bus_get_dev(scn));
}

/**
 * hif_pm_runtime_mark_last_busy() - Mark last busy time
 * @hif_ctx: hif context
 *
 * This function will mark the last busy time, this will be used
 * to check if auto suspend delay expired or not.
 *
 * Return: void
 */
void hif_pm_runtime_mark_last_busy(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;

	if (!scn)
		return;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	rpm_ctx->pm_stats.last_busy_marker = (void *)_RET_IP_;
	rpm_ctx->pm_stats.last_busy_timestamp = qdf_get_log_timestamp_usecs();

	pm_runtime_mark_last_busy(hif_bus_get_dev(scn));

	return;
}

/**
 * hif_pm_runtime_get_noresume() - Inc usage count without resume
 * @hif_ctx: hif context
 * rtpm_dbgid: Id of the module calling get
 *
 * This function will increment device usage count to avoid runtime
 * suspend, but it would not do resume.
 *
 * Return: void
 */
void hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx,
				 wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn)
		return;

	if (!hif_pci_pm_runtime_enabled(scn))
		return;

	hif_pm_stats_runtime_get_record(scn, rtpm_dbgid);
	pm_runtime_get_noresume(hif_bus_get_dev(scn));
}

/**
 * hif_pm_runtime_get() - do a get opperation on the device
 * @hif_ctx: pointer of HIF context
 * @rtpm_dbgid: dbgid to trace who use it
 * @is_critical_ctx: Indication if this function called via a
 *		     critical context
 *
 * A get opperation will prevent a runtime suspend until a
 * corresponding put is done.  This api should be used when sending
 * data.
 *
 * CONTRARY TO THE REGULAR RUNTIME PM, WHEN THE BUS IS SUSPENDED,
 * THIS API WILL ONLY REQUEST THE RESUME AND NOT TO A GET!!!
 *
 * return: success if the bus is up and a get has been issued
 *   otherwise an error code.
 */
int hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx,
		       wlan_rtpm_dbgid rtpm_dbgid,
		       bool is_critical_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;
	struct device *dev;
	int ret;
	int pm_state;

	if (!scn) {
		hif_err("Could not do runtime get, scn is null");
		return -EFAULT;
	}

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	dev = hif_bus_get_dev(scn);
	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	pm_state = qdf_atomic_read(&rpm_ctx->pm_state);

	if (pm_state  == HIF_PM_RUNTIME_STATE_ON ||
	    pm_state == HIF_PM_RUNTIME_STATE_NONE) {
		hif_pm_stats_runtime_get_record(scn, rtpm_dbgid);
		ret = __hif_pm_runtime_get(dev);

		/* Get can return 1 if the device is already active, just return
		 * success in that case
		 */
		if (ret > 0)
			ret = 0;

		if (ret < 0)
			hif_pm_runtime_put(hif_ctx, rtpm_dbgid);

		if (ret && ret != -EINPROGRESS) {
			rpm_ctx->pm_stats.runtime_get_err++;
			hif_err("Runtime Get PM Error in pm_state:%d ret: %d",
				qdf_atomic_read(&rpm_ctx->pm_state), ret);
		}

		return ret;
	}

	if (pm_state == HIF_PM_RUNTIME_STATE_SUSPENDED ||
	    pm_state == HIF_PM_RUNTIME_STATE_SUSPENDING) {
		/* Do not log in performance path */
		if (!is_critical_ctx) {
			hif_info_high("request runtime PM resume, rtpm_dbgid(%d-%s)",
				      rtpm_dbgid,
				      rtpm_string_from_dbgid(rtpm_dbgid));
		}
		ret = -EAGAIN;
	} else {
		ret = -EBUSY;
	}

	rpm_ctx->pm_stats.request_resume++;
	rpm_ctx->pm_stats.last_resume_rtpm_dbgid = rtpm_dbgid;
	hif_pm_request_resume(dev);

	return ret;
}

/**
 * hif_pm_runtime_put() - do a put operation on the device
 * @hif_ctx: pointer of HIF context
 * @rtpm_dbgid: dbgid to trace who use it
 *
 * A put operation will allow a runtime suspend after a corresponding
 * get was done.  This api should be used when sending data.
 *
 * This api will return a failure if runtime pm is stopped
 * This api will return failure if it would decrement the usage count below 0.
 *
 * return: QDF_STATUS_SUCCESS if the put is performed
 */
int hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx,
		       wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct device *dev;
	int usage_count;
	char *error = NULL;

	if (!scn) {
		hif_err("Could not do runtime put, scn is null");
		return -EFAULT;
	}

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	dev = hif_bus_get_dev(scn);
	usage_count = atomic_read(&dev->power.usage_count);
	if (usage_count == 2 && !scn->hif_config.enable_runtime_pm)
		error = "Unexpected PUT when runtime PM is disabled";
	else if (usage_count == 0)
		error = "PUT without a GET operation";

	if (error) {
		hif_pci_runtime_pm_warn(scn, error);
		return -EINVAL;
	}

	hif_pm_stats_runtime_put_record(scn, rtpm_dbgid);

	hif_pm_runtime_mark_last_busy(hif_ctx);
	hif_pm_runtime_put_auto(dev);

	return 0;
}

/**
 * hif_pm_runtime_put_noidle() - do a put operation with no idle
 * @hif_ctx: pointer of HIF context
 * @rtpm_dbgid: dbgid to trace who use it
 *
 * This API will do a runtime put no idle operation
 *
 * Return: 0 for success otherwise an error code
 */
int hif_pm_runtime_put_noidle(struct hif_opaque_softc *hif_ctx,
			      wlan_rtpm_dbgid rtpm_dbgid)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct device *dev;
	int usage_count;
	char *err = NULL;

	if (!scn)
		return -EINVAL;

	if (!hif_pci_pm_runtime_enabled(scn))
		return 0;

	dev = hif_bus_get_dev(scn);
	usage_count = atomic_read(&dev->power.usage_count);
	if (usage_count == 2 && !scn->hif_config.enable_runtime_pm)
		err = "Unexpected PUT when runtime PM is disabled";
	else if (usage_count == 0)
		err = "PUT without a GET operation";

	if (err) {
		hif_pci_runtime_pm_warn(scn, err);
		return -EINVAL;
	}

	hif_pm_stats_runtime_put_record(scn, rtpm_dbgid);
	pm_runtime_put_noidle(dev);

	return 0;
}

/**
 * __hif_pm_runtime_prevent_suspend() - prevent runtime suspend for a protocol
 *                                      reason
 * @scn: hif context
 * @lock: runtime_pm lock being acquired
 *
 * Return 0 if successful.
 */
static int __hif_pm_runtime_prevent_suspend(struct hif_softc *scn,
					    struct hif_pm_runtime_lock *lock)
{
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	int ret = 0;

	/*
	 * We shouldn't be setting context->timeout to zero here when
	 * context is active as we will have a case where Timeout API's
	 * for the same context called back to back.
	 * eg: echo "1=T:10:T:20" > /d/cnss_runtime_pm
	 * Set context->timeout to zero in hif_pm_runtime_prevent_suspend
	 * API to ensure the timeout version is no more active and
	 * list entry of this context will be deleted during allow suspend.
	 */
	if (lock->active)
		return 0;

	ret = __hif_pm_runtime_get(dev);
	hif_debug("request runtime PM resume, rtpm_dbgid %s", lock->name);

	/**
	 * The ret can be -EINPROGRESS, if Runtime status is RPM_RESUMING or
	 * RPM_SUSPENDING. Any other negative value is an error.
	 * We shouldn't be do runtime_put here as in later point allow
	 * suspend gets called with the the context and there the usage count
	 * is decremented, so suspend will be prevented.
	 */

	if (ret < 0 && ret != -EINPROGRESS) {
		rpm_ctx->pm_stats.runtime_get_err++;
		hif_pci_runtime_pm_warn(scn,
					"Prevent Suspend Runtime PM Error");
	}

	rpm_ctx->prevent_suspend_cnt++;

	lock->active = true;

	list_add_tail(&lock->list, &rpm_ctx->prevent_suspend_list);

	qdf_atomic_inc(&rpm_ctx->pm_stats.prevent_suspend);

	hif_debug("%s: in pm_state:%s ret: %d", __func__,
		  hif_pm_runtime_state_to_string(
			  qdf_atomic_read(&rpm_ctx->pm_state)),
		  ret);

	return ret;
}

/**
 * __hif_pm_runtime_allow_suspend() - Allow Runtime suspend
 * @scn: hif context
 * @lock: runtime pm lock
 *
 * This function will allow runtime suspend, by decrementing
 * device's usage count.
 *
 * Return: status
 */
static int __hif_pm_runtime_allow_suspend(struct hif_softc *scn,
					  struct hif_pm_runtime_lock *lock)
{
	struct device *dev = hif_bus_get_dev(scn);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	int ret = 0;
	int usage_count;

	if (rpm_ctx->prevent_suspend_cnt == 0)
		return ret;

	if (!lock->active)
		return ret;

	usage_count = atomic_read(&dev->power.usage_count);

	/*
	 * For runtime PM enabled case, the usage count should never be 0
	 * at this point. For runtime PM disabled case, it should never be
	 * 2 at this point. Catch unexpected PUT without GET here.
	 */
	if ((usage_count == 2 && !scn->hif_config.enable_runtime_pm) ||
	    usage_count == 0) {
		hif_pci_runtime_pm_warn(scn, "PUT without a GET Operation");
		return -EINVAL;
	}

	list_del(&lock->list);

	rpm_ctx->prevent_suspend_cnt--;

	lock->active = false;
	lock->timeout = 0;

	hif_pm_runtime_mark_last_busy(GET_HIF_OPAQUE_HDL(scn));
	ret = hif_pm_runtime_put_auto(dev);

	hif_debug("%s: in pm_state:%s ret: %d", __func__,
		  hif_pm_runtime_state_to_string(
			  qdf_atomic_read(&rpm_ctx->pm_state)),
		  ret);

	qdf_atomic_inc(&rpm_ctx->pm_stats.allow_suspend);
	return ret;
}

/**
 * hif_pm_runtime_lock_timeout_fn() - callback the runtime lock timeout
 * @data: calback data that is the pci context
 *
 * if runtime locks are acquired with a timeout, this function releases
 * the locks when the last runtime lock expires.
 *
 * dummy implementation until lock acquisition is implemented.
 */
static void hif_pm_runtime_lock_timeout_fn(void *data)
{
	struct hif_softc *scn = data;
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	unsigned long timer_expires;
	struct hif_pm_runtime_lock *context, *temp;

	spin_lock_bh(&rpm_ctx->runtime_lock);

	timer_expires = rpm_ctx->runtime_timer_expires;

	/* Make sure we are not called too early, this should take care of
	 * following case
	 *
	 * CPU0                         CPU1 (timeout function)
	 * ----                         ----------------------
	 * spin_lock_irq
	 *                              timeout function called
	 *
	 * mod_timer()
	 *
	 * spin_unlock_irq
	 *                              spin_lock_irq
	 */
	if (timer_expires > 0 && !time_after(timer_expires, jiffies)) {
		rpm_ctx->runtime_timer_expires = 0;
		list_for_each_entry_safe(context, temp,
					 &rpm_ctx->prevent_suspend_list, list) {
			if (context->timeout) {
				__hif_pm_runtime_allow_suspend(scn, context);
				rpm_ctx->pm_stats.allow_suspend_timeout++;
			}
		}
	}

	spin_unlock_bh(&rpm_ctx->runtime_lock);
}

/**
 * hif_pm_runtime_prevent_suspend() - Prevent Runtime suspend
 * @scn: hif context
 * @data: runtime pm lock
 *
 * This function will prevent runtime suspend, by incrementing
 * device's usage count.
 *
 * Return: status
 */
int hif_pm_runtime_prevent_suspend(struct hif_opaque_softc *ol_sc,
				   struct hif_pm_runtime_lock *data)
{
	struct hif_softc *scn = HIF_GET_SOFTC(ol_sc);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	struct hif_pm_runtime_lock *context = data;

	if (!scn->hif_config.enable_runtime_pm)
		return 0;

	if (!context)
		return -EINVAL;

	if (in_irq())
		WARN_ON(1);

	spin_lock_bh(&rpm_ctx->runtime_lock);
	context->timeout = 0;
	__hif_pm_runtime_prevent_suspend(scn, context);
	spin_unlock_bh(&rpm_ctx->runtime_lock);

	return 0;
}

/**
 * hif_pm_runtime_allow_suspend() - Allow Runtime suspend
 * @scn: hif context
 * @data: runtime pm lock
 *
 * This function will allow runtime suspend, by decrementing
 * device's usage count.
 *
 * Return: status
 */
int hif_pm_runtime_allow_suspend(struct hif_opaque_softc *ol_sc,
				 struct hif_pm_runtime_lock *data)
{
	struct hif_softc *scn = HIF_GET_SOFTC(ol_sc);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);
	struct hif_pm_runtime_lock *context = data;

	if (!scn->hif_config.enable_runtime_pm)
		return 0;

	if (!context)
		return -EINVAL;

	if (in_irq())
		WARN_ON(1);

	spin_lock_bh(&rpm_ctx->runtime_lock);

	__hif_pm_runtime_allow_suspend(scn, context);

	/* The list can be empty as well in cases where
	 * we have one context in the list and the allow
	 * suspend came before the timer expires and we delete
	 * context above from the list.
	 * When list is empty prevent_suspend count will be zero.
	 */
	if (rpm_ctx->prevent_suspend_cnt == 0 &&
	    rpm_ctx->runtime_timer_expires > 0) {
		qdf_timer_free(&rpm_ctx->runtime_timer);
		rpm_ctx->runtime_timer_expires = 0;
	}

	spin_unlock_bh(&rpm_ctx->runtime_lock);

	return 0;
}

/**
 * hif_runtime_lock_init() - API to initialize Runtime PM context
 * @name: Context name
 *
 * This API initializes the Runtime PM context of the caller and
 * return the pointer.
 *
 * Return: None
 */
int hif_runtime_lock_init(qdf_runtime_lock_t *lock, const char *name)
{
	struct hif_pm_runtime_lock *context;

	hif_debug("Initializing Runtime PM wakelock %s", name);

	context = qdf_mem_malloc(sizeof(*context));
	if (!context)
		return -ENOMEM;

	context->name = name ? name : "Default";
	lock->lock = context;

	return 0;
}

/**
 * hif_runtime_lock_deinit() - This API frees the runtime pm ctx
 * @data: Runtime PM context
 *
 * Return: void
 */
void hif_runtime_lock_deinit(struct hif_opaque_softc *hif_ctx,
			     struct hif_pm_runtime_lock *data)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;
	struct hif_pm_runtime_lock *context = data;

	if (!context) {
		hif_err("Runtime PM wakelock context is NULL");
		return;
	}

	hif_debug("Deinitializing Runtime PM wakelock %s", context->name);

	/*
	 * Ensure to delete the context list entry and reduce the usage count
	 * before freeing the context if context is active.
	 */
	if (scn) {
		rpm_ctx = hif_bus_get_rpm_ctx(scn);
		spin_lock_bh(&rpm_ctx->runtime_lock);
		__hif_pm_runtime_allow_suspend(scn, context);
		spin_unlock_bh(&rpm_ctx->runtime_lock);
	}

	qdf_mem_free(context);
}

/**
 * hif_pm_runtime_is_suspended() - API to check if driver has runtime suspended
 * @hif_ctx: HIF context
 *
 * Return: true for runtime suspended, otherwise false
 */
bool hif_pm_runtime_is_suspended(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	return qdf_atomic_read(&rpm_ctx->pm_state) ==
					HIF_PM_RUNTIME_STATE_SUSPENDED;
}

/*
 * hif_pm_runtime_suspend_lock() - spin_lock on marking runtime suspend
 * @hif_ctx: HIF context
 *
 * Return: void
 */
void hif_pm_runtime_suspend_lock(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	qdf_spin_lock_irqsave(&rpm_ctx->runtime_suspend_lock);
}

/*
 * hif_pm_runtime_suspend_unlock() - spin_unlock on marking runtime suspend
 * @hif_ctx: HIF context
 *
 * Return: void
 */
void hif_pm_runtime_suspend_unlock(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	qdf_spin_unlock_irqrestore(&rpm_ctx->runtime_suspend_lock);
}

/**
 * hif_pm_runtime_get_monitor_wake_intr() - API to get monitor_wake_intr
 * @hif_ctx: HIF context
 *
 * monitor_wake_intr variable can be used to indicate if driver expects wake
 * MSI for runtime PM
 *
 * Return: monitor_wake_intr variable
 */
int hif_pm_runtime_get_monitor_wake_intr(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	return qdf_atomic_read(&rpm_ctx->monitor_wake_intr);
}

/**
 * hif_pm_runtime_set_monitor_wake_intr() - API to set monitor_wake_intr
 * @hif_ctx: HIF context
 * @val: value to set
 *
 * monitor_wake_intr variable can be used to indicate if driver expects wake
 * MSI for runtime PM
 *
 * Return: void
 */
void hif_pm_runtime_set_monitor_wake_intr(struct hif_opaque_softc *hif_ctx,
					  int val)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx = hif_bus_get_rpm_ctx(scn);

	qdf_atomic_set(&rpm_ctx->monitor_wake_intr, val);
}

/**
 * hif_pm_runtime_check_and_request_resume() - check if the device is runtime
 *					       suspended and request resume.
 * @hif_ctx: HIF context
 *
 * This function is to check if the device is runtime suspended and
 * request for runtime resume.
 *
 * Return: void
 */
void hif_pm_runtime_check_and_request_resume(struct hif_opaque_softc *hif_ctx)
{
	hif_pm_runtime_suspend_lock(hif_ctx);
	if (hif_pm_runtime_is_suspended(hif_ctx)) {
		hif_pm_runtime_suspend_unlock(hif_ctx);
		hif_pm_runtime_request_resume(hif_ctx, RTPM_ID_CE_INTR_HANDLER);
	} else {
		hif_pm_runtime_suspend_unlock(hif_ctx);
	}
}

/**
 * hif_pm_runtime_mark_dp_rx_busy() - Set last busy mark my data path
 * @hif_ctx: HIF context
 *
 * Return: void
 */
void hif_pm_runtime_mark_dp_rx_busy(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;

	if (!scn)
		return;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	qdf_atomic_set(&rpm_ctx->pm_dp_rx_busy, 1);
	rpm_ctx->dp_last_busy_timestamp = qdf_get_log_timestamp_usecs();

	hif_pm_runtime_mark_last_busy(hif_ctx);
}

/**
 * hif_pm_runtime_is_dp_rx_busy() - Check if last mark busy by dp rx
 * @hif_ctx: HIF context
 *
 * Return: dp rx busy set value
 */
int hif_pm_runtime_is_dp_rx_busy(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;

	if (!scn)
		return 0;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	return qdf_atomic_read(&rpm_ctx->pm_dp_rx_busy);
}

/**
 * hif_pm_runtime_get_dp_rx_busy_mark() - Get last busy by dp rx timestamp
 * @hif_ctx: HIF context
 *
 * Return: timestamp of last mark busy by dp rx
 */
qdf_time_t hif_pm_runtime_get_dp_rx_busy_mark(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;

	if (!scn)
		return 0;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	return rpm_ctx->dp_last_busy_timestamp;
}

void hif_pm_set_link_state(struct hif_opaque_softc *hif_handle, uint8_t val)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_handle);

	qdf_atomic_set(&scn->pm_link_state, val);
}

uint8_t hif_pm_get_link_state(struct hif_opaque_softc *hif_handle)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_handle);

	return qdf_atomic_read(&scn->pm_link_state);
}

/**
 * hif_pm_runtime_update_stats() - API to update RTPM stats for HTC layer
 * @scn: hif context
 * @rtpm_dbgid: RTPM dbg_id
 * @hif_pm_htc_stats: Stats category
 *
 * Return: void
 */
void hif_pm_runtime_update_stats(struct hif_opaque_softc *hif_ctx,
				 wlan_rtpm_dbgid rtpm_dbgid,
				 enum hif_pm_htc_stats stats)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct hif_runtime_pm_ctx *rpm_ctx;

	if (rtpm_dbgid != RTPM_ID_HTC)
		return;

	if (!scn)
		return;

	if (!hif_pci_pm_runtime_enabled(scn))
		return;

	rpm_ctx = hif_bus_get_rpm_ctx(scn);
	if (!rpm_ctx)
		return;

	switch (stats) {
	case HIF_PM_HTC_STATS_GET_HTT_RESPONSE:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_get_htt_resp++;
		break;
	case HIF_PM_HTC_STATS_GET_HTT_NO_RESPONSE:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_get_htt_no_resp++;
		break;
	case HIF_PM_HTC_STATS_PUT_HTT_RESPONSE:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_put_htt_resp++;
		break;
	case HIF_PM_HTC_STATS_PUT_HTT_NO_RESPONSE:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_put_htt_no_resp++;
		break;
	case HIF_PM_HTC_STATS_PUT_HTT_ERROR:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_put_htt_error++;
		break;
	case HIF_PM_HTC_STATS_PUT_HTC_CLEANUP:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_put_htc_cleanup++;
		break;
	case HIF_PM_HTC_STATS_GET_HTC_KICK_QUEUES:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_get_htc_kick_queues++;
		break;
	case HIF_PM_HTC_STATS_PUT_HTC_KICK_QUEUES:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_put_htc_kick_queues++;
		break;
	case HIF_PM_HTC_STATS_GET_HTT_FETCH_PKTS:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_get_htt_fetch_pkts++;
		break;
	case HIF_PM_HTC_STATS_PUT_HTT_FETCH_PKTS:
		rpm_ctx->pm_stats.pm_stats_htc.rtpm_put_htt_fetch_pkts++;
		break;
	default:
		break;
	}
}

#endif /* FEATURE_RUNTIME_PM */
