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

#include "dfc.h"
#include "rmnet_qmi.h"
#include "rmnet_ctl.h"
#include "rmnet_qmap.h"

static atomic_t qmap_txid;
static void *rmnet_ctl_handle;
static void *rmnet_port;
static struct net_device *real_data_dev;
static struct rmnet_ctl_client_if *rmnet_ctl;

int rmnet_qmap_send(struct sk_buff *skb, u8 ch, bool flush)
{
	trace_dfc_qmap(skb->data, skb->len, false);

	if (ch != RMNET_CH_CTL && real_data_dev) {
		skb->protocol = htons(ETH_P_MAP);
		skb->dev = real_data_dev;
		rmnet_ctl->log(RMNET_CTL_LOG_DEBUG, "TXI", 0, skb->data,
			       skb->len);

		rmnet_map_tx_qmap_cmd(skb, ch, flush);
		return 0;
	}

	if (rmnet_ctl->send(rmnet_ctl_handle, skb)) {
		pr_err("Failed to send to rmnet ctl\n");
		return -ECOMM;
	}

	return 0;
}

static void rmnet_qmap_cmd_handler(struct sk_buff *skb)
{
	struct qmap_cmd_hdr *cmd;
	int rc = QMAP_CMD_DONE;

	if (!skb)
		return;

	trace_dfc_qmap(skb->data, skb->len, true);

	if (skb->len < sizeof(struct qmap_cmd_hdr))
		goto free_skb;

	cmd = (struct qmap_cmd_hdr *)skb->data;
	if (!cmd->cd_bit || skb->len != ntohs(cmd->pkt_len) + QMAP_HDR_LEN)
		goto free_skb;

	rcu_read_lock();

	switch (cmd->cmd_name) {
	case QMAP_DFC_CONFIG:
	case QMAP_DFC_IND:
	case QMAP_DFC_QUERY:
	case QMAP_DFC_END_MARKER:
	case QMAP_DFC_POWERSAVE:
		rc = dfc_qmap_cmd_handler(skb);
		break;

	case QMAP_LL_SWITCH:
	case QMAP_LL_SWITCH_STATUS:
		rc = ll_qmap_cmd_handler(skb);
		break;

	default:
		if (cmd->cmd_type == QMAP_CMD_REQUEST)
			rc = QMAP_CMD_UNSUPPORTED;
	}

	/* Send ack */
	if (rc != QMAP_CMD_DONE) {
		if (rc == QMAP_CMD_ACK_INBAND) {
			cmd->cmd_type = QMAP_CMD_ACK;
			rmnet_qmap_send(skb, RMNET_CH_DEFAULT, false);
		} else {
			cmd->cmd_type = rc;
			rmnet_qmap_send(skb, RMNET_CH_CTL, false);
		}
		rcu_read_unlock();
		return;
	}

	rcu_read_unlock();

free_skb:
	kfree_skb(skb);
}

static struct rmnet_ctl_client_hooks cb = {
	.ctl_dl_client_hook = rmnet_qmap_cmd_handler,
};

int rmnet_qmap_next_txid(void)
{
	return atomic_inc_return(&qmap_txid);
}

struct net_device *rmnet_qmap_get_dev(u8 mux_id)
{
	return rmnet_get_rmnet_dev(rmnet_port, mux_id);
}

int rmnet_qmap_init(void *port)
{
	if (rmnet_ctl_handle)
		return 0;

	atomic_set(&qmap_txid, 0);
	rmnet_port = port;
	real_data_dev = rmnet_get_real_dev(rmnet_port);

	rmnet_ctl = rmnet_ctl_if();
	if (!rmnet_ctl) {
		pr_err("rmnet_ctl module not loaded\n");
		return -EFAULT;
	}

	if (rmnet_ctl->reg)
		rmnet_ctl_handle = rmnet_ctl->reg(&cb);

	if (!rmnet_ctl_handle) {
		pr_err("Failed to register with rmnet ctl\n");
		return -EFAULT;
	}

	return 0;
}

void rmnet_qmap_exit(void)
{
	if (rmnet_ctl && rmnet_ctl->dereg)
		rmnet_ctl->dereg(rmnet_ctl_handle);

	rmnet_ctl_handle = NULL;
	real_data_dev = NULL;
	rmnet_port = NULL;
}
