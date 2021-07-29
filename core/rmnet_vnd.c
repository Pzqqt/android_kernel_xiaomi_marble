/* Copyright (c) 2013-2021, The Linux Foundation. All rights reserved.
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
 *
 * RMNET Data virtual network driver
 *
 */

#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/inet.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <net/pkt_sched.h>
#include <net/ipv6.h>
#include "rmnet_config.h"
#include "rmnet_handlers.h"
#include "rmnet_private.h"
#include "rmnet_map.h"
#include "rmnet_vnd.h"
#include "rmnet_genl.h"
#include "rmnet_ll.h"

#include "qmi_rmnet.h"
#include "rmnet_qmi.h"
#include "rmnet_trace.h"

typedef void (*rmnet_perf_tether_egress_hook_t)(struct sk_buff *skb);

rmnet_perf_tether_egress_hook_t rmnet_perf_tether_egress_hook __rcu __read_mostly;
EXPORT_SYMBOL(rmnet_perf_tether_egress_hook);

/* RX/TX Fixup */

void rmnet_vnd_rx_fixup(struct net_device *dev, u32 skb_len)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_pcpu_stats *pcpu_ptr;

	pcpu_ptr = this_cpu_ptr(priv->pcpu_stats);

	u64_stats_update_begin(&pcpu_ptr->syncp);
	pcpu_ptr->stats.rx_pkts++;
	pcpu_ptr->stats.rx_bytes += skb_len;
	u64_stats_update_end(&pcpu_ptr->syncp);
}

void rmnet_vnd_tx_fixup(struct net_device *dev, u32 skb_len)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_pcpu_stats *pcpu_ptr;

	pcpu_ptr = this_cpu_ptr(priv->pcpu_stats);

	u64_stats_update_begin(&pcpu_ptr->syncp);
	pcpu_ptr->stats.tx_pkts++;
	pcpu_ptr->stats.tx_bytes += skb_len;
	u64_stats_update_end(&pcpu_ptr->syncp);
}

/* Network Device Operations */

static netdev_tx_t rmnet_vnd_start_xmit(struct sk_buff *skb,
					struct net_device *dev)
{
	struct rmnet_priv *priv;
	int ip_type;
	u32 mark;
	unsigned int len;
	rmnet_perf_tether_egress_hook_t rmnet_perf_tether_egress;
	bool low_latency;

	priv = netdev_priv(dev);
	if (priv->real_dev) {
		ip_type = (ip_hdr(skb)->version == 4) ?
					AF_INET : AF_INET6;
		mark = skb->mark;
		len = skb->len;
		trace_rmnet_xmit_skb(skb);
		rmnet_perf_tether_egress = rcu_dereference(rmnet_perf_tether_egress_hook);
		if (rmnet_perf_tether_egress) {
			rmnet_perf_tether_egress(skb);
		}
		low_latency = qmi_rmnet_flow_is_low_latency(dev, skb);
		if (low_latency && skb_is_gso(skb)) {
			netdev_features_t features;
			struct sk_buff *segs, *tmp;

			features = dev->features & ~NETIF_F_GSO_MASK;
			segs = skb_gso_segment(skb, features);
			if (IS_ERR_OR_NULL(segs)) {
				this_cpu_add(priv->pcpu_stats->stats.tx_drops,
					     skb_shinfo(skb)->gso_segs);
				priv->stats.ll_tso_errs++;
				kfree_skb(skb);
				return NETDEV_TX_OK;
			}

			consume_skb(skb);
			for (skb = segs; skb; skb = tmp) {
				tmp = skb->next;
				skb->dev = dev;
				priv->stats.ll_tso_segs++;
				rmnet_egress_handler(skb, low_latency);
			}
		} else if (!low_latency && skb_is_gso(skb)) {
			u64 gso_limit = priv->real_dev->gso_max_size ? : 1;
			u16 gso_goal = 0;
			netdev_features_t features = NETIF_F_SG;
			u16 orig_gso_size = skb_shinfo(skb)->gso_size;
			unsigned int orig_gso_type = skb_shinfo(skb)->gso_type;
			struct sk_buff *segs, *tmp;

			features |=  NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;

			if (skb->len < gso_limit || gso_limit > 65535) {
				priv->stats.tso_segment_skip++;
				rmnet_egress_handler(skb, low_latency);
			} else {
				do_div(gso_limit, skb_shinfo(skb)->gso_size);
				gso_goal = gso_limit * skb_shinfo(skb)->gso_size;
				skb_shinfo(skb)->gso_size = gso_goal;

				segs = __skb_gso_segment(skb, features, false);
				if (IS_ERR_OR_NULL(segs)) {
					skb_shinfo(skb)->gso_size = orig_gso_size;
					skb_shinfo(skb)->gso_type = orig_gso_type;

					priv->stats.tso_segment_fail++;
					rmnet_egress_handler(skb, low_latency);
				} else {
					consume_skb(skb);

					for (skb = segs; skb; skb = tmp) {
						tmp = skb->next;
						skb->dev = dev;

						skb_shinfo(skb)->gso_size = orig_gso_size;
						skb_shinfo(skb)->gso_type = orig_gso_type;

						priv->stats.tso_segment_success++;
						rmnet_egress_handler(skb, low_latency);
					}
				}
			}
		} else {
			rmnet_egress_handler(skb, low_latency);
		}
		qmi_rmnet_burst_fc_check(dev, ip_type, mark, len);
		qmi_rmnet_work_maybe_restart(rmnet_get_rmnet_port(dev));
	} else {
		this_cpu_inc(priv->pcpu_stats->stats.tx_drops);
		kfree_skb(skb);
	}
	return NETDEV_TX_OK;
}

static int rmnet_vnd_change_mtu(struct net_device *rmnet_dev, int new_mtu)
{
	if (new_mtu < 0 || new_mtu > RMNET_MAX_PACKET_SIZE)
		return -EINVAL;

	rmnet_dev->mtu = new_mtu;
	return 0;
}

static int rmnet_vnd_get_iflink(const struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);

	return priv->real_dev->ifindex;
}

static int rmnet_vnd_init(struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	int err;

	priv->pcpu_stats = alloc_percpu(struct rmnet_pcpu_stats);
	if (!priv->pcpu_stats)
		return -ENOMEM;

	err = gro_cells_init(&priv->gro_cells, dev);
	if (err) {
		free_percpu(priv->pcpu_stats);
		return err;
	}

	return 0;
}

static void rmnet_vnd_uninit(struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	void *qos;

	gro_cells_destroy(&priv->gro_cells);
	free_percpu(priv->pcpu_stats);

	qos = rcu_dereference(priv->qos_info);
	RCU_INIT_POINTER(priv->qos_info, NULL);
	qmi_rmnet_qos_exit_pre(qos);
}

static void rmnet_get_stats64(struct net_device *dev,
			      struct rtnl_link_stats64 *s)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_vnd_stats total_stats;
	struct rmnet_pcpu_stats *pcpu_ptr;
	unsigned int cpu, start;

	memset(&total_stats, 0, sizeof(struct rmnet_vnd_stats));

	for_each_possible_cpu(cpu) {
		pcpu_ptr = per_cpu_ptr(priv->pcpu_stats, cpu);

		do {
			start = u64_stats_fetch_begin_irq(&pcpu_ptr->syncp);
			total_stats.rx_pkts += pcpu_ptr->stats.rx_pkts;
			total_stats.rx_bytes += pcpu_ptr->stats.rx_bytes;
			total_stats.tx_pkts += pcpu_ptr->stats.tx_pkts;
			total_stats.tx_bytes += pcpu_ptr->stats.tx_bytes;
		} while (u64_stats_fetch_retry_irq(&pcpu_ptr->syncp, start));

		total_stats.tx_drops += pcpu_ptr->stats.tx_drops;
	}

	s->rx_packets = total_stats.rx_pkts;
	s->rx_bytes = total_stats.rx_bytes;
	s->tx_packets = total_stats.tx_pkts;
	s->tx_bytes = total_stats.tx_bytes;
	s->tx_dropped = total_stats.tx_drops;
}

static u16 rmnet_vnd_select_queue(struct net_device *dev,
				  struct sk_buff *skb,
				  struct net_device *sb_dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	u64 boost_period = 0;
	int boost_trigger = 0;
	int txq = 0;

	if (trace_print_icmp_tx_enabled()) {
		char saddr[INET6_ADDRSTRLEN], daddr[INET6_ADDRSTRLEN];
		u16 ip_proto = 0;
		__be16 sequence = 0;
		u8 type = 0;

		memset(saddr, 0, INET6_ADDRSTRLEN);
		memset(daddr, 0, INET6_ADDRSTRLEN);

		if (skb->protocol == htons(ETH_P_IP)) {
			if (ip_hdr(skb)->protocol != IPPROTO_ICMP)
				goto skip_trace_print_icmp_tx;

			if (icmp_hdr(skb)->type != ICMP_ECHOREPLY &&
			    icmp_hdr(skb)->type != ICMP_ECHO)
				goto skip_trace_print_icmp_tx;

			ip_proto = htons(ETH_P_IP);
			type = icmp_hdr(skb)->type;
			sequence = icmp_hdr(skb)->un.echo.sequence;
			snprintf(saddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->daddr);
		}

		if (skb->protocol == htons(ETH_P_IPV6)) {
			if (ipv6_hdr(skb)->nexthdr != NEXTHDR_ICMP)
				goto skip_trace_print_icmp_tx;

			if (icmp6_hdr(skb)->icmp6_type != ICMPV6_ECHO_REQUEST &&
			    icmp6_hdr(skb)->icmp6_type != ICMPV6_ECHO_REPLY)
				goto skip_trace_print_icmp_tx;

			ip_proto = htons(ETH_P_IPV6);
			type = icmp6_hdr(skb)->icmp6_type;
			sequence = icmp6_hdr(skb)->icmp6_sequence;
			snprintf(saddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->daddr);
		}

		if (!ip_proto)
			goto skip_trace_print_icmp_tx;

		trace_print_icmp_tx(skb, ip_proto, type, sequence, saddr, daddr);
	}

skip_trace_print_icmp_tx:
	if (trace_print_tcp_tx_enabled()) {
		char saddr[INET6_ADDRSTRLEN], daddr[INET6_ADDRSTRLEN];

		memset(saddr, 0, INET6_ADDRSTRLEN);
		memset(daddr, 0, INET6_ADDRSTRLEN);

		if (skb->protocol == htons(ETH_P_IP)) {
			if (ip_hdr(skb)->protocol != IPPROTO_TCP)
				goto skip_trace_print_tcp_tx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->daddr);
		}

		if (skb->protocol == htons(ETH_P_IPV6)) {
			if (ipv6_hdr(skb)->nexthdr != IPPROTO_TCP)
				goto skip_trace_print_tcp_tx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->daddr);
		}

		trace_print_tcp_tx(skb, saddr, daddr, tcp_hdr(skb));
	}

skip_trace_print_tcp_tx:
	if (trace_print_udp_tx_enabled()) {
		char saddr[INET6_ADDRSTRLEN], daddr[INET6_ADDRSTRLEN];

		memset(saddr, 0, INET6_ADDRSTRLEN);
		memset(daddr, 0, INET6_ADDRSTRLEN);

		if (skb->protocol == htons(ETH_P_IP)) {
			if (ip_hdr(skb)->protocol != IPPROTO_UDP)
				goto skip_trace_print_udp_tx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->daddr);
		}

		if (skb->protocol == htons(ETH_P_IPV6)) {
			if (ipv6_hdr(skb)->nexthdr != IPPROTO_UDP)
				goto skip_trace_print_udp_tx;

			snprintf(saddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->daddr);
		}

		trace_print_udp_tx(skb, saddr, daddr, udp_hdr(skb));
	}

skip_trace_print_udp_tx:
	if (trace_print_skb_gso_enabled()) {
		char saddr[INET6_ADDRSTRLEN], daddr[INET6_ADDRSTRLEN];
		u16 ip_proto = 0, xport_proto = 0;

		if (!skb_shinfo(skb)->gso_size)
			goto skip_trace;

		memset(saddr, 0, INET6_ADDRSTRLEN);
		memset(daddr, 0, INET6_ADDRSTRLEN);

		if (skb->protocol == htons(ETH_P_IP)) {
			if (ip_hdr(skb)->protocol == IPPROTO_TCP)
				xport_proto = IPPROTO_TCP;
			else if (ip_hdr(skb)->protocol == IPPROTO_UDP)
				xport_proto = IPPROTO_UDP;
			else
				goto skip_trace;

			ip_proto = htons(ETH_P_IP);
			snprintf(saddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI4", &ip_hdr(skb)->daddr);
		}

		if (skb->protocol == htons(ETH_P_IPV6)) {
			if (ipv6_hdr(skb)->nexthdr == IPPROTO_TCP)
				xport_proto = IPPROTO_TCP;
			else if (ipv6_hdr(skb)->nexthdr == IPPROTO_UDP)
				xport_proto = IPPROTO_UDP;
			else
				goto skip_trace;

			ip_proto = htons(ETH_P_IPV6);
			snprintf(saddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->saddr);
			snprintf(daddr, INET6_ADDRSTRLEN, "%pI6", &ipv6_hdr(skb)->daddr);
		}

		trace_print_skb_gso(skb,
				    xport_proto == IPPROTO_TCP ? tcp_hdr(skb)->source :
								 udp_hdr(skb)->source,
				    xport_proto == IPPROTO_TCP ? tcp_hdr(skb)->dest :
								 udp_hdr(skb)->dest,
				    ip_proto, xport_proto, saddr, daddr);
	}

skip_trace:
	if (priv->real_dev)
		txq = qmi_rmnet_get_queue(dev, skb);

	if (rmnet_core_userspace_connected) {
		rmnet_update_pid_and_check_boost(task_pid_nr(current),
						 skb->len,
						 &boost_trigger,
						 &boost_period);

		if (boost_trigger)
			(void) boost_period;
	}

	return (txq < dev->real_num_tx_queues) ? txq : 0;
}

static const struct net_device_ops rmnet_vnd_ops = {
	.ndo_start_xmit = rmnet_vnd_start_xmit,
	.ndo_change_mtu = rmnet_vnd_change_mtu,
	.ndo_get_iflink = rmnet_vnd_get_iflink,
	.ndo_add_slave  = rmnet_add_bridge,
	.ndo_del_slave  = rmnet_del_bridge,
	.ndo_init       = rmnet_vnd_init,
	.ndo_uninit     = rmnet_vnd_uninit,
	.ndo_get_stats64 = rmnet_get_stats64,
	.ndo_select_queue = rmnet_vnd_select_queue,
};

static const char rmnet_gstrings_stats[][ETH_GSTRING_LEN] = {
	"Checksum ok",
	"Checksum valid bit not set",
	"Checksum validation failed",
	"Checksum error bad buffer",
	"Checksum error bad ip version",
	"Checksum error bad transport",
	"Checksum skipped on ip fragment",
	"Checksum skipped",
	"Checksum computed in software",
	"Checksum computed in hardware",
	"Coalescing packets received",
	"Coalesced packets",
	"Coalescing header NLO errors",
	"Coalescing header pcount errors",
	"Coalescing checksum errors",
	"Coalescing packet reconstructs",
	"Coalescing IP version invalid",
	"Coalescing L4 header invalid",
	"Coalescing close Non-coalescable",
	"Coalescing close L3 mismatch",
	"Coalescing close L4 mismatch",
	"Coalescing close HW NLO limit",
	"Coalescing close HW packet limit",
	"Coalescing close HW byte limit",
	"Coalescing close HW time limit",
	"Coalescing close HW eviction",
	"Coalescing close Coalescable",
	"Coalescing packets over VEID0",
	"Coalescing packets over VEID1",
	"Coalescing packets over VEID2",
	"Coalescing packets over VEID3",
	"Coalescing TCP frames",
	"Coalescing TCP bytes",
	"Coalescing UDP frames",
	"Coalescing UDP bytes",
	"Uplink priority packets",
	"TSO packets",
	"TSO packets arriving incorrectly",
	"TSO segment success",
	"TSO segment fail",
	"TSO segment skip",
	"LL TSO segment success",
	"LL TSO segment fail",
};

static const char rmnet_port_gstrings_stats[][ETH_GSTRING_LEN] = {
	"MAP Cmd last version",
	"MAP Cmd last ep id",
	"MAP Cmd last transaction id",
	"DL header last seen sequence",
	"DL header last seen bytes",
	"DL header last seen packets",
	"DL header last seen flows",
	"DL header pkts received",
	"DL header total bytes received",
	"DL header total pkts received",
	"DL trailer last seen sequence",
	"DL trailer pkts received",
	"UL agg reuse",
	"UL agg alloc",
	"DL chaining [0-10)",
	"DL chaining [10-20)",
	"DL chaining [20-30)",
	"DL chaining [30-40)",
	"DL chaining [40-50)",
	"DL chaining [50-60)",
	"DL chaining >= 60",
	"DL chaining frags [0-1]",
	"DL chaining frags [2-3]",
	"DL chaining frags [4-7]",
	"DL chaining frags [8-11]",
	"DL chaining frags [12-15]",
	"DL chaining frags = 16",
};

static const char rmnet_ll_gstrings_stats[][ETH_GSTRING_LEN] = {
	"LL TX queues",
	"LL TX queue errors",
	"LL TX completions",
	"LL TX completion errors",
	"LL RX queues",
	"LL RX queue errors",
	"LL RX status errors",
	"LL RX empty transfers",
	"LL RX OOM errors",
	"LL RX packets",
	"LL RX temp buffer allocations",
};

static void rmnet_get_strings(struct net_device *dev, u32 stringset, u8 *buf)
{
	size_t off = 0;

	switch (stringset) {
	case ETH_SS_STATS:
		memcpy(buf, &rmnet_gstrings_stats,
		       sizeof(rmnet_gstrings_stats));
		off += sizeof(rmnet_gstrings_stats);
		memcpy(buf + off,
		       &rmnet_port_gstrings_stats,
		       sizeof(rmnet_port_gstrings_stats));
		off += sizeof(rmnet_port_gstrings_stats);
		memcpy(buf + off, &rmnet_ll_gstrings_stats,
		       sizeof(rmnet_ll_gstrings_stats));
		break;
	}
}

static int rmnet_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(rmnet_gstrings_stats) +
		       ARRAY_SIZE(rmnet_port_gstrings_stats) +
		       ARRAY_SIZE(rmnet_ll_gstrings_stats);
	default:
		return -EOPNOTSUPP;
	}
}

static void rmnet_get_ethtool_stats(struct net_device *dev,
				    struct ethtool_stats *stats, u64 *data)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_priv_stats *st = &priv->stats;
	struct rmnet_port_priv_stats *stp;
	struct rmnet_ll_stats *llp;
	struct rmnet_port *port;
	size_t off = 0;

	port = rmnet_get_port(priv->real_dev);

	if (!data || !port)
		return;

	stp = &port->stats;
	llp = rmnet_ll_get_stats();

	memcpy(data, st, ARRAY_SIZE(rmnet_gstrings_stats) * sizeof(u64));
	off += ARRAY_SIZE(rmnet_gstrings_stats);
	memcpy(data + off, stp,
	       ARRAY_SIZE(rmnet_port_gstrings_stats) * sizeof(u64));
	off += ARRAY_SIZE(rmnet_port_gstrings_stats);
	memcpy(data + off, llp,
	       ARRAY_SIZE(rmnet_ll_gstrings_stats) * sizeof(u64));
}

static int rmnet_stats_reset(struct net_device *dev)
{
	struct rmnet_priv *priv = netdev_priv(dev);
	struct rmnet_port_priv_stats *stp;
	struct rmnet_priv_stats *st;
	struct rmnet_port *port;

	port = rmnet_get_port(priv->real_dev);
	if (!port)
		return -EINVAL;

	stp = &port->stats;

	memset(stp, 0, sizeof(*stp));

	st = &priv->stats;

	memset(st, 0, sizeof(*st));

	return 0;
}

static const struct ethtool_ops rmnet_ethtool_ops = {
	.get_ethtool_stats = rmnet_get_ethtool_stats,
	.get_strings = rmnet_get_strings,
	.get_sset_count = rmnet_get_sset_count,
	.nway_reset = rmnet_stats_reset,
};

/* Called by kernel whenever a new rmnet<n> device is created. Sets MTU,
 * flags, ARP type, needed headroom, etc...
 */
void rmnet_vnd_setup(struct net_device *rmnet_dev)
{
	rmnet_dev->netdev_ops = &rmnet_vnd_ops;
	rmnet_dev->mtu = RMNET_DFLT_PACKET_SIZE;
	rmnet_dev->needed_headroom = RMNET_NEEDED_HEADROOM;
	random_ether_addr(rmnet_dev->perm_addr);
	rmnet_dev->tx_queue_len = RMNET_TX_QUEUE_LEN;

	/* Raw IP mode */
	rmnet_dev->header_ops = NULL;  /* No header */
	rmnet_dev->type = ARPHRD_RAWIP;
	rmnet_dev->hard_header_len = 0;
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

	rmnet_dev->needs_free_netdev = true;
	rmnet_dev->ethtool_ops = &rmnet_ethtool_ops;
}

/* Exposed API */

int rmnet_vnd_newlink(u8 id, struct net_device *rmnet_dev,
		      struct rmnet_port *port,
		      struct net_device *real_dev,
		      struct rmnet_endpoint *ep)
{
	struct rmnet_priv *priv = netdev_priv(rmnet_dev);
	int rc;

	if (ep->egress_dev)
		return -EINVAL;

	if (rmnet_get_endpoint(port, id))
		return -EBUSY;

	rmnet_dev->hw_features = NETIF_F_RXCSUM;
	rmnet_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	rmnet_dev->hw_features |= NETIF_F_SG;
	rmnet_dev->hw_features |= NETIF_F_GRO_HW;
	rmnet_dev->hw_features |= NETIF_F_GSO_UDP_L4;
	rmnet_dev->hw_features |= NETIF_F_ALL_TSO;

	priv->real_dev = real_dev;

	rmnet_dev->gso_max_size = 64000;

	rc = register_netdevice(rmnet_dev);
	if (!rc) {
		ep->egress_dev = rmnet_dev;
		ep->mux_id = id;
		port->nr_rmnet_devs++;

		rmnet_dev->rtnl_link_ops = &rmnet_link_ops;

		priv->mux_id = id;
		rcu_assign_pointer(priv->qos_info,
			qmi_rmnet_qos_init(real_dev, rmnet_dev, id));

		netdev_dbg(rmnet_dev, "rmnet dev created\n");
	}

	return rc;
}

int rmnet_vnd_dellink(u8 id, struct rmnet_port *port,
		      struct rmnet_endpoint *ep)
{
	if (id >= RMNET_MAX_LOGICAL_EP || !ep->egress_dev)
		return -EINVAL;

	ep->egress_dev = NULL;
	port->nr_rmnet_devs--;
	return 0;
}

u8 rmnet_vnd_get_mux(struct net_device *rmnet_dev)
{
	struct rmnet_priv *priv;

	priv = netdev_priv(rmnet_dev);
	return priv->mux_id;
}

int rmnet_vnd_do_flow_control(struct net_device *rmnet_dev, int enable)
{
	netdev_dbg(rmnet_dev, "Setting VND TX queue state to %d\n", enable);
	/* Although we expect similar number of enable/disable
	 * commands, optimize for the disable. That is more
	 * latency sensitive than enable
	 */
	if (unlikely(enable))
		netif_wake_queue(rmnet_dev);
	else
		netif_stop_queue(rmnet_dev);

	return 0;
}

void rmnet_vnd_reset_mac_addr(struct net_device *dev)
{
	if (dev->netdev_ops != &rmnet_vnd_ops)
		return;

	random_ether_addr(dev->perm_addr);
}
