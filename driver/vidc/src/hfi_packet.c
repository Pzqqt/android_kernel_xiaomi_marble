// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include "hfi_packet.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_debug.h"
#include "msm_vidc_platform.h"

static u32 get_hfi_buffer_type(enum msm_vidc_domain_type domain,
	enum msm_vidc_buffer_type buf_type)
{
	switch (buf_type) {
	case MSM_VIDC_QUEUE:
		return 0; /* TODO */
	case MSM_VIDC_INPUT:
		if (domain == MSM_VIDC_DECODER)
			return HFI_PORT_BITSTREAM;
		else
			return HFI_PORT_RAW;
	case MSM_VIDC_OUTPUT:
		if (domain == MSM_VIDC_DECODER)
			return HFI_PORT_RAW;
		else
			return HFI_PORT_BITSTREAM;
	case MSM_VIDC_INPUT_META:
	case MSM_VIDC_OUTPUT_META:
		return HFI_BUFFER_METADATA;
	case MSM_VIDC_DPB:
		return HFI_BUFFER_DPB;
	case MSM_VIDC_ARP:
		return HFI_BUFFER_ARP;
	case MSM_VIDC_LINE:
		return HFI_BUFFER_LINE;
	case MSM_VIDC_BIN:
		return HFI_BUFFER_BIN;
	default:
		d_vpr_e("%s: Invalid buffer type %d\n",
			__func__, buf_type);
		return 0;
	}
}

static u32 get_hfi_buffer_flags(enum msm_vidc_buffer_attributes attr)
{
	switch (attr) {
	case MSM_VIDC_DEFERRED_SUBMISSION:
		return 0; /*not sure what it should be mapped to??*/
	case MSM_VIDC_READ_ONLY:
		return HFI_BUF_HOST_FLAG_READONLY;
	case MSM_VIDC_PENDING_RELEASE:
		return HFI_BUF_HOST_FLAG_RELEASE;
	default:
		d_vpr_e("%s: Invalid buffer attribute %d\n",
			__func__, attr);
		return 0;
	}
}

int hfi_create_header(u8 *pkt, u32 session_id,
	u32 header_id, u32 num_packets, u32 total_size)
{
	struct hfi_header *hdr = (struct hfi_header *)pkt;

	memset(hdr, 0, sizeof(struct hfi_header));

	hdr->size = total_size;
	hdr->session_id = session_id;
	hdr->header_id = header_id;
	hdr->num_packets = num_packets;
	return 0;
}

int hfi_create_packet(u8 *packet, u32 packet_size, u32 *offset,
	u32 pkt_type, u32 pkt_flags, u32 payload_type, u32 port,
	u32 packet_id, void *payload, u32 payload_size)
{
	u32 available_size = packet_size - *offset;
	u32 pkt_size = sizeof(struct hfi_packet) + payload_size;
	struct hfi_packet *pkt = (struct hfi_packet *)(packet + *offset);

	if (available_size < pkt_size) {
		d_vpr_e("%s: Bad packet Size for packet type %d\n",
			__func__, pkt_type);
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
	*offset = *offset + pkt->size;
	return 0;
}

int hfi_create_buffer(u8 *packet, u32 packet_size, u32 *offset,
	enum msm_vidc_domain_type domain, struct msm_vidc_buffer *data)
{
	u32 available_size = packet_size - *offset;
	u32 buf_size = sizeof(struct hfi_buffer);
	struct hfi_buffer *buf = (struct hfi_buffer *)packet;

	if (available_size < sizeof(struct hfi_buffer)) {
		d_vpr_e("%s: Bad buffer Size for buffer type %d\n",
			__func__, data->type);
		return -EINVAL;
	}

	memset(buf, 0, buf_size);

	buf->type = get_hfi_buffer_type(domain, data->type);
	buf->index = data->index;
	buf->base_address = data->device_addr;
	buf->addr_offset = 0;
	buf->buffer_size = data->buffer_size;
	buf->data_offset = data->data_offset;
	buf->data_size = data->data_size;
	buf->flags = get_hfi_buffer_flags(data->attr);
	buf->timestamp = data->timestamp;
	*offset = *offset + buf_size;
	return 0;
}

int hfi_packet_sys_init(struct msm_vidc_core *core,
	u8 *pkt, u32 pkt_size)
{
	int rc = 0;
	u32 offset = 0, payload = 0, num_packets = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	if (pkt_size < sizeof(struct hfi_header)) {
		d_vpr_e("%s: Invalid packet size\n", __func__);
		return -EINVAL;
	}

	/* HFI_CMD_SYSTEM_INIT */
	offset = sizeof(struct hfi_header);
	payload = HFI_VIDEO_ARCH_OX;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
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
	num_packets++;

	/* HFI_PROP_INTRA_FRAME_POWER_COLLAPSE */
	payload = 0;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_INTRA_FRAME_POWER_COLLAPSE,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_MAX_CHANNELS */
	payload = core->platform->data.ubwc_config->max_channels;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_MAX_CHANNELS,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_MAL_LENGTH */
	payload = core->platform->data.ubwc_config->mal_length;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_MAL_LENGTH,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_HBB */
	payload = core->platform->data.ubwc_config->highest_bank_bit;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_HBB,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_BANK_SWZL_LEVEL1 */
	payload = core->platform->data.ubwc_config->bank_swzl_level;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_BANK_SWZL_LEVEL1,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_BANK_SWZL_LEVEL2 */
	payload = core->platform->data.ubwc_config->bank_swz2_level;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_BANK_SWZL_LEVEL2,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_BANK_SWZL_LEVEL3 */
	payload = core->platform->data.ubwc_config->bank_swz3_level;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_BANK_SWZL_LEVEL3,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	/* HFI_PROP_UBWC_BANK_SPREADING */
	payload = core->platform->data.ubwc_config->bank_spreading;
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_UBWC_BANK_SPREADING,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_sys_init;
	num_packets++;

	rc = hfi_create_header(pkt, 0 /*session_id*/,
				   core->header_id++,
				   num_packets,
				   offset);

	if (rc)
		goto err_sys_init;

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
	u32 num_packets = 0, offset = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	/* HFI_PROP_IMAGE_VERSION */
	offset = sizeof(struct hfi_header);
	rc = hfi_create_packet(pkt, pkt_size, &offset,
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
	num_packets++;

	rc = hfi_create_header(pkt, 0 /*session_id*/,
				   core->header_id++,
				   num_packets,
				   offset);

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
	u32 num_packets = 0, offset = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	/* HFI_CMD_POWER_COLLAPSE */
	offset = sizeof(struct hfi_header);
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_CMD_POWER_COLLAPSE,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_NONE,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   NULL, 0);
	if (rc)
		goto err_sys_pc;
	num_packets++;

	rc = hfi_create_header(pkt, 0 /*session_id*/,
				   core->header_id++,
				   num_packets,
				   offset);

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
	u32 num_packets = 0, offset = 0, payload = 0;

	if (!core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	/* HFI_PROP_DEBUG_CONFIG */
	offset = sizeof(struct hfi_header);
	payload = debug_config; /*TODO:Change later*/
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_DEBUG_CONFIG,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32_ENUM,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_debug;
	num_packets++;

	/* HFI_PROP_DEBUG_LOG_LEVEL */
	payload = debug_config; /*TODO:Change later*/
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				   HFI_PROP_DEBUG_LOG_LEVEL,
				   HFI_HOST_FLAGS_NONE,
				   HFI_PAYLOAD_U32_ENUM,
				   HFI_PORT_NONE,
				   core->packet_id++,
				   &payload,
				   sizeof(u32));
	if (rc)
		goto err_debug;
	num_packets++;

	rc = hfi_create_header(pkt, 0 /*session_id*/,
				   core->header_id++,
				   num_packets,
				   offset);

	if (rc)
		goto err_debug;

	d_vpr_h("Debug packet created\n");
	return rc;

err_debug:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}

int hfi_packet_session_command(struct msm_vidc_inst *inst,
	u32 pkt_type, u32 flags, u32 port, u32 session_id,
	u32 payload_type, void *payload, u32 payload_size)
{
	int rc = 0;
	u32 num_packets = 0, offset = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	core = inst->core;
	offset = sizeof(struct hfi_header);
	rc = hfi_create_packet(inst->packet,
				inst->packet_size,
				&offset,
				pkt_type,
				flags,
				payload_type,
				port,
				core->packet_id++,
				payload,
				payload_size);
	if (rc)
		goto err_cmd;
	num_packets++;

	rc = hfi_create_header(inst->packet, session_id,
				   core->header_id++,
				   num_packets,
				   offset);

	if (rc)
		goto err_cmd;

	d_vpr_h("Command packet 0x%x created\n", pkt_type);
	return rc;

err_cmd:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}

int hfi_packet_create_property(struct msm_vidc_inst *inst,
	void *pkt, u32 pkt_size, u32 pkt_type, u32 flags,
	u32 port, u32 payload, u32 payload_type, u32 payload_size)
{
	int rc = 0;
	u32 num_packets = 0, offset = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core || !pkt) {
		d_vpr_e("%s: Invalid params\n", __func__);
		return -EINVAL;
	}

	core = inst->core;
	offset = sizeof(struct hfi_header);
	rc = hfi_create_packet(pkt, pkt_size, &offset,
				pkt_type,
				flags,
				payload_type,
				port,
				core->packet_id++,
				&payload,
				payload_size);
	if (rc)
		goto err_prop;
	num_packets++;

	rc = hfi_create_header(pkt, inst->session_id,
				   core->header_id++,
				   num_packets,
				   offset);

	if (rc)
		goto err_prop;

	d_vpr_h("Property packet 0x%x created\n", pkt_type);
	return rc;

err_prop:
	d_vpr_e("%s: create packet failed\n", __func__);
	return rc;
}
