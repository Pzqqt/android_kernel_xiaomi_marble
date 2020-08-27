/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _MSM_VIDC_INST_H_
#define _MSM_VIDC_INST_H_

#include "msm_vidc_internal.h"

struct msm_vidc_inst;

#define call_session_op(c, op, ...)			\
	(((c) && (c)->session_ops && (c)->session_ops->op) ? \
	((c)->session_ops->op(__VA_ARGS__)) : 0)

struct msm_vidc_session_ops {
	int (*calc_freq)(struct msm_vidc_inst *inst, struct msm_vidc_buffer *mbuf);
	int (*calc_bw)(struct msm_vidc_inst *inst, struct msm_vidc_buffer *mbuf);
	int (*decide_work_route)(struct msm_vidc_inst *inst);
	int (*decide_work_mode)(struct msm_vidc_inst *inst);
	int (*decide_core_and_power_mode)(struct msm_vidc_inst *inst);
	int (*buffer_size)(struct msm_vidc_inst *inst, enum msm_vidc_buffer_type type);
	int (*min_count)(struct msm_vidc_inst *inst, enum msm_vidc_buffer_type type);
	int (*extra_count)(struct msm_vidc_inst *inst, enum msm_vidc_buffer_type type);
};

struct msm_vidc_allocations {
	struct msm_vidc_alloc_info        scratch;
	struct msm_vidc_alloc_info        scratch_1;
	struct msm_vidc_alloc_info        scratch_2;
	struct msm_vidc_alloc_info        persist;
	struct msm_vidc_alloc_info        persist_1;
};

struct msm_vidc_maps {
	struct msm_vidc_map_info        input;
	struct msm_vidc_map_info        output;
	struct msm_vidc_map_info        input_meta;
	struct msm_vidc_map_info        output_meta;
	struct msm_vidc_map_info        scratch;
	struct msm_vidc_map_info        scratch_1;
	struct msm_vidc_map_info        scratch_2;
	struct msm_vidc_map_info        persist;
	struct msm_vidc_map_info        persist_1;
};

struct msm_vidc_buffers {
	struct msm_vidc_buffer_info        input;
	struct msm_vidc_buffer_info        output;
	struct msm_vidc_buffer_info        input_meta;
	struct msm_vidc_buffer_info        output_meta;
	struct msm_vidc_buffer_info        scratch;
	struct msm_vidc_buffer_info        scratch_1;
	struct msm_vidc_buffer_info        scratch_2;
	struct msm_vidc_buffer_info        persist;
	struct msm_vidc_buffer_info        persist_1;
};

enum msm_vidc_inst_state {
	MSM_VIDC_OPEN                      = 1,
	MSM_VIDC_START_INPUT               = 2,
	MSM_VIDC_START_OUTPUT              = 3,
	MSM_VIDC_START                     = 4,
	MSM_VIDC_DRC                       = 5,
	MSM_VIDC_DRC_LAST_FLAG             = 6,
	MSM_VIDC_DRAIN                     = 7,
	MSM_VIDC_DRAIN_LAST_FLAG           = 8,
	MSM_VIDC_DRC_DRAIN                 = 9,
	MSM_VIDC_DRC_DRAIN_LAST_FLAG       = 10,
	MSM_VIDC_DRAIN_START_INPUT         = 11,
	MSM_VIDC_ERROR                     = 12,
};

struct msm_vidc_inst {
	struct list_head                   list;
	struct mutex                       lock;
	enum msm_vidc_inst_state           state;
	enum msm_vidc_domain_type          domain;
	enum msm_vidc_codec_type           codec;
	void                              *core;
	struct kref                        kref;
	u32                                session_id;
	u32                                sid;
	void                              *packet;
	u32                                packet_size;
	struct v4l2_format                 fmts[MAX_PORT];
	struct v4l2_ctrl_handler           ctrl_handler;
	struct v4l2_fh                     event_handler;
	struct v4l2_ctrl                 **ctrls;
	u32                                num_ctrls;
	struct msm_vidc_inst_cap_entry     child_ctrls;
	struct msm_vidc_inst_cap_entry     fw_ctrls;
	bool                               request;
	struct vb2_queue                   vb2q[MAX_PORT];
	struct msm_vidc_crop               crop;
	struct msm_vidc_properties         prop;
	struct msm_vidc_power              power;
	struct msm_vidc_buffers            buffers;
	struct msm_vidc_maps               maps;
	struct msm_vidc_allocations        allocations;
	struct msm_vidc_port_settings      port_settings[MAX_PORT];
	struct msm_vidc_decode_batch       decode_batch;
	struct msm_vidc_decode_vpp_delay   decode_vpp_delay;
	struct msm_vidc_session_idle       session_idle;
	struct list_head                   input_ts;
	struct list_head                   enc_input_crs;
	struct list_head                   decode_bitrate_data;
	bool                               session_created;
	struct dentry                     *debugfs_root;
	struct msm_vidc_debug              debug;
	struct msm_vidc_inst_capability   *capabilities;
};

#endif // _MSM_VIDC_INST_H_
