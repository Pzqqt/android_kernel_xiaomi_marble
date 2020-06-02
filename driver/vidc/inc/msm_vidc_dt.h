/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_DT_H_
#define _MSM_VIDC_DT_H_

#include <linux/platform_device.h>
#include <linux/soc/qcom/llcc-qcom.h>

#include "msm_vidc_internal.h"

/*
 * These are helper macros to iterate over various lists within
 * msm_vidc_core->dt.  The intention is to cut down on a lot of boiler-plate
 * code
 */

/* Read as "for each 'thing' in a set of 'thingies'" */
#define venus_hfi_for_each_thing(__device, __thing, __thingy) \
	venus_hfi_for_each_thing_continue(__device, __thing, __thingy, 0)

#define venus_hfi_for_each_thing_reverse(__device, __thing, __thingy) \
	venus_hfi_for_each_thing_reverse_continue(__device, __thing, __thingy, \
			(__device)->dt->__thingy##_set.count - 1)

/* TODO: the __from parameter technically not required since we can figure it
 * out with some pointer magic (i.e. __thing - __thing##_tbl[0]).  If this macro
 * sees extensive use, probably worth cleaning it up but for now omitting it
 * since it introduces unnecessary complexity.
 */
#define venus_hfi_for_each_thing_continue(__device, __thing, __thingy, __from) \
	for (__thing = &(__device)->dt->\
			__thingy##_set.__thingy##_tbl[__from]; \
		__thing < &(__device)->dt->__thingy##_set.__thingy##_tbl[0] + \
			((__device)->dt->__thingy##_set.count - __from); \
		++__thing)

#define venus_hfi_for_each_thing_reverse_continue(__device, __thing, __thingy, \
		__from) \
	for (__thing = &(__device)->dt->\
			__thingy##_set.__thingy##_tbl[__from]; \
		__thing >= &(__device)->dt->__thingy##_set.__thingy##_tbl[0]; \
		--__thing)

/* Regular set helpers */
#define venus_hfi_for_each_regulator(__device, __rinfo) \
	venus_hfi_for_each_thing(__device, __rinfo, regulator)

#define venus_hfi_for_each_regulator_reverse(__device, __rinfo) \
	venus_hfi_for_each_thing_reverse(__device, __rinfo, regulator)

#define venus_hfi_for_each_regulator_reverse_continue(__device, __rinfo, \
		__from) \
	venus_hfi_for_each_thing_reverse_continue(__device, __rinfo, \
			regulator, __from)

/* Clock set helpers */
#define venus_hfi_for_each_clock(__device, __cinfo) \
	venus_hfi_for_each_thing(__device, __cinfo, clock)

#define venus_hfi_for_each_clock_reverse(__device, __cinfo) \
	venus_hfi_for_each_thing_reverse(__device, __cinfo, clock)

#define venus_hfi_for_each_clock_reverse_continue(__device, __rinfo, \
		__from) \
	venus_hfi_for_each_thing_reverse_continue(__device, __rinfo, \
			clock, __from)

/* Bus set helpers */
#define venus_hfi_for_each_bus(__device, __binfo) \
	venus_hfi_for_each_thing(__device, __binfo, bus)
#define venus_hfi_for_each_bus_reverse(__device, __binfo) \
	venus_hfi_for_each_thing_reverse(__device, __binfo, bus)

/* Subcache set helpers */
#define venus_hfi_for_each_subcache(__device, __sinfo) \
	venus_hfi_for_each_thing(__device, __sinfo, subcache)
#define venus_hfi_for_each_subcache_reverse(__device, __sinfo) \
	venus_hfi_for_each_thing_reverse(__device, __sinfo, subcache)

struct reg_value_pair {
	u32 reg;
	u32 value;
	u32 mask;
};

struct reg_set {
	struct reg_value_pair *reg_tbl;
	u32 count;
};

struct addr_range {
	u32 start;
	u32 size;
};

struct addr_set {
	struct addr_range *addr_tbl;
	u32 count;
};

struct context_bank_info {
	struct list_head list;
	const char *name;
	u32 buffer_type;
	bool is_secure;
	struct addr_range addr_range;
	struct device *dev;
	struct iommu_domain *domain;
};

struct buffer_usage_table {
	u32 buffer_type;
	u32 tz_usage;
};

struct buffer_usage_set {
	struct buffer_usage_table *buffer_usage_tbl;
	u32 count;
};

struct regulator_info {
	struct regulator *regulator;
	bool has_hw_power_collapse;
	char *name;
};

struct regulator_set {
	struct regulator_info *regulator_tbl;
	u32 count;
};

struct clock_info {
	const char *name;
	struct clk *clk;
	u32 count;
	bool has_scaling;
	bool has_mem_retention;
	u64 prev;
};

struct clock_set {
	struct clock_info *clock_tbl;
	u32 count;
};

struct bus_info {
	const char *name;
	u32 range[2];
	struct device *dev;
	struct icc_path *path;
};

struct bus_set {
	struct bus_info *bus_tbl;
	u32 count;
};

struct reset_info {
	struct reset_control *rst;
	const char *name;
};

struct reset_set {
	struct reset_info *reset_tbl;
	u32 count;
};

struct allowed_clock_rates_table {
	u32 clock_rate;
};

struct clock_profile_entry {
	u32 codec_mask;
	u32 vpp_cycles;
	u32 vsp_cycles;
	u32 low_power_cycles;
};

struct clock_freq_table {
	struct clock_profile_entry *clk_prof_entries;
	u32 count;
};

struct subcache_info {
	const char *name;
	bool isactive;
	bool isset;
	struct llcc_slice_desc *subcache;
};

struct subcache_set {
	struct subcache_info *subcache_tbl;
	u32 count;
};

struct msm_vidc_dt {
	void *core;
	phys_addr_t register_base;
	u32 register_size;
	u32 irq;
	u32 sku_version;
	struct allowed_clock_rates_table *allowed_clks_tbl;
	u32 allowed_clks_tbl_size;
	struct clock_freq_table clock_freq_tbl;
	bool sys_cache_present;
	bool sys_cache_res_set;
	struct subcache_set subcache_set;
	struct reg_set reg_set;
	struct addr_set qdss_addr_set;
	struct buffer_usage_set buffer_usage_set;
	struct regulator_set regulator_set;
	struct clock_set clock_set;
	struct bus_set bus_set;
	struct reset_set reset_set;
	struct list_head context_banks;
	struct mutex cb_lock;
	const char *fw_name;
	int fw_cookie;
};

int msm_vidc_init_dt(struct platform_device *pdev);
int msm_vidc_read_context_bank_resources_from_dt(struct platform_device *pdev);
void msm_vidc_deinit_dt(struct platform_device *pdev);

#endif // _MSM_VIDC_DT_H_
