/* Copyright (c) 2013-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/inet.h>
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#include "rmnet_config.h"
#include "rmnet_descriptor.h"
#include "rmnet_handlers.h"
#include "rmnet_private.h"
#include "rmnet_vnd.h"
#include "rmnet_qmi.h"
#include "rmnet_trace.h"
#include "qmi_rmnet.h"

#define RMNET_FRAG_DESCRIPTOR_POOL_SIZE 64
#define RMNET_DL_IND_HDR_SIZE (sizeof(struct rmnet_map_dl_ind_hdr) + \
			       sizeof(struct rmnet_map_header) + \
			       sizeof(struct rmnet_map_control_command_header))
#define RMNET_DL_IND_TRL_SIZE (sizeof(struct rmnet_map_dl_ind_trl) + \
			       sizeof(struct rmnet_map_header) + \
			       sizeof(struct rmnet_map_control_command_header))

#define rmnet_descriptor_for_each_frag(p, desc) \
	list_for_each_entry(p, &desc->frags, list)
#define rmnet_descriptor_for_each_frag_safe(p, tmp, desc) \
	list_for_each_entry_safe(p, tmp, &desc->frags, list)
#define rmnet_descriptor_for_each_frag_safe_reverse(p, tmp, desc) \
	list_for_each_entry_safe_reverse(p, tmp, &desc->frags, list)

typedef void (*rmnet_perf_desc_hook_t)(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port);
typedef void (*rmnet_perf_chain_hook_t)(void);

typedef void (*rmnet_perf_tether_ingress_hook_t)(struct tcphdr *tp, struct sk_buff *skb);
rmnet_perf_tether_ingress_hook_t rmnet_perf_tether_ingress_hook __rcu __read_mostly;
EXPORT_SYMBOL(rmnet_perf_tether_ingress_hook);

struct rmnet_frag_descriptor *
rmnet_get_frag_descriptor(struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool = port->frag_desc_pool;
	struct rmnet_frag_descriptor *frag_desc;
	unsigned long flags;

	spin_lock_irqsave(&port->desc_pool_lock, flags);
	if (!list_empty(&pool->free_list)) {
		frag_desc = list_first_entry(&pool->free_list,
					     struct rmnet_frag_descriptor,
					     list);
		list_del_init(&frag_desc->list);
	} else {
		frag_desc = kzalloc(sizeof(*frag_desc), GFP_ATOMIC);
		if (!frag_desc)
			goto out;

		INIT_LIST_HEAD(&frag_desc->list);
		INIT_LIST_HEAD(&frag_desc->frags);
		pool->pool_size++;
	}

out:
	spin_unlock_irqrestore(&port->desc_pool_lock, flags);
	return frag_desc;
}
EXPORT_SYMBOL(rmnet_get_frag_descriptor);

void rmnet_recycle_frag_descriptor(struct rmnet_frag_descriptor *frag_desc,
				   struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool = port->frag_desc_pool;
	struct rmnet_fragment *frag, *tmp;
	unsigned long flags;

	list_del(&frag_desc->list);

	rmnet_descriptor_for_each_frag_safe(frag, tmp, frag_desc) {
		struct page *page = skb_frag_page(&frag->frag);

		if (page)
			put_page(page);

		list_del(&frag->list);
		kfree(frag);
	}

	memset(frag_desc, 0, sizeof(*frag_desc));
	INIT_LIST_HEAD(&frag_desc->list);
	INIT_LIST_HEAD(&frag_desc->frags);
	spin_lock_irqsave(&port->desc_pool_lock, flags);
	list_add_tail(&frag_desc->list, &pool->free_list);
	spin_unlock_irqrestore(&port->desc_pool_lock, flags);
}
EXPORT_SYMBOL(rmnet_recycle_frag_descriptor);

void *rmnet_frag_pull(struct rmnet_frag_descriptor *frag_desc,
		      struct rmnet_port *port, unsigned int size)
{
	struct rmnet_fragment *frag, *tmp;

	if (size >= frag_desc->len) {
		pr_info("%s(): Pulling %u bytes from %u byte pkt. Dropping\n",
			__func__, size, frag_desc->len);
		rmnet_recycle_frag_descriptor(frag_desc, port);
		return NULL;
	}

	rmnet_descriptor_for_each_frag_safe(frag, tmp, frag_desc) {
		u32 frag_size = skb_frag_size(&frag->frag);

		if (!size)
			break;

		if (size >= frag_size) {
			/* Remove the whole frag */
			struct page *page = skb_frag_page(&frag->frag);

			if (page)
				put_page(page);

			list_del(&frag->list);
			size -= frag_size;
			frag_desc->len -= frag_size;
			kfree(frag);
			continue;
		}

		/* Pull off 'size' bytes */
		skb_frag_off_add(&frag->frag, size);
		skb_frag_size_sub(&frag->frag, size);
		frag_desc->len -= size;
		break;
	}

	return rmnet_frag_data_ptr(frag_desc);
}
EXPORT_SYMBOL(rmnet_frag_pull);

void *rmnet_frag_trim(struct rmnet_frag_descriptor *frag_desc,
		      struct rmnet_port *port, unsigned int size)
{
	struct rmnet_fragment *frag, *tmp;
	unsigned int eat;

	if (!size) {
		pr_info("%s(): Trimming %u byte pkt to 0. Dropping\n",
			__func__, frag_desc->len);
		rmnet_recycle_frag_descriptor(frag_desc, port);
		return NULL;
	}

	/* Growing bigger doesn't make sense */
	if (size >= frag_desc->len)
		goto out;

	/* Compute number of bytes to remove from the end */
	eat = frag_desc->len - size;
	rmnet_descriptor_for_each_frag_safe_reverse(frag, tmp, frag_desc) {
		u32 frag_size = skb_frag_size(&frag->frag);

		if (!eat)
			goto out;

		if (eat >= frag_size) {
			/* Remove the whole frag */
			struct page *page = skb_frag_page(&frag->frag);

			if (page)
				put_page(page);

			list_del(&frag->list);
			eat -= frag_size;
			frag_desc->len -= frag_size;
			kfree(frag);
			continue;
		}

		/* Chop off 'eat' bytes from the end */
		skb_frag_size_sub(&frag->frag, eat);
		frag_desc->len -= eat;
		goto out;
	}

out:
	return rmnet_frag_data_ptr(frag_desc);
}
EXPORT_SYMBOL(rmnet_frag_trim);

static int rmnet_frag_copy_data(struct rmnet_frag_descriptor *frag_desc,
				u32 off, u32 len, void *buf)
{
	struct rmnet_fragment *frag;
	u32 frag_size, copy_len;
	u32 buf_offset = 0;

	/* Don't make me do something we'd both regret */
	if (off > frag_desc->len || len > frag_desc->len ||
	    off + len > frag_desc->len)
		return -EINVAL;

	/* Copy 'len' bytes into the bufer starting from 'off' */
	rmnet_descriptor_for_each_frag(frag, frag_desc) {
		if (!len)
			break;

		frag_size = skb_frag_size(&frag->frag);
		if (off < frag_size) {
			copy_len = min_t(u32, len, frag_size - off);
			memcpy(buf + buf_offset,
			       skb_frag_address(&frag->frag) + off,
			       copy_len);
			buf_offset += copy_len;
			len -= copy_len;
			off = 0;
		} else {
			off -= frag_size;
		}
	}

	return 0;
}

void *rmnet_frag_header_ptr(struct rmnet_frag_descriptor *frag_desc, u32 off,
			    u32 len, void *buf)
{
	struct rmnet_fragment *frag;
	u8 *start;
	u32 frag_size, offset;

	/* Don't take a long pointer off a short frag */
	if (off > frag_desc->len || len > frag_desc->len ||
	    off + len > frag_desc->len)
		return NULL;

	/* Find the starting fragment */
	offset = off;
	rmnet_descriptor_for_each_frag(frag, frag_desc) {
		frag_size = skb_frag_size(&frag->frag);
		if (off < frag_size) {
			start = skb_frag_address(&frag->frag) + off;
			/* If the header is entirely on this frag, just return
			 * a pointer to it.
			 */
			if (off + len <= frag_size)
				return start;

			/* Otherwise, we need to copy the data into a linear
			 * buffer.
			 */
			break;
		}

		off -= frag_size;
	}

	if (rmnet_frag_copy_data(frag_desc, offset, len, buf) < 0)
		return NULL;

	return buf;
}
EXPORT_SYMBOL(rmnet_frag_header_ptr);

int rmnet_frag_descriptor_add_frag(struct rmnet_frag_descriptor *frag_desc,
				   struct page *p, u32 page_offset, u32 len)
{
	struct rmnet_fragment *frag;

	frag = kzalloc(sizeof(*frag), GFP_ATOMIC);
	if (!frag)
		return -ENOMEM;

	INIT_LIST_HEAD(&frag->list);
	get_page(p);
	__skb_frag_set_page(&frag->frag, p);
	skb_frag_size_set(&frag->frag, len);
	skb_frag_off_set(&frag->frag, page_offset);
	list_add_tail(&frag->list, &frag_desc->frags);
	frag_desc->len += len;
	return 0;
}
EXPORT_SYMBOL(rmnet_frag_descriptor_add_frag);

int rmnet_frag_descriptor_add_frags_from(struct rmnet_frag_descriptor *to,
					 struct rmnet_frag_descriptor *from,
					 u32 off, u32 len)
{
	struct rmnet_fragment *frag;
	int rc;

	/* Sanity check the lengths */
	if (off > from->len || len > from->len || off + len > from->len)
		return -EINVAL;

	rmnet_descriptor_for_each_frag(frag, from) {
		u32 frag_size;

		if (!len)
			break;

		frag_size = skb_frag_size(&frag->frag);
		if (off < frag_size) {
			struct page *p = skb_frag_page(&frag->frag);
			u32 page_off = skb_frag_off(&frag->frag);
			u32 copy_len = min_t(u32, len, frag_size - off);

			rc = rmnet_frag_descriptor_add_frag(to, p,
							    page_off + off,
							    copy_len);
			if (rc < 0)
				return rc;

			len -= copy_len;
			off = 0;
		} else {
			off -= frag_size;
		}
	}

	return 0;
}
EXPORT_SYMBOL(rmnet_frag_descriptor_add_frags_from);

int rmnet_frag_ipv6_skip_exthdr(struct rmnet_frag_descriptor *frag_desc,
				int start, u8 *nexthdrp, __be16 *frag_offp,
				bool *frag_hdrp)
{
	u8 nexthdr = *nexthdrp;

	*frag_offp = 0;
	*frag_hdrp = false;
	while (ipv6_ext_hdr(nexthdr)) {
		struct ipv6_opt_hdr *hp, __hp;
		int hdrlen;

		if (nexthdr == NEXTHDR_NONE)
			return -EINVAL;

		hp = rmnet_frag_header_ptr(frag_desc, (u32)start, sizeof(*hp),
					   &__hp);
		if (!hp)
			return -EINVAL;

		if (nexthdr == NEXTHDR_FRAGMENT) {
			u32 off = offsetof(struct frag_hdr, frag_off);
			__be16 *fp, __fp;

			fp = rmnet_frag_header_ptr(frag_desc, (u32)start + off,
						   sizeof(*fp), &__fp);
			if (!fp)
				return -EINVAL;

			*frag_offp = *fp;
			*frag_hdrp = true;
			if (ntohs(*frag_offp) & ~0x7)
				break;
			hdrlen = 8;
		} else if (nexthdr == NEXTHDR_AUTH) {
			hdrlen = (hp->hdrlen + 2) << 2;
		} else {
			hdrlen = ipv6_optlen(hp);
		}

		nexthdr = hp->nexthdr;
		start += hdrlen;
	}

	*nexthdrp = nexthdr;
	return start;
}
EXPORT_SYMBOL(rmnet_frag_ipv6_skip_exthdr);

static u8 rmnet_frag_do_flow_control(struct rmnet_map_header *qmap,
				     struct rmnet_map_control_command *cmd,
				     struct rmnet_port *port,
				     int enable)
{
	struct rmnet_endpoint *ep;
	struct net_device *vnd;
	u16 ip_family;
	u16 fc_seq;
	u32 qos_id;
	u8 mux_id;
	int r;

	mux_id = qmap->mux_id;
	if (mux_id >= RMNET_MAX_LOGICAL_EP)
		return RX_HANDLER_CONSUMED;

	ep = rmnet_get_endpoint(port, mux_id);
	if (!ep)
		return RX_HANDLER_CONSUMED;

	vnd = ep->egress_dev;

	ip_family = cmd->flow_control.ip_family;
	fc_seq = ntohs(cmd->flow_control.flow_control_seq_num);
	qos_id = ntohl(cmd->flow_control.qos_id);

	/* Ignore the ip family and pass the sequence number for both v4 and v6
	 * sequence. User space does not support creating dedicated flows for
	 * the 2 protocols
	 */
	r = rmnet_vnd_do_flow_control(vnd, enable);
	if (r)
		return RMNET_MAP_COMMAND_UNSUPPORTED;
	else
		return RMNET_MAP_COMMAND_ACK;
}

static void rmnet_frag_send_ack(struct rmnet_map_header *qmap,
				unsigned char type,
				struct rmnet_port *port)
{
	struct rmnet_map_control_command *cmd;
	struct net_device *dev = port->dev;
	struct sk_buff *skb;
	u16 alloc_len = ntohs(qmap->pkt_len) + sizeof(*qmap);

	skb = alloc_skb(alloc_len, GFP_ATOMIC);
	if (!skb)
		return;

	skb->protocol = htons(ETH_P_MAP);
	skb->dev = dev;

	cmd = rmnet_map_get_cmd_start(skb);
	cmd->cmd_type = type & 0x03;

	netif_tx_lock(dev);
	dev->netdev_ops->ndo_start_xmit(skb, dev);
	netif_tx_unlock(dev);
}

static void
rmnet_frag_process_flow_start(struct rmnet_frag_descriptor *frag_desc,
			      struct rmnet_map_control_command_header *cmd,
			      struct rmnet_port *port,
			      u16 cmd_len)
{
	struct rmnet_map_dl_ind_hdr *dlhdr, __dlhdr;
	u32 offset = sizeof(struct rmnet_map_header);
	u32 data_format;
	bool is_dl_mark_v2;

	if (cmd_len + offset < RMNET_DL_IND_HDR_SIZE)
		return;

	data_format = port->data_format;
	is_dl_mark_v2 = data_format & RMNET_INGRESS_FORMAT_DL_MARKER_V2;
	dlhdr = rmnet_frag_header_ptr(frag_desc, offset + sizeof(*cmd),
				      sizeof(*dlhdr), &__dlhdr);
	if (!dlhdr)
		return;

	port->stats.dl_hdr_last_ep_id = cmd->source_id;
	port->stats.dl_hdr_last_qmap_vers = cmd->reserved;
	port->stats.dl_hdr_last_trans_id = cmd->transaction_id;
	port->stats.dl_hdr_last_seq = dlhdr->le.seq;
	port->stats.dl_hdr_last_bytes = dlhdr->le.bytes;
	port->stats.dl_hdr_last_pkts = dlhdr->le.pkts;
	port->stats.dl_hdr_last_flows = dlhdr->le.flows;
	port->stats.dl_hdr_total_bytes += port->stats.dl_hdr_last_bytes;
	port->stats.dl_hdr_total_pkts += port->stats.dl_hdr_last_pkts;
	port->stats.dl_hdr_count++;

	/* If a target is taking frag path, we can assume DL marker v2 is in
	 * play
	 */
	if (is_dl_mark_v2)
		rmnet_map_dl_hdr_notify_v2(port, dlhdr, cmd);
}

static void
rmnet_frag_process_flow_end(struct rmnet_frag_descriptor *frag_desc,
			    struct rmnet_map_control_command_header *cmd,
			    struct rmnet_port *port, u16 cmd_len)
{
	struct rmnet_map_dl_ind_trl *dltrl, __dltrl;
	u32 offset = sizeof(struct rmnet_map_header);
	u32 data_format;
	bool is_dl_mark_v2;


	if (cmd_len + offset < RMNET_DL_IND_TRL_SIZE)
		return;

	data_format = port->data_format;
	is_dl_mark_v2 = data_format & RMNET_INGRESS_FORMAT_DL_MARKER_V2;
	dltrl = rmnet_frag_header_ptr(frag_desc, offset + sizeof(*cmd),
				      sizeof(*dltrl), &__dltrl);
	if (!dltrl)
		return;

	port->stats.dl_trl_last_seq = dltrl->seq_le;
	port->stats.dl_trl_count++;

	/* If a target is taking frag path, we can assume DL marker v2 is in
	 * play
	 */
	if (is_dl_mark_v2)
		rmnet_map_dl_trl_notify_v2(port, dltrl, cmd);
}

/* Process MAP command frame and send N/ACK message as appropriate. Message cmd
 * name is decoded here and appropriate handler is called.
 */
void rmnet_frag_command(struct rmnet_frag_descriptor *frag_desc,
			struct rmnet_map_header *qmap, struct rmnet_port *port)
{
	struct rmnet_map_control_command *cmd, __cmd;
	unsigned char rc = 0;

	cmd = rmnet_frag_header_ptr(frag_desc, sizeof(*qmap), sizeof(*cmd),
				    &__cmd);
	if (!cmd)
		return;

	switch (cmd->command_name) {
	case RMNET_MAP_COMMAND_FLOW_ENABLE:
		rc = rmnet_frag_do_flow_control(qmap, cmd, port, 1);
		break;

	case RMNET_MAP_COMMAND_FLOW_DISABLE:
		rc = rmnet_frag_do_flow_control(qmap, cmd, port, 0);
		break;

	default:
		rc = RMNET_MAP_COMMAND_UNSUPPORTED;
		break;
	}
	if (rc == RMNET_MAP_COMMAND_ACK)
		rmnet_frag_send_ack(qmap, rc, port);
}

int rmnet_frag_flow_command(struct rmnet_frag_descriptor *frag_desc,
			    struct rmnet_port *port, u16 pkt_len)
{
	struct rmnet_map_control_command_header *cmd, __cmd;

	cmd = rmnet_frag_header_ptr(frag_desc, sizeof(struct rmnet_map_header),
				    sizeof(*cmd), &__cmd);
	if (!cmd)
		return -1;

	/* Silently discard any marksers recived over the LL channel */
	if (frag_desc->priority == 0xda1a &&
	    (cmd->command_name == RMNET_MAP_COMMAND_FLOW_START ||
	     cmd->command_name == RMNET_MAP_COMMAND_FLOW_END))
		return 0;

	switch (cmd->command_name) {
	case RMNET_MAP_COMMAND_FLOW_START:
		rmnet_frag_process_flow_start(frag_desc, cmd, port, pkt_len);
		break;

	case RMNET_MAP_COMMAND_FLOW_END:
		rmnet_frag_process_flow_end(frag_desc, cmd, port, pkt_len);
		break;

	default:
		return 1;
	}

	return 0;
}
EXPORT_SYMBOL(rmnet_frag_flow_command);

static int rmnet_frag_deaggregate_one(struct sk_buff *skb,
				      struct rmnet_port *port,
				      struct list_head *list,
				      u32 start, u32 priority)
{
	struct skb_shared_info *shinfo = skb_shinfo(skb);
	struct rmnet_frag_descriptor *frag_desc;
	struct rmnet_map_header *maph, __maph;
	skb_frag_t *frag;
	u32 start_frag, offset, i;
	u32 start_frag_size, start_frag_off;
	u32 pkt_len, copy_len = 0;
	int rc;

	for (start_frag = 0, offset = 0; start_frag < shinfo->nr_frags;
	     start_frag++) {
		frag = &shinfo->frags[start_frag];
		if (start < skb_frag_size(frag) + offset)
			break;

		offset += skb_frag_size(frag);
	}

	if (start_frag == shinfo->nr_frags)
		return -1;

	/* start - offset is the additional offset into the page to account
	 * for any data on it we've already used.
	 */
	start_frag_size = skb_frag_size(frag) - (start - offset);
	start_frag_off = skb_frag_off(frag) + (start - offset);

	/* Grab the QMAP header. Careful, as there's no guarantee that it's
	 * continugous!
	 */
	if (likely(start_frag_size >= sizeof(*maph))) {
		maph = skb_frag_address(frag) + (start - offset);
	} else {
		/* The header's split across pages. We can rebuild it.
		 * Probably not faster or stronger than before. But certainly
		 * more linear.
		 */
		if (skb_copy_bits(skb, start, &__maph, sizeof(__maph)) < 0)
			return -1;

		maph = &__maph;
	}

	pkt_len = ntohs(maph->pkt_len);
	/* Catch empty frames */
	if (!pkt_len)
		return -1;

	frag_desc = rmnet_get_frag_descriptor(port);
	if (!frag_desc)
		return -1;

	frag_desc->priority = priority;
	pkt_len += sizeof(*maph);
	if (port->data_format & RMNET_FLAGS_INGRESS_MAP_CKSUMV4) {
		pkt_len += sizeof(struct rmnet_map_dl_csum_trailer);
	} else if ((port->data_format & (RMNET_FLAGS_INGRESS_MAP_CKSUMV5 |
					 RMNET_FLAGS_INGRESS_COALESCE)) &&
		   !maph->cd_bit) {
		u32 hsize = 0;
		u8 type;

		/* Check the type. This seems like should be overkill for less
		 * than a single byte, doesn't it?
		 */
		if (likely(start_frag_size >= sizeof(*maph) + 1)) {
			type = *((u8 *)maph + sizeof(*maph));
		} else {
			if (skb_copy_bits(skb, start + sizeof(*maph), &type,
					  sizeof(type)) < 0)
				return -1;
		}

		/* Type only uses the first 7 bits */
		switch ((type & 0xFE) >> 1) {
		case RMNET_MAP_HEADER_TYPE_COALESCING:
			hsize = sizeof(struct rmnet_map_v5_coal_header);
			break;
		case RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD:
			hsize = sizeof(struct rmnet_map_v5_csum_header);
			break;
		}

		pkt_len += hsize;
	}

	/* Add all frags containing the packet data to the descriptor */
	for (i = start_frag; pkt_len > 0 && i < shinfo->nr_frags; ) {
		u32 size, off;
		u32 copy;

		frag = &shinfo->frags[i];
		size = skb_frag_size(frag);
		off = skb_frag_off(frag);
		if (i == start_frag) {
			/* These are different for the first one to account for
			 * the starting offset.
			 */
			size = start_frag_size;
			off = start_frag_off;
		}

		copy = min_t(u32, size, pkt_len);
		rc = rmnet_frag_descriptor_add_frag(frag_desc,
						    skb_frag_page(frag), off,
						    copy);
		if (rc < 0) {
			rmnet_recycle_frag_descriptor(frag_desc, port);
			return -1;
		}

		pkt_len -= copy;
		copy_len += copy;
		/* If the fragment is exhausted, we can move to the next one */
		if (!(size - copy_len)) {
			i++;
			copy_len = 0;
		}
	}

	if (pkt_len) {
		/* Packet length is larger than the amount of data we have */
		rmnet_recycle_frag_descriptor(frag_desc, port);
		return -1;
	}

	list_add_tail(&frag_desc->list, list);
	return (int)frag_desc->len;
}

void rmnet_frag_deaggregate(struct sk_buff *skb, struct rmnet_port *port,
			    struct list_head *list, u32 priority)
{
	u32 start = 0;
	int rc;

	while (start < skb->len) {
		rc = rmnet_frag_deaggregate_one(skb, port, list, start,
						priority);
		if (rc < 0)
			return;

		start += (u32)rc;
	}
}

/* Fill in GSO metadata to allow the SKB to be segmented by the NW stack
 * if needed (i.e. forwarding, UDP GRO)
 */
static void rmnet_frag_gso_stamp(struct sk_buff *skb,
				 struct rmnet_frag_descriptor *frag_desc)
{
	struct skb_shared_info *shinfo = skb_shinfo(skb);

	if (frag_desc->trans_proto == IPPROTO_TCP)
		shinfo->gso_type = (frag_desc->ip_proto == 4) ?
				   SKB_GSO_TCPV4 : SKB_GSO_TCPV6;
	else
		shinfo->gso_type = SKB_GSO_UDP_L4;

	shinfo->gso_size = frag_desc->gso_size;
	shinfo->gso_segs = frag_desc->gso_segs;
}

/* Set the partial checksum information. Sets the transport checksum to the
 * pseudoheader checksum and sets the offload metadata.
 */
static void rmnet_frag_partial_csum(struct sk_buff *skb,
				    struct rmnet_frag_descriptor *frag_desc)
{
	rmnet_perf_tether_ingress_hook_t rmnet_perf_tether_ingress;
	struct iphdr *iph = (struct iphdr *)skb->data;
	__sum16 pseudo;
	u16 pkt_len = skb->len - frag_desc->ip_len;

	if (frag_desc->ip_proto == 4) {
		iph->tot_len = htons(skb->len);
		iph->check = 0;
		iph->check = ip_fast_csum(iph, iph->ihl);
		pseudo = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
					    pkt_len, frag_desc->trans_proto,
					    0);
	} else {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)iph;

		/* Payload length includes any extension headers */
		ip6h->payload_len = htons(skb->len - sizeof(*ip6h));
		pseudo = ~csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
					  pkt_len, frag_desc->trans_proto, 0);
	}

	if (frag_desc->trans_proto == IPPROTO_TCP) {
		struct tcphdr *tp = (struct tcphdr *)
				    ((u8 *)iph + frag_desc->ip_len);

		tp->check = pseudo;
		skb->csum_offset = offsetof(struct tcphdr, check);

		rmnet_perf_tether_ingress = rcu_dereference(rmnet_perf_tether_ingress_hook);
		if (rmnet_perf_tether_ingress)
			rmnet_perf_tether_ingress(tp, skb);
	} else {
		struct udphdr *up = (struct udphdr *)
				    ((u8 *)iph + frag_desc->ip_len);

		up->len = htons(pkt_len);
		up->check = pseudo;
		skb->csum_offset = offsetof(struct udphdr, check);
	}

	skb->ip_summed = CHECKSUM_PARTIAL;
	skb->csum_start = (u8 *)iph + frag_desc->ip_len - skb->head;
}

/* Allocate and populate an skb to contain the packet represented by the
 * frag descriptor.
 */
static struct sk_buff *rmnet_alloc_skb(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port)
{
	struct sk_buff *head_skb, *current_skb, *skb;
	struct skb_shared_info *shinfo;
	struct rmnet_fragment *frag, *tmp;
	struct rmnet_skb_cb *cb;

	/* Use the exact sizes if we know them (i.e. RSB/RSC, rmnet_perf) */
	if (frag_desc->hdrs_valid) {
		u16 hdr_len = frag_desc->ip_len + frag_desc->trans_len;

		head_skb = alloc_skb(hdr_len + RMNET_MAP_DEAGGR_HEADROOM,
				     GFP_ATOMIC);
		if (!head_skb)
			return NULL;

		skb_reserve(head_skb, RMNET_MAP_DEAGGR_HEADROOM);
		rmnet_frag_copy_data(frag_desc, 0, hdr_len,
				     skb_put(head_skb, hdr_len));
		skb_reset_network_header(head_skb);
		if (frag_desc->trans_len)
			skb_set_transport_header(head_skb, frag_desc->ip_len);

		/* Pull the headers off carefully */
		if (hdr_len == frag_desc->len)
			/* Fast forward "header only" packets */
			goto skip_frags;

		if (!rmnet_frag_pull(frag_desc, port, hdr_len)) {
			kfree(head_skb);
			return NULL;
		}
	} else {
		/* Allocate enough space to avoid penalties in the stack
		 * from __pskb_pull_tail()
		 */
		head_skb = alloc_skb(256 + RMNET_MAP_DEAGGR_HEADROOM,
				     GFP_ATOMIC);
		if (!head_skb)
			return NULL;

		skb_reserve(head_skb, RMNET_MAP_DEAGGR_HEADROOM);
	}

	shinfo = skb_shinfo(head_skb);
	current_skb = head_skb;

	/* Add in the page fragments */
	rmnet_descriptor_for_each_frag_safe(frag, tmp, frag_desc) {
		struct page *p = skb_frag_page(&frag->frag);
		u32 frag_size = skb_frag_size(&frag->frag);

add_frag:
		if (shinfo->nr_frags < MAX_SKB_FRAGS) {
			get_page(p);
			skb_add_rx_frag(current_skb, shinfo->nr_frags, p,
					skb_frag_off(&frag->frag), frag_size,
					frag_size);
			if (current_skb != head_skb) {
				head_skb->len += frag_size;
				head_skb->data_len += frag_size;
			}
		} else {
			/* Alloc a new skb and try again */
			skb = alloc_skb(0, GFP_ATOMIC);
			if (!skb)
				break;

			if (current_skb == head_skb)
				shinfo->frag_list = skb;
			else
				current_skb->next = skb;

			current_skb = skb;
			shinfo = skb_shinfo(current_skb);
			goto add_frag;
		}
	}

skip_frags:
	head_skb->dev = frag_desc->dev;
	rmnet_set_skb_proto(head_skb);
	cb = RMNET_SKB_CB(head_skb);
	cb->coal_bytes = frag_desc->coal_bytes;
	cb->coal_bufsize = frag_desc->coal_bufsize;

	/* Handle any header metadata that needs to be updated after RSB/RSC
	 * segmentation
	 */
	if (frag_desc->ip_id_set) {
		struct iphdr *iph;

		iph = (struct iphdr *)rmnet_map_data_ptr(head_skb);
		csum_replace2(&iph->check, iph->id, frag_desc->ip_id);
		iph->id = frag_desc->ip_id;
	}

	if (frag_desc->tcp_seq_set) {
		struct tcphdr *th;

		th = (struct tcphdr *)
		     (rmnet_map_data_ptr(head_skb) + frag_desc->ip_len);
		th->seq = frag_desc->tcp_seq;
	}

	if (frag_desc->tcp_flags_set) {
		struct tcphdr *th;
		__be16 *flags;

		th = (struct tcphdr *)
		     (rmnet_map_data_ptr(head_skb) + frag_desc->ip_len);
		flags = (__be16 *)&tcp_flag_word(th);
		*flags = frag_desc->tcp_flags;
	}

	/* Handle csum offloading */
	if (frag_desc->csum_valid && frag_desc->hdrs_valid) {
		/* Set the partial checksum information */
		rmnet_frag_partial_csum(head_skb, frag_desc);
	} else if (frag_desc->csum_valid) {
		/* Non-RSB/RSC/perf packet. The current checksum is fine */
		head_skb->ip_summed = CHECKSUM_UNNECESSARY;
	} else if (frag_desc->hdrs_valid &&
		   (frag_desc->trans_proto == IPPROTO_TCP ||
		    frag_desc->trans_proto == IPPROTO_UDP)) {
		/* Unfortunately, we have to fake a bad checksum here, since
		 * the original bad value is lost by the hardware. The only
		 * reliable way to do it is to calculate the actual checksum
		 * and corrupt it.
		 */
		__sum16 *check;
		__wsum csum;
		unsigned int offset = skb_transport_offset(head_skb);
		__sum16 pseudo;

		/* Calculate pseudo header and update header fields */
		if (frag_desc->ip_proto == 4) {
			struct iphdr *iph = ip_hdr(head_skb);
			__be16 tot_len = htons(head_skb->len);

			csum_replace2(&iph->check, iph->tot_len, tot_len);
			iph->tot_len = tot_len;
			pseudo = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
						    head_skb->len -
						    frag_desc->ip_len,
						    frag_desc->trans_proto, 0);
		} else {
			struct ipv6hdr *ip6h = ipv6_hdr(head_skb);

			ip6h->payload_len = htons(head_skb->len -
						  sizeof(*ip6h));
			pseudo = ~csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
						  head_skb->len -
						  frag_desc->ip_len,
						  frag_desc->trans_proto, 0);
		}

		if (frag_desc->trans_proto == IPPROTO_TCP) {
			check = &tcp_hdr(head_skb)->check;
		} else {
			udp_hdr(head_skb)->len = htons(head_skb->len -
						       frag_desc->ip_len);
			check = &udp_hdr(head_skb)->check;
		}

		*check = pseudo;
		csum = skb_checksum(head_skb, offset, head_skb->len - offset,
				    0);
		/* Add 1 to corrupt. This cannot produce a final value of 0
		 * since csum_fold() can't return a value of 0xFFFF
		 */
		*check = csum16_add(csum_fold(csum), htons(1));
		head_skb->ip_summed = CHECKSUM_NONE;
	}

	/* Handle any rmnet_perf metadata */
	if (frag_desc->hash) {
		head_skb->hash = frag_desc->hash;
		head_skb->sw_hash = 1;
	}

	if (frag_desc->flush_shs)
		cb->flush_shs = 1;

	/* Handle coalesced packets */
	if (frag_desc->gso_segs > 1)
		rmnet_frag_gso_stamp(head_skb, frag_desc);

	/* Propagate original priority value */
	head_skb->priority = frag_desc->priority;

	if (trace_print_tcp_rx_enabled()) {
		char saddr[INET6_ADDRSTRLEN], daddr[INET6_ADDRSTRLEN];

		if (!frag_desc->hdrs_valid && !frag_desc->trans_len)
			goto skip_trace_print_tcp_rx;

		memset(saddr, 0, INET6_ADDRSTRLEN);
		memset(daddr, 0, INET6_ADDRSTRLEN);

		if (head_skb->protocol == htons(ETH_P_IP)) {
			if (ip_hdr(head_skb)->protocol != IPPROTO_TCP)
				goto skip_trace_print_tcp_rx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(head_skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(head_skb)->daddr);
		}

		if (head_skb->protocol == htons(ETH_P_IPV6)) {
			if (ipv6_hdr(head_skb)->nexthdr != IPPROTO_TCP)
				goto skip_trace_print_tcp_rx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(head_skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(head_skb)->daddr);
		}

		trace_print_tcp_rx(head_skb, saddr, daddr, tcp_hdr(head_skb));
	}
skip_trace_print_tcp_rx:

	if (trace_print_udp_rx_enabled()) {
		char saddr[INET6_ADDRSTRLEN], daddr[INET6_ADDRSTRLEN];

		if (!frag_desc->hdrs_valid && !frag_desc->trans_len)
			goto skip_trace_print_udp_rx;

		memset(saddr, 0, INET6_ADDRSTRLEN);
		memset(daddr, 0, INET6_ADDRSTRLEN);

		if (head_skb->protocol == htons(ETH_P_IP)) {
			if (ip_hdr(head_skb)->protocol != IPPROTO_UDP)
				goto skip_trace_print_udp_rx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(head_skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(head_skb)->daddr);
		}

		if (head_skb->protocol == htons(ETH_P_IPV6)) {
			if (ipv6_hdr(head_skb)->nexthdr != IPPROTO_UDP)
				goto skip_trace_print_udp_rx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(head_skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(head_skb)->daddr);
		}

		trace_print_udp_rx(head_skb, saddr, daddr, udp_hdr(head_skb));
	}
skip_trace_print_udp_rx:

	return head_skb;
}

/* Deliver the packets contained within a frag descriptor */
void rmnet_frag_deliver(struct rmnet_frag_descriptor *frag_desc,
			struct rmnet_port *port)
{
	struct sk_buff *skb;

	skb = rmnet_alloc_skb(frag_desc, port);
	if (skb)
		rmnet_deliver_skb(skb, port);
	rmnet_recycle_frag_descriptor(frag_desc, port);
}
EXPORT_SYMBOL(rmnet_frag_deliver);

static void __rmnet_frag_segment_data(struct rmnet_frag_descriptor *coal_desc,
				      struct rmnet_port *port,
				      struct list_head *list, u8 pkt_id,
				      bool csum_valid)
{
	struct rmnet_priv *priv = netdev_priv(coal_desc->dev);
	struct rmnet_frag_descriptor *new_desc;
	u32 dlen = coal_desc->gso_size * coal_desc->gso_segs;
	u32 hlen = coal_desc->ip_len + coal_desc->trans_len;
	u32 offset = hlen + coal_desc->data_offset;
	int rc;

	new_desc = rmnet_get_frag_descriptor(port);
	if (!new_desc)
		return;

	/* Header information and most metadata is the same as the original */
	memcpy(new_desc, coal_desc, sizeof(*coal_desc));
	INIT_LIST_HEAD(&new_desc->list);
	INIT_LIST_HEAD(&new_desc->frags);
	new_desc->len = 0;

	/* Add the header fragments */
	rc = rmnet_frag_descriptor_add_frags_from(new_desc, coal_desc, 0,
						  hlen);
	if (rc < 0)
		goto recycle;

	/* Add in the data fragments */
	rc = rmnet_frag_descriptor_add_frags_from(new_desc, coal_desc, offset,
						  dlen);
	if (rc < 0)
		goto recycle;

	/* Update protocol-specific metadata */
	if (coal_desc->trans_proto == IPPROTO_TCP) {
		struct tcphdr *th, __th;

		th = rmnet_frag_header_ptr(coal_desc, coal_desc->ip_len,
					   sizeof(*th), &__th);
		if (!th)
			goto recycle;

		new_desc->tcp_seq_set = 1;
		new_desc->tcp_seq = htonl(ntohl(th->seq) +
					  coal_desc->data_offset);

		/* Don't allow any dangerous flags to appear in any segments
		 * other than the last.
		 */
		if (th->fin || th->psh) {
			if (offset + dlen < coal_desc->len) {
				__be32 flag_word = tcp_flag_word(th);

				/* Clear the FIN and PSH flags from this
				 * segment.
				 */
				flag_word &= ~TCP_FLAG_FIN;
				flag_word &= ~TCP_FLAG_PSH;

				new_desc->tcp_flags_set = 1;
				new_desc->tcp_flags = *((__be16 *)&flag_word);
			}
		}
	} else if (coal_desc->trans_proto == IPPROTO_UDP) {
		struct udphdr *uh, __uh;

		uh = rmnet_frag_header_ptr(coal_desc, coal_desc->ip_len,
					   sizeof(*uh), &__uh);
		if (!uh)
			goto recycle;

		if (coal_desc->ip_proto == 4 && !uh->check)
			csum_valid = true;
	}

	if (coal_desc->ip_proto == 4) {
		struct iphdr *iph, __iph;

		iph = rmnet_frag_header_ptr(coal_desc, 0, sizeof(*iph),
					    &__iph);
		if (!iph)
			goto recycle;

		new_desc->ip_id_set = 1;
		new_desc->ip_id = htons(ntohs(iph->id) + coal_desc->pkt_id);
	}

	new_desc->csum_valid = csum_valid;
	priv->stats.coal.coal_reconstruct++;

	/* Update meta information to move past the data we just segmented */
	coal_desc->data_offset += dlen;
	coal_desc->pkt_id = pkt_id + 1;
	coal_desc->gso_segs = 0;

	/* Only relevant for the first segment to avoid overcoutning */
	coal_desc->coal_bytes = 0;
	coal_desc->coal_bufsize = 0;

	list_add_tail(&new_desc->list, list);
	return;

recycle:
	rmnet_recycle_frag_descriptor(new_desc, port);
}

static bool rmnet_frag_validate_csum(struct rmnet_frag_descriptor *frag_desc)
{
	u8 *data = rmnet_frag_data_ptr(frag_desc);
	unsigned int datagram_len;
	__wsum csum;
	__sum16 pseudo;

	/* Keep analysis tools happy, since they will see that
	 * rmnet_frag_data_ptr() could return NULL. It can't in this case,
	 * since we can't get this far otherwise...
	 */
	if (unlikely(!data))
		return false;

	datagram_len = frag_desc->len - frag_desc->ip_len;
	if (frag_desc->ip_proto == 4) {
		struct iphdr *iph = (struct iphdr *)data;

		pseudo = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
					    datagram_len,
					    frag_desc->trans_proto, 0);
	} else {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)data;

		pseudo = ~csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
					  datagram_len, frag_desc->trans_proto,
					  0);
	}

	csum = csum_partial(data + frag_desc->ip_len, datagram_len,
			    csum_unfold(pseudo));
	return !csum_fold(csum);
}

/* Converts the coalesced frame into a list of descriptors */
static void
rmnet_frag_segment_coal_data(struct rmnet_frag_descriptor *coal_desc,
			     u64 nlo_err_mask, struct rmnet_port *port,
			     struct list_head *list)
{
	struct rmnet_priv *priv = netdev_priv(coal_desc->dev);
	struct rmnet_map_v5_coal_header coal_hdr;
	struct rmnet_fragment *frag;
	u8 *version;
	u16 pkt_len;
	u8 pkt, total_pkt = 0;
	u8 nlo;
	bool gro = coal_desc->dev->features & NETIF_F_GRO_HW;
	bool zero_csum = false;

	/* Copy the coal header into our local storage before pulling it. It's
	 * possible that this header (or part of it) is the last port of a page
	 * a pulling it off would cause it to be freed. Referring back to the
	 * header would be invalid in that case.
	 */
	if (rmnet_frag_copy_data(coal_desc, sizeof(struct rmnet_map_header),
				 sizeof(coal_hdr), &coal_hdr) < 0)
		return;

	/* Pull off the headers we no longer need */
	if (!rmnet_frag_pull(coal_desc, port, sizeof(struct rmnet_map_header) +
					      sizeof(coal_hdr)))
		return;

	/* By definition, this byte is linear, and the first byte on the
	 * first fragment. ;) Hence why no header_ptr() call is needed
	 * for it.
	*/
	version = rmnet_frag_data_ptr(coal_desc);
	if (unlikely(!version))
		return;

	if ((*version & 0xF0) == 0x40) {
		struct iphdr *iph, __iph;

		iph = rmnet_frag_header_ptr(coal_desc, 0, sizeof(*iph),
					    &__iph);
		if (!iph)
			return;

		coal_desc->ip_proto = 4;
		coal_desc->ip_len = iph->ihl * 4;
		coal_desc->trans_proto = iph->protocol;

		/* Don't allow coalescing of any packets with IP options */
		if (iph->ihl != 5)
			gro = false;
	} else if ((*version & 0xF0) == 0x60) {
		struct ipv6hdr *ip6h, __ip6h;
		int ip_len;
		__be16 frag_off;
		bool frag_hdr;
		u8 protocol;

		ip6h = rmnet_frag_header_ptr(coal_desc, 0, sizeof(*ip6h),
					     &__ip6h);
		if (!ip6h)
			return;

		coal_desc->ip_proto = 6;
		protocol = ip6h->nexthdr;
		ip_len = rmnet_frag_ipv6_skip_exthdr(coal_desc,
						     sizeof(*ip6h),
						     &protocol,
						     &frag_off,
						     &frag_hdr);
		coal_desc->trans_proto = protocol;

		/* If we run into a problem, or this is fragmented packet
		 * (which should technically not be possible, if the HW
		 * works as intended...), bail.
		 */
		if (ip_len < 0 || frag_off) {
			priv->stats.coal.coal_ip_invalid++;
			return;
		}

		if (frag_hdr) {
			/* There is a fragment header, but this is not a
			 * fragmented packet. We can handle this, but it
			 * cannot be coalesced because of kernel limitations.
			 */
			gro = false;
		}

		coal_desc->ip_len = (u16)ip_len;
		if (coal_desc->ip_len > sizeof(*ip6h)) {
			/* Don't allow coalescing of any packets with IPv6
			 * extension headers.
			 */
			gro = false;
		}
	} else {
		priv->stats.coal.coal_ip_invalid++;
		return;
	}

	if (coal_desc->trans_proto == IPPROTO_TCP) {
		struct tcphdr *th, __th;

		th = rmnet_frag_header_ptr(coal_desc,
					   coal_desc->ip_len, sizeof(*th),
					   &__th);
		if (!th)
			return;

		coal_desc->trans_len = th->doff * 4;
		priv->stats.coal.coal_tcp++;
		priv->stats.coal.coal_tcp_bytes += coal_desc->len;
	} else if (coal_desc->trans_proto == IPPROTO_UDP) {
		struct udphdr *uh, __uh;

		uh = rmnet_frag_header_ptr(coal_desc,
					   coal_desc->ip_len, sizeof(*uh),
					   &__uh);
		if (!uh)
			return;

		coal_desc->trans_len = sizeof(*uh);
		priv->stats.coal.coal_udp++;
		priv->stats.coal.coal_udp_bytes += coal_desc->len;
		if (coal_desc->ip_proto == 4 && !uh->check)
			zero_csum = true;
	} else {
		priv->stats.coal.coal_trans_invalid++;
		return;
	}

	coal_desc->hdrs_valid = 1;
	coal_desc->coal_bytes = coal_desc->len;
	rmnet_descriptor_for_each_frag(frag, coal_desc)
		coal_desc->coal_bufsize +=
			page_size(skb_frag_page(&frag->frag));

	if (rmnet_map_v5_csum_buggy(&coal_hdr) && !zero_csum) {
		/* Mark the checksum as valid if it checks out */
		if (rmnet_frag_validate_csum(coal_desc))
			coal_desc->csum_valid = true;

		coal_desc->gso_size = ntohs(coal_hdr.nl_pairs[0].pkt_len);
		coal_desc->gso_size -= coal_desc->ip_len + coal_desc->trans_len;
		coal_desc->gso_segs = coal_hdr.nl_pairs[0].num_packets;
		list_add_tail(&coal_desc->list, list);
		return;
	}

	/* Fast-forward the case where we have 1 NLO (i.e. 1 packet length),
	 * no checksum errors, and are allowing GRO. We can just reuse this
	 * descriptor unchanged.
	 */
	if (gro && coal_hdr.num_nlos == 1 && coal_hdr.csum_valid) {
		coal_desc->csum_valid = true;
		coal_desc->gso_size = ntohs(coal_hdr.nl_pairs[0].pkt_len);
		coal_desc->gso_size -= coal_desc->ip_len + coal_desc->trans_len;
		coal_desc->gso_segs = coal_hdr.nl_pairs[0].num_packets;
		list_add_tail(&coal_desc->list, list);
		return;
	}

	/* Segment the coalesced descriptor into new packets */
	for (nlo = 0; nlo < coal_hdr.num_nlos; nlo++) {
		pkt_len = ntohs(coal_hdr.nl_pairs[nlo].pkt_len);
		pkt_len -= coal_desc->ip_len + coal_desc->trans_len;
		coal_desc->gso_size = pkt_len;
		for (pkt = 0; pkt < coal_hdr.nl_pairs[nlo].num_packets;
		     pkt++, total_pkt++, nlo_err_mask >>= 1) {
			bool csum_err = nlo_err_mask & 1;

			/* Segment the packet if we're not sending the larger
			 * packet up the stack.
			 */
			if (!gro) {
				coal_desc->gso_segs = 1;
				if (csum_err)
					priv->stats.coal.coal_csum_err++;

				__rmnet_frag_segment_data(coal_desc, port,
							  list, total_pkt,
							  !csum_err);
				continue;
			}

			if (csum_err) {
				priv->stats.coal.coal_csum_err++;

				/* Segment out the good data */
				if (coal_desc->gso_segs)
					__rmnet_frag_segment_data(coal_desc,
								  port,
								  list,
								  total_pkt,
								  true);

				/* Segment out the bad checksum */
				coal_desc->gso_segs = 1;
				__rmnet_frag_segment_data(coal_desc, port,
							  list, total_pkt,
							  false);
			} else {
				coal_desc->gso_segs++;
			}
		}

		/* If we're switching NLOs, we need to send out everything from
		 * the previous one, if we haven't done so. NLOs only switch
		 * when the packet length changes.
		 */
		if (coal_desc->gso_segs)
			__rmnet_frag_segment_data(coal_desc, port, list,
						  total_pkt, true);
	}
}

/* Record reason for coalescing pipe closure */
static void rmnet_frag_data_log_close_stats(struct rmnet_priv *priv, u8 type,
					    u8 code)
{
	struct rmnet_coal_close_stats *stats = &priv->stats.coal.close;

	switch (type) {
	case RMNET_MAP_COAL_CLOSE_NON_COAL:
		stats->non_coal++;
		break;
	case RMNET_MAP_COAL_CLOSE_IP_MISS:
		stats->ip_miss++;
		break;
	case RMNET_MAP_COAL_CLOSE_TRANS_MISS:
		stats->trans_miss++;
		break;
	case RMNET_MAP_COAL_CLOSE_HW:
		switch (code) {
		case RMNET_MAP_COAL_CLOSE_HW_NL:
			stats->hw_nl++;
			break;
		case RMNET_MAP_COAL_CLOSE_HW_PKT:
			stats->hw_pkt++;
			break;
		case RMNET_MAP_COAL_CLOSE_HW_BYTE:
			stats->hw_byte++;
			break;
		case RMNET_MAP_COAL_CLOSE_HW_TIME:
			stats->hw_time++;
			break;
		case RMNET_MAP_COAL_CLOSE_HW_EVICT:
			stats->hw_evict++;
			break;
		default:
			break;
		}
		break;
	case RMNET_MAP_COAL_CLOSE_COAL:
		stats->coal++;
		break;
	default:
		break;
	}
}

/* Check if the coalesced header has any incorrect values, in which case, the
 * entire coalesced frame must be dropped. Then check if there are any
 * checksum issues
 */
static int
rmnet_frag_data_check_coal_header(struct rmnet_frag_descriptor *frag_desc,
				  u64 *nlo_err_mask)
{
	struct rmnet_map_v5_coal_header *coal_hdr, __coal_hdr;
	struct rmnet_priv *priv = netdev_priv(frag_desc->dev);
	u64 mask = 0;
	int i;
	u8 veid, pkts = 0;

	coal_hdr = rmnet_frag_header_ptr(frag_desc,
					 sizeof(struct rmnet_map_header),
					 sizeof(*coal_hdr), &__coal_hdr);
	if (!coal_hdr)
		return -EINVAL;

	veid = coal_hdr->virtual_channel_id;

	if (coal_hdr->num_nlos == 0 ||
	    coal_hdr->num_nlos > RMNET_MAP_V5_MAX_NLOS) {
		priv->stats.coal.coal_hdr_nlo_err++;
		return -EINVAL;
	}

	for (i = 0; i < RMNET_MAP_V5_MAX_NLOS; i++) {
		/* If there is a checksum issue, we need to split
		 * up the skb. Rebuild the full csum error field
		 */
		u8 err = coal_hdr->nl_pairs[i].csum_error_bitmap;
		u8 pkt = coal_hdr->nl_pairs[i].num_packets;

		mask |= ((u64)err) << (8 * i);

		/* Track total packets in frame */
		pkts += pkt;
		if (pkts > RMNET_MAP_V5_MAX_PACKETS) {
			priv->stats.coal.coal_hdr_pkt_err++;
			return -EINVAL;
		}
	}

	/* Track number of packets we get inside of coalesced frames */
	priv->stats.coal.coal_pkts += pkts;

	/* Update ethtool stats */
	rmnet_frag_data_log_close_stats(priv,
					coal_hdr->close_type,
					coal_hdr->close_value);
	if (veid < RMNET_MAX_VEID)
		priv->stats.coal.coal_veid[veid]++;

	*nlo_err_mask = mask;

	return 0;
}

static int rmnet_frag_checksum_pkt(struct rmnet_frag_descriptor *frag_desc)
{
	struct rmnet_priv *priv = netdev_priv(frag_desc->dev);
	struct rmnet_fragment *frag;
	int offset = sizeof(struct rmnet_map_header) +
		     sizeof(struct rmnet_map_v5_csum_header);
	u8 *version, __version;
	__wsum csum;
	u16 csum_len;

	version = rmnet_frag_header_ptr(frag_desc, offset, sizeof(*version),
					&__version);
	if (!version)
		return -EINVAL;

	if ((*version & 0xF0) == 0x40) {
		struct iphdr *iph;
		u8 __iph[60]; /* Max IP header size (0xF * 4) */

		/* We need to access the entire IP header including options
		 * to validate its checksum. Fortunately, the version byte
		 * also will tell us the length, so we only need to pull
		 * once ;)
		 */
		frag_desc->ip_len = (*version & 0xF) * 4;
		iph = rmnet_frag_header_ptr(frag_desc, offset,
					    frag_desc->ip_len,
					    __iph);
		if (!iph || ip_is_fragment(iph))
			return -EINVAL;

		/* Length needs to be sensible */
		csum_len = ntohs(iph->tot_len);
		if (csum_len > frag_desc->len - offset)
			return -EINVAL;

		csum_len -= frag_desc->ip_len;
		/* IPv4 checksum must be valid */
		if (ip_fast_csum((u8 *)iph, iph->ihl)) {
			priv->stats.csum_sw++;
			return 0;
		}

		frag_desc->ip_proto = 4;
		frag_desc->trans_proto = iph->protocol;
		csum = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
					  csum_len,
					  frag_desc->trans_proto, 0);
	} else if ((*version & 0xF0) == 0x60) {
		struct ipv6hdr *ip6h, __ip6h;
		int ip_len;
		__be16 frag_off;
		bool frag_hdr;
		u8 protocol;

		ip6h = rmnet_frag_header_ptr(frag_desc, offset, sizeof(*ip6h),
					     &__ip6h);
		if (!ip6h)
			return -EINVAL;

		frag_desc->ip_proto = 6;
		protocol = ip6h->nexthdr;
		ip_len = rmnet_frag_ipv6_skip_exthdr(frag_desc,
						     offset + sizeof(*ip6h),
						     &protocol, &frag_off,
						     &frag_hdr);
		if (ip_len < 0 || frag_off)
			return -EINVAL;

		/* Length needs to be sensible */
		frag_desc->ip_len = (u16)ip_len;
		csum_len = ntohs(ip6h->payload_len);
		if (csum_len + frag_desc->ip_len > frag_desc->len - offset)
			return -EINVAL;

		frag_desc->trans_proto = protocol;
		csum = ~csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
					csum_len,
					frag_desc->trans_proto, 0);
	} else {
		/* Not checksumable */
		return -EINVAL;
	}

	/* Protocol check */
	if (frag_desc->trans_proto != IPPROTO_TCP &&
	    frag_desc->trans_proto != IPPROTO_UDP)
		return -EINVAL;

	offset += frag_desc->ip_len;
	/* Check for UDP zero csum packets */
	if (frag_desc->trans_proto == IPPROTO_UDP) {
		struct udphdr *uh, __uh;

		uh = rmnet_frag_header_ptr(frag_desc, offset, sizeof(*uh),
					   &__uh);
		if (!uh)
			return -EINVAL;

		if (!uh->check) {
			if (frag_desc->ip_proto == 4) {
				/* Zero checksum is valid */
				priv->stats.csum_sw++;
				return 1;
			}

			/* Not valid in IPv6 */
			priv->stats.csum_sw++;
			return 0;
		}
	}

	/* Walk the frags and checksum each chunk */
	list_for_each_entry(frag, &frag_desc->frags, list) {
		u32 frag_size = skb_frag_size(&frag->frag);

		if (!csum_len)
			break;

		if (offset < frag_size) {
			void *addr = skb_frag_address(&frag->frag) + offset;
			u32 len = min_t(u32, csum_len, frag_size - offset);

			/* Checksum 'len' bytes and add them in */
			csum = csum_partial(addr, len, csum);
			csum_len -= len;
			offset = 0;
		} else {
			offset -= frag_size;
		}
	}

	priv->stats.csum_sw++;
	return !csum_fold(csum);
}

/* Process a QMAPv5 packet header */
int rmnet_frag_process_next_hdr_packet(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port,
				       struct list_head *list,
				       u16 len)
{
	struct rmnet_map_v5_csum_header *csum_hdr, __csum_hdr;
	struct rmnet_priv *priv = netdev_priv(frag_desc->dev);
	u64 nlo_err_mask;
	u32 offset = sizeof(struct rmnet_map_header);
	int rc = 0;

	/* Grab the header type. It's easier to grab enough for a full csum
	 * offload header here since it's only 8 bytes and then check the
	 * header type using that. This also doubles as a check to make sure
	 * there's enough data after the QMAP header to ensure that another
	 * header is present.
	 */
	csum_hdr = rmnet_frag_header_ptr(frag_desc, offset, sizeof(*csum_hdr),
					 &__csum_hdr);
	if (!csum_hdr)
		return -EINVAL;

	switch (csum_hdr->header_type) {
	case RMNET_MAP_HEADER_TYPE_COALESCING:
		priv->stats.coal.coal_rx++;
		rc = rmnet_frag_data_check_coal_header(frag_desc,
						       &nlo_err_mask);
		if (rc)
			return rc;

		rmnet_frag_segment_coal_data(frag_desc, nlo_err_mask, port,
					     list);
		if (list_first_entry(list, struct rmnet_frag_descriptor,
				     list) != frag_desc)
			rmnet_recycle_frag_descriptor(frag_desc, port);
		break;
	case RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD:
		if (unlikely(!(frag_desc->dev->features & NETIF_F_RXCSUM))) {
			priv->stats.csum_sw++;
		} else if (csum_hdr->csum_valid_required) {
			priv->stats.csum_ok++;
			frag_desc->csum_valid = true;
		} else {
			int valid = rmnet_frag_checksum_pkt(frag_desc);

			if (valid < 0) {
				priv->stats.csum_validation_failed++;
			} else if (valid) {
				/* All's good */
				priv->stats.csum_ok++;
				frag_desc->csum_valid = true;
			} else {
				/* Checksum is actually bad */
				priv->stats.csum_valid_unset++;
			}
		}

		if (!rmnet_frag_pull(frag_desc, port,
				     offset + sizeof(*csum_hdr))) {
			rc = -EINVAL;
			break;
		}

		/* Remove padding only for csum offload packets.
		 * Coalesced packets should never have padding.
		 */
		if (!rmnet_frag_trim(frag_desc, port, len)) {
			rc = -EINVAL;
			break;
		}

		list_del_init(&frag_desc->list);
		list_add_tail(&frag_desc->list, list);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

/* Perf hook handler */
rmnet_perf_desc_hook_t rmnet_perf_desc_entry __rcu __read_mostly;
EXPORT_SYMBOL(rmnet_perf_desc_entry);

static void
__rmnet_frag_ingress_handler(struct rmnet_frag_descriptor *frag_desc,
			     struct rmnet_port *port)
{
	rmnet_perf_desc_hook_t rmnet_perf_ingress;
	struct rmnet_map_header *qmap, __qmap;
	struct rmnet_endpoint *ep;
	struct rmnet_frag_descriptor *frag, *tmp;
	LIST_HEAD(segs);
	u16 len, pad;
	u8 mux_id;
	bool skip_perf = (frag_desc->priority == 0xda1a);

	qmap = rmnet_frag_header_ptr(frag_desc, 0, sizeof(*qmap), &__qmap);
	if (!qmap)
		goto recycle;

	mux_id = qmap->mux_id;
	pad = qmap->pad_len;
	len = ntohs(qmap->pkt_len) - pad;

	if (qmap->cd_bit) {
		qmi_rmnet_set_dl_msg_active(port);
		if (port->data_format & RMNET_INGRESS_FORMAT_DL_MARKER) {
			rmnet_frag_flow_command(frag_desc, port, len);
			goto recycle;
		}

		if (port->data_format & RMNET_FLAGS_INGRESS_MAP_COMMANDS)
			rmnet_frag_command(frag_desc, qmap, port);

		goto recycle;
	}

	if (mux_id >= RMNET_MAX_LOGICAL_EP)
		goto recycle;

	ep = rmnet_get_endpoint(port, mux_id);
	if (!ep)
		goto recycle;

	frag_desc->dev = ep->egress_dev;

	/* Handle QMAPv5 packet */
	if (qmap->next_hdr &&
	    (port->data_format & (RMNET_FLAGS_INGRESS_COALESCE |
				  RMNET_FLAGS_INGRESS_MAP_CKSUMV5))) {
		if (rmnet_frag_process_next_hdr_packet(frag_desc, port, &segs,
						       len))
			goto recycle;
	} else {
		/* We only have the main QMAP header to worry about */
		if (!rmnet_frag_pull(frag_desc, port, sizeof(*qmap)))
			return;

		if (!rmnet_frag_trim(frag_desc, port, len))
			return;

		list_add_tail(&frag_desc->list, &segs);
	}

	if (port->data_format & RMNET_INGRESS_FORMAT_PS)
		qmi_rmnet_work_maybe_restart(port);

	if (skip_perf)
		goto no_perf;

	rcu_read_lock();
	rmnet_perf_ingress = rcu_dereference(rmnet_perf_desc_entry);
	if (rmnet_perf_ingress) {
		list_for_each_entry_safe(frag, tmp, &segs, list) {
			list_del_init(&frag->list);
			rmnet_perf_ingress(frag, port);
		}
		rcu_read_unlock();
		return;
	}
	rcu_read_unlock();

no_perf:
	list_for_each_entry_safe(frag, tmp, &segs, list) {
		list_del_init(&frag->list);
		rmnet_frag_deliver(frag, port);
	}
	return;

recycle:
	rmnet_recycle_frag_descriptor(frag_desc, port);
}

/* Notify perf at the end of SKB chain */
rmnet_perf_chain_hook_t rmnet_perf_chain_end __rcu __read_mostly;
EXPORT_SYMBOL(rmnet_perf_chain_end);

void rmnet_descriptor_classify_chain_count(u64 chain_count,
					   struct rmnet_port *port)
{
	u64 index;

	if (chain_count >= 60) {
		port->stats.dl_chain_stat[6] += chain_count;
		return;
	}

	index = chain_count;
	do_div(index, 10);
	port->stats.dl_chain_stat[index] += chain_count;
}

void rmnet_descriptor_classify_frag_count(u64 frag_count,
					  struct rmnet_port *port)
{
	u64 index;

	if (frag_count <= 1) {
		port->stats.dl_frag_stat_1 += frag_count;
		return;
	}

	if (frag_count >= 16) {
		port->stats.dl_frag_stat[4] += frag_count;
		return;
	}

	index = frag_count;
	do_div(index, 4);
	port->stats.dl_frag_stat[index] += frag_count;
}

void rmnet_frag_ingress_handler(struct sk_buff *skb,
				struct rmnet_port *port)
{
	rmnet_perf_chain_hook_t rmnet_perf_opt_chain_end;
	LIST_HEAD(desc_list);
	bool skip_perf = (skb->priority == 0xda1a);
	u64 chain_count = 0;

	/* Deaggregation and freeing of HW originating
	 * buffers is done within here
	 */
	while (skb) {
		struct sk_buff *skb_frag;

		chain_count++;
		rmnet_descriptor_classify_frag_count(skb_shinfo(skb)->nr_frags,
						     port);

		rmnet_frag_deaggregate(skb, port, &desc_list, skb->priority);
		if (!list_empty(&desc_list)) {
			struct rmnet_frag_descriptor *frag_desc, *tmp;

			list_for_each_entry_safe(frag_desc, tmp, &desc_list,
						 list) {
				list_del_init(&frag_desc->list);
				__rmnet_frag_ingress_handler(frag_desc, port);
			}
		}

		skb_frag = skb_shinfo(skb)->frag_list;
		skb_shinfo(skb)->frag_list = NULL;
		consume_skb(skb);
		skb = skb_frag;
	}

	rmnet_descriptor_classify_chain_count(chain_count, port);

	if (skip_perf)
		return;

	rcu_read_lock();
	rmnet_perf_opt_chain_end = rcu_dereference(rmnet_perf_chain_end);
	if (rmnet_perf_opt_chain_end)
		rmnet_perf_opt_chain_end();
	rcu_read_unlock();
}

void rmnet_descriptor_deinit(struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool;
	struct rmnet_frag_descriptor *frag_desc, *tmp;

	pool = port->frag_desc_pool;

	list_for_each_entry_safe(frag_desc, tmp, &pool->free_list, list) {
		kfree(frag_desc);
		pool->pool_size--;
	}

	kfree(pool);
}

int rmnet_descriptor_init(struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool;
	int i;

	spin_lock_init(&port->desc_pool_lock);
	pool = kzalloc(sizeof(*pool), GFP_ATOMIC);
	if (!pool)
		return -ENOMEM;

	INIT_LIST_HEAD(&pool->free_list);
	port->frag_desc_pool = pool;

	for (i = 0; i < RMNET_FRAG_DESCRIPTOR_POOL_SIZE; i++) {
		struct rmnet_frag_descriptor *frag_desc;

		frag_desc = kzalloc(sizeof(*frag_desc), GFP_ATOMIC);
		if (!frag_desc)
			return -ENOMEM;

		INIT_LIST_HEAD(&frag_desc->list);
		INIT_LIST_HEAD(&frag_desc->frags);
		list_add_tail(&frag_desc->list, &pool->free_list);
		pool->pool_size++;
	}

	return 0;
}
