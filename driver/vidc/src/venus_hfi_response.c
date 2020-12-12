// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "hfi_packet.h"
#include "venus_hfi.h"
#include "venus_hfi_response.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_driver.h"
#include "msm_vdec.h"

u32 vidc_port_from_hfi(struct msm_vidc_inst *inst,
	enum hfi_packet_port_type hfi_port)
{
	enum msm_vidc_port_type port = MAX_PORT;

	if (is_decode_session(inst)) {
		switch (hfi_port) {
		case HFI_PORT_BITSTREAM:
			port = INPUT_PORT;
			break;
		case HFI_PORT_RAW:
			port = OUTPUT_PORT;
			break;
		default:
			s_vpr_e(inst->sid, "%s: invalid hfi port type %d\n",
				__func__, hfi_port);
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (hfi_port) {
		case HFI_PORT_RAW:
			port = INPUT_PORT;
			break;
		case HFI_PORT_BITSTREAM:
			port = OUTPUT_PORT;
			break;
		default:
			s_vpr_e(inst->sid, "%s: invalid hfi port type %d\n",
				__func__, hfi_port);
			break;
		}
	} else {
		s_vpr_e(inst->sid, "%s: invalid domain %#x\n",
			__func__, inst->domain);
	}

	return port;
}

bool is_valid_hfi_port(struct msm_vidc_inst *inst, u32 port,
	const char *func)
{
	if (!inst) {
		s_vpr_e(inst->sid, "%s: invalid params\n", func);
		return false;
	}

	if (port != HFI_PORT_BITSTREAM && port != HFI_PORT_RAW) {
		s_vpr_e(inst->sid, "%s: invalid port %#x\n", func, port);
		return false;
	}
	return true;
}

bool is_valid_hfi_buffer_type(struct msm_vidc_inst *inst,
	u32 buffer_type, const char *func)
{
	if (!inst) {
		s_vpr_e(inst->sid, "%s: invalid params\n", func);
		return false;
	}

	if (buffer_type != HFI_BUFFER_BITSTREAM &&
	    buffer_type != HFI_BUFFER_RAW &&
	    buffer_type != HFI_BUFFER_METADATA &&
	    buffer_type != HFI_BUFFER_BIN &&
	    buffer_type != HFI_BUFFER_ARP &&
	    buffer_type != HFI_BUFFER_COMV &&
	    buffer_type != HFI_BUFFER_NON_COMV &&
	    buffer_type != HFI_BUFFER_LINE &&
	    buffer_type != HFI_BUFFER_DPB &&
	    buffer_type != HFI_BUFFER_PERSIST) {
		s_vpr_e(inst->sid, "%s: invalid buffer type %#x\n",
			func, buffer_type);
		return false;
	}
	return true;
}

static int signal_session_msg_receipt(struct msm_vidc_inst *inst,
	enum signal_session_response cmd)
{
	if (cmd < MAX_SIGNAL)
		complete(&inst->completions[cmd]);
	return 0;
}

int validate_packet(u8 *response_pkt, u8 *core_resp_pkt,
	u32 core_resp_pkt_size, const char *func)
{
	u8 *response_limit;
	u32 response_pkt_size = 0;

	if (!response_pkt || !core_resp_pkt || !core_resp_pkt_size) {
		d_vpr_e("%s: invalid params\n", func);
		return -EINVAL;
	}

	response_limit = core_resp_pkt + core_resp_pkt_size -
		max(sizeof(struct hfi_header), sizeof(struct hfi_packet));

	if (response_pkt < core_resp_pkt || response_pkt > response_limit) {
		d_vpr_e("%s: invalid packet address\n", func);
		return -EINVAL;
	}

	response_pkt_size = *(u32 *)response_pkt;
	if (!response_pkt_size) {
		d_vpr_e("%s: response packet size cannot be zero\n", func);
		return -EINVAL;
	}

	if (response_pkt + response_pkt_size > response_limit) {
		d_vpr_e("%s: invalid packet size %d\n",
			func, *(u32 *)response_pkt);
		return -EINVAL;
	}
	return 0;
}

static int handle_session_info(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{

	int rc = 0;
	char *info;

	switch (pkt->type) {
	case HFI_INFO_UNSUPPORTED:
		info = "unsupported";
		break;
	case HFI_INFO_DATA_CORRUPT:
		info = "data corrupt";
		break;
	default:
		info = "unknown";
		break;
	}

	s_vpr_e(inst->sid, "session info (%#x): %s\n", pkt->type, info);

	return rc;
}

static int handle_session_error(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	int rc = 0;
	char *error;

	switch (pkt->type) {
	case HFI_ERROR_MAX_SESSIONS:
		error = "exceeded max sessions";
		break;
	case HFI_ERROR_UNKNOWN_SESSION:
		error = "unknown session id";
		break;
	case HFI_ERROR_INVALID_STATE:
		error = "invalid operation for current state";
		break;
	case HFI_ERROR_INSUFFICIENT_RESOURCES:
		error = "insufficient resources";
		break;
	case HFI_ERROR_BUFFER_NOT_SET:
		error = "internal buffers not set";
		break;
	case HFI_ERROR_FATAL:
		error = "fatal error";
		break;
	default:
		error = "unknown";
		break;
	}

	s_vpr_e(inst->sid, "session error (%#x): %s\n", pkt->type, error);

	rc = msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	return rc;
}

static int handle_system_error(struct msm_vidc_core *core,
	struct hfi_packet *pkt)
{
	mutex_lock(&core->lock);
	if (core->state == MSM_VIDC_CORE_DEINIT) {
		d_vpr_e("%s: core already deinitialized\n", __func__);
		mutex_unlock(&core->lock);
		return 0;
	}

	d_vpr_e("%s: system error received\n", __func__);
	core->state = MSM_VIDC_CORE_DEINIT;
	mutex_unlock(&core->lock);
	return 0;
}

static int handle_system_init(struct msm_vidc_core *core,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SYSTEM_ERROR) {
		d_vpr_e("%s: received system error\n", __func__);
		return 0;
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS) {
		d_vpr_h("%s: successful\n", __func__);
		complete(&core->init_done);
	}

	return 0;
}

static int handle_session_open(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful\n", __func__);

	return 0;
}

static int handle_session_close(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful\n", __func__);

	signal_session_msg_receipt(inst, SIGNAL_CMD_CLOSE);
	return 0;
}

static int handle_session_start(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful for port %d\n",
			__func__, pkt->port);
	return 0;
}

static int handle_session_stop(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	int signal_type = -1;

	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful for port %d\n",
			__func__, pkt->port);

	if (is_encode_session(inst)) {
		if (pkt->port == HFI_PORT_RAW) {
			signal_type = SIGNAL_CMD_STOP_INPUT;
		} else if (pkt->port == HFI_PORT_BITSTREAM) {
			signal_type = SIGNAL_CMD_STOP_OUTPUT;
		} else {
			s_vpr_e(inst->sid, "%s: invalid port: %d\n",
				__func__, pkt->port);
			return -EINVAL;
		}
	} else if (is_decode_session(inst)) {
		if (pkt->port == HFI_PORT_RAW) {
			signal_type = SIGNAL_CMD_STOP_OUTPUT;
		} else if (pkt->port == HFI_PORT_BITSTREAM) {
			signal_type = SIGNAL_CMD_STOP_INPUT;
		} else {
			s_vpr_e(inst->sid, "%s: invalid port: %d\n",
				__func__, pkt->port);
			return -EINVAL;
		}
	} else {
		s_vpr_e(inst->sid, "%s: invalid session\n", __func__);
		return -EINVAL;
	}

	if (signal_type != -1)
		signal_session_msg_receipt(inst, signal_type);
	return 0;
}

static int handle_session_drain(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful\n", __func__);
	return 0;
}

static int handle_input_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_INPUT, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (!found) {
		s_vpr_e(inst->sid, "%s: buffer not found for idx %d addr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	buf->data_offset = buffer->data_offset;
	buf->data_size = buffer->data_size;
	buf->attr &= ~MSM_VIDC_ATTR_QUEUED;
	buf->attr |= MSM_VIDC_ATTR_DEQUEUED;
	buf->flags = 0;
	//todo:
	/*if (buffer->flags & HFI_BUF_FW_FLAG_CORRUPT) {
		s_vpr_h(inst->sid, "%s: data corrupted\n", __func__);
		buf->flags |= MSM_VIDC_BUF_FLAG_ERROR;
	}
	if (buffer->flags & HFI_BUF_FW_FLAG_UNSUPPORTED) {
		s_vpr_e(inst->sid, "%s: unsupported input\n", __func__);
		buf->flags |= MSM_VIDC_BUF_FLAG_ERROR;
		// TODO: move inst->state to error state
	}*/

	print_vidc_buffer(VIDC_HIGH, "EBD", inst, buf);

	return rc;
}

static int handle_output_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_OUTPUT, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (!found) {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	buf->data_offset = buffer->data_offset;
	buf->data_size = buffer->data_size;
	buf->timestamp = buffer->timestamp;

	buf->attr &= ~MSM_VIDC_ATTR_QUEUED;
	buf->attr |= MSM_VIDC_ATTR_DEQUEUED;
	if (buffer->flags & HFI_BUF_FW_FLAG_READONLY)
		buf->attr |= MSM_VIDC_ATTR_READ_ONLY;
	else
		buf->attr &= ~MSM_VIDC_ATTR_READ_ONLY;

	buf->flags = 0;
	//todo: moved to HFI_PROP_PICTURE_TYPE
	/*if (buffer->flags & HFI_BUF_FW_FLAG_KEYFRAME)
		buf->flags |= MSM_VIDC_BUF_FLAG_KEYFRAME;*/
	if (buffer->flags & HFI_BUF_FW_FLAG_LAST)
		buf->flags |= MSM_VIDC_BUF_FLAG_LAST;
	//moved to HFI_INFO_DATA_CORRUPT
	/*if (buffer->flags & HFI_BUF_FW_FLAG_CORRUPT)
		buf->flags |= MSM_VIDC_BUF_FLAG_ERROR;*/
	if (buffer->flags & HFI_BUF_FW_FLAG_CODEC_CONFIG)
		buf->flags |= MSM_VIDC_BUF_FLAG_CODECCONFIG;
	//moved to HFI_PROP_SUBFRAME_OUTPUT
	/*if (buffer->flags & HFI_BUF_FW_FLAG_SUBFRAME)
		buf->flags |= MSM_VIDC_BUF_FLAG_SUBFRAME;*/

	print_vidc_buffer(VIDC_HIGH, "FBD", inst, buf);

	return rc;
}

static int handle_input_metadata_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_INPUT_META, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (!found) {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	buf->data_size = buffer->data_size;
	buf->attr &= ~MSM_VIDC_ATTR_QUEUED;
	buf->attr |= MSM_VIDC_ATTR_DEQUEUED;
	buf->flags = 0;
	if (buffer->flags & HFI_BUF_FW_FLAG_LAST)
		buf->flags |= MSM_VIDC_BUF_FLAG_LAST;

	print_vidc_buffer(VIDC_HIGH, "EBD META", inst, buf);
	return rc;
}

static int handle_output_metadata_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_OUTPUT_META, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (!found) {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}

	buf->data_size = buffer->data_size;
	buf->attr &= ~MSM_VIDC_ATTR_QUEUED;
	buf->attr |= MSM_VIDC_ATTR_DEQUEUED;
	buf->flags = 0;
	if (buffer->flags & HFI_BUF_FW_FLAG_LAST)
		buf->flags |= MSM_VIDC_BUF_FLAG_LAST;

	print_vidc_buffer(VIDC_HIGH, "FBD META", inst, buf);
	return rc;
}

static int handle_dequeue_buffers(struct msm_vidc_inst* inst)
{
	int rc = 0;
	int i;
	struct msm_vidc_buffers* buffers;
	struct msm_vidc_buffer* buf;
	struct msm_vidc_buffer* dummy;
	enum msm_vidc_buffer_type buffer_type[] = {
		MSM_VIDC_BUF_INPUT_META,
		MSM_VIDC_BUF_INPUT,
		MSM_VIDC_BUF_OUTPUT_META,
		MSM_VIDC_BUF_OUTPUT,
	};

	for (i = 0; i < ARRAY_SIZE(buffer_type); i++) {
		buffers = msm_vidc_get_buffers(inst, buffer_type[i], __func__);
		if (!buffers)
			return -EINVAL;

		list_for_each_entry_safe(buf, dummy, &buffers->list, list) {
			if (buf->attr & MSM_VIDC_ATTR_DEQUEUED) {
				buf->attr &= ~MSM_VIDC_ATTR_DEQUEUED;
				msm_vidc_vb2_buffer_done(inst, buf);
				msm_vidc_put_driver_buf(inst, buf);
			}
		}
	}

	return rc;
}

static int handle_persist_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_PERSIST, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (found) {
		rc = msm_vidc_destroy_internal_buffer(inst, buf);
	} else {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	return rc;
}

static int handle_line_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_LINE, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (found) {
		rc = msm_vidc_destroy_internal_buffer(inst, buf);
	} else {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	return rc;
}

static int handle_non_comv_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_NON_COMV, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (found) {
		rc = msm_vidc_destroy_internal_buffer(inst, buf);
	} else {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	return rc;
}

static int handle_comv_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_COMV, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (found) {
		rc = msm_vidc_destroy_internal_buffer(inst, buf);
	} else {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	return rc;
}

static int handle_bin_buffer(struct msm_vidc_inst *inst,
	struct hfi_buffer *buffer)
{
	int rc = 0;
	struct msm_vidc_buffers *buffers;
	struct msm_vidc_buffer *buf;
	bool found;

	buffers = msm_vidc_get_buffers(inst, MSM_VIDC_BUF_BIN, __func__);
	if (!buffers)
		return -EINVAL;

	found = false;
	list_for_each_entry(buf, &buffers->list, list) {
		if (buf->device_addr == buffer->base_address) {
			found = true;
			break;
		}
	}
	if (found) {
		rc = msm_vidc_destroy_internal_buffer(inst, buf);
	} else {
		s_vpr_e(inst->sid, "%s: invalid idx %d daddr %#x\n",
			__func__, buffer->index, buffer->base_address);
		return -EINVAL;
	}
	return rc;
}

static int handle_session_buffer(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	int rc = 0;
	struct hfi_buffer *buffer;
	u32 buf_type = 0, port_type = 0;

	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return 0;
	}

	port_type = pkt->port;
	if (!is_valid_hfi_port(inst, port_type, __func__)) {
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return 0;
	}

	buffer = (struct hfi_buffer *)((u8 *)pkt + sizeof(struct hfi_packet));
	buf_type = buffer->type;
	if (!is_valid_hfi_buffer_type(inst, buf_type, __func__)) {
		/* TODO */
		//msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
		return 0;
	}

	if (is_encode_session(inst)) {
		if (port_type == HFI_PORT_BITSTREAM) {
			if (buf_type == HFI_BUFFER_METADATA)
				rc = handle_output_metadata_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_BITSTREAM)
				rc = handle_output_buffer(inst, buffer);
		} else if (port_type == HFI_PORT_RAW) {
			if (buf_type == HFI_BUFFER_METADATA)
				rc = handle_input_metadata_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_RAW)
				rc = handle_input_buffer(inst, buffer);
		}
	} else if (is_decode_session(inst)) {
		if (port_type == HFI_PORT_BITSTREAM) {
			if (buf_type == HFI_BUFFER_METADATA)
				rc = handle_input_metadata_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_BITSTREAM)
				rc = handle_input_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_BIN)
				rc = handle_bin_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_COMV)
				rc = handle_comv_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_NON_COMV)
				rc = handle_non_comv_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_LINE)
				rc = handle_line_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_PERSIST)
				rc = handle_persist_buffer(inst, buffer);
		} else if (port_type == HFI_PORT_RAW) {
			if (buf_type == HFI_BUFFER_METADATA)
				rc = handle_output_metadata_buffer(inst, buffer);
			else if (buf_type == HFI_BUFFER_RAW)
				rc = handle_output_buffer(inst, buffer);
		}
	} else {
		s_vpr_e(inst->sid, "%s: invalid session %d\n",
			__func__, inst->domain);
		return -EINVAL;
	}

	return rc;
}

static int handle_port_settings_change(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	s_vpr_h(inst->sid, "%s: Received port settings change, type %d\n",
		__func__, pkt->port);
	return 0;
}

static int handle_session_subscribe_mode(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful\n", __func__);
	return 0;
}

static int handle_session_delivery_mode(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	if (pkt->flags & HFI_FW_FLAGS_SESSION_ERROR) {
		s_vpr_e(inst->sid, "%s: received session error\n", __func__);
		msm_vidc_change_inst_state(inst, MSM_VIDC_ERROR, __func__);
	}

	if (pkt->flags & HFI_FW_FLAGS_SUCCESS)
		s_vpr_h(inst->sid, "%s: successful\n", __func__);
	return 0;
}

static int handle_session_command(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	switch (pkt->type) {
	case HFI_CMD_OPEN:
		return handle_session_open(inst, pkt);
	case HFI_CMD_CLOSE:
		return handle_session_close(inst, pkt);
	case HFI_CMD_START:
		return handle_session_start(inst, pkt);
	case HFI_CMD_STOP:
		return handle_session_stop(inst, pkt);
	case HFI_CMD_DRAIN:
		return handle_session_drain(inst, pkt);
	case HFI_CMD_BUFFER:
		return handle_session_buffer(inst, pkt);
	case HFI_CMD_SETTINGS_CHANGE:
		return handle_port_settings_change(inst, pkt);
	case HFI_CMD_SUBSCRIBE_MODE:
		return handle_session_subscribe_mode(inst, pkt);
	case HFI_CMD_DELIVERY_MODE:
		return handle_session_delivery_mode(inst, pkt);
	default:
		s_vpr_e(inst->sid, "%s: Unsupported command type: %#x\n",
			__func__, pkt->type);
		return -EINVAL;
	}
	return 0;
}

static int handle_session_property(struct msm_vidc_inst *inst,
	struct hfi_packet *pkt)
{
	u32 port;
	u32 *payload_ptr;

	s_vpr_h(inst->sid, "%s: property type %#x\n", __func__, pkt->type);

	port = vidc_port_from_hfi(inst, pkt->port);
	payload_ptr = (u32 *)((u8 *)pkt + sizeof(struct hfi_packet));

	switch (pkt->type) {
	case HFI_PROP_BITSTREAM_RESOLUTION:
		inst->subcr_params[port].bitstream_resolution = payload_ptr[0];
		break;
	case HFI_PROP_CROP_OFFSETS:
		inst->subcr_params[port].crop_offsets =
			(u64)payload_ptr[0] << 32 | payload_ptr[1];
		break;
	case HFI_PROP_LUMA_CHROMA_BIT_DEPTH:
		inst->subcr_params[port].bit_depth = payload_ptr[0];
		break;
	case HFI_PROP_CABAC_SESSION:
		inst->subcr_params[port].cabac = payload_ptr[0];
		break;
	case HFI_PROP_CODED_FRAMES:
		inst->subcr_params[port].coded_frames = payload_ptr[0];
		break;
	case HFI_PROP_BUFFER_FW_MIN_OUTPUT_COUNT:
		inst->subcr_params[port].fw_min_count = payload_ptr[0];
		break;
	case HFI_PROP_PIC_ORDER_CNT_TYPE:
		inst->subcr_params[port].pic_order_cnt = payload_ptr[0];
		break;
	case HFI_PROP_SIGNAL_COLOR_INFO:
		inst->subcr_params[port].color_info = payload_ptr[0];
		break;
	case HFI_PROP_PROFILE:
		inst->subcr_params[port].profile = payload_ptr[0];
		break;
	case HFI_PROP_LEVEL:
		inst->subcr_params[port].level = payload_ptr[0];
		break;
	case HFI_PROP_TIER:
		inst->subcr_params[port].tier = payload_ptr[0];
		break;
	default:
		s_vpr_e(inst->sid, "%s: invalid port settings property %#x\n",
			__func__, pkt->type);
		return -EINVAL;
	}
	return 0;
}

static int handle_image_version_property(struct hfi_packet *pkt)
{
	u32 i = 0;
	char version[256];
	const u32 version_string_size = 128;
	u8 *str_image_version;
	u32 req_bytes;

	req_bytes = pkt->size - sizeof(*pkt);
	if (req_bytes < version_string_size) {
		d_vpr_e("%s: bad_pkt: %d\n", __func__, req_bytes);
		return -EINVAL;
	}
	str_image_version = (u8 *)pkt + sizeof(struct hfi_packet);
	/*
	 * The version string returned by firmware includes null
	 * characters at the start and in between. Replace the null
	 * characters with space, to print the version info.
	 */
	for (i = 0; i < version_string_size; i++) {
		if (str_image_version[i] != '\0')
			version[i] = str_image_version[i];
		else
			version[i] = ' ';
	}
	version[i] = '\0';
	d_vpr_h("%s: F/W version: %s\n", __func__, version);
	return 0;
}

static int handle_system_property(struct msm_vidc_core *core,
	struct hfi_packet *pkt)
{
	int rc = 0;

	if (pkt->flags & HFI_FW_FLAGS_SYSTEM_ERROR) {
		d_vpr_e("%s: received system error for property type %#x\n",
			__func__, pkt->type);
		return handle_system_error(core, pkt);
	}

	switch (pkt->type) {
	case HFI_PROP_IMAGE_VERSION:
		rc = handle_image_version_property(pkt);
		break;
	default:
		d_vpr_h("%s: property type %#x successful\n",
			__func__, pkt->type);
		break;
	}
	return rc;
}

static int handle_system_response(struct msm_vidc_core *core,
	struct hfi_header *hdr)
{
	int rc = 0;
	struct hfi_packet *packet;
	u8 *pkt;
	int i;

	pkt = (u8 *)((u8 *)hdr + sizeof(struct hfi_header));

	for (i = 0; i < hdr->num_packets; i++) {
		if (validate_packet((u8 *)pkt, core->response_packet,
				core->packet_size, __func__)) {
			rc = -EINVAL;
			goto exit;
		}
		packet = (struct hfi_packet *)pkt;
		if (packet->type == HFI_CMD_INIT) {
			rc = handle_system_init(core, packet);
		} else if (packet->type > HFI_SYSTEM_ERROR_BEGIN &&
			   packet->type < HFI_SYSTEM_ERROR_END) {
			rc = handle_system_error(core, packet);
		} else if (packet->type > HFI_PROP_BEGIN &&
			   packet->type < HFI_PROP_CODEC) {
			rc = handle_system_property(core, packet);
		} else {
			d_vpr_e("%s: Unknown packet type: %#x\n",
			__func__, packet->type);
			rc = -EINVAL;
			goto exit;
		}
		pkt += packet->size;
	}
exit:
	return rc;
}

static int handle_session_response(struct msm_vidc_core *core,
	struct hfi_header *hdr)
{
	int rc = 0;
	struct msm_vidc_inst *inst;
	struct hfi_packet *packet;
	u8 *pkt;
	u32 hfi_cmd_type = 0;
	u32 hfi_port = 0;
	int i;

	inst = get_inst(core, hdr->session_id);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&inst->lock);
	hfi_cmd_type = 0;
	hfi_port = 0;
	pkt = (u8 *)((u8 *)hdr + sizeof(struct hfi_header));

	for (i = 0; i < hdr->num_packets; i++) {
		if (validate_packet(pkt, core->response_packet,
				core->packet_size, __func__)) {
			rc = -EINVAL;
			goto exit;
		}
		packet = (struct hfi_packet *)pkt;
		if (packet->type > HFI_CMD_BEGIN &&
		    packet->type < HFI_CMD_END) {
			if (hfi_cmd_type == HFI_CMD_SETTINGS_CHANGE) {
				s_vpr_e(inst->sid,
					"%s: invalid packet type %d in port settings change\n",
					__func__, packet->type);
				rc = -EINVAL;
				goto exit;
			}
			hfi_cmd_type = packet->type;
			hfi_port = packet->port;
			rc = handle_session_command(inst, packet);
		} else if (packet->type > HFI_PROP_BEGIN &&
			   packet->type < HFI_PROP_END) {
			rc = handle_session_property(inst, packet);
		} else if (packet->type > HFI_SESSION_ERROR_BEGIN &&
			   packet->type < HFI_SESSION_ERROR_END) {
			rc = handle_session_error(inst, packet);
		} else if (packet->type > HFI_INFORMATION_BEGIN &&
				packet->type < HFI_INFORMATION_END) {
			rc = handle_session_info(inst, packet);
		} else {
			s_vpr_e(inst->sid, "%s: Unknown packet type: %#x\n",
				__func__, packet->type);
			rc = -EINVAL;
			goto exit;
		}
		pkt += packet->size;
	}

	if (hfi_cmd_type == HFI_CMD_BUFFER) {
		rc = handle_dequeue_buffers(inst);
		if (rc)
			goto exit;
	}

	if (hfi_cmd_type == HFI_CMD_SETTINGS_CHANGE) {
		if (hfi_port == HFI_PORT_BITSTREAM) {
			rc = msm_vdec_input_port_settings_change(inst);
			if (rc)
				goto exit;
		} else if (hfi_port == HFI_PORT_RAW) {
			rc = msm_vdec_output_port_settings_change(inst);
			if (rc)
				goto exit;
		}
	}

exit:
	mutex_unlock(&inst->lock);
	put_inst(inst);
	return rc;
}

int handle_response(struct msm_vidc_core *core, void *response)
{
	struct hfi_header *hdr;

	if (!core || !response) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	hdr = (struct hfi_header *)response;
	if (validate_packet((u8 *)hdr, core->response_packet,
			core->packet_size, __func__))
		return -EINVAL;

	if (!hdr->session_id)
		return handle_system_response(core, hdr);
	else
		return handle_session_response(core, hdr);

	return 0;
}
