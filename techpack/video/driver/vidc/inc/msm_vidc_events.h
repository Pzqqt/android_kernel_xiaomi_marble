/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 */

#if !defined(_TRACE_MSM_VIDC_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_MSM_VIDC_H

#undef TRACE_SYSTEM
#define TRACE_SYSTEM msm_vidc_events
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE msm_vidc_events

#include <linux/tracepoint.h>
#include "msm_vidc_inst.h"

DECLARE_EVENT_CLASS(msm_v4l2_vidc_inst,

	TP_PROTO(char *dummy, struct msm_vidc_inst *inst),

	TP_ARGS(dummy, inst),

	TP_STRUCT__entry(
		__field(char *, dummy)
		__field(const char *, debug_str)
	),

	TP_fast_assign(
		__entry->dummy = dummy;
		__entry->debug_str = inst ? inst->debug_str : (u8 *)"";
	),

	TP_printk("%s: %s\n", __entry->dummy, __entry->debug_str)
);

DEFINE_EVENT(msm_v4l2_vidc_inst, msm_v4l2_vidc_open,

	TP_PROTO(char *dummy, struct msm_vidc_inst *inst),

	TP_ARGS(dummy, inst)
);

DEFINE_EVENT(msm_v4l2_vidc_inst, msm_v4l2_vidc_close,

	TP_PROTO(char *dummy, struct msm_vidc_inst *inst),

	TP_ARGS(dummy, inst)
);

DECLARE_EVENT_CLASS(msm_v4l2_vidc_fw_load,

	TP_PROTO(char *dummy),

	TP_ARGS(dummy),

	TP_STRUCT__entry(
		__field(char *, dummy)
	),

	TP_fast_assign(
		__entry->dummy = dummy;
	),

	TP_printk("%s\n", __entry->dummy)
);

DEFINE_EVENT(msm_v4l2_vidc_fw_load, msm_v4l2_vidc_fw_load,

	TP_PROTO(char *dummy),

	TP_ARGS(dummy)
);

DECLARE_EVENT_CLASS(msm_vidc_driver,

	TP_PROTO(struct msm_vidc_inst *inst, const char *func,
		const char *old_state, const char *new_state),

	TP_ARGS(inst, func, old_state, new_state),

	TP_STRUCT__entry(
		__field(const char *, debug_str)
		__field(const char *, func)
		__field(const char *, old_state)
		__field(const char *, new_state)
	),

	TP_fast_assign(
		__entry->debug_str = inst ? inst->debug_str : (u8 *)"";
		__entry->func = func;
		__entry->old_state = old_state;
		__entry->new_state = new_state;
	),

	TP_printk("%s: %s: state changed to %s from %s\n",
		__entry->debug_str,
		__entry->func,
		__entry->new_state,
		__entry->old_state)
);

DEFINE_EVENT(msm_vidc_driver, msm_vidc_common_state_change,

	TP_PROTO(struct msm_vidc_inst *inst, const char *func,
		const char *old_state, const char *new_state),

	TP_ARGS(inst, func, old_state, new_state)
);

DECLARE_EVENT_CLASS(venus_hfi_var,

	TP_PROTO(u32 cp_start, u32 cp_size,
		u32 cp_nonpixel_start, u32 cp_nonpixel_size),

	TP_ARGS(cp_start, cp_size, cp_nonpixel_start, cp_nonpixel_size),

	TP_STRUCT__entry(
		__field(u32, cp_start)
		__field(u32, cp_size)
		__field(u32, cp_nonpixel_start)
		__field(u32, cp_nonpixel_size)
	),

	TP_fast_assign(
		__entry->cp_start = cp_start;
		__entry->cp_size = cp_size;
		__entry->cp_nonpixel_start = cp_nonpixel_start;
		__entry->cp_nonpixel_size = cp_nonpixel_size;
	),

	TP_printk(
		"TZBSP_MEM_PROTECT_VIDEO_VAR done, cp_start : 0x%x, cp_size : 0x%x, cp_nonpixel_start : 0x%x, cp_nonpixel_size : 0x%x\n",
		__entry->cp_start,
		__entry->cp_size,
		__entry->cp_nonpixel_start,
		__entry->cp_nonpixel_size)
);

DEFINE_EVENT(venus_hfi_var, venus_hfi_var_done,

	TP_PROTO(u32 cp_start, u32 cp_size,
		u32 cp_nonpixel_start, u32 cp_nonpixel_size),

	TP_ARGS(cp_start, cp_size, cp_nonpixel_start, cp_nonpixel_size)
);

DECLARE_EVENT_CLASS(msm_v4l2_vidc_buffer_events,

	TP_PROTO(struct msm_vidc_inst *inst, const char *str, const char *buf_type,
			struct msm_vidc_buffer *vbuf, unsigned long inode, long ref_count),

	TP_ARGS(inst, str, buf_type, vbuf, inode, ref_count),

	TP_STRUCT__entry(
		__field(u8 *, debug_str)
		__field(const char *, str)
		__field(const char *, buf_type)
		__field(u32, index)
		__field(int, fd)
		__field(u32, data_offset)
		__field(u64, device_addr)
		__field(unsigned long, inode)
		__field(long, ref_count)
		__field(u32, buffer_size)
		__field(u32, data_size)
		__field(u32, flags)
		__field(u64, timestamp)
		__field(int, attr)
		__field(u64, etb)
		__field(u64, ebd)
		__field(u64, ftb)
		__field(u64, fbd)
	),

	TP_fast_assign(
		__entry->debug_str = inst ? inst->debug_str : (u8 *)"";
		__entry->str = str;
		__entry->buf_type = buf_type;
		__entry->index = vbuf ? vbuf->index : -1;
		__entry->fd = vbuf ? vbuf->fd : 0;
		__entry->data_offset = vbuf ? vbuf->data_offset : 0;
		__entry->device_addr = vbuf ? vbuf->device_addr : 0;
		__entry->inode = inode;
		__entry->ref_count = ref_count;
		__entry->buffer_size = vbuf ? vbuf->buffer_size : 0;
		__entry->data_size = vbuf ? vbuf->data_size : 0;
		__entry->flags = vbuf ? vbuf->flags : 0;
		__entry->timestamp = vbuf ? vbuf->timestamp : 0;
		__entry->attr = vbuf ? vbuf->attr : 0;
		__entry->etb = inst ? inst->debug_count.etb : 0;
		__entry->ebd = inst ? inst->debug_count.ebd : 0;
		__entry->ftb = inst ? inst->debug_count.ftb : 0;
		__entry->fbd = inst ? inst->debug_count.fbd : 0;
	),

	TP_printk(
		"%s: %s: %s: idx %2d fd %3d off %d daddr %#llx inode %8lu ref %2ld size %8d filled %8d flags %#x ts %8lld attr %#x counts(etb ebd ftb fbd) %4llu %4llu %4llu %4llu\n",
		__entry->debug_str, __entry->str, __entry->buf_type, __entry->index, __entry->fd,
		__entry->data_offset, __entry->device_addr, __entry->inode, __entry->ref_count,
		__entry->buffer_size, __entry->data_size, __entry->flags, __entry->timestamp,
		__entry->attr, __entry->etb, __entry->ebd, __entry->ftb, __entry->fbd)
);

DEFINE_EVENT(msm_v4l2_vidc_buffer_events, msm_v4l2_vidc_buffer_event_log,

	TP_PROTO(struct msm_vidc_inst *inst, const char *str, const char *buf_type,
		struct msm_vidc_buffer *vbuf, unsigned long inode, long ref_count),

	TP_ARGS(inst, str, buf_type, vbuf, inode, ref_count)
);

DECLARE_EVENT_CLASS(msm_vidc_perf,

	TP_PROTO(struct msm_vidc_inst *inst, u64 clk_freq, u64 bw_ddr, u64 bw_llcc),

	TP_ARGS(inst, clk_freq, bw_ddr, bw_llcc),

	TP_STRUCT__entry(
		__field(u8 *, debug_str)
		__field(u64, curr_freq)
		__field(u32, ddr_bw)
		__field(u32, sys_cache_bw)
		__field(u32, dcvs_flags)
		__field(u64, clk_freq)
		__field(u64, bw_ddr)
		__field(u64, bw_llcc)
	),

	TP_fast_assign(
		__entry->debug_str = inst ? inst->debug_str : (u8 *)"";
		__entry->curr_freq = inst ? inst->power.curr_freq : 0;
		__entry->ddr_bw = inst ? inst->power.ddr_bw : 0;
		__entry->sys_cache_bw = inst ? inst->power.sys_cache_bw : 0;
		__entry->dcvs_flags = inst ? inst->power.dcvs_flags : 0;
		__entry->clk_freq = clk_freq;
		__entry->bw_ddr = bw_ddr;
		__entry->bw_llcc = bw_llcc;
	),

	TP_printk("%s: power: inst: clk %lld ddr %d llcc %d dcvs flags %#x, core: clk %lld ddr %lld llcc %lld\n",
		__entry->debug_str, __entry->curr_freq, __entry->ddr_bw, __entry->sys_cache_bw,
		__entry->dcvs_flags, __entry->clk_freq, __entry->bw_ddr, __entry->bw_llcc)
);

DEFINE_EVENT(msm_vidc_perf, msm_vidc_perf_power_scale,

	TP_PROTO(struct msm_vidc_inst *inst, u64 clk_freq, u64 bw_ddr, u64 bw_llcc),

	TP_ARGS(inst, clk_freq, bw_ddr, bw_llcc)
);

DECLARE_EVENT_CLASS(msm_vidc_buffer_dma_ops,

	TP_PROTO(const char *buffer_op, void *dmabuf, u8 size, void *kvaddr,
			const char *buf_name, u8 secure, u32 region),

	TP_ARGS(buffer_op, dmabuf, size, kvaddr, buf_name, secure, region),

	TP_STRUCT__entry(
		__field(const char *, buffer_op)
		__field(void *, dmabuf)
		__field(u8, size)
		__field(void *, kvaddr)
		__field(const char *, buf_name)
		__field(u8, secure)
		__field(u32, region)
	),

	TP_fast_assign(
		__entry->buffer_op = buffer_op;
		__entry->dmabuf = dmabuf;
		__entry->size = size;
		__entry->kvaddr = kvaddr;
		__entry->buf_name = buf_name;
		__entry->secure = secure;
		__entry->region = region;
	),

	TP_printk(
		"%s: dmabuf %pK, size %d, kvaddr %pK, buffer_type %s, secure %d, region %d\n",
		__entry->buffer_op, __entry->dmabuf, __entry->size, __entry->kvaddr,
		__entry->buf_name, __entry->secure, __entry->region)
);

DEFINE_EVENT(msm_vidc_buffer_dma_ops, msm_vidc_dma_buffer,

	TP_PROTO(const char *buffer_op, void *dmabuf, u8 size, void *kvaddr,
			const char *buf_name, u8 secure, u32 region),

	TP_ARGS(buffer_op, dmabuf, size, kvaddr, buf_name, secure, region)
);

#endif

/* This part must be outside protection */
#include <trace/define_trace.h>
