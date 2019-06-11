/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
#include "wlan_nlink_srv.h"
#include "wlan_ptt_sock_svc.h"
#include "wlan_nlink_common.h"
#include "os_if_wifi_pos.h"
#include "wifi_pos_api.h"
#include "wlan_cfg80211.h"
#include "wlan_objmgr_psoc_obj.h"
#ifdef CNSS_GENL
#include <net/cnss_nl.h>
#endif

/**
 * os_if_wifi_pos_send_rsp() - send oem registration response
 *
 * This function sends oem message to registered application process
 *
 * Return:  none
 */
static void os_if_wifi_pos_send_rsp(uint32_t pid, uint32_t rsp_msg_type,
				    uint32_t buf_len, uint8_t *buf)
{
	tAniMsgHdr *aniHdr;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;

	/* OEM msg is always to a specific process and cannot be a broadcast */
	if (pid == 0) {
		osif_err("invalid dest pid");
		return;
	}

	skb = alloc_skb(NLMSG_SPACE(sizeof(tAniMsgHdr) + buf_len), GFP_ATOMIC);
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
	aniHdr->type = rsp_msg_type;
	qdf_mem_copy(&aniHdr[1], buf, buf_len);
	aniHdr->length = buf_len;

	skb_put(skb, NLMSG_SPACE(sizeof(tAniMsgHdr) + buf_len));
	osif_debug("sending oem rsp: type: %d len(%d) to pid (%d)",
		   rsp_msg_type, buf_len, pid);
	nl_srv_ucast_oem(skb, pid, MSG_DONTWAIT);
}

#ifdef CNSS_GENL
static int  wifi_pos_parse_req(const void *data, int len, int pid,
		    struct wifi_pos_req_msg *req)
{
	tAniMsgHdr *msg_hdr;
	struct nlattr *tb[CLD80211_ATTR_MAX + 1];

	if (wlan_cfg80211_nla_parse(tb, CLD80211_ATTR_MAX, data, len, NULL)) {
		osif_err("invalid data in request");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}

	if (!tb[CLD80211_ATTR_DATA]) {
		osif_err("CLD80211_ATTR_DATA not present");
		return OEM_ERR_INVALID_MESSAGE_TYPE;
	}

	msg_hdr = (tAniMsgHdr *)nla_data(tb[CLD80211_ATTR_DATA]);
	if (!msg_hdr) {
		osif_err("msg_hdr null");
		return OEM_ERR_NULL_MESSAGE_HEADER;
	}

	req->msg_type = msg_hdr->type;
	req->buf_len = msg_hdr->length;
	req->buf = (uint8_t *)&msg_hdr[1];
	req->pid = pid;

	if (tb[CLD80211_ATTR_META_DATA]) {
		req->field_info_buf = (struct wifi_pos_field_info *)
					nla_data(tb[CLD80211_ATTR_META_DATA]);
		req->field_info_buf_len = nla_len(tb[CLD80211_ATTR_META_DATA]);
	}

	return 0;
}
#else
static int wifi_pos_parse_req(struct sk_buff *skb, struct wifi_pos_req_msg *req)
{
	/* SKB->data contains NL msg */
	/* NLMSG_DATA(nlh) contains ANI msg */
	struct nlmsghdr *nlh;
	tAniMsgHdr *msg_hdr;

	nlh = (struct nlmsghdr *)skb->data;
	if (!nlh) {
		osif_err("Netlink header null");
		return OEM_ERR_NULL_MESSAGE_HEADER;
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

	req->msg_type = msg_hdr->type;
	req->buf_len = msg_hdr->length;
	req->buf = (uint8_t *)&msg_hdr[1];
	req->pid = nlh->nlmsg_pid;

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
		os_if_wifi_pos_send_rsp(wifi_pos_get_app_pid(psoc),
					ANI_MSG_OEM_ERROR, sizeof(err), &err);
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
	struct wlan_objmgr_psoc *psoc = wifi_pos_get_psoc();

	osif_debug("enter");
	if (!psoc) {
		osif_err("global psoc object not registered yet.");
		return -EINVAL;
	}

	wlan_objmgr_psoc_get_ref(psoc, WLAN_WIFI_POS_OSIF_ID);
	err = wifi_pos_parse_req(skb, &req);
	if (err) {
		os_if_wifi_pos_send_rsp(wifi_pos_get_app_pid(psoc),
					ANI_MSG_OEM_ERROR, sizeof(err), &err);
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
	struct wmi_pos_peer_status_info *peer_info;

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

	os_if_wifi_pos_send_rsp(wifi_pos_get_app_pid(psoc),
				ANI_MSG_PEER_STATUS_IND,
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
