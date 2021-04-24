/*
 * Copyright (c) 2018-2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <osdep.h>
#include "wmi.h"
#include "wmi_unified_priv.h"
#include "wmi_unified_pmo_api.h"

#ifdef FEATURE_WLAN_D0WOW
/**
 *  send_d0wow_enable_cmd_tlv() - WMI d0 wow enable function
 *  @param wmi_handle: handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: 0  on success  and  error code on failure.
 */
static QDF_STATUS send_d0wow_enable_cmd_tlv(wmi_unified_t wmi_handle,
					    uint8_t mac_id)
{
	wmi_d0_wow_enable_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS status;

	len = sizeof(wmi_d0_wow_enable_disable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_d0_wow_enable_disable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_d0_wow_enable_disable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_d0_wow_enable_disable_cmd_fixed_param));

	cmd->enable = true;

	wmi_mtrace(WMI_D0_WOW_ENABLE_DISABLE_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_D0_WOW_ENABLE_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

/**
 *  send_d0wow_disable_cmd_tlv() - WMI d0 wow disable function
 *  @param wmi_handle: handle to WMI.
 *  @mac_id: radio context
 *
 *  Return: 0  on success  and  error code on failure.
 */
static QDF_STATUS send_d0wow_disable_cmd_tlv(wmi_unified_t wmi_handle,
					     uint8_t mac_id)
{
	wmi_d0_wow_enable_disable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	QDF_STATUS status;

	len = sizeof(wmi_d0_wow_enable_disable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (wmi_d0_wow_enable_disable_cmd_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_d0_wow_enable_disable_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_d0_wow_enable_disable_cmd_fixed_param));

	cmd->enable = false;

	wmi_mtrace(WMI_D0_WOW_ENABLE_DISABLE_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf, len,
				      WMI_D0_WOW_ENABLE_DISABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status))
		wmi_buf_free(buf);

	return status;
}

void wmi_d0wow_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_d0wow_enable_cmd = send_d0wow_enable_cmd_tlv;
	ops->send_d0wow_disable_cmd = send_d0wow_disable_cmd_tlv;
}
#endif /* FEATURE_WLAN_D0WOW */

/**
 * send_add_wow_wakeup_event_cmd_tlv() -  Configures wow wakeup events.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 * @enable: enable/disable
 *
 * Return: CDF status
 */
static QDF_STATUS send_add_wow_wakeup_event_cmd_tlv(wmi_unified_t wmi_handle,
						    uint32_t vdev_id,
						    uint32_t *bitmap,
						    bool enable)
{
	WMI_WOW_ADD_DEL_EVT_CMD_fixed_param *cmd;
	uint16_t len;
	wmi_buf_t buf;
	int ret;

	len = sizeof(WMI_WOW_ADD_DEL_EVT_CMD_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_ADD_DEL_EVT_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_ADD_DEL_EVT_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_WOW_ADD_DEL_EVT_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->is_add = enable;
	qdf_mem_copy(&(cmd->event_bitmaps[0]), bitmap, sizeof(uint32_t) *
		     WMI_WOW_MAX_EVENT_BM_LEN);

	wmi_debug("Wakeup pattern 0x%x%x%x%x %s in fw", cmd->event_bitmaps[0],
		 cmd->event_bitmaps[1], cmd->event_bitmaps[2],
		 cmd->event_bitmaps[3], enable ? "enabled" : "disabled");

	wmi_mtrace(WMI_WOW_ENABLE_DISABLE_WAKE_EVENT_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ENABLE_DISABLE_WAKE_EVENT_CMDID);
	if (ret) {
		wmi_err("Failed to config wow wakeup event");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_wow_patterns_to_fw_cmd_tlv() - Sends WOW patterns to FW.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @ptrn_id: pattern id
 * @ptrn: pattern
 * @ptrn_len: pattern length
 * @ptrn_offset: pattern offset
 * @mask: mask
 * @mask_len: mask length
 * @user: true for user configured pattern and false for default pattern
 * @default_patterns: default patterns
 *
 * Return: CDF status
 */
static QDF_STATUS send_wow_patterns_to_fw_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t vdev_id, uint8_t ptrn_id,
				const uint8_t *ptrn, uint8_t ptrn_len,
				uint8_t ptrn_offset, const uint8_t *mask,
				uint8_t mask_len, bool user,
				uint8_t default_patterns)
{
	WMI_WOW_ADD_PATTERN_CMD_fixed_param *cmd;
	WOW_BITMAP_PATTERN_T *bitmap_pattern;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param) +
		WMI_TLV_HDR_SIZE +
		1 * sizeof(WOW_BITMAP_PATTERN_T) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(WOW_IPV4_SYNC_PATTERN_T) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(WOW_IPV6_SYNC_PATTERN_T) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(WOW_MAGIC_PATTERN_CMD) +
		WMI_TLV_HDR_SIZE +
		0 * sizeof(uint32_t) + WMI_TLV_HDR_SIZE + 1 * sizeof(uint32_t);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_ADD_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_ADD_PATTERN_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_WOW_ADD_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = ptrn_id;

	cmd->pattern_type = WOW_BITMAP_PATTERN;
	buf_ptr += sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param);

	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(WOW_BITMAP_PATTERN_T));
	buf_ptr += WMI_TLV_HDR_SIZE;
	bitmap_pattern = (WOW_BITMAP_PATTERN_T *) buf_ptr;

	WMITLV_SET_HDR(&bitmap_pattern->tlv_header,
		       WMITLV_TAG_STRUC_WOW_BITMAP_PATTERN_T,
		       WMITLV_GET_STRUCT_TLVLEN(WOW_BITMAP_PATTERN_T));

	qdf_mem_copy(&bitmap_pattern->patternbuf[0], ptrn, ptrn_len);
	qdf_mem_copy(&bitmap_pattern->bitmaskbuf[0], mask, mask_len);

	bitmap_pattern->pattern_offset = ptrn_offset;
	bitmap_pattern->pattern_len = ptrn_len;

	if (bitmap_pattern->pattern_len > WOW_DEFAULT_BITMAP_PATTERN_SIZE)
		bitmap_pattern->pattern_len = WOW_DEFAULT_BITMAP_PATTERN_SIZE;

	if (bitmap_pattern->pattern_len > WOW_DEFAULT_BITMASK_SIZE)
		bitmap_pattern->pattern_len = WOW_DEFAULT_BITMASK_SIZE;

	bitmap_pattern->bitmask_len = bitmap_pattern->pattern_len;
	bitmap_pattern->pattern_id = ptrn_id;

	wmi_debug("vdev: %d, ptrn id: %d, ptrn len: %d, ptrn offset: %d user %d",
		 cmd->vdev_id, cmd->pattern_id, bitmap_pattern->pattern_len,
		 bitmap_pattern->pattern_offset, user);
	wmi_debug("Pattern: ");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   &bitmap_pattern->patternbuf[0],
			   bitmap_pattern->pattern_len);

	wmi_debug("Mask: ");
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_WMI, QDF_TRACE_LEVEL_DEBUG,
			   &bitmap_pattern->bitmaskbuf[0],
			   bitmap_pattern->pattern_len);

	buf_ptr += sizeof(WOW_BITMAP_PATTERN_T);

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV4_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV6_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_MAGIC_PATTERN_CMD but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for pattern_info_timeout but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for ratelimit_interval with dummy data as this fix elem */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 1 * sizeof(uint32_t));
	buf_ptr += WMI_TLV_HDR_SIZE;
	*(uint32_t *) buf_ptr = 0;

	wmi_mtrace(WMI_WOW_ADD_WAKE_PATTERN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ADD_WAKE_PATTERN_CMDID);
	if (ret) {
		wmi_err("Failed to send wow ptrn to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * fill_arp_offload_params_tlv() - Fill ARP offload data
 * @wmi_handle: wmi handle
 * @offload_req: offload request
 * @buf_ptr: buffer pointer
 *
 * To fill ARP offload data to firmware
 * when target goes to wow mode.
 *
 * Return: None
 */
static void fill_arp_offload_params_tlv(wmi_unified_t wmi_handle,
		struct pmo_arp_offload_params *offload_req, uint8_t **buf_ptr)
{

	int i;
	WMI_ARP_OFFLOAD_TUPLE *arp_tuple;
	bool enable_or_disable = offload_req->enable;

	WMITLV_SET_HDR(*buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(WMI_MAX_ARP_OFFLOADS*sizeof(WMI_ARP_OFFLOAD_TUPLE)));
	*buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < WMI_MAX_ARP_OFFLOADS; i++) {
		arp_tuple = (WMI_ARP_OFFLOAD_TUPLE *)*buf_ptr;
		WMITLV_SET_HDR(&arp_tuple->tlv_header,
			WMITLV_TAG_STRUC_WMI_ARP_OFFLOAD_TUPLE,
			WMITLV_GET_STRUCT_TLVLEN(WMI_ARP_OFFLOAD_TUPLE));

		/* Fill data for ARP and NS in the first tupple for LA */
		if ((enable_or_disable & PMO_OFFLOAD_ENABLE) && (i == 0)) {
			/* Copy the target ip addr and flags */
			arp_tuple->flags = WMI_ARPOFF_FLAGS_VALID;
			qdf_mem_copy(&arp_tuple->target_ipaddr,
					offload_req->host_ipv4_addr,
					WMI_IPV4_ADDR_LEN);
			wmi_debug("ARPOffload IP4 address: %pI4",
				 offload_req->host_ipv4_addr);
		}
		*buf_ptr += sizeof(WMI_ARP_OFFLOAD_TUPLE);
	}
}

#ifdef WLAN_NS_OFFLOAD
/**
 * fill_ns_offload_params_tlv() - Fill NS offload data
 * @wmi|_handle: wmi handle
 * @offload_req: offload request
 * @buf_ptr: buffer pointer
 *
 * To fill NS offload data to firmware
 * when target goes to wow mode.
 *
 * Return: None
 */
static void fill_ns_offload_params_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{

	int i;
	WMI_NS_OFFLOAD_TUPLE *ns_tuple;

	WMITLV_SET_HDR(*buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       (WMI_MAX_NS_OFFLOADS * sizeof(WMI_NS_OFFLOAD_TUPLE)));
	*buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = 0; i < WMI_MAX_NS_OFFLOADS; i++) {
		ns_tuple = (WMI_NS_OFFLOAD_TUPLE *)*buf_ptr;
		WMITLV_SET_HDR(&ns_tuple->tlv_header,
			WMITLV_TAG_STRUC_WMI_NS_OFFLOAD_TUPLE,
			(sizeof(WMI_NS_OFFLOAD_TUPLE) - WMI_TLV_HDR_SIZE));

		/*
		 * Fill data only for NS offload in the first ARP tuple for LA
		 */
		if ((ns_req->enable & PMO_OFFLOAD_ENABLE)) {
			ns_tuple->flags |= WMI_NSOFF_FLAGS_VALID;
			/* Copy the target/solicitation/remote ip addr */
			if (ns_req->target_ipv6_addr_valid[i])
				qdf_mem_copy(&ns_tuple->target_ipaddr[0],
					&ns_req->target_ipv6_addr[i],
					sizeof(WMI_IPV6_ADDR));
			qdf_mem_copy(&ns_tuple->solicitation_ipaddr,
				&ns_req->self_ipv6_addr[i],
				sizeof(WMI_IPV6_ADDR));
			if (ns_req->target_ipv6_addr_ac_type[i]) {
				ns_tuple->flags |=
					WMI_NSOFF_FLAGS_IS_IPV6_ANYCAST;
			}
			wmi_debug("Index %d NS solicitedIp %pI6, targetIp %pI6",
				i, &ns_req->self_ipv6_addr[i],
				&ns_req->target_ipv6_addr[i]);

			/* target MAC is optional, check if it is valid,
			 * if this is not valid, the target will use the known
			 * local MAC address rather than the tuple
			 */
			WMI_CHAR_ARRAY_TO_MAC_ADDR(
				ns_req->self_macaddr.bytes,
				&ns_tuple->target_mac);
			if ((ns_tuple->target_mac.mac_addr31to0 != 0) ||
				(ns_tuple->target_mac.mac_addr47to32 != 0)) {
				ns_tuple->flags |= WMI_NSOFF_FLAGS_MAC_VALID;
			}
		}
		*buf_ptr += sizeof(WMI_NS_OFFLOAD_TUPLE);
	}
}

/**
 * fill_nsoffload_ext_tlv() - Fill NS offload ext data
 * @wmi: wmi handle
 * @offload_req: offload request
 * @buf_ptr: buffer pointer
 *
 * To fill extended NS offload extended data to firmware
 * when target goes to wow mode.
 *
 * Return: None
 */
static void fill_nsoffload_ext_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{
	int i;
	WMI_NS_OFFLOAD_TUPLE *ns_tuple;
	uint32_t count, num_ns_ext_tuples;

	count = ns_req->num_ns_offload_count;
	num_ns_ext_tuples = ns_req->num_ns_offload_count -
		WMI_MAX_NS_OFFLOADS;

	/* Populate extended NS offload tuples */
	WMITLV_SET_HDR(*buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		(num_ns_ext_tuples * sizeof(WMI_NS_OFFLOAD_TUPLE)));
	*buf_ptr += WMI_TLV_HDR_SIZE;
	for (i = WMI_MAX_NS_OFFLOADS; i < count; i++) {
		ns_tuple = (WMI_NS_OFFLOAD_TUPLE *)*buf_ptr;
		WMITLV_SET_HDR(&ns_tuple->tlv_header,
			WMITLV_TAG_STRUC_WMI_NS_OFFLOAD_TUPLE,
			(sizeof(WMI_NS_OFFLOAD_TUPLE)-WMI_TLV_HDR_SIZE));

		/*
		 * Fill data only for NS offload in the first ARP tuple for LA
		 */
		if ((ns_req->enable & PMO_OFFLOAD_ENABLE)) {
			ns_tuple->flags |= WMI_NSOFF_FLAGS_VALID;
			/* Copy the target/solicitation/remote ip addr */
			if (ns_req->target_ipv6_addr_valid[i])
				qdf_mem_copy(&ns_tuple->target_ipaddr[0],
					&ns_req->target_ipv6_addr[i],
					sizeof(WMI_IPV6_ADDR));
			qdf_mem_copy(&ns_tuple->solicitation_ipaddr,
				&ns_req->self_ipv6_addr[i],
				sizeof(WMI_IPV6_ADDR));
			if (ns_req->target_ipv6_addr_ac_type[i]) {
				ns_tuple->flags |=
					WMI_NSOFF_FLAGS_IS_IPV6_ANYCAST;
			}
			wmi_debug("Index %d NS solicitedIp %pI6, targetIp %pI6",
				i, &ns_req->self_ipv6_addr[i],
				&ns_req->target_ipv6_addr[i]);

			/* target MAC is optional, check if it is valid,
			 * if this is not valid, the target will use the
			 * known local MAC address rather than the tuple
			 */
			 WMI_CHAR_ARRAY_TO_MAC_ADDR(
				ns_req->self_macaddr.bytes,
				&ns_tuple->target_mac);
			if ((ns_tuple->target_mac.mac_addr31to0 != 0) ||
				(ns_tuple->target_mac.mac_addr47to32 != 0)) {
				ns_tuple->flags |= WMI_NSOFF_FLAGS_MAC_VALID;
			}
		}
		*buf_ptr += sizeof(WMI_NS_OFFLOAD_TUPLE);
	}
}
#else
static void fill_ns_offload_params_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{
}

static void fill_nsoffload_ext_tlv(wmi_unified_t wmi_handle,
		struct pmo_ns_offload_params *ns_req, uint8_t **buf_ptr)
{
}
#endif

/**
 * send_enable_arp_ns_offload_cmd_tlv() - enable ARP NS offload
 * @wma: wmi handle
 * @arp_offload_req: arp offload request
 * @ns_offload_req: ns offload request
 * @arp_only: flag
 *
 * To configure ARP NS off load data to firmware
 * when target goes to wow mode.
 *
 * Return: QDF Status
 */
static QDF_STATUS send_enable_arp_ns_offload_cmd_tlv(wmi_unified_t wmi_handle,
			   struct pmo_arp_offload_params *arp_offload_req,
			   struct pmo_ns_offload_params *ns_offload_req,
			   uint8_t vdev_id)
{
	int32_t res;
	WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param *cmd;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	int32_t len;
	uint32_t count = 0, num_ns_ext_tuples = 0;

	count = ns_offload_req->num_ns_offload_count;

	/*
	 * TLV place holder size for array of NS tuples
	 * TLV place holder size for array of ARP tuples
	 */
	len = sizeof(WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param) +
		WMI_TLV_HDR_SIZE +
		WMI_MAX_NS_OFFLOADS * sizeof(WMI_NS_OFFLOAD_TUPLE) +
		WMI_TLV_HDR_SIZE +
		WMI_MAX_ARP_OFFLOADS * sizeof(WMI_ARP_OFFLOAD_TUPLE);

	/*
	 * If there are more than WMI_MAX_NS_OFFLOADS addresses then allocate
	 * extra length for extended NS offload tuples which follows ARP offload
	 * tuples. Host needs to fill this structure in following format:
	 * 2 NS ofload tuples
	 * 2 ARP offload tuples
	 * N numbers of extended NS offload tuples if HDD has given more than
	 * 2 NS offload addresses
	 */
	if (count > WMI_MAX_NS_OFFLOADS) {
		num_ns_ext_tuples = count - WMI_MAX_NS_OFFLOADS;
		len += WMI_TLV_HDR_SIZE + num_ns_ext_tuples
			   * sizeof(WMI_NS_OFFLOAD_TUPLE);
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param));
	cmd->flags = 0;
	cmd->vdev_id = vdev_id;
	cmd->num_ns_ext_tuples = num_ns_ext_tuples;

	wmi_debug("ARP NS Offload vdev_id: %d", cmd->vdev_id);

	buf_ptr += sizeof(WMI_SET_ARP_NS_OFFLOAD_CMD_fixed_param);
	fill_ns_offload_params_tlv(wmi_handle, ns_offload_req, &buf_ptr);
	fill_arp_offload_params_tlv(wmi_handle, arp_offload_req, &buf_ptr);
	if (num_ns_ext_tuples)
		fill_nsoffload_ext_tlv(wmi_handle, ns_offload_req, &buf_ptr);

	wmi_mtrace(WMI_SET_ARP_NS_OFFLOAD_CMDID, cmd->vdev_id, 0);
	res = wmi_unified_cmd_send(wmi_handle, buf, len,
				     WMI_SET_ARP_NS_OFFLOAD_CMDID);
	if (res) {
		wmi_err("Failed to enable ARP NDP/NSffload");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_add_clear_mcbc_filter_cmd_tlv() - set mcast filter command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @multicastAddr: mcast address
 * @clearList: clear list flag
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_add_clear_mcbc_filter_cmd_tlv(wmi_unified_t wmi_handle,
				     uint8_t vdev_id,
				     struct qdf_mac_addr multicast_addr,
				     bool clearList)
{
	WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int err;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param *) wmi_buf_data(buf);
	qdf_mem_zero(cmd, sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_SET_MCASTBCAST_FILTER_CMD_fixed_param));
	cmd->action =
		(clearList ? WMI_MCAST_FILTER_DELETE : WMI_MCAST_FILTER_SET);
	cmd->vdev_id = vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(multicast_addr.bytes, &cmd->mcastbdcastaddr);

	wmi_debug("Action:%d; vdev_id:%d; clearList:%d; MCBC MAC Addr: "QDF_MAC_ADDR_FMT,
		 cmd->action, vdev_id, clearList,
		 QDF_MAC_ADDR_REF(multicast_addr.bytes));

	wmi_mtrace(WMI_SET_MCASTBCAST_FILTER_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   sizeof(*cmd),
				   WMI_SET_MCASTBCAST_FILTER_CMDID);
	if (err) {
		wmi_err("Failed to send set_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_multiple_add_clear_mcbc_filter_cmd_tlv() - send multiple  mcast filter
 *						   command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @mcast_filter_params: mcast filter params
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_multiple_add_clear_mcbc_filter_cmd_tlv(
				wmi_unified_t wmi_handle,
				uint8_t vdev_id,
				struct pmo_mcast_filter_params *filter_param)

{
	WMI_SET_MULTIPLE_MCAST_FILTER_CMD_fixed_param *cmd;
	uint8_t *buf_ptr;
	wmi_buf_t buf;
	int err;
	int i;
	uint8_t *mac_addr_src_ptr = NULL;
	wmi_mac_addr *mac_addr_dst_ptr;
	uint32_t len = sizeof(*cmd) + WMI_TLV_HDR_SIZE +
		       sizeof(wmi_mac_addr) * filter_param->multicast_addr_cnt;

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	cmd = (WMI_SET_MULTIPLE_MCAST_FILTER_CMD_fixed_param *)
		wmi_buf_data(buf);
	qdf_mem_zero(cmd, sizeof(*cmd));

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_set_multiple_mcast_filter_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (WMI_SET_MULTIPLE_MCAST_FILTER_CMD_fixed_param));
	cmd->operation =
		((filter_param->action == 0) ? WMI_MULTIPLE_MCAST_FILTER_DELETE
					: WMI_MULTIPLE_MCAST_FILTER_ADD);
	cmd->vdev_id = vdev_id;
	cmd->num_mcastaddrs = filter_param->multicast_addr_cnt;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       sizeof(wmi_mac_addr) *
			       filter_param->multicast_addr_cnt);

	if (filter_param->multicast_addr_cnt == 0)
		goto send_cmd;

	mac_addr_src_ptr = (uint8_t *)&filter_param->multicast_addr;
	mac_addr_dst_ptr = (wmi_mac_addr *)
			(buf_ptr + WMI_TLV_HDR_SIZE);

	for (i = 0; i < filter_param->multicast_addr_cnt; i++) {
		WMI_CHAR_ARRAY_TO_MAC_ADDR(mac_addr_src_ptr, mac_addr_dst_ptr);
		mac_addr_src_ptr += ATH_MAC_LEN;
		mac_addr_dst_ptr++;
	}

send_cmd:
	wmi_mtrace(WMI_SET_MULTIPLE_MCAST_FILTER_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   len,
				   WMI_SET_MULTIPLE_MCAST_FILTER_CMDID);
	if (err) {
		wmi_err("Failed to send set_param cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS send_conf_hw_filter_cmd_tlv(wmi_unified_t wmi,
					      struct pmo_hw_filter_params *req)
{
	QDF_STATUS status;
	wmi_hw_data_filter_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;

	if (!req) {
		wmi_err("req is null");
		return QDF_STATUS_E_INVAL;
	}

	wmi_buf = wmi_buf_alloc(wmi, sizeof(*cmd));
	if (!wmi_buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_hw_data_filter_cmd_fixed_param *)wmi_buf_data(wmi_buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		  WMITLV_TAG_STRUC_wmi_hw_data_filter_cmd_fixed_param,
		  WMITLV_GET_STRUCT_TLVLEN(wmi_hw_data_filter_cmd_fixed_param));
	cmd->vdev_id = req->vdev_id;
	cmd->enable = req->enable;
	/* Set all modes in case of disable */
	if (!cmd->enable)
		cmd->hw_filter_bitmap = ((uint32_t)~0U);
	else
		cmd->hw_filter_bitmap = req->mode_bitmap;

	wmi_debug("Send %s hw filter mode: 0x%X for vdev id %d",
		 req->enable ? "enable" : "disable", req->mode_bitmap,
		 req->vdev_id);

	wmi_mtrace(WMI_HW_DATA_FILTER_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi, wmi_buf, sizeof(*cmd),
				      WMI_HW_DATA_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("Failed to configure hw filter");
		wmi_buf_free(wmi_buf);
	}

	return status;
}

static void
fill_fils_tlv_params(WMI_GTK_OFFLOAD_CMD_fixed_param *cmd,
			  uint8_t vdev_id,
			  struct pmo_gtk_req *params)
{
	uint8_t *buf_ptr;
	wmi_gtk_offload_fils_tlv_param *ext_param;

	buf_ptr = (uint8_t *) cmd + sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
		       sizeof(*ext_param));
	buf_ptr += WMI_TLV_HDR_SIZE;

	ext_param = (wmi_gtk_offload_fils_tlv_param *)buf_ptr;
	WMITLV_SET_HDR(&ext_param->tlv_header,
		       WMITLV_TAG_STRUC_wmi_gtk_offload_extended_tlv_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				wmi_gtk_offload_fils_tlv_param));
	ext_param->vdev_id = vdev_id;
	ext_param->flags = cmd->flags;
	ext_param->kek_len = params->kek_len;
	qdf_mem_copy(ext_param->KEK, params->kek, params->kek_len);
	qdf_mem_copy(ext_param->KCK, params->kck,
		     WMI_GTK_OFFLOAD_KCK_BYTES);
	qdf_mem_copy(ext_param->replay_counter, &params->replay_counter,
		     GTK_REPLAY_COUNTER_BYTES);
}

#ifdef WLAN_FEATURE_IGMP_OFFLOAD
/**
 * send_igmp_offload_cmd_tlv() - send IGMP offload command to fw
 * @wmi_handle: wmi handle
 * @params: IGMP offload parameters
 *
 * Return: CDF status
 */
static
QDF_STATUS send_igmp_offload_cmd_tlv(wmi_unified_t wmi_handle,
				     struct pmo_igmp_offload_req *pmo_igmp_req)
{
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int len;
	int i = 0;
	WMI_IPV4_ADDR *ipv4_list;
	wmi_igmp_offload_fixed_param *cmd;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	len = sizeof(wmi_igmp_offload_fixed_param) + WMI_TLV_HDR_SIZE +
	     (pmo_igmp_req->num_grp_ip_address) * sizeof(WMI_IPV4_ADDR);
	/* alloc wmi buffer */
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}
	buf_ptr = (uint8_t *)wmi_buf_data(buf);
	cmd = (wmi_igmp_offload_fixed_param *)wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_igmp_offload_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(wmi_igmp_offload_fixed_param));

	cmd->vdev_id = pmo_igmp_req->vdev_id;
	cmd->enable = pmo_igmp_req->enable;
	cmd->version_support_bitmask =
				pmo_igmp_req->version_support;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC,
		       sizeof(WMI_IPV4_ADDR) *
		       pmo_igmp_req->num_grp_ip_address);

	ipv4_list = (WMI_IPV4_ADDR *)(buf_ptr + WMI_TLV_HDR_SIZE);

	while (i < pmo_igmp_req->num_grp_ip_address) {
		qdf_mem_copy((void *)((*(ipv4_list + i)).address),
			     (void *)&(pmo_igmp_req->grp_ip_address[i]),
			     WMI_IPV4_ADDR_LEN);
		wmi_debug("piv4[%d]:%x", i, *(uint32_t *)(ipv4_list + i));
		i++;
	}

	wmi_debug("VDEVID:%d, FLAG:x%x version support:%d",
		  cmd->vdev_id, cmd->enable,
		  cmd->version_support_bitmask);

	/* send the wmi command */
	wmi_mtrace(WMI_VDEV_IGMP_OFFLOAD_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_VDEV_IGMP_OFFLOAD_CMDID)) {
		wmi_err("Failed to send WMI_VDEV_IGMP_OFFLOAD_CMDID");
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
	}
out:
	return status;
}
#endif

/**
 * send_gtk_offload_cmd_tlv() - send GTK offload command to fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @params: GTK offload parameters
 *
 * Return: CDF status
 */
static
QDF_STATUS send_gtk_offload_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
				    struct pmo_gtk_req *params,
				    bool enable_offload,
				    uint32_t gtk_offload_opcode)
{
	int len;
	wmi_buf_t buf;
	WMI_GTK_OFFLOAD_CMD_fixed_param *cmd;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	len = sizeof(*cmd);

	if (params->is_fils_connection)
		len += WMI_TLV_HDR_SIZE +
		       sizeof(wmi_gtk_offload_fils_tlv_param);

	/* alloc wmi buffer */
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	cmd = (WMI_GTK_OFFLOAD_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_GTK_OFFLOAD_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_GTK_OFFLOAD_CMD_fixed_param));

	cmd->vdev_id = vdev_id;

	/* Request target to enable GTK offload */
	if (enable_offload == PMO_GTK_OFFLOAD_ENABLE) {
		cmd->flags = gtk_offload_opcode;

		/* Copy the keys and replay counter */
		qdf_mem_copy(cmd->KCK, params->kck, sizeof(cmd->KCK));
		qdf_mem_copy(cmd->KEK, params->kek, sizeof(cmd->KEK));
		qdf_mem_copy(cmd->replay_counter, &params->replay_counter,
			     GTK_REPLAY_COUNTER_BYTES);
	} else {
		cmd->flags = gtk_offload_opcode;
	}
	if (params->is_fils_connection)
		fill_fils_tlv_params(cmd, vdev_id, params);

	wmi_debug("VDEVID: %d, GTK_FLAGS: x%x kek len %d",
		 vdev_id, cmd->flags, params->kek_len);
	/* send the wmi command */
	wmi_mtrace(WMI_GTK_OFFLOAD_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_GTK_OFFLOAD_CMDID)) {
		wmi_err("Failed to send WMI_GTK_OFFLOAD_CMDID");
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
	}

out:
	return status;
}

/**
 * send_process_gtk_offload_getinfo_cmd_tlv() - send GTK offload cmd to fw
 * @wmi_handle: wmi handle
 * @params: GTK offload params
 *
 * Return: CDF status
 */
static QDF_STATUS send_process_gtk_offload_getinfo_cmd_tlv(
			wmi_unified_t wmi_handle,
			uint8_t vdev_id,
			uint64_t offload_req_opcode)
{
	int len;
	wmi_buf_t buf;
	WMI_GTK_OFFLOAD_CMD_fixed_param *cmd;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	len = sizeof(*cmd);

	/* alloc wmi buffer */
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		status = QDF_STATUS_E_NOMEM;
		goto out;
	}

	cmd = (WMI_GTK_OFFLOAD_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_GTK_OFFLOAD_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_GTK_OFFLOAD_CMD_fixed_param));

	/* Request for GTK offload status */
	cmd->flags = offload_req_opcode;
	cmd->vdev_id = vdev_id;

	/* send the wmi command */
	wmi_mtrace(WMI_GTK_OFFLOAD_CMDID, cmd->vdev_id, 0);
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
				 WMI_GTK_OFFLOAD_CMDID)) {
		wmi_err("Failed to send WMI_GTK_OFFLOAD_CMDID for req info");
		wmi_buf_free(buf);
		status = QDF_STATUS_E_FAILURE;
	}

out:
	return status;
}

/**
 * send_enable_enhance_multicast_offload_tlv() - send enhance multicast offload
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @action: true for enable else false
 *
 * To enable enhance multicast offload to firmware
 * when target goes to wow mode.
 *
 * Return: QDF Status
 */

static
QDF_STATUS send_enable_enhance_multicast_offload_tlv(
		wmi_unified_t wmi_handle,
		uint8_t vdev_id, bool action)
{
	QDF_STATUS status;
	wmi_buf_t buf;
	wmi_config_enhanced_mcast_filter_cmd_fixed_param *cmd;

	buf = wmi_buf_alloc(wmi_handle, sizeof(*cmd));
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_config_enhanced_mcast_filter_cmd_fixed_param *)
							wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_config_enhanced_mcast_filter_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			wmi_config_enhanced_mcast_filter_cmd_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->enable = ((action == 0) ? ENHANCED_MCAST_FILTER_DISABLED :
			ENHANCED_MCAST_FILTER_ENABLED);
	wmi_debug("config enhance multicast offload action %d for vdev %d",
		 action, vdev_id);
	wmi_mtrace(WMI_CONFIG_ENHANCED_MCAST_FILTER_CMDID, cmd->vdev_id, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
			sizeof(*cmd), WMI_CONFIG_ENHANCED_MCAST_FILTER_CMDID);
	if (status != QDF_STATUS_SUCCESS) {
		wmi_buf_free(buf);
		wmi_err("Failed to send ENHANCED_MCAST_FILTER_CMDID");
	}

	return status;
}

/**
 * extract_gtk_rsp_event_tlv() - extract gtk rsp params from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param hdr: Pointer to hold header
 * @param bufp: Pointer to hold pointer to rx param buffer
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS extract_gtk_rsp_event_tlv(wmi_unified_t wmi_handle,
	void *evt_buf, struct pmo_gtk_rsp_params *gtk_rsp_param, uint32_t len)
{
	WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param *fixed_param;
	WMI_GTK_OFFLOAD_STATUS_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_GTK_OFFLOAD_STATUS_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("gtk param_buf is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (len < sizeof(WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param)) {
		wmi_err("Invalid length for GTK status");
		return QDF_STATUS_E_INVAL;
	}

	fixed_param = (WMI_GTK_OFFLOAD_STATUS_EVENT_fixed_param *)
		param_buf->fixed_param;

	if (fixed_param->vdev_id >= WLAN_UMAC_PSOC_MAX_VDEVS) {
		wmi_err_rl("Invalid vdev_id %u", fixed_param->vdev_id);
		return QDF_STATUS_E_INVAL;
	}

	gtk_rsp_param->vdev_id = fixed_param->vdev_id;
	gtk_rsp_param->status_flag = QDF_STATUS_SUCCESS;
	gtk_rsp_param->refresh_cnt = fixed_param->refresh_cnt;
	qdf_mem_copy(&gtk_rsp_param->replay_counter,
		&fixed_param->replay_counter,
		GTK_REPLAY_COUNTER_BYTES);

	return QDF_STATUS_SUCCESS;

}

#ifdef FEATURE_WLAN_RA_FILTERING
/**
 * send_wow_sta_ra_filter_cmd_tlv() - set RA filter pattern in fw
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_wow_sta_ra_filter_cmd_tlv(wmi_unified_t wmi_handle,
						 uint8_t vdev_id,
						 uint8_t default_pattern,
						 uint16_t rate_limit_interval)
{

	WMI_WOW_ADD_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_BITMAP_PATTERN_T) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_IPV4_SYNC_PATTERN_T) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_IPV6_SYNC_PATTERN_T) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(WOW_MAGIC_PATTERN_CMD) +
	      WMI_TLV_HDR_SIZE +
	      0 * sizeof(uint32_t) + WMI_TLV_HDR_SIZE + 1 * sizeof(uint32_t);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_ADD_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_ADD_PATTERN_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (WMI_WOW_ADD_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = default_pattern,
	cmd->pattern_type = WOW_IPV6_RA_PATTERN;
	buf_ptr += sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param);

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_BITMAP_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV4_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV6_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_MAGIC_PATTERN_CMD but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for pattern_info_timeout but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for ra_ratelimit_interval. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, sizeof(uint32_t));
	buf_ptr += WMI_TLV_HDR_SIZE;

	*((uint32_t *) buf_ptr) = rate_limit_interval;

	wmi_debug("send RA rate limit [%d] to fw vdev = %d",
		 rate_limit_interval, vdev_id);

	wmi_mtrace(WMI_WOW_ADD_WAKE_PATTERN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_ADD_WAKE_PATTERN_CMDID);
	if (ret) {
		wmi_err("Failed to send RA rate limit to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void wmi_ra_filtering_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_wow_sta_ra_filter_cmd = send_wow_sta_ra_filter_cmd_tlv;
}
#endif /* FEATURE_WLAN_RA_FILTERING */

/**
 * send_action_frame_patterns_cmd_tlv() - send wmi cmd of action filter params
 * @wmi_handle: wmi handler
 * @action_params: pointer to action_params
 *
 * Return: 0 for success, otherwise appropriate error code
 */
static QDF_STATUS send_action_frame_patterns_cmd_tlv(wmi_unified_t wmi_handle,
		struct pmo_action_wakeup_set_params *action_params)
{
	WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int i;
	int32_t err;
	uint32_t len = 0, *cmd_args;
	uint8_t *buf_ptr;

	len = (PMO_SUPPORTED_ACTION_CATE * sizeof(uint32_t))
				+ WMI_TLV_HDR_SIZE + sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}
	cmd = (WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *)cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_wow_set_action_wake_up_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
				WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param));

	cmd->vdev_id = action_params->vdev_id;
	cmd->operation = action_params->operation;

	for (i = 0; i < MAX_SUPPORTED_ACTION_CATEGORY_ELE_LIST; i++)
		cmd->action_category_map[i] =
				action_params->action_category_map[i];

	buf_ptr += sizeof(WMI_WOW_SET_ACTION_WAKE_UP_CMD_fixed_param);
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32,
		       (PMO_SUPPORTED_ACTION_CATE * sizeof(uint32_t)));
	buf_ptr += WMI_TLV_HDR_SIZE;
	cmd_args = (uint32_t *) buf_ptr;
	for (i = 0; i < PMO_SUPPORTED_ACTION_CATE; i++)
		cmd_args[i] = action_params->action_per_category[i];

	wmi_mtrace(WMI_WOW_SET_ACTION_WAKE_UP_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf,
				   len, WMI_WOW_SET_ACTION_WAKE_UP_CMDID);
	if (err) {
		wmi_err("Failed to send ap_ps_egap cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_LPHB
/**
 * send_lphb_config_hbenable_cmd_tlv() - enable command of LPHB configuration
 * @wmi_handle: wmi handle
 * @lphb_conf_req: configuration info
 *
 * Return: CDF status
 */
static QDF_STATUS send_lphb_config_hbenable_cmd_tlv(wmi_unified_t wmi_handle,
				wmi_hb_set_enable_cmd_fixed_param *params)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_enable_cmd_fixed_param *hb_enable_fp;
	int len = sizeof(wmi_hb_set_enable_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_enable_fp = (wmi_hb_set_enable_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_enable_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_hb_set_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_hb_set_enable_cmd_fixed_param));

	/* fill in values */
	hb_enable_fp->vdev_id = params->session;
	hb_enable_fp->enable = params->enable;
	hb_enable_fp->item = params->item;
	hb_enable_fp->session = params->session;

	wmi_mtrace(WMI_HB_SET_ENABLE_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_ENABLE_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("cmd_send WMI_HB_SET_ENABLE returned Error %d",
			 status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_tcp_params_cmd_tlv() - set tcp params of LPHB configuration
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static QDF_STATUS send_lphb_config_tcp_params_cmd_tlv(wmi_unified_t wmi_handle,
	    wmi_hb_set_tcp_params_cmd_fixed_param *lphb_conf_req)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_tcp_params_cmd_fixed_param *hb_tcp_params_fp;
	int len = sizeof(wmi_hb_set_tcp_params_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_tcp_params_fp = (wmi_hb_set_tcp_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_tcp_params_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_hb_set_tcp_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_hb_set_tcp_params_cmd_fixed_param));

	/* fill in values */
	hb_tcp_params_fp->vdev_id = lphb_conf_req->vdev_id;
	hb_tcp_params_fp->srv_ip = lphb_conf_req->srv_ip;
	hb_tcp_params_fp->dev_ip = lphb_conf_req->dev_ip;
	hb_tcp_params_fp->seq = lphb_conf_req->seq;
	hb_tcp_params_fp->src_port = lphb_conf_req->src_port;
	hb_tcp_params_fp->dst_port = lphb_conf_req->dst_port;
	hb_tcp_params_fp->interval = lphb_conf_req->interval;
	hb_tcp_params_fp->timeout = lphb_conf_req->timeout;
	hb_tcp_params_fp->session = lphb_conf_req->session;
	qdf_mem_copy(&hb_tcp_params_fp->gateway_mac,
		     &lphb_conf_req->gateway_mac,
		     sizeof(hb_tcp_params_fp->gateway_mac));

	wmi_mtrace(WMI_HB_SET_TCP_PARAMS_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_TCP_PARAMS_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("cmd_send WMI_HB_SET_TCP_PARAMS returned Error %d",
			 status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_tcp_pkt_filter_cmd_tlv() - configure tcp packet filter cmd
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static
QDF_STATUS send_lphb_config_tcp_pkt_filter_cmd_tlv(wmi_unified_t wmi_handle,
		wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *g_hb_tcp_filter_fp)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *hb_tcp_filter_fp;
	int len = sizeof(wmi_hb_set_tcp_pkt_filter_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_tcp_filter_fp =
		(wmi_hb_set_tcp_pkt_filter_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_tcp_filter_fp->tlv_header,
		WMITLV_TAG_STRUC_wmi_hb_set_tcp_pkt_filter_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		       (wmi_hb_set_tcp_pkt_filter_cmd_fixed_param));

	/* fill in values */
	hb_tcp_filter_fp->vdev_id = g_hb_tcp_filter_fp->vdev_id;
	hb_tcp_filter_fp->length = g_hb_tcp_filter_fp->length;
	hb_tcp_filter_fp->offset = g_hb_tcp_filter_fp->offset;
	hb_tcp_filter_fp->session = g_hb_tcp_filter_fp->session;
	memcpy((void *)&hb_tcp_filter_fp->filter,
	       (void *)&g_hb_tcp_filter_fp->filter,
	       WMI_WLAN_HB_MAX_FILTER_SIZE);

	wmi_mtrace(WMI_HB_SET_TCP_PKT_FILTER_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_TCP_PKT_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("cmd_send WMI_HB_SET_TCP_PKT_FILTER returned Error %d",
			 status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_udp_params_cmd_tlv() - configure udp param command of LPHB
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static QDF_STATUS send_lphb_config_udp_params_cmd_tlv(wmi_unified_t wmi_handle,
		   wmi_hb_set_udp_params_cmd_fixed_param *lphb_conf_req)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_udp_params_cmd_fixed_param *hb_udp_params_fp;
	int len = sizeof(wmi_hb_set_udp_params_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_udp_params_fp = (wmi_hb_set_udp_params_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_udp_params_fp->tlv_header,
		       WMITLV_TAG_STRUC_wmi_hb_set_udp_params_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_hb_set_udp_params_cmd_fixed_param));

	/* fill in values */
	hb_udp_params_fp->vdev_id = lphb_conf_req->vdev_id;
	hb_udp_params_fp->srv_ip = lphb_conf_req->srv_ip;
	hb_udp_params_fp->dev_ip = lphb_conf_req->dev_ip;
	hb_udp_params_fp->src_port = lphb_conf_req->src_port;
	hb_udp_params_fp->dst_port = lphb_conf_req->dst_port;
	hb_udp_params_fp->interval = lphb_conf_req->interval;
	hb_udp_params_fp->timeout = lphb_conf_req->timeout;
	hb_udp_params_fp->session = lphb_conf_req->session;
	qdf_mem_copy(&hb_udp_params_fp->gateway_mac,
		     &lphb_conf_req->gateway_mac,
		     sizeof(lphb_conf_req->gateway_mac));

	wmi_mtrace(WMI_HB_SET_UDP_PARAMS_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_UDP_PARAMS_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("cmd_send WMI_HB_SET_UDP_PARAMS returned Error %d",
			 status);
		wmi_buf_free(buf);
	}

	return status;
}

/**
 * send_lphb_config_udp_pkt_filter_cmd_tlv() - configure udp pkt filter command
 * @wmi_handle: wmi handle
 * @lphb_conf_req: lphb config request
 *
 * Return: CDF status
 */
static
QDF_STATUS send_lphb_config_udp_pkt_filter_cmd_tlv(wmi_unified_t wmi_handle,
		wmi_hb_set_udp_pkt_filter_cmd_fixed_param *lphb_conf_req)
{
	QDF_STATUS status;
	wmi_buf_t buf = NULL;
	uint8_t *buf_ptr;
	wmi_hb_set_udp_pkt_filter_cmd_fixed_param *hb_udp_filter_fp;
	int len = sizeof(wmi_hb_set_udp_pkt_filter_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(buf);
	hb_udp_filter_fp =
		(wmi_hb_set_udp_pkt_filter_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&hb_udp_filter_fp->tlv_header,
		WMITLV_TAG_STRUC_wmi_hb_set_udp_pkt_filter_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		       (wmi_hb_set_udp_pkt_filter_cmd_fixed_param));

	/* fill in values */
	hb_udp_filter_fp->vdev_id = lphb_conf_req->vdev_id;
	hb_udp_filter_fp->length = lphb_conf_req->length;
	hb_udp_filter_fp->offset = lphb_conf_req->offset;
	hb_udp_filter_fp->session = lphb_conf_req->session;
	memcpy((void *)&hb_udp_filter_fp->filter,
	       (void *)&lphb_conf_req->filter,
	       WMI_WLAN_HB_MAX_FILTER_SIZE);

	wmi_mtrace(WMI_HB_SET_UDP_PKT_FILTER_CMDID, NO_SESSION, 0);
	status = wmi_unified_cmd_send(wmi_handle, buf,
				      len, WMI_HB_SET_UDP_PKT_FILTER_CMDID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wmi_err("cmd_send WMI_HB_SET_UDP_PKT_FILTER returned Error %d",
			 status);
		wmi_buf_free(buf);
	}

	return status;
}

void wmi_lphb_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_lphb_config_hbenable_cmd =
		send_lphb_config_hbenable_cmd_tlv;
	ops->send_lphb_config_tcp_params_cmd =
		send_lphb_config_tcp_params_cmd_tlv;
	ops->send_lphb_config_tcp_pkt_filter_cmd =
		send_lphb_config_tcp_pkt_filter_cmd_tlv;
	ops->send_lphb_config_udp_params_cmd =
		send_lphb_config_udp_params_cmd_tlv;
	ops->send_lphb_config_udp_pkt_filter_cmd =
		send_lphb_config_udp_pkt_filter_cmd_tlv;
}
#endif /* FEATURE_WLAN_LPHB */

#ifdef WLAN_FEATURE_PACKET_FILTERING
/**
 * send_enable_disable_packet_filter_cmd_tlv() - enable/disable packet filter
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @enable: Flag to enable/disable packet filter
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_enable_disable_packet_filter_cmd_tlv(
		wmi_unified_t wmi_handle, uint8_t vdev_id, bool enable)
{
	int32_t len;
	int ret = 0;
	wmi_buf_t buf;
	WMI_PACKET_FILTER_ENABLE_CMD_fixed_param *cmd;

	len = sizeof(WMI_PACKET_FILTER_ENABLE_CMD_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_PACKET_FILTER_ENABLE_CMD_fixed_param *) wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_packet_filter_enable_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN(
			WMI_PACKET_FILTER_ENABLE_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	if (enable)
		cmd->enable = PACKET_FILTER_SET_ENABLE;
	else
		cmd->enable = PACKET_FILTER_SET_DISABLE;

	wmi_err("Packet filter enable %d for vdev_id %d", cmd->enable, vdev_id);

	wmi_mtrace(WMI_PACKET_FILTER_ENABLE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PACKET_FILTER_ENABLE_CMDID);
	if (ret) {
		wmi_err("Failed to send packet filter wmi cmd to fw");
		wmi_buf_free(buf);
	}

	return ret;
}

/**
 * send_config_packet_filter_cmd_tlv() - configure packet filter in target
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @rcv_filter_param: Packet filter parameters
 * @filter_id: Filter id
 * @enable: Flag to add/delete packet filter configuration
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS send_config_packet_filter_cmd_tlv(wmi_unified_t wmi_handle,
		uint8_t vdev_id, struct pmo_rcv_pkt_fltr_cfg *rcv_filter_param,
		uint8_t filter_id, bool enable)
{
	int len, i;
	int err = 0;
	wmi_buf_t buf;
	WMI_PACKET_FILTER_CONFIG_CMD_fixed_param *cmd;

	/* allocate the memory */
	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_PACKET_FILTER_CONFIG_CMD_fixed_param *)wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_packet_filter_config_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			       (WMI_PACKET_FILTER_CONFIG_CMD_fixed_param));

	cmd->vdev_id = vdev_id;
	cmd->filter_id = filter_id;
	if (enable)
		cmd->filter_action = PACKET_FILTER_SET_ACTIVE;
	else
		cmd->filter_action = PACKET_FILTER_SET_INACTIVE;

	if (enable) {
		cmd->num_params = QDF_MIN(
			WMI_PACKET_FILTER_MAX_CMP_PER_PACKET_FILTER,
			rcv_filter_param->num_params);
		cmd->filter_type = rcv_filter_param->filter_type;
		cmd->coalesce_time = rcv_filter_param->coalesce_time;

		for (i = 0; i < cmd->num_params; i++) {
			cmd->paramsData[i].proto_type =
				rcv_filter_param->params_data[i].protocol_layer;
			cmd->paramsData[i].cmp_type =
				rcv_filter_param->params_data[i].compare_flag;
			cmd->paramsData[i].data_length =
				rcv_filter_param->params_data[i].data_length;
			cmd->paramsData[i].data_offset =
				rcv_filter_param->params_data[i].data_offset;
			memcpy(&cmd->paramsData[i].compareData,
				rcv_filter_param->params_data[i].compare_data,
				sizeof(cmd->paramsData[i].compareData));
			memcpy(&cmd->paramsData[i].dataMask,
				rcv_filter_param->params_data[i].data_mask,
				sizeof(cmd->paramsData[i].dataMask));
		}
	}

	wmi_err("Packet filter action %d filter with id: %d, num_params=%d",
		 cmd->filter_action, cmd->filter_id, cmd->num_params);
	/* send the command along with data */
	wmi_mtrace(WMI_PACKET_FILTER_CONFIG_CMDID, cmd->vdev_id, 0);
	err = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PACKET_FILTER_CONFIG_CMDID);
	if (err) {
		wmi_err("Failed to send pkt_filter cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void wmi_packet_filtering_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_enable_disable_packet_filter_cmd =
		send_enable_disable_packet_filter_cmd_tlv;
	ops->send_config_packet_filter_cmd =
		send_config_packet_filter_cmd_tlv;
}
#endif /* WLAN_FEATURE_PACKET_FILTERING */

/**
 * send_wow_delete_pattern_cmd_tlv() - delete wow pattern in target
 * @wmi_handle: wmi handle
 * @ptrn_id: pattern id
 * @vdev_id: vdev id
 *
 * Return: CDF status
 */
static QDF_STATUS send_wow_delete_pattern_cmd_tlv(wmi_unified_t wmi_handle,
						  uint8_t ptrn_id,
						  uint8_t vdev_id)
{
	WMI_WOW_DEL_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_DEL_PATTERN_CMD_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_DEL_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_WMI_WOW_DEL_PATTERN_CMD_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
				WMI_WOW_DEL_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = ptrn_id;
	cmd->pattern_type = WOW_BITMAP_PATTERN;

	wmi_mtrace(WMI_WOW_DEL_WAKE_PATTERN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_WOW_DEL_WAKE_PATTERN_CMDID);
	if (ret) {
		wmi_err("Failed to delete wow ptrn from fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WMI_HOST_WAKEUP_OVER_QMI
static inline
QDF_STATUS wmi_unified_cmd_send_chk(struct wmi_unified *wmi_handle,
				    wmi_buf_t buf,
				    uint32_t buflen, uint32_t cmd_id)
{
	wmi_debug("Send WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID over QMI");
	return wmi_unified_cmd_send_over_qmi(wmi_handle, buf,
					     buflen, cmd_id);
}
#else
static inline
QDF_STATUS wmi_unified_cmd_send_chk(struct wmi_unified *wmi_handle,
				    wmi_buf_t buf,
				    uint32_t buflen, uint32_t cmd_id)
{
	return wmi_unified_cmd_send(wmi_handle, buf,
				    buflen, cmd_id);
}
#endif

/**
 * send_host_wakeup_ind_to_fw_cmd_tlv() - send wakeup ind to fw
 * @wmi_handle: wmi handle
 *
 * Sends host wakeup indication to FW. On receiving this indication,
 * FW will come out of WOW.
 *
 * Return: CDF status
 */
static QDF_STATUS send_host_wakeup_ind_to_fw_cmd_tlv(wmi_unified_t wmi_handle)
{
	wmi_wow_hostwakeup_from_sleep_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	int32_t len;
	int ret;

	len = sizeof(wmi_wow_hostwakeup_from_sleep_cmd_fixed_param);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_wow_hostwakeup_from_sleep_cmd_fixed_param *)
	      wmi_buf_data(buf);
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_wow_hostwakeup_from_sleep_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(wmi_wow_hostwakeup_from_sleep_cmd_fixed_param));

	wmi_mtrace(WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID, NO_SESSION, 0);
	ret = wmi_unified_cmd_send_chk(wmi_handle, buf, len,
				       WMI_WOW_HOSTWAKEUP_FROM_SLEEP_CMDID);
	if (ret) {
		wmi_err("Failed to send host wakeup indication to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return qdf_status;
}

/**
 * send_wow_timer_pattern_cmd_tlv() - set timer pattern tlv, so that firmware
 * will wake up host after specified time is elapsed
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 * @cookie: value to identify reason why host set up wake call.
 * @time: time in ms
 *
 * Return: QDF status
 */
static QDF_STATUS send_wow_timer_pattern_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t vdev_id, uint32_t cookie, uint32_t time)
{
	WMI_WOW_ADD_PATTERN_CMD_fixed_param *cmd;
	wmi_buf_t buf;
	uint8_t *buf_ptr;
	int32_t len;
	int ret;

	len = sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param) +
		WMI_TLV_HDR_SIZE + 0 * sizeof(WOW_BITMAP_PATTERN_T) +
		WMI_TLV_HDR_SIZE + 0 * sizeof(WOW_IPV4_SYNC_PATTERN_T) +
		WMI_TLV_HDR_SIZE + 0 * sizeof(WOW_IPV6_SYNC_PATTERN_T) +
		WMI_TLV_HDR_SIZE + 0 * sizeof(WOW_MAGIC_PATTERN_CMD) +
		WMI_TLV_HDR_SIZE + 1 * sizeof(uint32_t) +
		WMI_TLV_HDR_SIZE + 1 * sizeof(uint32_t);

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (WMI_WOW_ADD_PATTERN_CMD_fixed_param *) wmi_buf_data(buf);
	buf_ptr = (uint8_t *) cmd;

	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_WMI_WOW_ADD_PATTERN_CMD_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
			(WMI_WOW_ADD_PATTERN_CMD_fixed_param));
	cmd->vdev_id = vdev_id;
	cmd->pattern_id = cookie,
	cmd->pattern_type = WOW_TIMER_PATTERN;
	buf_ptr += sizeof(WMI_WOW_ADD_PATTERN_CMD_fixed_param);

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_BITMAP_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV4_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_IPV6_SYNC_PATTERN_T but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for WMITLV_TAG_STRUC_WOW_MAGIC_PATTERN_CMD but no data. */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
	buf_ptr += WMI_TLV_HDR_SIZE;

	/* Fill TLV for pattern_info_timeout, and time value */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, sizeof(uint32_t));
	buf_ptr += WMI_TLV_HDR_SIZE;
	*((uint32_t *) buf_ptr) = time;
	buf_ptr += sizeof(uint32_t);

	/* Fill TLV for ra_ratelimit_interval. with dummy 0 value */
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, sizeof(uint32_t));
	buf_ptr += WMI_TLV_HDR_SIZE;
	*((uint32_t *) buf_ptr) = 0;

	wmi_debug("send wake timer pattern with time[%d] to fw vdev = %d",
		 time, vdev_id);

	wmi_mtrace(WMI_WOW_ADD_WAKE_PATTERN_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_WOW_ADD_WAKE_PATTERN_CMDID);
	if (ret) {
		wmi_err("Failed to send wake timer pattern to fw");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
/**
 * send_enable_ext_wow_cmd_tlv() - enable ext wow in fw
 * @wmi_handle: wmi handle
 * @params: ext wow params
 *
 * Return:0 for success or error code
 */
static QDF_STATUS send_enable_ext_wow_cmd_tlv(wmi_unified_t wmi_handle,
					      struct ext_wow_params *params)
{
	wmi_extwow_enable_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(wmi_extwow_enable_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_extwow_enable_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_extwow_enable_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_extwow_enable_cmd_fixed_param));

	cmd->vdev_id = params->vdev_id;
	cmd->type = params->type;
	cmd->wakeup_pin_num = params->wakeup_pin_num;

	wmi_debug("vdev_id %d type %d Wakeup_pin_num %x",
		 cmd->vdev_id, cmd->type, cmd->wakeup_pin_num);

	wmi_mtrace(WMI_EXTWOW_ENABLE_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_EXTWOW_ENABLE_CMDID);
	if (ret) {
		wmi_err("Failed to set EXTWOW Enable");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * send_set_app_type2_params_in_fw_cmd_tlv() - set app type2 params in fw
 * @wmi_handle: wmi handle
 * @appType2Params: app type2 params
 *
 * Return: CDF status
 */
static QDF_STATUS send_set_app_type2_params_in_fw_cmd_tlv(wmi_unified_t wmi_handle,
			  struct app_type2_params *appType2Params)
{
	wmi_extwow_set_app_type2_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(wmi_extwow_set_app_type2_params_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_extwow_set_app_type2_params_cmd_fixed_param *)
	      wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_extwow_set_app_type2_params_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
			(wmi_extwow_set_app_type2_params_cmd_fixed_param));

	cmd->vdev_id = appType2Params->vdev_id;

	qdf_mem_copy(cmd->rc4_key, appType2Params->rc4_key, 16);
	cmd->rc4_key_len = appType2Params->rc4_key_len;

	cmd->ip_id = appType2Params->ip_id;
	cmd->ip_device_ip = appType2Params->ip_device_ip;
	cmd->ip_server_ip = appType2Params->ip_server_ip;

	cmd->tcp_src_port = appType2Params->tcp_src_port;
	cmd->tcp_dst_port = appType2Params->tcp_dst_port;
	cmd->tcp_seq = appType2Params->tcp_seq;
	cmd->tcp_ack_seq = appType2Params->tcp_ack_seq;

	cmd->keepalive_init = appType2Params->keepalive_init;
	cmd->keepalive_min = appType2Params->keepalive_min;
	cmd->keepalive_max = appType2Params->keepalive_max;
	cmd->keepalive_inc = appType2Params->keepalive_inc;

	WMI_CHAR_ARRAY_TO_MAC_ADDR(appType2Params->gateway_mac.bytes,
				   &cmd->gateway_mac);
	cmd->tcp_tx_timeout_val = appType2Params->tcp_tx_timeout_val;
	cmd->tcp_rx_timeout_val = appType2Params->tcp_rx_timeout_val;

	wmi_debug("vdev_id %d gateway_mac "QDF_MAC_ADDR_FMT" "
		 "rc4_key %.16s rc4_key_len %u "
		 "ip_id %x ip_device_ip %x ip_server_ip %x "
		 "tcp_src_port %u tcp_dst_port %u tcp_seq %u "
		 "tcp_ack_seq %u keepalive_init %u keepalive_min %u "
		 "keepalive_max %u keepalive_inc %u "
		 "tcp_tx_timeout_val %u tcp_rx_timeout_val %u",
		 cmd->vdev_id,
		 QDF_MAC_ADDR_REF(appType2Params->gateway_mac.bytes),
		 cmd->rc4_key, cmd->rc4_key_len,
		 cmd->ip_id, cmd->ip_device_ip, cmd->ip_server_ip,
		 cmd->tcp_src_port, cmd->tcp_dst_port, cmd->tcp_seq,
		 cmd->tcp_ack_seq, cmd->keepalive_init, cmd->keepalive_min,
		 cmd->keepalive_max, cmd->keepalive_inc,
		 cmd->tcp_tx_timeout_val, cmd->tcp_rx_timeout_val);

	wmi_mtrace(WMI_EXTWOW_SET_APP_TYPE2_PARAMS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_EXTWOW_SET_APP_TYPE2_PARAMS_CMDID);
	if (ret) {
		wmi_err("Failed to set APP TYPE2 PARAMS");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;

}

/**
 * send_app_type1_params_in_fw_cmd_tlv() - set app type1 params in fw
 * @wmi_handle: wmi handle
 * @app_type1_params: app type1 params
 *
 * Return: CDF status
 */
static QDF_STATUS send_app_type1_params_in_fw_cmd_tlv(wmi_unified_t wmi_handle,
				   struct app_type1_params *app_type1_params)
{
	wmi_extwow_set_app_type1_params_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	int32_t len;
	int ret;

	len = sizeof(wmi_extwow_set_app_type1_params_cmd_fixed_param);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_extwow_set_app_type1_params_cmd_fixed_param *)
	      wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_extwow_set_app_type1_params_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extwow_set_app_type1_params_cmd_fixed_param));

	cmd->vdev_id = app_type1_params->vdev_id;
	WMI_CHAR_ARRAY_TO_MAC_ADDR(app_type1_params->wakee_mac_addr.bytes,
				   &cmd->wakee_mac);
	qdf_mem_copy(cmd->ident, app_type1_params->identification_id, 8);
	cmd->ident_len = app_type1_params->id_length;
	qdf_mem_copy(cmd->passwd, app_type1_params->password, 16);
	cmd->passwd_len = app_type1_params->pass_length;

	wmi_debug("vdev_id %d wakee_mac_addr "QDF_MAC_ADDR_FMT" "
		 "identification_id %.8s id_length %u "
		 "password %.16s pass_length %u",
		 cmd->vdev_id,
		 QDF_MAC_ADDR_REF(app_type1_params->wakee_mac_addr.bytes),
		 cmd->ident, cmd->ident_len, cmd->passwd, cmd->passwd_len);

	wmi_mtrace(WMI_EXTWOW_SET_APP_TYPE1_PARAMS_CMDID, cmd->vdev_id, 0);
	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_EXTWOW_SET_APP_TYPE1_PARAMS_CMDID);
	if (ret) {
		wmi_err("Failed to set APP TYPE1 PARAMS");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void wmi_extwow_attach_tlv(struct wmi_unified *wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_enable_ext_wow_cmd = send_enable_ext_wow_cmd_tlv;
	ops->send_set_app_type2_params_in_fw_cmd =
		send_set_app_type2_params_in_fw_cmd_tlv;
	ops->send_app_type1_params_in_fw_cmd =
		send_app_type1_params_in_fw_cmd_tlv;
}
#endif /* WLAN_FEATURE_EXTWOW_SUPPORT */

void wmi_pmo_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->send_add_wow_wakeup_event_cmd =
		send_add_wow_wakeup_event_cmd_tlv;
	ops->send_wow_patterns_to_fw_cmd = send_wow_patterns_to_fw_cmd_tlv;
	ops->send_enable_arp_ns_offload_cmd =
		send_enable_arp_ns_offload_cmd_tlv;
	ops->send_add_clear_mcbc_filter_cmd =
		send_add_clear_mcbc_filter_cmd_tlv;
	ops->send_multiple_add_clear_mcbc_filter_cmd =
		send_multiple_add_clear_mcbc_filter_cmd_tlv;
	ops->send_conf_hw_filter_cmd = send_conf_hw_filter_cmd_tlv;
	ops->send_gtk_offload_cmd = send_gtk_offload_cmd_tlv;
#ifdef WLAN_FEATURE_IGMP_OFFLOAD
	ops->send_igmp_offload_cmd = send_igmp_offload_cmd_tlv;
#endif
	ops->send_process_gtk_offload_getinfo_cmd =
		send_process_gtk_offload_getinfo_cmd_tlv;
	ops->send_enable_enhance_multicast_offload_cmd =
		send_enable_enhance_multicast_offload_tlv;
	ops->extract_gtk_rsp_event = extract_gtk_rsp_event_tlv;
	ops->send_action_frame_patterns_cmd =
		send_action_frame_patterns_cmd_tlv;
	ops->send_wow_delete_pattern_cmd = send_wow_delete_pattern_cmd_tlv;
	ops->send_host_wakeup_ind_to_fw_cmd =
		send_host_wakeup_ind_to_fw_cmd_tlv;
	ops->send_wow_timer_pattern_cmd = send_wow_timer_pattern_cmd_tlv;

	wmi_d0wow_attach_tlv(wmi_handle);
	wmi_ra_filtering_attach_tlv(wmi_handle);
	wmi_lphb_attach_tlv(wmi_handle);
	wmi_packet_filtering_attach_tlv(wmi_handle);
	wmi_extwow_attach_tlv(wmi_handle);
}
