/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/******************************************************************************
* wlan_ptt_sock_svc.c
*
******************************************************************************/
#ifdef PTT_SOCK_SVC_ENABLE
#include <wlan_nlink_srv.h>
#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_trace.h>
#include <wlan_nlink_common.h>
#include <wlan_ptt_sock_svc.h>
#include <qdf_types.h>
#include <qdf_trace.h>

#define PTT_SOCK_DEBUG
#ifdef PTT_SOCK_DEBUG
#define PTT_TRACE(level, args ...) QDF_TRACE(QDF_MODULE_ID_QDF, level, ## args)
#else
#define PTT_TRACE(level, args ...)
#endif

/** ptt Process ID */
static int32_t ptt_pid = INVALID_PID;

#ifdef PTT_SOCK_DEBUG_VERBOSE
/* Utility function to perform a hex dump */
static void ptt_sock_dump_buf(const unsigned char *pbuf, int cnt)
{
	int i;
	for (i = 0; i < cnt; i++) {
		if ((i % 16) == 0)
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
				  "\n%p:", pbuf);
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO, " %02X",
			  *pbuf);
		pbuf++;
	}
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO, "\n");
}
#endif

/**
 * ptt_sock_send_msg_to_app() - Send nl message to user space
 * wmsg: Message header
 * radio: Unit number of the radio
 * src_mod: Message type
 * pid: Process ID to which message will be unicast. Message
 * will be broadcast when PID is INVALID_PID
 *
 * Utility function to send a netlink message to an application in user space
 *
 * Return: 0 on success and negative value on failure
 */
int ptt_sock_send_msg_to_app(tAniHdr *wmsg, int radio, int src_mod, int pid)
{
	int err = -1;
	int payload_len;
	int tot_msg_len;
	tAniNlHdr *wnl;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int wmsg_length = be16_to_cpu(wmsg->length);
	static int nlmsg_seq;

	if (radio < 0 || radio > ANI_MAX_RADIOS) {
		PTT_TRACE(QDF_TRACE_LEVEL_ERROR, "%s: invalid radio id [%d]\n",
			  __func__, radio);
		return -EINVAL;
	}
	payload_len = wmsg_length + sizeof(wnl->radio) + sizeof(*wmsg);
	tot_msg_len = NLMSG_SPACE(payload_len);
	skb = dev_alloc_skb(tot_msg_len);
	if (skb  == NULL) {
		PTT_TRACE(QDF_TRACE_LEVEL_ERROR,
			  "%s: dev_alloc_skb() failed for msg size[%d]\n",
			  __func__, tot_msg_len);
		return -ENOMEM;
	}
	nlh =
		nlmsg_put(skb, pid, nlmsg_seq++, src_mod, payload_len,
			  NLM_F_REQUEST);
	if (NULL == nlh) {
		PTT_TRACE(QDF_TRACE_LEVEL_ERROR,
			  "%s: nlmsg_put() failed for msg size[%d]\n", __func__,
			  tot_msg_len);
		kfree_skb(skb);
		return -ENOMEM;
	}
	wnl = (tAniNlHdr *) nlh;
	wnl->radio = radio;
	memcpy(&wnl->wmsg, wmsg, wmsg_length);
#ifdef PTT_SOCK_DEBUG_VERBOSE
	ptt_sock_dump_buf((const unsigned char *)skb->data, skb->len);
#endif

	if (pid != INVALID_PID)
		err = nl_srv_ucast(skb, pid, MSG_DONTWAIT);
	else
		err = nl_srv_bcast(skb);

	if (err)
		PTT_TRACE(QDF_TRACE_LEVEL_INFO,
			  "%s:Failed sending Msg Type [0x%X] to pid[%d]\n",
			  __func__, be16_to_cpu(wmsg->type), pid);
	return err;
}

/*
 * Process tregisteration request and send registration response messages
 * to the PTT Socket App in user space
 */
static void ptt_sock_proc_reg_req(tAniHdr *wmsg, int radio)
{
	tAniNlAppRegReq *reg_req;
	tAniNlAppRegRsp rspmsg;
	reg_req = (tAniNlAppRegReq *) (wmsg + 1);
	memset((char *)&rspmsg, 0, sizeof(rspmsg));
	/* send reg response message to the application */
	rspmsg.ret = ANI_NL_MSG_OK;
	rspmsg.regReq.type = reg_req->type;
	/*Save the pid */
	ptt_pid = reg_req->pid;
	rspmsg.regReq.pid = reg_req->pid;
	rspmsg.wniHdr.type = cpu_to_be16(ANI_MSG_APP_REG_RSP);
	rspmsg.wniHdr.length = cpu_to_be16(sizeof(rspmsg));
	if (ptt_sock_send_msg_to_app((tAniHdr *) &rspmsg.wniHdr, radio,
				     ANI_NL_MSG_PUMAC, ptt_pid) < 0) {
		PTT_TRACE(QDF_TRACE_LEVEL_INFO,
			  "%s: Error sending ANI_MSG_APP_REG_RSP to pid[%d]\n",
			  __func__, ptt_pid);
	}
}

/*
 * Process all the messages from the PTT Socket App in user space
 */
static void ptt_proc_pumac_msg(struct sk_buff *skb, tAniHdr *wmsg, int radio)
{
	u16 ani_msg_type = be16_to_cpu(wmsg->type);
	switch (ani_msg_type) {
	case ANI_MSG_APP_REG_REQ:
		PTT_TRACE(QDF_TRACE_LEVEL_INFO,
			  "%s: Received ANI_MSG_APP_REG_REQ [0x%X]\n", __func__,
			  ani_msg_type);
		ptt_sock_proc_reg_req(wmsg, radio);
		break;
	default:
		PTT_TRACE(QDF_TRACE_LEVEL_ERROR,
			  "%s: Received Unknown Msg Type[0x%X]\n", __func__,
			  ani_msg_type);
		break;
	}
}

/*
 * Process all the Netlink messages from PTT Socket app in user space
 */
static int ptt_sock_rx_nlink_msg(struct sk_buff *skb)
{
	tAniNlHdr *wnl;
	int radio;
	int type;
	wnl = (tAniNlHdr *) skb->data;
	radio = wnl->radio;
	type = wnl->nlh.nlmsg_type;
	switch (type) {
	case ANI_NL_MSG_PUMAC:  /* Message from the PTT socket APP */
		PTT_TRACE(QDF_TRACE_LEVEL_INFO,
			  "%s: Received ANI_NL_MSG_PUMAC Msg [0x%X]\n",
			  __func__, type);
		ptt_proc_pumac_msg(skb, &wnl->wmsg, radio);
		break;
	default:
		PTT_TRACE(QDF_TRACE_LEVEL_ERROR, "%s: Unknown NL Msg [0x%X]\n",
			  __func__, type);
		break;
	}
	return 0;
}

/**
 * ptt_sock_activate_svc() - activate PTT service
 *
 * Return: 0
 */
int ptt_sock_activate_svc(void)
{
	ptt_pid = INVALID_PID;
	nl_srv_register(ANI_NL_MSG_PUMAC, ptt_sock_rx_nlink_msg);
	nl_srv_register(ANI_NL_MSG_PTT, ptt_sock_rx_nlink_msg);
	return 0;
}

/**
 * ptt_sock_deactivate_svc() - deactivate PTT service
 *
 * Return: Void
 */
void ptt_sock_deactivate_svc(void)
{
	ptt_pid = INVALID_PID;
}

#endif /* PTT_SOCK_SVC_ENABLE */
