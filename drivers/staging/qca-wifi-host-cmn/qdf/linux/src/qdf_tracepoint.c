/*
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <qdf_tracepoint.h>

#define CREATE_TRACE_POINTS
#include "qdf_tracepoint_defs.h"

#ifdef WLAN_TRACEPOINTS
static void qdf_trace_dp_tx_ip_packet(qdf_nbuf_t nbuf, uint8_t *trans_hdr,
				      uint8_t ip_proto, uint16_t ip_id,
				      struct qdf_tso_seg_elem_t *tso_desc,
				      uint64_t latency)
{
	if (ip_proto == QDF_NBUF_TRAC_TCP_TYPE &&
	    __qdf_trace_dp_tx_comp_tcp_pkt_enabled()) {
		qdf_net_tcphdr_t *tcph = (qdf_net_tcphdr_t *)trans_hdr;
		uint32_t tcp_seq;

		if (tso_desc)
			tcp_seq = tso_desc->seg.tso_flags.tcp_seq_num;
		else
			tcp_seq = qdf_ntohl(tcph->seq);

		__qdf_trace_dp_tx_comp_tcp_pkt(nbuf, tcp_seq,
					       qdf_ntohl(tcph->ack_seq),
					       qdf_ntohs(tcph->source),
					       qdf_ntohs(tcph->dest),
					       latency);
	} else if (ip_proto == QDF_NBUF_TRAC_UDP_TYPE &&
		   __qdf_trace_dp_tx_comp_udp_pkt_enabled()) {
		qdf_net_udphdr_t *udph = (qdf_net_udphdr_t *)trans_hdr;

		__qdf_trace_dp_tx_comp_udp_pkt(nbuf, qdf_ntohs(ip_id),
					       qdf_ntohs(udph->src_port),
					       qdf_ntohs(udph->dst_port),
					       latency);
	} else if (__qdf_trace_dp_tx_comp_generic_ip_pkt_enabled()) {
		__qdf_trace_dp_tx_comp_generic_ip_pkt(nbuf, ip_proto, ip_id,
						      QDF_SWAP_U32(*(uint32_t *)trans_hdr),
						      latency);
	}
}

static void qdf_trace_dp_rx_ip_packet(qdf_nbuf_t nbuf, uint8_t *trans_hdr,
				      uint8_t ip_proto, uint16_t ip_id,
				      uint64_t latency)
{
	if (ip_proto == QDF_NBUF_TRAC_TCP_TYPE &&
	    __qdf_trace_dp_rx_tcp_pkt_enabled()) {
		qdf_net_tcphdr_t *tcph = (qdf_net_tcphdr_t *)trans_hdr;

		__qdf_trace_dp_rx_tcp_pkt(nbuf, qdf_ntohl(tcph->seq),
					  qdf_ntohl(tcph->ack_seq),
					  qdf_ntohs(tcph->source),
					  qdf_ntohs(tcph->dest),
					  latency);
	} else if (ip_proto == QDF_NBUF_TRAC_UDP_TYPE &&
		   __qdf_trace_dp_rx_udp_pkt_enabled()) {
		qdf_net_udphdr_t *udph = (qdf_net_udphdr_t *)trans_hdr;

		__qdf_trace_dp_rx_udp_pkt(nbuf, qdf_ntohs(ip_id),
					  qdf_ntohs(udph->src_port),
					  qdf_ntohs(udph->dst_port),
					  latency);
	} else if (__qdf_trace_dp_rx_generic_ip_pkt_enabled()) {
		__qdf_trace_dp_rx_generic_ip_pkt(nbuf, ip_proto, ip_id,
						 QDF_SWAP_U32(*(uint32_t *)trans_hdr),
						 latency);
	}
}

void qdf_trace_dp_packet(qdf_nbuf_t nbuf, enum qdf_proto_dir dir,
			 struct qdf_tso_seg_elem_t *tso_desc, uint64_t enq_time)
{
	uint8_t *data = qdf_nbuf_data(nbuf);
	uint64_t latency;
	uint16_t ether_type;
	uint8_t ip_offset = QDF_NBUF_TRAC_IP_OFFSET;

	if (dir == QDF_TX)
		latency = (qdf_ktime_to_ms(qdf_ktime_real_get()) - enq_time);
	else
		latency = qdf_nbuf_get_timedelta_ms(nbuf);

	ether_type = QDF_SWAP_U16(*(uint16_t *)(data +
						QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	if (unlikely(ether_type == QDF_ETH_TYPE_8021Q)) {
		ether_type = QDF_SWAP_U16(*(uint16_t *)(data +
					   QDF_NBUF_TRAC_VLAN_ETH_TYPE_OFFSET));
		ip_offset = QDF_NBUF_TRAC_VLAN_IP_OFFSET;
	} else if (unlikely(ether_type == QDF_ETH_TYPE_8021AD)) {
		ether_type = QDF_SWAP_U16(*(uint16_t *)(data +
				    QDF_NBUF_TRAC_DOUBLE_VLAN_ETH_TYPE_OFFSET));
		ip_offset = QDF_NBUF_TRAC_DOUBLE_VLAN_IP_OFFSET;
	}

	switch (ether_type) {
	case QDF_NBUF_TRAC_IPV4_ETH_TYPE:
	case QDF_NBUF_TRAC_IPV6_ETH_TYPE:
	{
		uint8_t *net_hdr;
		uint8_t *trans_hdr;
		uint8_t ip_proto;
		uint16_t ip_id = 0;

		net_hdr = data + ip_offset;

		if (ether_type == QDF_NBUF_TRAC_IPV4_ETH_TYPE) {
			ip_proto = ((qdf_net_iphdr_t *)net_hdr)->ip_proto;
			ip_id = ((qdf_net_iphdr_t *)net_hdr)->ip_id;
			trans_hdr = net_hdr + QDF_NBUF_TRAC_IPV4_HEADER_SIZE;
		} else {
			ip_proto = ((qdf_net_ipv6hdr_t *)net_hdr)->ipv6_nexthdr;
			trans_hdr = net_hdr + QDF_NBUF_TRAC_IPV6_HEADER_SIZE;
		}

		if (dir == QDF_TX)
			qdf_trace_dp_tx_ip_packet(nbuf, trans_hdr, ip_proto,
						  ip_id, tso_desc, latency);
		else
			qdf_trace_dp_rx_ip_packet(nbuf, trans_hdr, ip_proto,
						  ip_id, latency);

		break;
	}
	default:
		if (dir == QDF_TX && __qdf_trace_dp_tx_comp_pkt_enabled())
			__qdf_trace_dp_tx_comp_pkt(nbuf, ether_type, latency);
		else if (__qdf_trace_dp_rx_pkt_enabled())
			__qdf_trace_dp_rx_pkt(nbuf, ether_type, latency);

		break;
	}
}
#endif
