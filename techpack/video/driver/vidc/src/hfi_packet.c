// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "hfi_packet.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_platform.h"

u32 get_hfi_port(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	u32 hfi_port = HFI_PORT_NONE;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return hfi_port;
	}

	if (is_decode_session(inst)) {
		switch(port) {
		case INPUT_PORT:
		case INPUT_META_PORT:
			hfi_port = HFI_PORT_BITSTREAM;
			break;
		case OUTPUT_PORT:
		case OUTPUT_META_PORT:
			hfi_port = HFI_PORT_RAW;
			break;
		default:
			i_vpr_e(inst, "%s: invalid port type %d\n",
				__func__, port);
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (port) {
		case INPUT_PORT:
		case INPUT_META_PORT:
			hfi_port = HFI_PORT_RAW;
			break;
		case OUTPUT_PORT:
		case OUTPUT_META_PORT:
			hfi_port = HFI_PORT_BITSTREAM;
			break;
		default:
			i_vpr_e(inst, "%s: invalid port type %d\n",
				__func__, port);
			break;
		}
	} else {
		i_vpr_e(inst, "%s: invalid domain %#x\n",
			__func__, inst->domain);
	}

	return hfi_port;
}

u32 get_hfi_port_from_buffer_type(struct msm_vidc_inst *inst,
	enum msm_vidc_buffer_type buffer_type)
{
	u32 hfi_port = HFI_PORT_NONE;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return hfi_port;
	}

	if (is_decode_session(inst)) {
		switch(buffer_type) {
		case MSM_VIDC_BUF_INPUT:
		case MSM_VIDC_BUF_INPUT_META:
		case MSM_VIDC_BUF_BIN:
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
			hfi_port = HFI_PORT_BITSTREAM;
			break;
		case MSM_VIDC_BUF_OUTPUT:
		case MSM_VIDC_BUF_OUTPUT_META:
		case MSM_VIDC_BUF_DPB:
			hfi_port = HFI_PORT_RAW;
			break;
		case MSM_VIDC_BUF_PERSIST:
			hfi_port = HFI_PORT_NONE;
			break;
		default:
			i_vpr_e(inst, "%s: invalid buffer type %d\n",
				__func__, buffer_type);
			break;
		}
	} else if (is_encode_session(inst)) {
		switch (buffer_type) {
		case MSM_VIDC_BUF_INPUT:
		case MSM_VIDC_BUF_INPUT_META:
		case MSM_VIDC_BUF_VPSS:
			hfi_port = HFI_PORT_RAW;
			break;
		case MSM_VIDC_BUF_OUTPUT:
		case MSM_VIDC_BUF_OUTPUT_META:
		case MSM_VIDC_BUF_BIN:
		case MSM_VIDC_BUF_COMV:
		case MSM_VIDC_BUF_NON_COMV:
		case MSM_VIDC_BUF_LINE:
		case MSM_VIDC_BUF_DPB:
			hfi_port = HFI_PORT_BITSTREAM;
			break;
		case MSM_VIDC_BUF_ARP:
			hfi_port = HFI_PORT_NONE;
			break;
		default:
			i_vpr_e(inst, "%s: invalid buffer type %d\n",
				__func__, buffer_type);
			break;
		}
	} else {
		i_vpr_e(inst, "%s: invalid domain %#x\n",
			__func__, inst->domain);
	}

	return hfi_port;
}

u32 hfi_buf_type_from_driver(enum msm_vidc_domain_type domain,
	enum msm_vidc_buffer_type buffer_type)
{
	switch (buffer_type) {
	case MSM_VIDC_BUF_INPUT:
		if (domain == MSM_VIDC_DECODER)
			return HFI_BUFFER_BITSTREAM;
		else
			return HFI_BUFFER_RAW;
	case MSM_VIDC_BUF_OUTPUT:
		if (domain == MSM_VIDC_DECODER)
			return HFI_BUFFER_RAW;
		else
			return HFI_BUFFER_BITSTREAM;
	case MSM_VIDC_BUF_INPUT_META:
	case MSM_VIDC_BUF_OUTPUT_META:
		return HFI_BUFFER_METADATA;
	case MSM_VIDC_BUF_BIN:
		return HFI_BUFFER_BIN;
	case MSM_VIDC_BUF_ARP:
		return HFI_BUFFER_ARP;
	case MSM_VIDC_BUF_COMV:
		return HFI_BUFFER_COMV;
	case MSM_VIDC_BUF_NON_COMV:
		return HFI_BUFFER_NON_COMV;
	case MSM_VIDC_BUF_LINE:
		return HFI_BUFFER_LINE;
	case MSM_VIDC_BUF_DPB:
		return HFI_BUFFER_DPB;
	case MSM_VIDC_BUF_PERSIST:
		return HFI_BUFFER_PERSIST;
	case MSM_VIDC_BUF_VPSS:
		return HFI_BUFFER_VPSS;
	default:
		d_vpr_e("invalid buffer type %d\n",
			buffer_type);
		return 0;
	}
}

u32 hfi_buf_type_to_driver(enum msm_vidc_domain_type domain,
	enum hfi_buffer_type buffer_type, enum hfi_packet_port_type port_type)
{
	switch (buffer_type) {
	case HFI_BUFFER_BITSTREAM:
		if (domain == MSM_VIDC_DECODER)
			return MSM_VIDC_BUF_INPUT;
		else
			return MSM_VIDC_BUF_OUTPUT;
	case HFI_BUFFER_RAW:
		if (domain == MSM_VIDC_DECODER)
			return MSM_VIDC_BUF_OUTPUT;
		else
			return MSM_VIDC_BUF_INPUT;
	case HFI_BUFFER_METADATA:
		if (domain == MSM_VIDC_DECODER)
			if (port_type == HFI_PORT_BITSTREAM)
				return MSM_VIDC_BUF_INPUT_META;
			else
				return MSM_VIDC_BUF_OUTPUT_META;
		else
			if (port_type == HFI_PORT_BITSTREAM)
				return MSM_VIDC_BUF_OUTPUT_META;
			else
				return MSM_VIDC_BUF_INPUT_META;
	case HFI_BUFFER_BIN:
		return MSM_VIDC_BUF_BIN;
	case HFI_BUFFER_ARP:
		return MSM_VIDC_BUF_ARP;
	case HFI_BUFFER_COMV:
		return MSM_VIDC_BUF_COMV;
	case HFI_BUFFER_NON_COMV:
		return MSM_VIDC_BUF_NON_COMV;
	case HFI_BUFFER_LINE:
		return MSM_VIDC_BUF_LINE;
	case HFI_BUFFER_DPB:
		return MSM_VIDC_BUF_DPB;
	case HFI_BUFFER_PERSIST:
		return MSM_VIDC_BUF_PERSIST;
	case HFI_BUFFER_VPSS:
		return MSM_VIDC_BUF_VPSS;
	default:
		d_vpr_e("invalid buffer type %d\n",
			buffer_type);
		return 0;
	}
}

u32 get_hfi_codec(struct msm_vidc_inst *inst)
{
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return 0;
	}

	switch (inst->codec) {
	case MSM_VIDC_H264:
		if (inst->domain == MSM_VIDC_ENCODER)
			return HFI_CODEC_ENCODE_AVC;
		else
			return HFI_CODEC_DECODE_AVC;
	case MSM_VIDC_HEVC:
	case MSM_VIDC_HEIC:
		if (inst->domain == MSM_VIDC_ENCODER)
			return HFI_CODEC_ENCODE_HEVC;
		else
			return HFI_CODEC_DECODE_HEVC;
	case MSM_VIDC_VP9:
		return HFI_CODEC_DECODE_VP9;
	default:
		i_vpr_e(inst, "invalid codec %d, domain %d\n",
			inst->codec, inst->domain);
		return 0;
	}
}

u32 get_hfi_colorformat(struct msm_vidc_inst *inst,
	enum msm_vidc_colorformat_type colorformat)
{
	u32 hfi_colorformat = HFI_COLOR_FMT_NV12_UBWC;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return hfi_colorformat;
	}

	switch(colorformat) {
	case MSM_VIDC_FMT_NV12:
		hfi_colorformat = HFI_COLOR_FMT_NV12;
		break;
	case MSM_VIDC_FMT_NV12C:
		hfi_colorformat = HFI_COLOR_FMT_NV12_UBWC;
		break;
	case MSM_VIDC_FMT_P010:
		hfi_colorformat = HFI_COLOR_FMT_P010;
		break;
	case MSM_VIDC_FMT_TP10C:
		hfi_colorformat = HFI_COLOR_FMT_TP10_UBWC;
		break;
	case MSM_VIDC_FMT_RGBA8888:
		hfi_colorformat = HFI_COLOR_FMT_RGBA8888;
		break;
	case MSM_VIDC_FMT_RGBA8888C:
		hfi_colorformat = HFI_COLOR_FMT_RGBA8888_UBWC;
		break;
	case MSM_VIDC_FMT_NV21:
		hfi_colorformat = HFI_COLOR_FMT_NV21;
		break;
	default:
		i_vpr_e(inst, "%s: invalid colorformat %d\n",
			__func__, colorformat);
		break;
	}

	return hfi_colorformat;
}

int get_hfi_buffer(struct msm_vidc_inst *inst,
	struct msm_vidc_buffer *buffer, struct hfi_buffer *buf)
{
	if (!inst || !buffer || !buf) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	memset(buf, 0, sizeof(struct hfi_buffer));
	buf->type = hfi_buf_type_from_driver(inst->domain, buffer->type);
	buf->index = buffer->index;
	buf->base_address = buffer->device_addr;
	buf->addr_offset = 0;
	buf->buffer_size = buffer->buffer_size;
	buf->data_offset = buffer->data_offset;
	buf->data_size = buffer->data_size;
	if (buffer->attr & MSM_VIDC_ATTR_READ_ONLY)
		buf->flags |= HFI_BUF_HOST_FLAG_READONLY;
	if (buffer->attr & MSM_VIDC_ATTR_PENDING_RELEASE)
		buf->flags |= HFI_BUF_HOST_FLAG_RELEASE;
	if (buffer->flags & MSM_VIDC_BUF_FLAG_CODECCONFIG)
		buf->flags |= HFI_BUF_HOST_FLAG_CODEC_CONFIG;
	buf->timestamp = buffer->timestamp;

	return 0;
}

int hfi_create_header(u8 *packet, u32 packet_size, u32 session_id,
	u32 header_id)
{
	struct hfi_header *hdr = (struct hfi_header *)packet;

	if (!packet || packet_size < sizeof(struct hfi_header)) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	memset(hdr, 0, sizeof(struct hfi_header));

	hdr->size = sizeof(struct hfi_header);
	hdr->session_id = session_id;
	hdr->header_id = header_id;
	hdr->num_packets = 0;
	return 0;
}

int hfi_create_packet(u8 *packet, u32 packet_size,
	u32 pkt_type, u32 pkt_flags, u32 payload_type, u32 port,
	u32 packet_id, void *payload, u32 payload_size)
{
	struct hfi_header *hdr;
	struct hfi_packet *pkt;
	u32 pkt_size;

	if (!packet) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	hdr = (struct hfi_header *)packet;
	if (hdr->size < sizeof(struct hfi_header)) {
		d_vpr_e("%s: invalid hdr size %d\n", __func__, hdr->size);
		return -EINVAL;
	}
	pkt = (struct hfi_packet *)(packet + hdr->size);
	pkt_size = sizeof(struct hfi_packet) + payload_size;
	if (packet_size < hdr->size  + pkt_size) {
		d_vpr_e("%s: invalid packet_size %d, %d %d\n",
			__func__, packet_size, hdr->size, pkt_size);
		return -EINVAL;
	}
	memset(pkt, 0, pkt_size);
	pkt->size = pkt_size;
	pkt->type = pkt_type;
	pkt->flags = pkt_flags;
	pkt->payload_info = payload_type;
	pkt->port = port;
	pkt->packet_id = packet_id;
	if (payload_size)
		memcpy((u8 *)pkt + sizeof(struct hfi_packet),
			payload, payload_size);

	hdr->num_packets++;
	hdr->size += pkt->size;
	return 0;
}

int hfi_packet_sys_init(struct msm_vidc_core *core,
	u8 *pkt, u32 pkt_size)
{
	int rc = 0;
	u32 payload = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	rc = hfi_create_header(pkt, pkt_size,
				   0 /*session_id*/,
				   core->header_id++);
	if (rc)
		goto err_sys_init;

	/* HFI_CMD_SYSTEM_INIT */
	payload = HFI_VIDEO_ARCH_LX;
	d_vpr_h("%s: arch %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_CMD_INIT,
				   (HFI_HOST_FLAGS_RESPONSE_REQUIRED |
				   HFI_HOST_FLAGS_INTR_REQUIRED |
				   HFI_HOST_FLAGS_NON_DISCARDABLE),
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	if (!core->platform->data.ubwc_config)
		goto exit;

	/* HFI_PROP_UBWC_MAX_CHANNELS */
	payload = core->platform->data.ubwc_config->max_channels;
	d_vpr_h("%s: ubwc max channels %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_MAX_CHANNELS,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	/* HFI_PROP_UBWC_MAL_LENGTH */
	payload = core->platform->data.ubwc_config->mal_length;
	d_vpr_h("%s: ubwc mal length %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_MAL_LENGTH,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	/* HFI_PROP_UBWC_HBB */
	payload = core->platform->data.ubwc_config->highest_bank_bit;
	d_vpr_h("%s: ubwc hbb %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_HBB,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	/* HFI_PROP_UBWC_BANK_SWZL_LEVEL1 */
	payload = core->platform->data.ubwc_config->bank_swzl_level;
	d_vpr_h("%s: ubwc swzl1 %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_BANK_SWZL_LEVEL1,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	/* HFI_PROP_UBWC_BANK_SWZL_LEVEL2 */
	payload = core->platform->data.ubwc_config->bank_swz2_level;
	d_vpr_h("%s: ubwc swzl2 %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_BANK_SWZL_LEVEL2,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	/* HFI_PROP_UBWC_BANK_SWZL_LEVEL3 */
	payload = core->platform->data.ubwc_config->bank_swz3_level;
	d_vpr_h("%s: ubwc swzl3 %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_BANK_SWZL_LEVEL3,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

	/* HFI_PROP_UBWC_BANK_SPREADING */
	payload = core->platform->data.ubwc_config->bank_spreading;
	d_vpr_h("%s: ubwc bank spreading %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_UBWC_BANK_SPREADING,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;

exit:
	d_vpr_h("System init packet created\n");
	return rc;

err_sys_init:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}

int hfi_packet_image_version(struct msm_vidc_core *core,
	u8 *pkt, u32 pkt_size)
{
	int rc = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	rc = hfi_create_header(pkt, pkt_size,
				   0 /*session_id*/,
				   core->header_id++);
	if (rc)
		goto err_img_version;

	/* HFI_PROP_IMAGE_VERSION */
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_IMAGE_VERSION,
				   (HFI_HOST_FLAGS_RESPONSE_REQUIRED |
				   HFI_HOST_FLAGS_INTR_REQUIRED |
				   HFI_HOST_FLAGS_GET_PROPERTY),
				   HFI_PAYLOAD_NONE,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   NULL, 0);
	if (rc)
		goto err_img_version;

	d_vpr_h("Image version packet created\n");
	return rc;

err_img_version:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}

int hfi_packet_sys_pc_prep(struct msm_vidc_core *core,
	u8 *pkt, u32 pkt_size)
{
	int rc = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	rc = hfi_create_header(pkt, pkt_size,
			   0 /*session_id*/,
			   core->header_id++);
	if (rc)
		goto err_sys_pc;

	/* HFI_CMD_POWER_COLLAPSE */
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_CMD_POWER_COLLAPSE,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_NONE,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   NULL, 0);
	if (rc)
		goto err_sys_pc;

	d_vpr_h("Power collapse packet created\n");
	return rc;

err_sys_pc:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}

int hfi_packet_sys_debug_config(struct msm_vidc_core *core,
	u8 *pkt, u32 pkt_size, u32 debug_config)
{
	int rc = 0;
	u32 payload = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	rc = hfi_create_header(pkt, pkt_size,
				   0 /*session_id*/,
				   core->header_id++);
	if (rc)
		goto err_debug;

	/* HFI_PROP_DEBUG_CONFIG */
	payload = 0; /*TODO:Change later*/
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_DEBUG_CONFIG,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32_ENUM,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_debug;

	/* HFI_PROP_DEBUG_LOG_LEVEL */
	payload = debug_config; /*TODO:Change later*/
	rc = hfi_create_packet(pkt, pkt_size,
				   HFI_PROP_DEBUG_LOG_LEVEL,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32_ENUM,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_debug;

err_debug:
	if (rc)
		d_vpr_e("%s: create packet failed\n", __func__);

	return rc;
}

int hfi_packet_session_command(struct msm_vidc_inst *inst,
	u32 pkt_type, u32 flags, u32 port, u32 session_id,
	u32 payload_type, void *payload, u32 payload_size)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !inst->packet) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	rc = hfi_create_header(inst->packet, inst->packet_size,
				   session_id,
				   core->header_id++);
	if (rc)
		goto err_cmd;

	rc = hfi_create_packet(inst->packet,
				inst->packet_size,
				pkt_type,
				flags,
				payload_type,
				port,
				core->packet_id++,
				payload,
				payload_size);
	if (rc)
		goto err_cmd;

	i_vpr_h(inst, "Command packet 0x%x created\n", pkt_type);
	return rc;

err_cmd:
	i_vpr_e(inst, "%s: create packet failed\n", __func__);
	return rc;
}

int hfi_packet_sys_intraframe_powercollapse(struct msm_vidc_core* core,
	u8* pkt, u32 pkt_size, u32 enable)
{
	int rc = 0;
	u32 payload = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	rc = hfi_create_header(pkt, pkt_size,
		0 /*session_id*/,
		core->header_id++);
	if (rc)
		goto err;

	/* HFI_PROP_INTRA_FRAME_POWER_COLLAPSE */
	payload = enable;
	d_vpr_h("%s: intra frame power collapse %d\n", __func__, payload);
	rc = hfi_create_packet(pkt, pkt_size,
		HFI_PROP_INTRA_FRAME_POWER_COLLAPSE,
		HFI_HOST_FLAGS_NONE,
		HFI_PAYLOAD_U32,
		HFI_PORT_NONE,
		core->packet_id++,
		&payload,
		sizeof(u32));
	if (rc)
		goto err;

	d_vpr_h("IFPC packet created\n");
	return rc;

err:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}
