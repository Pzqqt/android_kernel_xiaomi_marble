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

#ifndef __RMNET_QMAP_H
#define __RMNET_QMAP_H

#include "qmi_rmnet_i.h"

#define QMAP_CMD_DONE		-1
#define QMAP_CMD_ACK_INBAND	-2

#define QMAP_CMD_REQUEST	0
#define QMAP_CMD_ACK		1
#define QMAP_CMD_UNSUPPORTED	2
#define QMAP_CMD_INVALID	3

struct qmap_hdr {
	u8	cd_pad;
	u8	mux_id;
	__be16	pkt_len;
} __aligned(1);

#define QMAP_HDR_LEN sizeof(struct qmap_hdr)

struct qmap_cmd_hdr {
	u8	pad_len:6;
	u8	reserved_bit:1;
	u8	cd_bit:1;
	u8	mux_id;
	__be16	pkt_len;
	u8	cmd_name;
	u8	cmd_type:2;
	u8	reserved:6;
	u16	reserved2;
	__be32	tx_id;
} __aligned(1);

int rmnet_qmap_init(void *port);
void rmnet_qmap_exit(void);
int rmnet_qmap_next_txid(void);
int rmnet_qmap_send(struct sk_buff *skb, u8 ch, bool flush);
struct net_device *rmnet_qmap_get_dev(u8 mux_id);

#define QMAP_DFC_CONFIG		10
#define QMAP_DFC_IND		11
#define QMAP_DFC_QUERY		12
#define QMAP_DFC_END_MARKER	13
#define QMAP_DFC_POWERSAVE	14
int dfc_qmap_cmd_handler(struct sk_buff *skb);

#define QMAP_LL_SWITCH		25
#define QMAP_LL_SWITCH_STATUS	26
int ll_qmap_cmd_handler(struct sk_buff *skb);

#endif /* __RMNET_QMAP_H */
