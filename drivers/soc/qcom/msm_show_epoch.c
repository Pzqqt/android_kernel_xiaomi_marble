// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include <linux/module.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <trace/hooks/epoch.h>

// CST = UTC+8, offset = 3600*8 = 28800
#define CST_PRINT(tag) \
	do { \
		struct timespec64 ts; \
		struct tm tm; \
		ktime_get_real_ts64(&ts); \
		time64_to_tm(ts.tv_sec, 28800, &tm); \
		pr_warn("%s [%ld-%02d-%02d %02d:%02d:%02d.%06lu]\n", \
			tag, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, \
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec/1000); \
	} while(0)

static u64 suspend_ns;
static u64 suspend_cycles;
static u64 resume_cycles;

static void msm_show_suspend_epoch_val(void *data, u64 ns, u64 cycles)
{
	suspend_ns = ns;
	suspend_cycles = cycles;
	CST_PRINT("Suspend at");
	pr_info("suspend ns:%17llu      suspend cycles:%17llu\n",
						suspend_ns, suspend_cycles);
}

static void msm_show_resume_epoch_val(void *data, u64 cycles)
{
	resume_cycles = cycles;
	CST_PRINT("Resume at");
	pr_info("resume cycles:%17llu\n", resume_cycles);
}

static int __init msm_show_epoch_init(void)
{
	register_trace_android_vh_show_suspend_epoch_val(
					msm_show_suspend_epoch_val, NULL);
	register_trace_android_vh_show_resume_epoch_val(
					msm_show_resume_epoch_val, NULL);

	return 0;
}

#if IS_MODULE(CONFIG_SHOW_SUSPEND_EPOCH)
module_init(msm_show_epoch_init);
#else
pure_initcall(msm_show_epoch_init);
#endif

MODULE_DESCRIPTION("Qualcomm Technologies, Inc. show epoch values driver");
MODULE_LICENSE("GPL v2");
