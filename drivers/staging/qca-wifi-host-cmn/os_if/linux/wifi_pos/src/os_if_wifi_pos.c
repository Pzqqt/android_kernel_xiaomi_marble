/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: wlan_hdd_wifi_pos.c
 * This file defines the important functions pertinent to wifi positioning
 * component's os_if layer.
 */

#include "qdf_platform.h"
#include "qdf_module.h"
#include "wlan_nlink_srv.h"
#include "wlan_ptt_sock_svc.h"
#include "wlan_nlink_common.h"
#include "os_if_wifi_pos.h"
#include <wlan_lmac_if_def.h>
#include "wifi_pos_api.h"
#include "wlan_cfg80211.h"
#include "wlan_objmgr_psoc_obj.h"
#ifdef CNSS_GENL
#ifdef CONFIG_CNSS_OUT_OF_TREE
#include "cnss_nl.h"
#else
#include <net/cnss_nl.h>
#endif
#include "linux/genetlink.h"
#include "wifi_pos_utils_pub.h"
#endif

#ifdef CNSS_GENL
#define WLAN_CLD80211_MAX_SIZE SKB_WITH_OVERHEAD(8192UL)

#define CLD80211_ATTR_CMD 4
#define CLD80211_ATTR_CMD_TAG_DATA 5
#define CLD80211_ATTR_MAX 5

static const uint32_t
cap_resp_sub_attr_len[CLD80211_SUB_ATTR_CAPS_MAX + 1] = {
	[CLD80211_SUB_ATTR_CAPS_OEM_TARGET_SIGNATURE] =
				OEM_TARGET_SIGNATURE_LEN,
	[CLD80211_SUB_ATTR_CAPS_OEM_TARGET_TYPE] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CAPS_OEM_FW_VERSION] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MAJOR] = sizeof(uint8_t),
	[CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MINOR] = sizeof(uint8_t),
	[CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_PATCH] = sizeof(uint8_t),
	[CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_BUILD] = sizeof(uint8_t),
	[CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MIN] = sizeof(uint16_t),
	[CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MAX] = sizeof(uint16_t),
	[CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MIN] = sizeof(uint16_t),
	[CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MAX] = sizeof(uint16_t),
	[CLD80211_SUB_ATTR_CAPS_SUPPORTED_BANDS] = sizeof(uint16_t),
	[CLD80211_SUB_ATTR_CAPS_USER_DEFINED_CAPS] =
				sizeof(struct wifi_pos_user_defined_caps),
};

static const uint32_t
peer_status_sub_attr_len[CLD80211_SUB_ATTR_PEER_MAX + 1] = {
	[CLD80211_SUB_ATTR_PEER_MAC_ADDR] = ETH_ALEN,
	[CLD80211_SUB_ATTR_PEER_STATUS] = sizeof(uint8_t),
	[CLD80211_SUB_ATTR_PEER_VDEV_ID] = sizeof(uint8_t),
	[CLD80211_SUB_ATTR_PEER_CAPABILITY] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_PEER_RESERVED] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_PEER_CHAN_INFO] =
				sizeof(struct wifi_pos_ch_info_rsp),
};

static const uint32_t
ch_resp_sub_attr_len[CLD80211_SUB_ATTR_CH_MAX + 1] = {
	[CLD80211_SUB_ATTR_CHANNEL_NUM_CHAN] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_LIST] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_CHAN_ID] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_MHZ] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_BAND_CF_1] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_BAND_CF_2] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_INFO] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_REG_INFO_1] = sizeof(uint32_t),
	[CLD80211_SUB_ATTR_CH_REG_INFO_2] = sizeof(uint32_t),
};
#endif

static int map_wifi_pos_cmd_to_ani_msg_rsp(
		enum wifi_pos_cmd_ids cmd)
{
	switch (cmd) {
	case WIFI_POS_CMD_REGISTRATION:
		return ANI_MSG_APP_REG_RSP;
	case WIFI_POS_CMD_SET_CAPS:
		return ANI_MSG_SET_OEM_CAP_RSP;
	case WIFI_POS_CMD_GET_CAPS:
		return ANI_MSG_GET_OEM_CAP_RSP;
	case WIFI_POS_CMD_GET_CH_INFO:
		return ANI_MSG_CHANNEL_INFO_RSP;
	case WIFI_POS_CMD_OEM_DATA:
		return ANI_MSG_OEM_DATA_RSP;
	case WIFI_POS_CMD_ERROR:
		return ANI_MSG_OEM_ERROR;
	case WIFI_POS_PEER_STATUS_IND:
		return ANI_MSG_PEER_STATUS_IND;
	default:
		osif_err("response message is invalid :%d", cmd);
		return -EINVAL;
	}
}

static enum wifi_pos_cmd_ids
map_ani_msg_req_to_wifi_pos_cmd(uint32_t cmd)
{
	switch (cmd) {
	case ANI_MSG_APP_REG_REQ:
		return WIFI_POS_CMD_REGISTRATION;
	case ANI_MSG_SET_OEM_CAP_REQ:
		return WIFI_POS_CMD_SET_CAPS;
	case ANI_MSG_GET_OEM_CAP_REQ:
		return WIFI_POS_CMD_GET_CAPS;
	case ANI_MSG_CHANNEL_INFO_REQ:
		return WIFI_POS_CMD_GET_CH_INFO;
	case ANI_MSG_OEM_DATA_REQ:
		return WIFI_POS_CMD_OEM_DATA;
	default:
		osif_err("ani req is invalid :%d", cmd);
		return WIFI_POS_CMD_INVALID;
	}
}

#ifdef CNSS_GENL
static enum wifi_pos_cmd_ids
map_cld_vendor_sub_cmd_to_wifi_pos_cmd(
		enum cld80211_vendor_sub_cmds cmd)
{
	switch (cmd) {
	case CLD80211_VENDOR_SUB_CMD_REGISTRATION:
		return WIFI_POS_CMD_REGISTRATION;
	case CLD80211_VENDOR_SUB_CMD_SET_CAPS:
		return WIFI_POS_CMD_SET_CAPS;
	case CLD80211_VENDOR_SUB_CMD_GET_CAPS:
		return WIFI_POS_CMD_GET_CAPS;
	case CLD80211_VENDOR_SUB_CMD_GET_CH_INFO:
		return WIFI_POS_CMD_GET_CH_INFO;
	case CLD80211_VENDOR_SUB_CMD_OEM_DATA:
		return WIFI_POS_CMD_OEM_DATA;
	default:
		osif_err("cld vendor subcmd is invalid :%d", cmd);
		return WIFI_POS_CMD_INVALID;
	}
}

static enum cld80211_vendor_sub_cmds
map_wifi_pos_cmd_to_cld_vendor_sub_cmd(
		enum wifi_pos_cmd_ids cmd)
{
	switch (cmd) {
	case WIFI_POS_CMD_REGISTRATION:
		return CLD80211_VENDOR_SUB_CMD_REGISTRATION;
	case WIFI_POS_CMD_SET_CAPS:
		return CLD80211_VENDOR_SUB_CMD_SET_CAPS;
	case WIFI_POS_CMD_GET_CAPS:
		return CLD80211_VENDOR_SUB_CMD_GET_CAPS;
	case WIFI_POS_CMD_GET_CH_INFO:
		return CLD80211_VENDOR_SUB_CMD_GET_CH_INFO;
	case WIFI_POS_CMD_OEM_DATA:
		return CLD80211_VENDOR_SUB_CMD_OEM_DATA;
	case WIFI_POS_CMD_ERROR:
		return ANI_MSG_OEM_ERROR;
	case WIFI_POS_PEER_STATUS_IND:
		return ANI_MSG_PEER_STATUS_IND;
	default:
		osif_err("response message is invalid :%d", cmd);
		return CLD80211_VENDOR_SUB_CMD_INVALID;
	}
}

static void os_if_wifi_pos_send_peer_nl_status(uint32_t pid, uint8_t *buf)
{
	void *hdr;
	int flags = GFP_KERNEL;
	struct sk_buff *msg = NULL;
	struct nlattr *nest1, *nest2, *nest3;
	struct wifi_pos_peer_status_info *peer_info;
	struct wifi_pos_ch_info_rsp *chan_info;

	msg = cld80211_oem_rsp_alloc_skb(pid, &hdr, &nest1, &flags);
	if (!msg) {
		osif_err("alloc_skb failed");
		return;
	}

	peer_info = (struct wifi_pos_peer_status_info *)buf;
	chan_info = &peer_info->peer_chan_info;

	nla_put_u32(msg, CLD80211_ATTR_CMD,
			 CLD80211_VENDOR_SUB_CMD_PEER_STATUS_IND);
	nest2 = nla_nest_start(msg, CLD80211_ATTR_CMD_TAG_DATA);
	if (!nest2) {
		osif_err("nla_nest_start failed");
		dev_kfree_skb(msg);
		return;
	}

	nla_put(msg, CLD80211_SUB_ATTR_PEER_MAC_ADDR,
			ETH_ALEN, peer_info->peer_mac_addr);
	nla_put_u8(msg, CLD80211_SUB_ATTR_PEER_STATUS,
						peer_info->peer_status);
	nla_put_u8(msg, CLD80211_SUB_ATTR_PEER_VDEV_ID,
						peer_info->vdev_id);
	nla_put_u32(msg, CLD80211_SUB_ATTR_PEER_CAPABILITY,
						peer_info->peer_capability);
	nla_put_u32(msg, CLD80211_SUB_ATTR_PEER_RESERVED,
							peer_info->reserved0);
	nest3 = nla_nest_start(msg, CLD80211_SUB_ATTR_PEER_CHAN_INFO);
	if (!nest3) {
		osif_err("nla_nest_start failed");
		dev_kfree_skb(msg);
		return;
	}
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_CHAN_ID,
			chan_info->chan_id);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_MHZ, chan_info->mhz);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_BAND_CF_1,
			chan_info->band_center_freq1);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_BAND_CF_2,
			chan_info->band_center_freq2);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_INFO, chan_info->info);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_REG_INFO_1,
			chan_info->reg_info_1);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CH_REG_INFO_2,
			chan_info->reg_info_2);

	nla_nest_end(msg, nest3);
	nla_nest_end(msg, nest2);

	osif_debug("sending oem rsp: type: %d to pid (%d)",
		    CLD80211_VENDOR_SUB_CMD_PEER_STATUS_IND, pid);

	cld80211_oem_send_reply(msg, hdr, nest1, flags);
}

static void os_if_send_cap_nl_resp(uint32_t pid, uint8_t *buf)
{
	void *hdr;
	int flags = GFP_KERNEL;
	struct sk_buff *msg = NULL;
	struct nlattr *nest1, *nest2;
	struct wifi_pos_oem_get_cap_rsp *cap_rsp;

	msg = cld80211_oem_rsp_alloc_skb(pid, &hdr, &nest1, &flags);
	if (!msg) {
		osif_err("alloc_skb failed");
		return;
	}

	nla_put_u32(msg, CLD80211_ATTR_CMD,
	map_wifi_pos_cmd_to_cld_vendor_sub_cmd(WIFI_POS_CMD_GET_CAPS));

	cap_rsp = (struct wifi_pos_oem_get_cap_rsp *)(buf);
	nest2 = nla_nest_start(msg, CLD80211_ATTR_CMD_TAG_DATA);

	if (!nest2) {
		osif_err("nla_nest_start failed");
		dev_kfree_skb(msg);
		return;
	}

	nla_put(msg, CLD80211_SUB_ATTR_CAPS_OEM_TARGET_SIGNATURE,
		OEM_TARGET_SIGNATURE_LEN, OEM_TARGET_SIGNATURE);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CAPS_OEM_TARGET_TYPE,
		    cap_rsp->driver_cap.oem_target_type);
	nla_put_u32(msg, CLD80211_SUB_ATTR_CAPS_OEM_FW_VERSION,
		    cap_rsp->driver_cap.oem_fw_version);
	nla_put_u8(msg, CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MAJOR,
		   cap_rsp->driver_cap.driver_version.major);
	nla_put_u8(msg, CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_MINOR,
		   cap_rsp->driver_cap.driver_version.minor);
	nla_put_u8(msg, CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_PATCH,
		   cap_rsp->driver_cap.driver_version.patch);
	nla_put_u8(msg, CLD80211_SUB_ATTR_CAPS_DRIVER_VERSION_BUILD,
		   cap_rsp->driver_cap.driver_version.build);
	nla_put_u16(msg, CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MIN,
		    cap_rsp->driver_cap.allowed_dwell_time_min);
	nla_put_u16(msg, CLD80211_SUB_ATTR_CAPS_ALLOWED_DWELL_TIME_MAX,
		    cap_rsp->driver_cap.allowed_dwell_time_max);
	nla_put_u16(msg, CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MIN,
		    cap_rsp->driver_cap.curr_dwell_time_min);
	nla_put_u16(msg, CLD80211_SUB_ATTR_CAPS_CURRENT_DWELL_TIME_MAX,
		    cap_rsp->driver_cap.curr_dwell_time_max);
	nla_put_u16(msg, CLD80211_SUB_ATTR_CAPS_SUPPORTED_BANDS,
		    cap_rsp->driver_cap.supported_bands);
	nla_put(msg, CLD80211_SUB_ATTR_CAPS_USER_DEFINED_CAPS,
		sizeof(struct wifi_pos_user_defined_caps),
		&cap_rsp->user_defined_cap);
	nla_nest_end(msg, nest2);

	osif_debug("sending oem rsp: type: %d to pid (%d)",
		    CLD80211_VENDOR_SUB_CMD_GET_CAPS, pid);

	cld80211_oem_send_reply(msg, hdr, nest1, flags);
}

static void
os_if_get_chan_nl_resp_len(uint32_t *chan_info, uint32_t *attr_headers)
{
	uint32_t i;
	struct nlattr more_data;
	struct nlattr attr_tag_data;
	struct nlattr cld80211_subattr_ch_list;
	struct nlattr chan_iter;

	*attr_headers = NLA_ALIGN(sizeof(attr_tag_data));
	*attr_headers += NLA_ALIGN(sizeof(more_data));
	*attr_headers += nla_total_size(
		ch_resp_sub_attr_len[CLD80211_SUB_ATTR_CHANNEL_NUM_CHAN]);
	*attr_headers += sizeof(cld80211_subattr_ch_list);

	*chan_info = NLA_ALIGN(sizeof(chan_iter));
	i = CLD80211_SUB_ATTR_CH_LIST;
	for (; i <= CLD80211_SUB_ATTR_CH_MAX; i++)
		*chan_info += nla_total_size(ch_resp_sub_attr_len[i]);
}

static uint8_t os_if_get_max_chan_nl_resp(uint8_t chan_num)
{
	struct nlattr vendor_data;
	struct nlattr attr_cmd;
	uint32_t chan_info = 0, attr_headers = 0;
	uint32_t chan_info_msg_len, chan_allow = 0;

	os_if_get_chan_nl_resp_len(&chan_info, &attr_headers);
	attr_headers += NLA_ALIGN(sizeof(vendor_data));
	attr_headers += NLA_ALIGN(sizeof(attr_cmd));

	chan_info_msg_len = WLAN_CLD80211_MAX_SIZE;
	chan_info_msg_len -= WIFIPOS_RESERVE_BYTES;
	chan_info_msg_len -= attr_headers;

	chan_allow = chan_info_msg_len / chan_info;

	if (chan_num > chan_allow)
		return chan_allow;
	else
		return chan_num;
}

static int
os_if_create_ch_nl_resp(uint32_t pid, uint8_t *buf, uint16_t num_chan,
			bool is_frag)
{
	void *hdr;
	int i;
	int flags = GFP_KERNEL;
	struct sk_buff *msg = NULL;
	struct nlattr *nest1, *nest2;
	struct nlattr *nest3, *nest4;
	struct wifi_pos_ch_info_rsp *channel_rsp;

	channel_rsp = (struct wifi_pos_ch_info_rsp *)buf;

	msg = cld80211_oem_rsp_alloc_skb(pid, &hdr, &nest1, &flags);
	if (!msg) {
		osif_err("alloc_skb failed");
		return -EPERM;
	}

	nla_put_u32(msg, CLD80211_ATTR_CMD,
		    CLD80211_VENDOR_SUB_CMD_GET_CH_INFO);

	nest2 = nla_nest_start(msg, CLD80211_ATTR_CMD_TAG_DATA);
	if (!nest2)
		goto fail;

	if (is_frag)
		nla_put_flag(msg, CLD80211_SUB_ATTR_CH_MORE_DATA);

	nla_put_u32(msg, CLD80211_SUB_ATTR_CHANNEL_NUM_CHAN, num_chan);

	nest3 = nla_nest_start(msg, CLD80211_SUB_ATTR_CH_LIST);
	if (!nest3)
		goto fail;
	for (i = 0; i < num_chan; i++) {
		nest4 = nla_nest_start(msg, i);
		if (!nest4)
			goto fail;

		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_CHAN_ID,
			    channel_rsp->chan_id);
		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_MHZ, channel_rsp->mhz);
		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_BAND_CF_1,
			    channel_rsp->band_center_freq1);
		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_BAND_CF_2,
			    channel_rsp->band_center_freq2);
		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_INFO, channel_rsp->info);
		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_REG_INFO_1,
			    channel_rsp->reg_info_1);
		nla_put_u32(msg, CLD80211_SUB_ATTR_CH_REG_INFO_2,
			    channel_rsp->reg_info_2);
		nla_nest_end(msg, nest4);
		channel_rsp++;
	}

	nla_nest_end(msg, nest3);
	nla_nest_end(msg, nest2);

	osif_debug("sending oem rsp: type: %d to pid (%d)",
		   CLD80211_VENDOR_SUB_CMD_GET_CH_INFO, pid);

	cld80211_oem_send_reply(msg, hdr, nest1, flags);
	return 0;

fail:
	osif_err("failed to fill CHAN_RESP attributes");
	dev_kfree_skb(msg);
	return -EPERM;
}

static void os_if_send_chan_nl_resp(uint32_t pid, uint8_t *buf)
{
	int err;
	uint8_t check_chans = 0;
	uint8_t  *chnk_ptr, chan_allow = 0;
	bool resp_frag = false;

	check_chans = buf[0];
	chnk_ptr = &buf[1];

	do {
		chan_allow = os_if_get_max_chan_nl_resp(check_chans);

		if (check_chans > chan_allow)
			resp_frag = true;
		else
			resp_frag = false;
		check_chans -= chan_allow;

		err = os_if_create_ch_nl_resp(pid, chnk_ptr,
					      chan_allow, resp_frag);
		if (err) {
			osif_err("failed to alloc memory for ch_nl_resp");
			return;
		}
		chnk_ptr += (sizeof(struct wifi_pos_ch_info_rsp) *
				      chan_allow);
	} while (resp_frag);
}

static int
os_if_create_oemdata_resp(uint32_t pid, uint8_t *buf, bool frag_resp,
			  uint32_t chnk_len)
{
	void *hdr;
	int flags = GFP_KERNEL;
	struct sk_buff *msg = NULL;
	struct nlattr *nest1, *nest2;

	msg = cld80211_oem_rsp_alloc_skb(pid, &hdr, &nest1, &flags);
	if (!msg) {
		osif_err("alloc_skb failed");
		return -EPERM;
	}

	nla_put_u32(msg, CLD80211_ATTR_CMD, CLD80211_VENDOR_SUB_CMD_OEM_DATA);

	nest2 = nla_nest_start(msg, CLD80211_ATTR_CMD_TAG_DATA);
	if (!nest2)
		goto fail;

	if (frag_resp)
		nla_put_flag(msg, CLD80211_SUB_ATTR_OEM_MORE_DATA);

	nla_put(msg, CLD80211_SUB_ATTR_BINARY_DATA, chnk_len, buf);

	nla_nest_end(msg, nest2);
	osif_debug("sending oem rsp: type: %d to pid (%d)",
		   CLD80211_VENDOR_SUB_CMD_OEM_DATA, pid);
	cld80211_oem_send_reply(msg, hdr, nest1, flags);
	return 0;

fail:
	osif_err("failed to fill CHAN_RESP attributes");
	dev_kfree_skb(msg);
	return -EPERM;
}

static void
os_if_send_oem_data_nl_resp(uint32_t pid, uint8_t *buf,
			    uint32_t buf_len)
{
	int err;
	uint32_t attr_len;
	uint32_t chnk_len, remain_len;
	uint8_t *chnk_ptr;
	bool frag_resp = false;

	struct nlattr vendor_data;
	struct nlattr attr_cmd;
	struct nlattr attr_tag_data;
	struct nlattr cld80211_subattr_bindata;
	struct nlattr more_data;

	attr_len = WIFIPOS_RESERVE_BYTES;
	attr_len += NLMSG_ALIGN(sizeof(vendor_data));
	attr_len += NLMSG_ALIGN(sizeof(attr_cmd));
	attr_len += NLMSG_ALIGN(sizeof(attr_tag_data));
	attr_len += NLMSG_ALIGN(sizeof(more_data));

	chnk_ptr = buf;
	chnk_len = buf_len;
	remain_len = buf_len;
	do {
		if (attr_len + nla_total_size(chnk_len) >
		    WLAN_CLD80211_MAX_SIZE) {
			frag_resp = true;

			chnk_len = WLAN_CLD80211_MAX_SIZE - (attr_len +
					sizeof(cld80211_subattr_bindata));
		} else {
			frag_resp = false;
		}

		remain_len -= chnk_len;

		err = os_if_create_oemdata_resp(pid, chnk_ptr,
						frag_resp, chnk_len);
		if (err) {
			osif_err("failed to alloc memory for oem_nl_resp");
			return;
		}
		chnk_ptr += chnk_len;
		chnk_len = remain_len;
	} while (frag_resp);
}

static void os_if_send_nl_resp(uint32_t pid, uint8_t *buf,
			       enum wifi_pos_cmd_ids cmd, uint32_t len)
{
	switch (cmd) {
	case WIFI_POS_CMD_GET_CAPS:
		os_if_send_cap_nl_resp(pid, buf);
		break;
	case WIFI_POS_CMD_GET_CH_INFO:
		os_if_send_chan_nl_resp(pid, buf);
		break;
	case WIFI_POS_CMD_OEM_DATA:
		os_if_send_oem_data_nl_resp(pid, buf, len);
		break;
	case WIFI_POS_PEER_STATUS_IND:
		os_if_wifi_pos_send_peer_nl_status(pid, buf);
		break;
	default:
		osif_err("response message is invalid :%d", cmd);
	}
}
#else
static void os_if_send_nl_resp(uint32_t pid, uint8_t *buf,
			       enum wifi_pos_cmd_ids cmd, uint32_t len)
{
}
#endif

/**
 * os_if_wifi_pos_send_rsp() - send oem registration response
 *
 * This function sends oem message to registered application process
 *
 * Return:  none
 */
static void os_if_wifi_pos_send_rsp(struct wlan_objmgr_psoc *psoc, uint32_t pid,
				    enum wifi_pos_cmd_ids cmd, uint32_t buf_len,
				    uint8_t *buf)
{
	tAniMsgHdr *aniHdr;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;

	/* OEM msg is always to a specific process and cannot be a broadcast */
	if (pid == 0) {
		osif_err("invalid dest pid");
		return;
	}

	if (ucfg_wifi_pos_is_nl_rsp(psoc)) {
		os_if_send_nl_resp(pid, buf, cmd, buf_len);
	} else {
		skb = alloc_skb(NLMSG_SPACE(sizeof(tAniMsgHdr) + buf_len),
				GFP_ATOMIC);
		if (!skb) {
			osif_alert("alloc_skb failed");
			return;
		}

		nlh = (struct nlmsghdr *)skb->data;
		nlh->nlmsg_pid = 0;     /* from kernel */
		nlh->nlmsg_flags = 0;
		nlh->nlmsg_seq = 0;
		nlh->nlmsg_type = WLAN_NL_MSG_OEM;
		nlh->nlmsg_len = NLMSG_LENGTH(sizeof(tAniMsgHdr) + buf_len);

		aniHdr = NLMSG_DATA(nlh);
		aniHdr->type = map_wifi_pos_cmd_to_ani_msg_rsp(cmd);
		qdf_mem_copy(&aniHdr[1], buf, buf_len);
		aniHdr->length = buf_len;

		skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr) + buf_len));
		osif_debug("sending oem rsp: type: %d len(%d) to pid (%d)",
			   aniHdr->type, buf_len, pid);
		nl_srv_ucast_oem(skb, pid, MSG_DONTWAIT);
	}
}

#ifdef CNSS_GENL
static int
wifi_pos_parse_nla_oemdata_req(uint32_t len, uint8_t *buf,
			       struct wifi_pos_req_msg *req)
{
	struct nlattr *tb_oem_data[CLD80211_SUB_ATTR_MSG_OEM_DATA_REQ_MAX + 1];

	if (wlan_cfg80211_nla_parse(tb_oem_data,
				    CLD80211_SUB_ATTR_MSG_OEM_DATA_REQ_MAX,
				    (struct nlattr *)buf, len, NULL)) {
		osif_err("invalid data in request");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}

	if (!tb_oem_data[CLD80211_SUB_ATTR_MSG_OEM_DATA_FW]) {
		osif_err("CLD80211_SUB_ATTR_MSG_OEM_DATA_FW not present");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}
	req->buf_len = nla_len(
				tb_oem_data[CLD80211_SUB_ATTR_MSG_OEM_DATA_FW]);
	req->buf = nla_data(
				tb_oem_data[CLD80211_SUB_ATTR_MSG_OEM_DATA_FW]);

	return 0;
}

static int  wifi_pos_parse_nla_req(const void *data, int len, int pid,
		    struct wifi_pos_req_msg *req)
{
	uint8_t *msg;
	struct nlattr *tb[CLD80211_ATTR_MAX + 1];
	uint32_t msg_len;

	if (wlan_cfg80211_nla_parse(tb, CLD80211_ATTR_MAX, data, len, NULL)) {
		osif_err("invalid data in request");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}

	req->pid = pid;
	req->msg_type = map_cld_vendor_sub_cmd_to_wifi_pos_cmd(
				nla_get_u32(tb[CLD80211_ATTR_CMD]));
	req->rsp_version = WIFI_POS_RSP_V2_NL;

	if (tb[CLD80211_ATTR_CMD_TAG_DATA]) {
		msg_len = nla_len(tb[CLD80211_ATTR_CMD_TAG_DATA]);
		msg = nla_data(tb[CLD80211_ATTR_CMD_TAG_DATA]);

		if (req->msg_type == WIFI_POS_CMD_OEM_DATA) {
			if (wifi_pos_parse_nla_oemdata_req(msg_len, msg, req)) {
				osif_err("parsing oemdata req failed");
				return OEM_ERR_INVALID_MESSAGE_LENGTH;
			}
		} else {
			req->buf_len = msg_len;
			req->buf = msg;
		}
	}
	if (tb[CLD80211_ATTR_META_DATA])
		osif_err("meta data dropped. Apps can use CLD80211_ATTR_CMD_TAG_DATA sub attrs");

	return 0;
}

static int  wifi_pos_parse_ani_req(const void *data, int len, int pid,
		    struct wifi_pos_req_msg *req)
{
	tAniMsgHdr *msg_hdr;
	struct nlattr *tb[CLD80211_ATTR_MAX + 1];
	uint32_t msg_len, id, nl_field_info_size, expected_field_info_size;
	struct wifi_pos_field_info *field_info;

	if (wlan_cfg80211_nla_parse(tb, CLD80211_ATTR_MAX, data, len, NULL)) {
		osif_err("invalid data in request");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}

	msg_len = nla_len(tb[CLD80211_ATTR_DATA]);
	if (msg_len < sizeof(*msg_hdr)) {
		osif_err("Insufficient length for msg_hdr: %u", msg_len);
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	msg_hdr = nla_data(tb[CLD80211_ATTR_DATA]);
	req->msg_type = map_ani_msg_req_to_wifi_pos_cmd(
				(uint32_t)msg_hdr->type);
	req->rsp_version = WIFI_POS_RSP_V1_FLAT_MEMORY;

	if (msg_len < sizeof(*msg_hdr) + msg_hdr->length) {
		osif_err("Insufficient length for msg_hdr buffer: %u",
			 msg_len);
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	req->buf_len = msg_hdr->length;
	req->buf = (uint8_t *)&msg_hdr[1];
	req->pid = pid;

	id = CLD80211_ATTR_META_DATA;
	if (!tb[id])
		return 0;

	nl_field_info_size = nla_len(tb[id]);
	if (nl_field_info_size < sizeof(*field_info)) {
		osif_err("Insufficient length for field_info_buf: %u",
			 nl_field_info_size);
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	field_info = nla_data(tb[id]);
	if (!field_info->count) {
		osif_debug("field_info->count is zero, ignoring META_DATA");
		return 0;
	}

	if ((field_info->count - 1) >
	    ((UINT_MAX - sizeof(*field_info)) /
	    sizeof(struct wifi_pos_field))) {
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	expected_field_info_size = sizeof(*field_info) +
		(field_info->count - 1) * sizeof(struct wifi_pos_field);

	if (nl_field_info_size < expected_field_info_size) {
		osif_err("Insufficient len for total no.of %u fields",
			 field_info->count);
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	req->field_info_buf = field_info;
	req->field_info_buf_len = nl_field_info_size;

	return 0;
}


static int  wifi_pos_parse_req(const void *data, int len, int pid,
		    struct wifi_pos_req_msg *req)
{
	int status = 0;
	struct nlattr *tb[CLD80211_ATTR_MAX + 1];

	if (wlan_cfg80211_nla_parse(tb, CLD80211_ATTR_MAX, data, len, NULL)) {
		osif_err("invalid data in request");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}

	if (tb[CLD80211_ATTR_DATA]) {
		status = wifi_pos_parse_ani_req(data, len, pid, req);
	} else if (tb[CLD80211_ATTR_CMD]) {
		status = wifi_pos_parse_nla_req(data, len, pid, req);
	} else {
		osif_err("Valid CLD80211 ATTR not present");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}
	return status;
}
#else
static int wifi_pos_parse_req(struct sk_buff *skb, struct wifi_pos_req_msg *req,
			      struct wlan_objmgr_psoc **psoc)
{
	/* SKB->data contains NL msg */
	/* NLMSG_DATA(nlh) contains ANI msg */
	struct nlmsghdr *nlh;
	tAniMsgHdr *msg_hdr;
	size_t field_info_len;
	int interface_len;
	char *interface = NULL;
	uint8_t pdev_id = 0;
	uint32_t tgt_pdev_id = 0;
	uint8_t i;
	uint32_t offset;
	QDF_STATUS status;

	nlh = (struct nlmsghdr *)skb->data;
	if (!nlh) {
		osif_err("Netlink header null");
		return OEM_ERR_NULL_MESSAGE_HEADER;
	}

	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*msg_hdr))) {
		osif_err("nlmsg_len(%d) and msg_hdr_size(%zu) mis-match",
			 nlh->nlmsg_len, sizeof(*msg_hdr));
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	msg_hdr = NLMSG_DATA(nlh);
	if (!msg_hdr) {
		osif_err("Message header null");
		return OEM_ERR_NULL_MESSAGE_HEADER;
	}

	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(*msg_hdr) + msg_hdr->length)) {
		osif_err("nlmsg_len(%d) and animsg_len(%d) mis-match",
			 nlh->nlmsg_len, msg_hdr->length);
		return OEM_ERR_INVALID_MESSAGE_LENGTH;
	}

	req->msg_type = map_ani_msg_req_to_wifi_pos_cmd(
				(uint32_t)msg_hdr->type);
	req->rsp_version = WIFI_POS_RSP_V1_FLAT_MEMORY;
	req->buf_len = msg_hdr->length;
	req->buf = (uint8_t *)&msg_hdr[1];
	req->pid = nlh->nlmsg_pid;
	req->field_info_buf = NULL;
	req->field_info_buf_len = 0;

	field_info_len = nlh->nlmsg_len -
		(NLMSG_LENGTH(sizeof(*msg_hdr) + msg_hdr->length +
			      sizeof(struct wifi_pos_interface)));
	if (field_info_len) {
		req->field_info_buf =
			(struct wifi_pos_field_info *)(req->buf + req->buf_len);
		req->field_info_buf_len = sizeof(struct wifi_pos_field_info);
	}

	interface_len = nlh->nlmsg_len -
		(NLMSG_LENGTH(sizeof(*msg_hdr) + msg_hdr->length +
			      req->field_info_buf_len));

	if (interface_len) {
		interface = (char *)(req->buf + req->buf_len +
				     req->field_info_buf_len);
		req->interface.length = *interface;

		if (req->interface.length > sizeof(req->interface.dev_name)) {
			osif_err("interface length exceeds array length");
			return OEM_ERR_INVALID_MESSAGE_LENGTH;
		}

		qdf_mem_copy(req->interface.dev_name,
			     (interface + sizeof(uint8_t)),
			     req->interface.length);

		status = ucfg_wifi_psoc_get_pdev_id_by_dev_name(
				req->interface.dev_name, &pdev_id, psoc);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("failed to get pdev_id and psoc");
			return OEM_ERR_NULL_CONTEXT;
		}

		status = wifi_pos_convert_host_pdev_id_to_target(
				*psoc, pdev_id, &tgt_pdev_id);
		if (QDF_IS_STATUS_ERROR(status)) {
			osif_err("failed to get target pdev_id");
			return OEM_ERR_NULL_CONTEXT;
		}

		for (i = 0;
		     (req->field_info_buf && (i < req->field_info_buf->count));
		     i++) {
			if (req->field_info_buf->fields[i].id ==
					META_DATA_PDEV) {
				offset = req->field_info_buf->fields[i].offset;
				*((uint32_t *)&req->buf[offset]) = tgt_pdev_id;
			}
		}
	}

	return 0;
}
#endif

/**
 * __os_if_wifi_pos_callback() - callback registered with NL service socket to
 * process wifi pos request
 * @skb: request message sk_buff
 *
 * Return: status of operation
 */
#ifdef CNSS_GENL
static void __os_if_wifi_pos_callback(const void *data, int data_len,
				      void *ctx, int pid)
{
	uint8_t err;
	QDF_STATUS status;
	struct wifi_pos_req_msg req = {0};
	struct wlan_objmgr_psoc *psoc = wifi_pos_get_psoc();

	osif_debug("enter: pid %d", pid);
	if (!psoc) {
		osif_err("global psoc object not registered yet.");
		return;
	}

	wlan_objmgr_psoc_get_ref(psoc, WLAN_WIFI_POS_OSIF_ID);
	err = wifi_pos_parse_req(data, data_len, pid, &req);
	if (err) {
		os_if_wifi_pos_send_rsp(psoc, wifi_pos_get_app_pid(psoc),
					WIFI_POS_CMD_ERROR, sizeof(err), &err);
		status = QDF_STATUS_E_INVAL;
		goto release_psoc_ref;
	}

	status = ucfg_wifi_pos_process_req(psoc, &req, os_if_wifi_pos_send_rsp);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("ucfg_wifi_pos_process_req failed. status: %d",
			 status);

release_psoc_ref:
	wlan_objmgr_psoc_release_ref(psoc, WLAN_WIFI_POS_OSIF_ID);
}

static void os_if_wifi_pos_callback(const void *data, int data_len,
				    void *ctx, int pid)
{
	struct qdf_op_sync *op_sync;

	if (qdf_op_protect(&op_sync))
		return;

	__os_if_wifi_pos_callback(data, data_len, ctx, pid);
	qdf_op_unprotect(op_sync);
}
#else
static int __os_if_wifi_pos_callback(struct sk_buff *skb)
{
	uint8_t err;
	QDF_STATUS status;
	struct wifi_pos_req_msg req = {0};
	struct wlan_objmgr_psoc *psoc = NULL;

	osif_debug("enter");

	err = wifi_pos_parse_req(skb, &req, &psoc);
	if (err) {
		osif_err("wifi_pos_parse_req failed");
		return -EINVAL;
	}

	wlan_objmgr_psoc_get_ref(psoc, WLAN_WIFI_POS_OSIF_ID);

	if (err) {
		os_if_wifi_pos_send_rsp(psoc, wifi_pos_get_app_pid(psoc),
					WIFI_POS_CMD_ERROR, sizeof(err), &err);
		status = QDF_STATUS_E_INVAL;
		goto release_psoc_ref;
	}

	status = ucfg_wifi_pos_process_req(psoc, &req, os_if_wifi_pos_send_rsp);
	if (QDF_IS_STATUS_ERROR(status))
		osif_err("ucfg_wifi_pos_process_req failed. status: %d",
			 status);

release_psoc_ref:
	wlan_objmgr_psoc_release_ref(psoc, WLAN_WIFI_POS_OSIF_ID);

	return qdf_status_to_os_return(status);
}

static int os_if_wifi_pos_callback(struct sk_buff *skb)
{
	struct qdf_op_sync *op_sync;
	int err;

	if (qdf_op_protect(&op_sync))
		return -EINVAL;

	err = __os_if_wifi_pos_callback(skb);
	qdf_op_unprotect(op_sync);

	return err;
}
#endif

#ifdef CNSS_GENL
int os_if_wifi_pos_register_nl(void)
{
	int ret = register_cld_cmd_cb(WLAN_NL_MSG_OEM,
				os_if_wifi_pos_callback, NULL);
	if (ret)
		osif_err("register_cld_cmd_cb failed");

	return ret;
}
#else
int os_if_wifi_pos_register_nl(void)
{
	return nl_srv_register(WLAN_NL_MSG_OEM, os_if_wifi_pos_callback);
}
#endif /* CNSS_GENL */
qdf_export_symbol(os_if_wifi_pos_register_nl);

#ifdef CNSS_GENL
int os_if_wifi_pos_deregister_nl(void)
{
	int ret = deregister_cld_cmd_cb(WLAN_NL_MSG_OEM);
	if (ret)
		osif_err("deregister_cld_cmd_cb failed");

	return ret;
}
#else
int os_if_wifi_pos_deregister_nl(void)
{
	return 0;
}
#endif /* CNSS_GENL */

void os_if_wifi_pos_send_peer_status(struct qdf_mac_addr *peer_mac,
				uint8_t peer_status,
				uint8_t peer_timing_meas_cap,
				uint8_t session_id,
				struct wifi_pos_ch_info *chan_info,
				enum QDF_OPMODE dev_mode)
{
	struct wlan_objmgr_psoc *psoc = wifi_pos_get_psoc();
	struct wifi_pos_peer_status_info *peer_info;

	if (!psoc) {
		osif_err("global wifi_pos psoc object not registered");
		return;
	}

	if (!wifi_pos_is_app_registered(psoc) ||
			wifi_pos_get_app_pid(psoc) == 0) {
		osif_debug("app is not registered or pid is invalid");
		return;
	}

	peer_info = qdf_mem_malloc(sizeof(*peer_info));
	if (!peer_info)
		return;

	qdf_mem_copy(peer_info->peer_mac_addr, peer_mac->bytes,
		     sizeof(peer_mac->bytes));
	peer_info->peer_status = peer_status;
	peer_info->vdev_id = session_id;
	peer_info->peer_capability = peer_timing_meas_cap;
	peer_info->reserved0 = 0;
	/* Set 0th bit of reserved0 for STA mode */
	if (QDF_STA_MODE == dev_mode)
		peer_info->reserved0 |= 0x01;

	if (chan_info) {
		peer_info->peer_chan_info.chan_id = chan_info->chan_id;
		peer_info->peer_chan_info.reserved0 = 0;
		peer_info->peer_chan_info.mhz = chan_info->mhz;
		peer_info->peer_chan_info.band_center_freq1 =
			chan_info->band_center_freq1;
		peer_info->peer_chan_info.band_center_freq2 =
			chan_info->band_center_freq2;
		peer_info->peer_chan_info.info = chan_info->info;
		peer_info->peer_chan_info.reg_info_1 = chan_info->reg_info_1;
		peer_info->peer_chan_info.reg_info_2 = chan_info->reg_info_2;
	}

	os_if_wifi_pos_send_rsp(psoc, wifi_pos_get_app_pid(psoc),
				WIFI_POS_PEER_STATUS_IND,
				sizeof(*peer_info), (uint8_t *)peer_info);
	qdf_mem_free(peer_info);
}

int os_if_wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
				   struct wifi_pos_driver_caps *caps)
{
	if (!psoc || !caps) {
		osif_err("psoc or caps buffer is null");
		return -EINVAL;
	}

	return qdf_status_to_os_return(wifi_pos_populate_caps(psoc, caps));
}
