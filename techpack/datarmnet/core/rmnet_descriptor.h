/* Copyright (c) 2013-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * RMNET Packet Descriptor Framework
 *
 */

#ifndef _RMNET_DESCRIPTOR_H_
#define _RMNET_DESCRIPTOR_H_

#include <linux/netdevice.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include "rmnet_config.h"
#include "rmnet_map.h"

struct rmnet_frag_descriptor_pool {
	struct list_head free_list;
	u32 pool_size;
};

struct rmnet_fragment {
	struct list_head list;
	skb_frag_t frag;
};

struct rmnet_frag_descriptor {
	struct list_head list;
	struct list_head frags;
	struct net_device *dev;
	u32 coal_bufsize;
	u32 coal_bytes;
	u32 len;
	u32 hash;
	u32 priority;
	__be32 tcp_seq;
	__be16 ip_id;
	__be16 tcp_flags;
	u16 data_offset;
	u16 gso_size;
	u16 gso_segs;
	u16 ip_len;
	u16 trans_len;
	u8 ip_proto;
	u8 trans_proto;
	u8 pkt_id;
	u8 csum_valid:1,
	   hdrs_valid:1,
	   ip_id_set:1,
	   tcp_seq_set:1,
	   flush_shs:1,
	   tcp_flags_set:1,
	   reserved:2;
};

/* Descriptor management */
struct rmnet_frag_descriptor *
rmnet_get_frag_descriptor(struct rmnet_port *port);
void rmnet_recycle_frag_descriptor(struct rmnet_frag_descriptor *frag_desc,
				   struct rmnet_port *port);
void *rmnet_frag_pull(struct rmnet_frag_descriptor *frag_desc,
		      struct rmnet_port *port, unsigned int size);
void *rmnet_frag_trim(struct rmnet_frag_descriptor *frag_desc,
		      struct rmnet_port *port, unsigned int size);
void *rmnet_frag_header_ptr(struct rmnet_frag_descriptor *frag_desc, u32 off,
			    u32 len, void *buf);
int rmnet_frag_descriptor_add_frag(struct rmnet_frag_descriptor *frag_desc,
				   struct page *p, u32 page_offset, u32 len);
int rmnet_frag_descriptor_add_frags_from(struct rmnet_frag_descriptor *to,
					 struct rmnet_frag_descriptor *from,
					 u32 off, u32 len);
int rmnet_frag_ipv6_skip_exthdr(struct rmnet_frag_descriptor *frag_desc,
				int start, u8 *nexthdrp, __be16 *frag_offp,
				bool *frag_hdrp);

/* QMAP command packets */
void rmnet_frag_command(struct rmnet_frag_descriptor *frag_desc,
			struct rmnet_map_header *qmap, struct rmnet_port *port);
int rmnet_frag_flow_command(struct rmnet_frag_descriptor *frag_desc,
			    struct rmnet_port *port, u16 pkt_len);

/* Ingress data handlers */
void rmnet_frag_deaggregate(struct sk_buff *skb, struct rmnet_port *port,
			    struct list_head *list, u32 priority);
void rmnet_frag_deliver(struct rmnet_frag_descriptor *frag_desc,
			struct rmnet_port *port);
int rmnet_frag_process_next_hdr_packet(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port,
				       struct list_head *list,
				       u16 len);
void rmnet_frag_ingress_handler(struct sk_buff *skb,
				struct rmnet_port *port);

int rmnet_descriptor_init(struct rmnet_port *port);
void rmnet_descriptor_deinit(struct rmnet_port *port);

static inline void *rmnet_frag_data_ptr(struct rmnet_frag_descriptor *frag_desc)
{
	struct rmnet_fragment *frag;

	frag = list_first_entry_or_null(&frag_desc->frags,
					struct rmnet_fragment, list);

	if (!frag)
		return NULL;

	return skb_frag_address(&frag->frag);
}

#endif /* _RMNET_DESCRIPTOR_H_ */
