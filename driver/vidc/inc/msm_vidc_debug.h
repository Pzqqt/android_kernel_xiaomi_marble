/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __MSM_VIDC_DEBUG__
#define __MSM_VIDC_DEBUG__

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/types.h>

#ifndef VIDC_DBG_LABEL
#define VIDC_DBG_LABEL "msm_vidc"
#endif

/* Allow only 6 prints/sec */
#define VIDC_DBG_SESSION_RATELIMIT_INTERVAL (1 * HZ)
#define VIDC_DBG_SESSION_RATELIMIT_BURST 6

#define VIDC_DBG_TAG_INST VIDC_DBG_LABEL ": %4s: %s: "
#define VIDC_DBG_TAG_CORE VIDC_DBG_LABEL ": %4s: %08x: %s: "
#define FW_DBG_TAG VIDC_DBG_LABEL ": %6s: "
#define DEFAULT_SID ((u32)-1)

#ifndef MSM_VIDC_EMPTY_BRACE
#define MSM_VIDC_EMPTY_BRACE {},
#endif

extern int msm_vidc_debug;
extern bool msm_vidc_lossless_encode;
extern bool msm_vidc_syscache_disable;
extern int msm_vidc_clock_voting;

/* To enable messages OR these values and
 * echo the result to debugfs file.
 *
 * To enable all messages set debug_level = 0x101F
 */

enum vidc_msg_prio {
	VIDC_ERR        = 0x00000001,
	VIDC_HIGH       = 0x00000002,
	VIDC_LOW        = 0x00000004,
	VIDC_PERF       = 0x00000008,
	VIDC_PKT        = 0x00000010,
	VIDC_BUS        = 0x00000020,
	VIDC_ENCODER    = 0x00000100,
	VIDC_DECODER    = 0x00000200,
	VIDC_PRINTK     = 0x00001000,
	VIDC_FTRACE     = 0x00002000,
	FW_LOW          = 0x00010000,
	FW_MED          = 0x00020000,
	FW_HIGH         = 0x00040000,
	FW_ERROR        = 0x00080000,
	FW_FATAL        = 0x00100000,
	FW_PERF         = 0x00200000,
	FW_PRINTK       = 0x10000000,
	FW_FTRACE       = 0x20000000,
};
#define FW_LOGSHIFT    16
#define FW_LOGMASK     0x0FFF0000

#define dprintk_inst(__level, __level_str, inst, __fmt, ...) \
	do { \
		if (inst && (msm_vidc_debug & __level)) { \
			pr_info(VIDC_DBG_TAG_INST __fmt, \
				__level_str, \
				inst->debug_str, \
				##__VA_ARGS__); \
		} \
	} while (0)

#define i_vpr_e(inst, __fmt, ...) dprintk_inst(VIDC_ERR,  "err ", inst, __fmt, ##__VA_ARGS__)
#define i_vpr_i(inst, __fmt, ...) dprintk_inst(VIDC_HIGH, "high", inst, __fmt, ##__VA_ARGS__)
#define i_vpr_h(inst, __fmt, ...) dprintk_inst(VIDC_HIGH, "high", inst, __fmt, ##__VA_ARGS__)
#define i_vpr_l(inst, __fmt, ...) dprintk_inst(VIDC_LOW,  "low ", inst, __fmt, ##__VA_ARGS__)
#define i_vpr_p(inst, __fmt, ...) dprintk_inst(VIDC_PERF, "perf", inst, __fmt, ##__VA_ARGS__)
#define i_vpr_t(inst, __fmt, ...) dprintk_inst(VIDC_PKT,  "pkt ", inst, __fmt, ##__VA_ARGS__)
#define i_vpr_b(inst, __fmt, ...) dprintk_inst(VIDC_BUS,  "bus ", inst, __fmt, ##__VA_ARGS__)

#define dprintk_core(__level, __level_str, __fmt, ...) \
	do { \
		if (msm_vidc_debug & __level) { \
			pr_info(VIDC_DBG_TAG_CORE __fmt, \
				__level_str, \
				DEFAULT_SID, \
				"codec", \
				##__VA_ARGS__); \
		} \
	} while (0)

#define d_vpr_e(__fmt, ...) dprintk_core(VIDC_ERR,  "err ", __fmt, ##__VA_ARGS__)
#define d_vpr_h(__fmt, ...) dprintk_core(VIDC_HIGH, "high", __fmt, ##__VA_ARGS__)
#define d_vpr_l(__fmt, ...) dprintk_core(VIDC_LOW,  "low ", __fmt, ##__VA_ARGS__)
#define d_vpr_p(__fmt, ...) dprintk_core(VIDC_PERF, "perf", __fmt, ##__VA_ARGS__)
#define d_vpr_t(__fmt, ...) dprintk_core(VIDC_PKT,  "pkt ", __fmt, ##__VA_ARGS__)
#define d_vpr_b(__fmt, ...) dprintk_core(VIDC_BUS,  "bus ", __fmt, ##__VA_ARGS__)

#define dprintk_ratelimit(__level, __level_str, __fmt, ...) \
	do { \
		if (msm_vidc_check_ratelimit()) { \
			dprintk_core(__level, __level_str, __fmt, ##__VA_ARGS__); \
		} \
	} while (0)

#define dprintk_firmware(__level, __fmt, ...)	\
	do { \
		if (__level & FW_PRINTK) { \
			pr_info(FW_DBG_TAG __fmt, \
				"fw", \
				##__VA_ARGS__); \
		} \
	} while (0)

#define MSM_VIDC_ERROR(value)					\
	do {	if (value)					\
			d_vpr_e("BugOn");		\
	} while (0)

enum msm_vidc_debugfs_event {
	MSM_VIDC_DEBUGFS_EVENT_ETB,
	MSM_VIDC_DEBUGFS_EVENT_EBD,
	MSM_VIDC_DEBUGFS_EVENT_FTB,
	MSM_VIDC_DEBUGFS_EVENT_FBD,
};

struct dentry *msm_vidc_debugfs_init_drv(void);
struct dentry *msm_vidc_debugfs_init_core(void *core);
struct dentry *msm_vidc_debugfs_init_inst(void *inst,
		struct dentry *parent);
void msm_vidc_debugfs_deinit_inst(void *inst);
void msm_vidc_debugfs_update(void *inst,
		enum msm_vidc_debugfs_event e);
int msm_vidc_check_ratelimit(void);

#endif
