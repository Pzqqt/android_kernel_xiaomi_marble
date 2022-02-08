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

/**
 * DOC: i_qdf_tracepoint.h
 * This file provides internal interface for triggering tracepoints
 */
#if  !defined(_I_QDF_TRACEPOINT_H)
#define _I_QDF_TRACEPOINT_H

#include <qdf_tracepoint_defs.h>

/**
 * __qdf_trace_dp_rx_tcp_pkt_enabled() - Get the dp_rx_tcp_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_rx_tcp_pkt_enabled(void)
{
	return trace_dp_rx_tcp_pkt_enabled();
}

/**
 * __qdf_trace_dp_rx_tcp_pkt() - Trace tcp packet in rx direction
 * @skb: pointer to network buffer
 * @tcp_seq_num: TCP sequence number
 * @tcp_ack_num: TCP acknowlegment number
 * @srcport: TCP source port
 * @dstport: TCP destination port
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_rx_tcp_pkt(struct sk_buff *skb, uint32_t tcp_seq_num,
			       uint32_t tcp_ack_num, uint16_t srcport,
			       uint16_t dstport, uint64_t latency)
{
	trace_dp_rx_tcp_pkt(skb, tcp_seq_num, tcp_ack_num, srcport, dstport,
			    latency);
}

/**
 * __qdf_trace_dp_tx_comp_tcp_pkt_enabled() - Get the dp_tx_comp_tcp_pkt
 *  tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_tx_comp_tcp_pkt_enabled(void)
{
	return trace_dp_tx_comp_tcp_pkt_enabled();
}

/**
 * __qdf_trace_dp_tx_comp_tcp_pkt() - Trace tcp packet in tx completion
 * @skb: pointer to network buffer
 * @tcp_seq_num: TCP sequence number
 * @tcp_ack_num: TCP acknowlegment number
 * @srcport: TCP source port
 * @dstport: TCP destination port
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_tx_comp_tcp_pkt(struct sk_buff *skb, uint32_t tcp_seq_num,
				    uint32_t tcp_ack_num, uint16_t srcport,
				    uint16_t dstport, uint64_t latency)
{
	trace_dp_tx_comp_tcp_pkt(skb, tcp_seq_num, tcp_ack_num, srcport,
				 dstport, latency);
}

/**
 * __qdf_trace_dp_rx_udp_pkt_enabled() - Get the dp_rx_udp_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_rx_udp_pkt_enabled(void)
{
	return trace_dp_rx_udp_pkt_enabled();
}

/**
 * __qdf_trace_dp_rx_udp_pkt() - Trace udp packet in rx direction
 * @skb: pointer to network buffer
 * @ip_id: ip identification field
 * @srcport: UDP source port
 * @dstport: UDP destination port
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_rx_udp_pkt(struct sk_buff *skb, uint16_t ip_id,
			       uint16_t srcport, uint16_t dstport,
			       uint64_t latency)
{
	trace_dp_rx_udp_pkt(skb, ip_id, srcport, dstport, latency);
}

/**
 * __qdf_trace_dp_tx_comp_udp_pkt_enabled() - Get the dp_tx_comp_udp_pkt
 *  tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_tx_comp_udp_pkt_enabled(void)
{
	return trace_dp_tx_comp_udp_pkt_enabled();
}

/**
 * __qdf_trace_dp_tx_comp_udp_pkt() - Trace udp packet in tx completion
 * @skb: pointer to network buffer
 * @ip_id: ip identification field
 * @srcport: UDP source port
 * @dstport: UDP destination port
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_tx_comp_udp_pkt(struct sk_buff *skb, uint16_t ip_id,
				    uint16_t srcport, uint16_t dstport,
				    uint64_t latency)
{
	trace_dp_tx_comp_udp_pkt(skb, ip_id, srcport, dstport, latency);
}

/**
 * __qdf_trace_dp_rx_generic_ip_pkt_enabled() - Get the dp_rx_generic_ip_pkt
 *					tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_rx_generic_ip_pkt_enabled(void)
{
	return trace_dp_rx_generic_ip_pkt_enabled();
}

/**
 * __qdf_trace_dp_rx_generic_ip_pkt() - Trace generic ip packet in rx direction
 * @skb: pointer to network buffer
 * @ip_proto: ip protocol type
 * @ip_id: ip identification field
 * @trans_hdr_4_bytes: transport header first 4 bytes
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_rx_generic_ip_pkt(struct sk_buff *skb, uint8_t ip_proto,
				      uint16_t ip_id, uint32_t trans_hdr_4_bytes,
				      uint64_t latency)
{
	trace_dp_rx_generic_ip_pkt(skb, ip_proto, ip_id,
				   trans_hdr_4_bytes,
				   latency);
}

/**
 * __qdf_trace_dp_tx_comp_generic_ip_pkt_enabled() - Get the dp_tx_comp_generic_ip_pkt
 *						tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_tx_comp_generic_ip_pkt_enabled(void)
{
	return trace_dp_tx_comp_generic_ip_pkt_enabled();
}

/**
 * __qdf_trace_dp_tx_comp_generic_ip_pkt() - Trace generic ip packet in
 *					     tx direction
 * @skb: pointer to network buffer
 * @ip_id: ip identification field
 * @trans_hdr_4_bytes: transport header first 4 bytes
 * @latency: latency

 *
 * Return: None
 */
static inline
void __qdf_trace_dp_tx_comp_generic_ip_pkt(struct sk_buff *skb,
					   uint8_t ip_proto,
					   uint16_t ip_id,
					   uint32_t trans_hdr_4_bytes,
					   uint64_t latency)
{
	trace_dp_tx_comp_generic_ip_pkt(skb, ip_proto, ip_id,
					trans_hdr_4_bytes,
					latency);
}

/**
 * __qdf_trace_dp_rx_pkt_enablei() - Get the dp_rx_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_rx_pkt_enabled(void)
{
	return trace_dp_rx_pkt_enabled();
}

/**
 * __qdf_trace_dp_rx_pkt() - Trace non-tcp/udp packet in rx direction
 * @skb: pointer to network buffer
 * @ether_type: type in ethernet header
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_rx_pkt(struct sk_buff *skb, uint16_t ether_type,
			   uint64_t latency)
{
	trace_dp_rx_pkt(skb, ether_type, latency);
}

/**
 * __qdf_trace_dp_tx_comp_pkt_enabled() - Get the dp_tx_comp_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_tx_comp_pkt_enabled(void)
{
	return trace_dp_tx_comp_pkt_enabled();
}

/**
 * __qdf_trace_dp_tx_comp_pkt() - Trace non-tcp/udp packet in rx direction
 * @skb: pointer to network buffer
 * @ether_type: type in ethernet header
 * @latency: latency
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_tx_comp_pkt(struct sk_buff *skb, uint16_t ether_type,
				uint64_t latency)
{
	trace_dp_tx_comp_pkt(skb, ether_type, latency);
}

/**
 * __qdf_trace_dp_del_reg_write_enabled() - Get the dp_del_reg_write tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_del_reg_write_enabled(void)
{
	return trace_dp_del_reg_write_enabled();
}

/**
 * __qdf_trace_dp_del_reg_write() - Trace delayed register writes
 * @srng_id: srng id
 * @enq_val: enqueue value
 * @deq_val: dequeue value
 * @enq_time: enqueue time
 * @deq_time: dequeue time
 *
 * Return: None
 */
static inline
void __qdf_trace_dp_del_reg_write(uint8_t srng_id, uint32_t enq_val,
				  uint32_t deq_val, uint64_t enq_time,
				  uint64_t deq_time)
{
	trace_dp_del_reg_write(srng_id, enq_val, deq_val, enq_time,
			       deq_time);
}

/**
 * __qdf_trace_dp_ce_tasklet_sched_latency_enabled() - Get the
 *  dp_ce_tasklet_sched_lat tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool __qdf_trace_dp_ce_tasklet_sched_latency_enabled(void)
{
	return trace_dp_ce_tasklet_sched_latency_enabled();
}

/**
 * __qdf_trace_dp_ce_tasklet_sched_latency() - Trace ce tasklet scheduling
 *  latency
 * @ce_id: ce id
 * @sched_latency: ce tasklet sched latency
 *
 * Return: None
 */
static inline void
__qdf_trace_dp_ce_tasklet_sched_latency(uint8_t ce_id, uint64_t sched_latency)
{
	trace_dp_ce_tasklet_sched_latency(ce_id, sched_latency);
}
#endif /* _I_QDF_TRACEPOINT_H */
