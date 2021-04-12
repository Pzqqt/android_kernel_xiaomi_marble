/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/netlink.h>
#include <uapi/linux/rtnetlink.h>
#include <linux/net.h>
#include <linux/workqueue.h>
#include <net/sock.h>
#include "dfc.h"
#include "rmnet_qmi.h"
#include "rmnet_qmap.h"
#include "qmi_rmnet_i.h"

#define QMAP_LL_VER		1
#define QMAP_LL_MAX_BEARER	15

#define QMAP_SWITCH_TO_LL	1
#define QMAP_SWITCH_TO_DEFAULT	2
#define QMAP_SWITCH_QUERY	3

/* Switch status from modem */
#define SWITCH_STATUS_ERROR	0
#define SWITCH_STATUS_SUCCESS	1
#define SWITCH_STATUS_DEFAULT	2
#define SWITCH_STATUS_LL	3
#define SWITCH_STATUS_FAIL_TEMP	4
#define SWITCH_STATUS_FAIL_PERM	5

/* Internal switch status */
#define SWITCH_STATUS_NONE	0xFF
#define SWITCH_STATUS_TIMEOUT	0xFE
#define SWITCH_STATUS_NO_EFFECT	0xFD

#define LL_MASK_NL_ACK 1
#define LL_MASK_AUTO_RETRY 2

#define LL_TIMEOUT (5 * HZ)
#define LL_RETRY_TIME (10 * HZ)
#define LL_MAX_RETRY (3)

struct qmap_ll_bearer {
	u8			bearer_id;
	u8			status;
	u8			reserved[2];
}  __aligned(1);

struct qmap_ll_switch {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved;
	u8			request_type;
	u8			num_bearers;
	struct qmap_ll_bearer	bearer[0];
} __aligned(1);

struct qmap_ll_switch_resp {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved[2];
	u8			num_bearers;
	struct qmap_ll_bearer	bearer[0];
} __aligned(1);

struct qmap_ll_switch_status {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved[2];
	u8			num_bearers;
	struct qmap_ll_bearer	bearer[0];
} __aligned(1);

/*
 * LL workqueue
 */
static DEFINE_SPINLOCK(ll_wq_lock);
static struct workqueue_struct *ll_wq;

struct ll_ack_work {
	struct work_struct work;
	u32 nl_pid;
	u32 nl_seq;
	u8 bearer_id;
	u8 status_code;
	u8 current_ch;
};

static void ll_ack_fn(struct work_struct *work)
{
	struct ll_ack_work *ack_work;
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	struct nlmsgerr *errmsg;
	unsigned int flags = NLM_F_CAPPED;

	ack_work = container_of(work, struct ll_ack_work, work);

	skb = nlmsg_new(sizeof(*errmsg), GFP_KERNEL);
	if (!skb)
		goto out;

	nlh = __nlmsg_put(skb, ack_work->nl_pid,
			  ack_work->nl_seq, NLMSG_ERROR,
			  sizeof(*errmsg), flags);
	errmsg = nlmsg_data(nlh);
	errmsg->error = 0;
	errmsg->msg.nlmsg_len = sizeof(struct nlmsghdr);
	errmsg->msg.nlmsg_type = ack_work->bearer_id;
	errmsg->msg.nlmsg_flags = ack_work->status_code;
	errmsg->msg.nlmsg_seq = ack_work->current_ch;
	errmsg->msg.nlmsg_pid = ack_work->nl_pid;
	nlmsg_end(skb, nlh);

	rtnl_unicast(skb, &init_net, ack_work->nl_pid);
out:
	kfree(ack_work);
}

static void ll_send_nl_ack(struct rmnet_bearer_map *bearer)
{
	struct ll_ack_work *ack_work;

	if (!(bearer->ch_switch.flags & LL_MASK_NL_ACK))
		return;

	ack_work = kzalloc(sizeof(*ack_work), GFP_ATOMIC);
	if (!ack_work)
		return;

	ack_work->nl_pid = bearer->ch_switch.nl_pid;
	ack_work->nl_seq = bearer->ch_switch.nl_seq;
	ack_work->bearer_id = bearer->bearer_id;
	ack_work->status_code = bearer->ch_switch.status_code;
	ack_work->current_ch = bearer->ch_switch.current_ch;
	INIT_WORK(&ack_work->work, ll_ack_fn);

	spin_lock_bh(&ll_wq_lock);
	if (ll_wq)
		queue_work(ll_wq, &ack_work->work);
	else
		kfree(ack_work);
	spin_unlock_bh(&ll_wq_lock);
}

void rmnet_ll_wq_init(void)
{
	WARN_ON(ll_wq);
	ll_wq = alloc_ordered_workqueue("rmnet_ll_wq", 0);
}

void rmnet_ll_wq_exit(void)
{
	struct workqueue_struct *tmp = NULL;

	spin_lock_bh(&ll_wq_lock);
	if (ll_wq) {
		tmp = ll_wq;
		ll_wq = NULL;
	}
	spin_unlock_bh(&ll_wq_lock);

	if (tmp)
		destroy_workqueue(tmp);
}

/*
 * LLC switch
 */

static void ll_qmap_maybe_set_ch(struct qos_info *qos,
				 struct rmnet_bearer_map *bearer, u8 status)
{
	u8 ch;

	if (status == SWITCH_STATUS_DEFAULT)
		ch = RMNET_CH_DEFAULT;
	else if (status == SWITCH_STATUS_LL)
		ch = RMNET_CH_LL;
	else
		return;

	bearer->ch_switch.current_ch = ch;
	if (bearer->mq_idx < MAX_MQ_NUM)
		qos->mq[bearer->mq_idx].is_ll_ch = ch;
}

static void ll_switch_complete(struct rmnet_bearer_map *bearer, u8 status)
{
	bearer->ch_switch.status_code = status;

	if (status == SWITCH_STATUS_FAIL_TEMP &&
	    bearer->ch_switch.retry_left) {
		/* Temp failure retry */
		bearer->ch_switch.state = CH_SWITCH_FAILED_RETRY;
		mod_timer(&bearer->ch_switch.guard_timer,
			  jiffies + LL_RETRY_TIME);
		bearer->ch_switch.retry_left--;
	} else {
		/* Success or permanent failure */
		bearer->ch_switch.timer_quit = true;
		del_timer(&bearer->ch_switch.guard_timer);
		bearer->ch_switch.state = CH_SWITCH_NONE;
		bearer->ch_switch.retry_left = 0;
		ll_send_nl_ack(bearer);
		bearer->ch_switch.flags = 0;
	}
}

static int ll_qmap_handle_switch_resp(struct sk_buff *skb)
{
	struct qmap_ll_switch_resp *cmd;
	struct rmnet_bearer_map *bearer;
	struct qos_info *qos;
	struct net_device *dev;
	int i;

	if (skb->len < sizeof(struct qmap_ll_switch_resp))
		return QMAP_CMD_DONE;

	cmd = (struct qmap_ll_switch_resp *)skb->data;
	if (!cmd->num_bearers)
		return QMAP_CMD_DONE;

	if (skb->len < sizeof(*cmd) +
		       cmd->num_bearers * sizeof(struct qmap_ll_bearer))
		return QMAP_CMD_DONE;

	dev = rmnet_qmap_get_dev(cmd->hdr.mux_id);
	if (!dev)
		return QMAP_CMD_DONE;

	qos = rmnet_get_qos_pt(dev);
	if (!qos)
		return QMAP_CMD_DONE;

	trace_dfc_ll_switch("ACK", 0, cmd->num_bearers, cmd->bearer);

	spin_lock_bh(&qos->qos_lock);

	for (i = 0; i < cmd->num_bearers; i++) {
		bearer = qmi_rmnet_get_bearer_map(qos,
						  cmd->bearer[i].bearer_id);
		if (!bearer)
			continue;

		ll_qmap_maybe_set_ch(qos, bearer, cmd->bearer[i].status);

		if (bearer->ch_switch.state == CH_SWITCH_STARTED &&
		    bearer->ch_switch.switch_txid == cmd->hdr.tx_id) {
			/* This is an ACK to the switch request */
			if (cmd->bearer[i].status == SWITCH_STATUS_SUCCESS)
				bearer->ch_switch.state = CH_SWITCH_ACKED;
			else
				ll_switch_complete(bearer,
						   cmd->bearer[i].status);
		}
	}

	spin_unlock_bh(&qos->qos_lock);

	return QMAP_CMD_DONE;
}

static int ll_qmap_handle_switch_status(struct sk_buff *skb)
{
	struct qmap_ll_switch_status *cmd;
	struct rmnet_bearer_map *bearer;
	struct qos_info *qos;
	struct net_device *dev;
	int i;

	if (skb->len < sizeof(struct qmap_ll_switch_status))
		return QMAP_CMD_INVALID;

	cmd = (struct qmap_ll_switch_status *)skb->data;
	if (!cmd->num_bearers)
		return QMAP_CMD_ACK;

	if (skb->len < sizeof(*cmd) +
		       cmd->num_bearers * sizeof(struct qmap_ll_bearer))
		return QMAP_CMD_INVALID;

	dev = rmnet_qmap_get_dev(cmd->hdr.mux_id);
	if (!dev)
		return QMAP_CMD_ACK;

	qos = rmnet_get_qos_pt(dev);
	if (!qos)
		return QMAP_CMD_ACK;

	trace_dfc_ll_switch("STS", 0, cmd->num_bearers, cmd->bearer);

	spin_lock_bh(&qos->qos_lock);

	for (i = 0; i < cmd->num_bearers; i++) {
		bearer = qmi_rmnet_get_bearer_map(qos,
						  cmd->bearer[i].bearer_id);
		if (!bearer)
			continue;

		ll_qmap_maybe_set_ch(qos, bearer, cmd->bearer[i].status);

		if (bearer->ch_switch.state == CH_SWITCH_ACKED)
			ll_switch_complete(bearer, cmd->bearer[i].status);
	}

	spin_unlock_bh(&qos->qos_lock);

	return QMAP_CMD_ACK;
}

int ll_qmap_cmd_handler(struct sk_buff *skb)
{
	struct qmap_cmd_hdr *cmd;
	int rc = QMAP_CMD_DONE;

	cmd = (struct qmap_cmd_hdr *)skb->data;

	if (cmd->cmd_name == QMAP_LL_SWITCH) {
		if (cmd->cmd_type != QMAP_CMD_ACK)
			return rc;
	} else if (cmd->cmd_type != QMAP_CMD_REQUEST) {
		return rc;
	}

	switch (cmd->cmd_name) {
	case QMAP_LL_SWITCH:
		rc = ll_qmap_handle_switch_resp(skb);
		break;

	case QMAP_LL_SWITCH_STATUS:
		rc = ll_qmap_handle_switch_status(skb);
		break;

	default:
		if (cmd->cmd_type == QMAP_CMD_REQUEST)
			rc = QMAP_CMD_UNSUPPORTED;
	}

	return rc;
}

static int ll_qmap_send_switch(u8 mux_id, u8 channel, u8 num_bearers,
			       u8 *bearer_list, __be32 *txid)
{
	struct sk_buff *skb;
	struct qmap_ll_switch *ll_switch;
	unsigned int len;
	int i;

	if (!num_bearers || num_bearers > QMAP_LL_MAX_BEARER || !bearer_list)
		return -EINVAL;

	len  = sizeof(struct qmap_ll_switch) +
			num_bearers * sizeof(struct qmap_ll_bearer);

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	skb->protocol = htons(ETH_P_MAP);
	ll_switch = skb_put(skb, len);
	memset(ll_switch, 0, len);

	ll_switch->hdr.cd_bit = 1;
	ll_switch->hdr.mux_id = mux_id;
	ll_switch->hdr.pkt_len = htons(len - QMAP_HDR_LEN);
	ll_switch->hdr.cmd_name = QMAP_LL_SWITCH;
	ll_switch->hdr.cmd_type = QMAP_CMD_REQUEST;
	ll_switch->hdr.tx_id = htonl(rmnet_qmap_next_txid());

	ll_switch->cmd_ver = QMAP_LL_VER;
	if (channel == RMNET_CH_CTL)
		ll_switch->request_type = QMAP_SWITCH_QUERY;
	else if (channel == RMNET_CH_LL)
		ll_switch->request_type = QMAP_SWITCH_TO_LL;
	else
		ll_switch->request_type = QMAP_SWITCH_TO_DEFAULT;
	ll_switch->num_bearers = num_bearers;
	for (i = 0; i < num_bearers; i++)
		ll_switch->bearer[i].bearer_id = bearer_list[i];

	if (txid)
		*txid = ll_switch->hdr.tx_id;

	trace_dfc_ll_switch("REQ", ll_switch->request_type,
			    ll_switch->num_bearers, ll_switch->bearer);

	return rmnet_qmap_send(skb, RMNET_CH_CTL, false);
}

/*
 * Start channel switch. The switch request is sent only if all bearers
 * are eligible to switch. Return 0 if switch request is sent.
 */
int rmnet_ll_switch(struct net_device *dev, struct tcmsg *tcm, int attrlen)
{
	u8 switch_to_ch;
	u8 num_bearers;
	u8 *bearer_list;
	u32 flags;
	struct qos_info *qos;
	struct rmnet_bearer_map *bearer;
	__be32 txid;
	int i;
	int j;
	int rc = -EINVAL;

	if (!dev || !tcm)
		return -EINVAL;
	/*
	 * tcm__pad1: switch type (ch #, 0xFF query)
	 * tcm__pad2: num bearers
	 * tcm_info: flags
	 * tcm_ifindex: netlink fd
	 * tcm_handle: pid
	 * tcm_parent: seq
	 */

	switch_to_ch = tcm->tcm__pad1;
	num_bearers = tcm->tcm__pad2;
	flags = tcm->tcm_info;

	if (switch_to_ch != RMNET_CH_CTL && switch_to_ch >= RMNET_CH_MAX)
		return -EOPNOTSUPP;
	if (!num_bearers || num_bearers > QMAP_LL_MAX_BEARER)
		return -EINVAL;
	if (attrlen - sizeof(*tcm) < num_bearers)
		return -EINVAL;

	bearer_list = (u8 *)tcm + sizeof(*tcm);

	for (i = 0; i < num_bearers; i++)
		for (j = 0; j < num_bearers; j++)
			if (j != i && bearer_list[i] == bearer_list[j])
				return -EINVAL;

	qos = rmnet_get_qos_pt(dev);
	if (!qos)
		return -EINVAL;

	spin_lock_bh(&qos->qos_lock);

	/* Validate the bearer list */
	for (i = 0; i < num_bearers; i++) {
		bearer = qmi_rmnet_get_bearer_map(qos, bearer_list[i]);
		if (!bearer) {
			rc = -EFAULT;
			goto out;
		}
		if (bearer->ch_switch.state != CH_SWITCH_NONE) {
			rc = -EBUSY;
			goto out;
		}
	}

	/* Send QMAP switch command */
	rc = ll_qmap_send_switch(qos->mux_id, switch_to_ch,
				 num_bearers, bearer_list, &txid);
	if (rc)
		goto out;

	/* Update state */
	for (i = 0; i < num_bearers; i++) {
		bearer = qmi_rmnet_get_bearer_map(qos, bearer_list[i]);
		if (!bearer)
			continue;
		bearer->ch_switch.switch_to_ch = switch_to_ch;
		bearer->ch_switch.switch_txid = txid;
		bearer->ch_switch.state = CH_SWITCH_STARTED;
		bearer->ch_switch.status_code = SWITCH_STATUS_NONE;
		bearer->ch_switch.retry_left =
			(flags & LL_MASK_AUTO_RETRY) ? LL_MAX_RETRY : 0;
		bearer->ch_switch.flags = flags;
		bearer->ch_switch.timer_quit = false;
		mod_timer(&bearer->ch_switch.guard_timer,
			  jiffies + LL_TIMEOUT);

		bearer->ch_switch.nl_pid = tcm->tcm_handle;
		bearer->ch_switch.nl_seq = tcm->tcm_parent;
	}
out:
	spin_unlock_bh(&qos->qos_lock);
	return rc;
}

void rmnet_ll_guard_fn(struct timer_list *t)
{
	struct rmnet_ch_switch *ch_switch;
	struct rmnet_bearer_map *bearer;
	int switch_status = SWITCH_STATUS_TIMEOUT;
	__be32 txid;
	int rc;

	ch_switch = container_of(t, struct rmnet_ch_switch, guard_timer);
	bearer = container_of(ch_switch, struct rmnet_bearer_map, ch_switch);

	spin_lock_bh(&bearer->qos->qos_lock);

	if (bearer->ch_switch.timer_quit ||
	    bearer->ch_switch.state == CH_SWITCH_NONE)
		goto out;

	if (bearer->ch_switch.state == CH_SWITCH_FAILED_RETRY) {
		if (bearer->ch_switch.current_ch ==
		    bearer->ch_switch.switch_to_ch) {
			switch_status = SWITCH_STATUS_NO_EFFECT;
			goto send_err;
		}

		rc = ll_qmap_send_switch(bearer->qos->mux_id,
					 bearer->ch_switch.switch_to_ch,
					 1,
					 &bearer->bearer_id,
					 &txid);
		if (!rc) {
			bearer->ch_switch.switch_txid = txid;
			bearer->ch_switch.state = CH_SWITCH_STARTED;
			bearer->ch_switch.status_code = SWITCH_STATUS_NONE;
			goto out;
		}
	}

send_err:
	bearer->ch_switch.state = CH_SWITCH_NONE;
	bearer->ch_switch.status_code = switch_status;
	bearer->ch_switch.retry_left = 0;
	ll_send_nl_ack(bearer);
	bearer->ch_switch.flags = 0;

out:
	spin_unlock_bh(&bearer->qos->qos_lock);
}
