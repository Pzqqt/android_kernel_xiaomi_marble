/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 */

#ifndef __MMRM_DEBUG__
#define __MMRM_DEBUG__

#include <linux/debugfs.h>
#include <linux/printk.h>

#ifndef MMRM_DBG_LABEL
#define MMRM_DBG_LABEL "msm_mmrm"
#endif

#define MMRM_DBG_TAG MMRM_DBG_LABEL ": %4s: "

/* To enable messages OR these values and
 * echo the result to debugfs file.
 */
enum mmrm_msg_prio {
	MMRM_ERR = 0x000001,
	MMRM_HIGH = 0x000002,
	MMRM_LOW = 0x000004,
	MMRM_WARN = 0x000008,
	MMRM_POWER = 0x000010,
	MMRM_PRINTK = 0x010000,
	MMRM_FTRACE = 0x020000,
};

extern int msm_mmrm_debug;
extern u8 msm_mmrm_allow_multiple_register;

#define dprintk(__level, __fmt, ...) \
	do { \
		if (msm_mmrm_debug & __level) { \
			if (msm_mmrm_debug & MMRM_PRINTK) { \
				pr_info(MMRM_DBG_TAG __fmt, \
					get_debug_level_str(__level), \
					##__VA_ARGS__); \
			} \
		} \
	} while (0)

#define d_mpr_e(__fmt, ...) dprintk(MMRM_ERR, __fmt, ##__VA_ARGS__)
#define d_mpr_h(__fmt, ...) dprintk(MMRM_HIGH, __fmt, ##__VA_ARGS__)
#define d_mpr_l(__fmt, ...) dprintk(MMRM_LOW, __fmt, ##__VA_ARGS__)
#define d_mpr_w(__fmt, ...) dprintk(MMRM_WARN, __fmt, ##__VA_ARGS__)
#define d_mpr_p(__fmt, ...) dprintk(MMRM_POWER, __fmt, ##__VA_ARGS__)

static inline char *get_debug_level_str(int level)
{
	switch (level) {
	case MMRM_ERR:
		return "err ";
	case MMRM_HIGH:
		return "high";
	case MMRM_LOW:
		return "low ";
	case MMRM_WARN:
		return "warn";
	case MMRM_POWER:
		return "power";
	default:
		return "????";
	}
}

struct dentry *msm_mmrm_debugfs_init(void);
void msm_mmrm_debugfs_deinit(struct dentry *dir);

#endif
