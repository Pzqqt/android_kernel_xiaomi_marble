// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 */

#include <net/pkt_sched.h>
#include <linux/module.h>
#include "rmnet_qmap.h"
#include "dfc_defs.h"
#include "rmnet_qmi.h"
#include "qmi_rmnet.h"
#include "dfc.h"

#define QMAP_DFC_VER		1
#define QMAP_PS_MAX_BEARERS	32

struct qmap_dfc_config {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			cmd_id;
	u8			reserved;
	u8			tx_info:1;
	u8			reserved2:7;
	__be32			ep_type;
	__be32			iface_id;
	u32			reserved3;
} __aligned(1);

struct qmap_dfc_ind {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved;
	__be16			seq_num;
	u8			reserved2;
	u8			tx_info_valid:1;
	u8			tx_info:1;
	u8			rx_bytes_valid:1;
	u8			reserved3:5;
	u8			bearer_id;
	u8			tcp_bidir:1;
	u8			bearer_status:3;
	u8			reserved4:4;
	__be32			grant;
	__be32			rx_bytes;
	u32			reserved6;
} __aligned(1);

struct qmap_dfc_query {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved;
	u8			bearer_id;
	u8			reserved2;
	u32			reserved3;
} __aligned(1);

struct qmap_dfc_query_resp {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			bearer_id;
	u8			tcp_bidir:1;
	u8			rx_bytes_valid:1;
	u8			reserved:6;
	u8			invalid:1;
	u8			reserved2:7;
	__be32			grant;
	__be32			rx_bytes;
	u32			reserved4;
} __aligned(1);

struct qmap_dfc_end_marker_req {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved;
	u8			bearer_id;
	u8			reserved2;
	u16			reserved3;
	__be16			seq_num;
	u32			reserved4;
} __aligned(1);

struct qmap_dfc_end_marker_cnf {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			reserved;
	u8			bearer_id;
	u8			reserved2;
	u16			reserved3;
	__be16			seq_num;
	u32			reserved4;
} __aligned(1);

struct qmap_dfc_powersave_req {
	struct qmap_cmd_hdr	hdr;
	u8			cmd_ver;
	u8			allow:1;
	u8			autoshut:1;
	u8			reserved:6;
	u8			reserved2;
	u8			mode:1;
	u8			reserved3:7;
	__be32			ep_type;
	__be32			iface_id;
	u8			num_bearers;
	u8			bearer_id[QMAP_PS_MAX_BEARERS];
	u8			reserved4[3];
} __aligned(1);

static struct dfc_flow_status_ind_msg_v01 qmap_flow_ind;
static struct dfc_tx_link_status_ind_msg_v01 qmap_tx_ind;
static struct dfc_qmi_data __rcu *qmap_dfc_data;
static bool dfc_config_acked;

static void dfc_qmap_send_config(struct dfc_qmi_data *data);
static void dfc_qmap_send_end_marker_cnf(struct qos_info *qos,
					 struct rmnet_bearer_map *bearer,
					 u16 seq, u32 tx_id);

static int dfc_qmap_handle_ind(struct dfc_qmi_data *dfc,
			       struct sk_buff *skb)
{
	struct qmap_dfc_ind *cmd;

	if (skb->len < sizeof(struct qmap_dfc_ind))
		return QMAP_CMD_INVALID;

	cmd = (struct qmap_dfc_ind *)skb->data;

	if (cmd->tx_info_valid) {
		memset(&qmap_tx_ind, 0, sizeof(qmap_tx_ind));
		qmap_tx_ind.tx_status = cmd->tx_info;
		qmap_tx_ind.bearer_info_valid = 1;
		qmap_tx_ind.bearer_info_len = 1;
		qmap_tx_ind.bearer_info[0].mux_id = cmd->hdr.mux_id;
		qmap_tx_ind.bearer_info[0].bearer_id = cmd->bearer_id;

		dfc_handle_tx_link_status_ind(dfc, &qmap_tx_ind);

		/* Ignore grant since it is always 0 */
		goto done;
	}

	memset(&qmap_flow_ind, 0, sizeof(qmap_flow_ind));
	qmap_flow_ind.flow_status_valid = 1;
	qmap_flow_ind.flow_status_len = 1;
	qmap_flow_ind.flow_status[0].mux_id = cmd->hdr.mux_id;
	qmap_flow_ind.flow_status[0].bearer_id = cmd->bearer_id;
	qmap_flow_ind.flow_status[0].num_bytes = ntohl(cmd->grant);
	qmap_flow_ind.flow_status[0].seq_num = ntohs(cmd->seq_num);

	if (cmd->rx_bytes_valid) {
		qmap_flow_ind.flow_status[0].rx_bytes_valid = 1;
		qmap_flow_ind.flow_status[0].rx_bytes = ntohl(cmd->rx_bytes);
	}

	if (cmd->tcp_bidir) {
		qmap_flow_ind.ancillary_info_valid = 1;
		qmap_flow_ind.ancillary_info_len = 1;
		qmap_flow_ind.ancillary_info[0].mux_id = cmd->hdr.mux_id;
		qmap_flow_ind.ancillary_info[0].bearer_id = cmd->bearer_id;
		qmap_flow_ind.ancillary_info[0].reserved = DFC_MASK_TCP_BIDIR;
	}

	dfc_do_burst_flow_control(dfc, &qmap_flow_ind, false);

done:
	return QMAP_CMD_ACK;
}

static int dfc_qmap_handle_query_resp(struct dfc_qmi_data *dfc,
				      struct sk_buff *skb)
{
	struct qmap_dfc_query_resp *cmd;

	if (skb->len < sizeof(struct qmap_dfc_query_resp))
		return QMAP_CMD_DONE;

	cmd = (struct qmap_dfc_query_resp *)skb->data;

	if (cmd->invalid)
		return QMAP_CMD_DONE;

	memset(&qmap_flow_ind, 0, sizeof(qmap_flow_ind));
	qmap_flow_ind.flow_status_valid = 1;
	qmap_flow_ind.flow_status_len = 1;

	qmap_flow_ind.flow_status[0].mux_id = cmd->hdr.mux_id;
	qmap_flow_ind.flow_status[0].bearer_id = cmd->bearer_id;
	qmap_flow_ind.flow_status[0].num_bytes = ntohl(cmd->grant);
	qmap_flow_ind.flow_status[0].seq_num = 0xFFFF;

	if (cmd->rx_bytes_valid) {
		qmap_flow_ind.flow_status[0].rx_bytes_valid = 1;
		qmap_flow_ind.flow_status[0].rx_bytes = ntohl(cmd->rx_bytes);
	}

	if (cmd->tcp_bidir) {
		qmap_flow_ind.ancillary_info_valid = 1;
		qmap_flow_ind.ancillary_info_len = 1;
		qmap_flow_ind.ancillary_info[0].mux_id = cmd->hdr.mux_id;
		qmap_flow_ind.ancillary_info[0].bearer_id = cmd->bearer_id;
		qmap_flow_ind.ancillary_info[0].reserved = DFC_MASK_TCP_BIDIR;
	}

	dfc_do_burst_flow_control(dfc, &qmap_flow_ind, true);

	return QMAP_CMD_DONE;
}

static int dfc_qmap_set_end_marker(struct dfc_qmi_data *dfc, u8 mux_id,
				   u8 bearer_id, u16 seq_num, u32 tx_id)
{
	struct net_device *dev;
	struct qos_info *qos;
	struct rmnet_bearer_map *bearer;
	int rc = QMAP_CMD_ACK;

	dev = rmnet_get_rmnet_dev(dfc->rmnet_port, mux_id);
	if (!dev)
		return rc;

	qos = (struct qos_info *)rmnet_get_qos_pt(dev);
	if (!qos)
		return rc;

	spin_lock_bh(&qos->qos_lock);

	bearer = qmi_rmnet_get_bearer_map(qos, bearer_id);
	if (!bearer) {
		spin_unlock_bh(&qos->qos_lock);
		return rc;
	}

	if (bearer->last_seq == seq_num && bearer->grant_size) {
		bearer->ack_req = 1;
		bearer->ack_txid = tx_id;
	} else {
		dfc_qmap_send_end_marker_cnf(qos, bearer, seq_num, tx_id);
	}

	spin_unlock_bh(&qos->qos_lock);

	return QMAP_CMD_DONE;
}

static int dfc_qmap_handle_end_marker_req(struct dfc_qmi_data *dfc,
					  struct sk_buff *skb)
{
	struct qmap_dfc_end_marker_req *cmd;

	if (skb->len < sizeof(struct qmap_dfc_end_marker_req))
		return QMAP_CMD_INVALID;

	cmd = (struct qmap_dfc_end_marker_req *)skb->data;

	return dfc_qmap_set_end_marker(dfc, cmd->hdr.mux_id, cmd->bearer_id,
				       ntohs(cmd->seq_num),
				       ntohl(cmd->hdr.tx_id));
}

int dfc_qmap_cmd_handler(struct sk_buff *skb)
{
	struct qmap_cmd_hdr *cmd;
	struct dfc_qmi_data *dfc;
	int rc = QMAP_CMD_DONE;

	cmd = (struct qmap_cmd_hdr *)skb->data;

	if (cmd->cmd_name == QMAP_DFC_QUERY) {
		if (cmd->cmd_type != QMAP_CMD_ACK)
			return rc;
	} else if (cmd->cmd_type != QMAP_CMD_REQUEST) {
		if (cmd->cmd_type == QMAP_CMD_ACK &&
		    cmd->cmd_name == QMAP_DFC_CONFIG)
			dfc_config_acked = true;
		return rc;
	}

	dfc = rcu_dereference(qmap_dfc_data);
	if (!dfc || READ_ONCE(dfc->restart_state))
		return rc;

	/* Re-send DFC config once if needed */
	if (unlikely(!dfc_config_acked)) {
		dfc_qmap_send_config(dfc);
		dfc_config_acked = true;
	}

	switch (cmd->cmd_name) {
	case QMAP_DFC_IND:
		rc = dfc_qmap_handle_ind(dfc, skb);
		qmi_rmnet_set_dl_msg_active(dfc->rmnet_port);
		break;

	case QMAP_DFC_QUERY:
		rc = dfc_qmap_handle_query_resp(dfc, skb);
		break;

	case QMAP_DFC_END_MARKER:
		rc = dfc_qmap_handle_end_marker_req(dfc, skb);
		break;

	default:
		if (cmd->cmd_type == QMAP_CMD_REQUEST)
			rc = QMAP_CMD_UNSUPPORTED;
	}

	return rc;
}

static void dfc_qmap_send_config(struct dfc_qmi_data *data)
{
	struct sk_buff *skb;
	struct qmap_dfc_config *dfc_config;
	unsigned int len = sizeof(struct qmap_dfc_config);

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return;

	skb->protocol = htons(ETH_P_MAP);
	dfc_config = (struct qmap_dfc_config *)skb_put(skb, len);
	memset(dfc_config, 0, len);

	dfc_config->hdr.cd_bit = 1;
	dfc_config->hdr.mux_id = 0;
	dfc_config->hdr.pkt_len = htons(len - QMAP_HDR_LEN);
	dfc_config->hdr.cmd_name = QMAP_DFC_CONFIG;
	dfc_config->hdr.cmd_type = QMAP_CMD_REQUEST;
	dfc_config->hdr.tx_id = htonl(rmnet_qmap_next_txid());

	dfc_config->cmd_ver = QMAP_DFC_VER;
	dfc_config->cmd_id = QMAP_DFC_IND;
	dfc_config->tx_info = 1;
	dfc_config->ep_type = htonl(data->svc.ep_type);
	dfc_config->iface_id = htonl(data->svc.iface_id);

	rmnet_qmap_send(skb, RMNET_CH_CTL, false);
}

static void dfc_qmap_send_query(u8 mux_id, u8 bearer_id)
{
	struct sk_buff *skb;
	struct qmap_dfc_query *dfc_query;
	unsigned int len = sizeof(struct qmap_dfc_query);

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return;

	skb->protocol = htons(ETH_P_MAP);
	dfc_query = (struct qmap_dfc_query *)skb_put(skb, len);
	memset(dfc_query, 0, len);

	dfc_query->hdr.cd_bit = 1;
	dfc_query->hdr.mux_id = mux_id;
	dfc_query->hdr.pkt_len = htons(len - QMAP_HDR_LEN);
	dfc_query->hdr.cmd_name = QMAP_DFC_QUERY;
	dfc_query->hdr.cmd_type = QMAP_CMD_REQUEST;
	dfc_query->hdr.tx_id = htonl(rmnet_qmap_next_txid());

	dfc_query->cmd_ver = QMAP_DFC_VER;
	dfc_query->bearer_id = bearer_id;

	rmnet_qmap_send(skb, RMNET_CH_CTL, false);
}

static void dfc_qmap_send_end_marker_cnf(struct qos_info *qos,
					 struct rmnet_bearer_map *bearer,
					 u16 seq, u32 tx_id)
{
	struct sk_buff *skb;
	struct qmap_dfc_end_marker_cnf *em_cnf;
	unsigned int len = sizeof(struct qmap_dfc_end_marker_cnf);

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return;

	em_cnf = (struct qmap_dfc_end_marker_cnf *)skb_put(skb, len);
	memset(em_cnf, 0, len);

	em_cnf->hdr.cd_bit = 1;
	em_cnf->hdr.mux_id = qos->mux_id;
	em_cnf->hdr.pkt_len = htons(len - QMAP_HDR_LEN);
	em_cnf->hdr.cmd_name = QMAP_DFC_END_MARKER;
	em_cnf->hdr.cmd_type = QMAP_CMD_ACK;
	em_cnf->hdr.tx_id = htonl(tx_id);

	em_cnf->cmd_ver = QMAP_DFC_VER;
	em_cnf->bearer_id = bearer->bearer_id;
	em_cnf->seq_num = htons(seq);

	/* This cmd needs to be sent in-band after data on the currnet
	 * channel. But due to IPA bug, it cannot be sent over LLC so send
	 * it over QMAP channel if current channel is LLC.
	 */
	if (bearer->ch_switch.current_ch == RMNET_CH_DEFAULT)
		rmnet_qmap_send(skb, bearer->ch_switch.current_ch, true);
	else
		rmnet_qmap_send(skb, RMNET_CH_CTL, false);
}

static int dfc_qmap_send_powersave(u8 enable, u8 num_bearers, u8 *bearer_id)
{
	struct sk_buff *skb;
	struct qmap_dfc_powersave_req *dfc_powersave;
	unsigned int len = sizeof(struct qmap_dfc_powersave_req);
	struct dfc_qmi_data *dfc;
	u32 ep_type = 0;
	u32 iface_id = 0;

	rcu_read_lock();
	dfc = rcu_dereference(qmap_dfc_data);
	if (dfc) {
		ep_type = dfc->svc.ep_type;
		iface_id = dfc->svc.iface_id;
	} else {
		rcu_read_unlock();
		return -EINVAL;
	}
	rcu_read_unlock();

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	skb->protocol = htons(ETH_P_MAP);
	dfc_powersave = (struct qmap_dfc_powersave_req *)skb_put(skb, len);
	memset(dfc_powersave, 0, len);

	dfc_powersave->hdr.cd_bit = 1;
	dfc_powersave->hdr.mux_id = 0;
	dfc_powersave->hdr.pkt_len = htons(len - QMAP_HDR_LEN);
	dfc_powersave->hdr.cmd_name = QMAP_DFC_POWERSAVE;
	dfc_powersave->hdr.cmd_type = QMAP_CMD_REQUEST;
	dfc_powersave->hdr.tx_id =  htonl(rmnet_qmap_next_txid());

	dfc_powersave->cmd_ver = 3;
	dfc_powersave->mode = enable ? 1 : 0;

	if (enable && num_bearers) {
		if (unlikely(num_bearers > QMAP_PS_MAX_BEARERS))
			num_bearers = QMAP_PS_MAX_BEARERS;
		dfc_powersave->allow = 1;
		dfc_powersave->autoshut = 1;
		dfc_powersave->num_bearers = num_bearers;
		memcpy(dfc_powersave->bearer_id, bearer_id, num_bearers);
	}

	dfc_powersave->ep_type = htonl(ep_type);
	dfc_powersave->iface_id = htonl(iface_id);

	return rmnet_qmap_send(skb, RMNET_CH_CTL, false);
}

int dfc_qmap_set_powersave(u8 enable, u8 num_bearers, u8 *bearer_id)
{
	trace_dfc_set_powersave_mode(enable);
	return dfc_qmap_send_powersave(enable, num_bearers, bearer_id);
}

void dfc_qmap_send_ack(struct qos_info *qos, u8 bearer_id, u16 seq, u8 type)
{
	struct rmnet_bearer_map *bearer;

	if (type == DFC_ACK_TYPE_DISABLE) {
		bearer = qmi_rmnet_get_bearer_map(qos, bearer_id);
		if (bearer)
			dfc_qmap_send_end_marker_cnf(qos, bearer,
						     seq, bearer->ack_txid);
	} else if (type == DFC_ACK_TYPE_THRESHOLD) {
		dfc_qmap_send_query(qos->mux_id, bearer_id);
	}
}

int dfc_qmap_client_init(void *port, int index, struct svc_info *psvc,
			 struct qmi_info *qmi)
{
	struct dfc_qmi_data *data;

	if (!port || !qmi)
		return -EINVAL;

	/* Prevent double init */
	data = rcu_dereference(qmap_dfc_data);
	if (data)
		return -EINVAL;

	data = kzalloc(sizeof(struct dfc_qmi_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->rmnet_port = port;
	data->index = index;
	memcpy(&data->svc, psvc, sizeof(data->svc));

	qmi->dfc_clients[index] = (void *)data;
	rcu_assign_pointer(qmap_dfc_data, data);

	rmnet_qmap_init(port);

	trace_dfc_client_state_up(data->index, data->svc.instance,
				  data->svc.ep_type, data->svc.iface_id);

	pr_info("DFC QMAP init\n");

	/* Currently if powersave ext is enabled, no need to do dfc config
	 * which only enables tx_info */
	if (qmi->ps_ext) {
		dfc_config_acked = true;
	} else {
		dfc_config_acked = false;
		dfc_qmap_send_config(data);
	}

	return 0;
}

void dfc_qmap_client_exit(void *dfc_data)
{
	struct dfc_qmi_data *data = (struct dfc_qmi_data *)dfc_data;

	if (!data) {
		pr_err("%s() data is null\n", __func__);
		return;
	}

	trace_dfc_client_state_down(data->index, 0);

	rmnet_qmap_exit();

	WRITE_ONCE(data->restart_state, 1);
	RCU_INIT_POINTER(qmap_dfc_data, NULL);
	synchronize_rcu();

	kfree(data);

	pr_info("DFC QMAP exit\n");
}
