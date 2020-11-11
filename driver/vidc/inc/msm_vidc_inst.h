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

struct msm_vidc_allocations_info {
	struct msm_vidc_allocations     bin;
	struct msm_vidc_allocations     arp;
	struct msm_vidc_allocations     comv;
	struct msm_vidc_allocations     non_comv;
	struct msm_vidc_allocations     line;
	struct msm_vidc_allocations     dpb;
	struct msm_vidc_allocations     persist;
};

struct msm_vidc_mappings_info {
	struct msm_vidc_mappings        input;
	struct msm_vidc_mappings        output;
	struct msm_vidc_mappings        input_meta;
	struct msm_vidc_mappings        output_meta;
	struct msm_vidc_mappings        bin;
	struct msm_vidc_mappings        arp;
	struct msm_vidc_mappings        comv;
	struct msm_vidc_mappings        non_comv;
	struct msm_vidc_mappings        line;
	struct msm_vidc_mappings        dpb;
	struct msm_vidc_mappings        persist;
};

struct msm_vidc_buffers_info {
	struct msm_vidc_buffers        input;
	struct msm_vidc_buffers        output;
	struct msm_vidc_buffers        input_meta;
	struct msm_vidc_buffers        output_meta;
	struct msm_vidc_buffers        bin;
	struct msm_vidc_buffers        arp;
	struct msm_vidc_buffers        comv;
	struct msm_vidc_buffers        non_comv;
	struct msm_vidc_buffers        line;
	struct msm_vidc_buffers        dpb;
	struct msm_vidc_buffers        persist;
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
	struct msm_vidc_inst_cap_entry     children;
	struct msm_vidc_inst_cap_entry     firmware;
	bool                               request;
	struct vb2_queue                   vb2q[MAX_PORT];
	struct msm_vidc_crop               crop;
	struct msm_vidc_properties         prop;
	enum msm_vidc_stage_type           stage;
	enum msm_vidc_pipe_type            pipe;
	struct msm_vidc_power              power;
	struct msm_vidc_buffers_info       buffers;
	struct msm_vidc_mappings_info      mappings;
	struct msm_vidc_allocations_info   allocations;
	bool                               subscribed_input_psc;
	bool                               subscribed_output_psc;
	bool                               subscribed_input_prop;
	bool                               subscribed_output_prop;
	struct msm_vidc_subscription_params       subcr_params[MAX_PORT];
	struct msm_vidc_decode_batch       decode_batch;
	struct msm_vidc_decode_vpp_delay   decode_vpp_delay;
	struct msm_vidc_session_idle       session_idle;
	struct list_head                   input_ts;
	struct list_head                   enc_input_crs;
	struct list_head                   decode_bitrate_data;
	bool                               codec_set;
	struct dentry                     *debugfs_root;
	struct msm_vidc_debug              debug;
	struct msm_vidc_inst_capability   *capabilities;
	struct completion                  completions[MAX_SIGNAL];
	u32                                hfi_cmd_type;
	u32                                hfi_port;

};

#endif // _MSM_VIDC_INST_H_
