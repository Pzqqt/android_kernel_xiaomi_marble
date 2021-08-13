/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_RUNTIME_PM_H__
#define __HIF_RUNTIME_PM_H__

#ifdef FEATURE_RUNTIME_PM

/**
 * enum hif_pm_runtime_state - Driver States for Runtime Power Management
 * HIF_PM_RUNTIME_STATE_NONE: runtime pm is off
 * HIF_PM_RUNTIME_STATE_ON: runtime pm is active and link is active
 * HIF_PM_RUNTIME_STATE_RESUMING: a runtime resume is in progress
 * HIF_PM_RUNTIME_STATE_SUSPENDING: a runtime suspend is in progress
 * HIF_PM_RUNTIME_STATE_SUSPENDED: the driver is runtime suspended
 */
enum hif_pm_runtime_state {
	HIF_PM_RUNTIME_STATE_NONE,
	HIF_PM_RUNTIME_STATE_ON,
	HIF_PM_RUNTIME_STATE_RESUMING,
	HIF_PM_RUNTIME_STATE_SUSPENDING,
	HIF_PM_RUNTIME_STATE_SUSPENDED,
};

/**
 * struct hif_pm_runtime_lock - data structure for preventing runtime suspend
 * @list - global list of runtime locks
 * @active - true if this lock is preventing suspend
 * @name - character string for tracking this lock
 */
struct hif_pm_runtime_lock {
	struct list_head list;
	bool active;
	uint32_t timeout;
	const char *name;
};

/* Debugging stats for RTPM for HTC layer */
struct hif_pm_runtime_htc_stats {
	uint32_t rtpm_get_htt_resp;
	uint32_t rtpm_get_htt_no_resp;
	uint32_t rtpm_put_htt_resp;
	uint32_t rtpm_put_htt_no_resp;
	uint32_t rtpm_put_htt_error;
	uint32_t rtpm_put_htc_cleanup;
	uint32_t rtpm_get_htc_kick_queues;
	uint32_t rtpm_put_htc_kick_queues;
	uint32_t rtpm_get_htt_fetch_pkts;
	uint32_t rtpm_put_htt_fetch_pkts;
};

/* Debugging stats for Runtime PM */
struct hif_pci_pm_stats {
	u32 suspended;
	u32 suspend_err;
	u32 resumed;
	atomic_t runtime_get;
	atomic_t runtime_put;
	atomic_t runtime_get_dbgid[RTPM_ID_MAX];
	atomic_t runtime_put_dbgid[RTPM_ID_MAX];
	uint64_t runtime_get_timestamp_dbgid[RTPM_ID_MAX];
	uint64_t runtime_put_timestamp_dbgid[RTPM_ID_MAX];
	u32 request_resume;
	atomic_t allow_suspend;
	atomic_t prevent_suspend;
	u32 prevent_suspend_timeout;
	u32 allow_suspend_timeout;
	u32 runtime_get_err;
	wlan_rtpm_dbgid last_resume_rtpm_dbgid;
	void *last_busy_marker;
	qdf_time_t last_busy_timestamp;
	unsigned long suspend_jiffies;
	struct hif_pm_runtime_htc_stats pm_stats_htc;
};

struct hif_runtime_pm_ctx {
	atomic_t pm_state;
	atomic_t monitor_wake_intr;
	uint32_t prevent_suspend_cnt;
	struct hif_pci_pm_stats pm_stats;
	struct work_struct pm_work;
	spinlock_t runtime_lock;	/* Generic spinlock for Runtime PM */
	qdf_spinlock_t runtime_suspend_lock;
	qdf_timer_t runtime_timer;
	struct list_head prevent_suspend_list;
	unsigned long runtime_timer_expires;
	qdf_runtime_lock_t prevent_linkdown_lock;
	atomic_t pm_dp_rx_busy;
	qdf_time_t dp_last_busy_timestamp;
#ifdef WLAN_OPEN_SOURCE
	struct dentry *pm_dentry;
#endif
};

#include <linux/pm_runtime.h>

static inline int hif_pm_request_resume(struct device *dev)
{
	return pm_request_resume(dev);
}

static inline int __hif_pm_runtime_get(struct device *dev)
{
	return pm_runtime_get(dev);
}

static inline int hif_pm_runtime_put_auto(struct device *dev)
{
	return pm_runtime_put_autosuspend(dev);
}

void hif_pm_runtime_open(struct hif_softc *scn);
void hif_pm_runtime_start(struct hif_softc *scn);
void hif_pm_runtime_stop(struct hif_softc *scn);
void hif_pm_runtime_close(struct hif_softc *scn);
void hif_runtime_prevent_linkdown(struct hif_softc *scn, bool flag);

#else
static inline void hif_pm_runtime_open(struct hif_softc *scn) {}
static inline void hif_pm_runtime_start(struct hif_softc *scn) {}
static inline void hif_pm_runtime_stop(struct hif_softc *scn) {}
static inline void hif_pm_runtime_close(struct hif_softc *scn) {}
static inline void hif_runtime_prevent_linkdown(struct hif_softc *scn,
						bool flag) {}
#endif /* FEATURE_RUNTIME_PM */
#endif /* __HIF_RUNTIME_PM_H__ */
