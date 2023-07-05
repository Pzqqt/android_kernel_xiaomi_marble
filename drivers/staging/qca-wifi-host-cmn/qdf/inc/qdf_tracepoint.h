/*
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_tracepoint.h
 * This file defines HLOS agnostic functions providing external interface
 * for triggering tracepoints.
 */
#if  !defined(_QDF_TRACEPOINT_H)
#define _QDF_TRACEPOINT_H

#include <i_qdf_tracepoint.h>
#include <qdf_nbuf.h>
#include <qdf_trace.h>

#ifdef WLAN_TRACEPOINTS
/**
 * qdf_trace_dp_packet() - Trace packet in tx or rx path
 * @nbuf: network buffer pointer
 * @dir: tx or rx direction
 * @tso_desc: TSO descriptor
 * @enq_time: tx hw enqueue wall clock time in milliseconds
 *
 * Return: None
 */
void qdf_trace_dp_packet(qdf_nbuf_t nbuf, enum qdf_proto_dir dir,
			 struct qdf_tso_seg_elem_t *tso_desc,
			 uint64_t enq_time);
#else
static inline
void qdf_trace_dp_packet(qdf_nbuf_t nbuf, enum qdf_proto_dir dir,
			 struct qdf_tso_seg_elem_t *tso_desc,
			 uint64_t enq_time)
{
}
#endif

/**
 * qdf_trace_dp_rx_tcp_pkt_enabled() - Get the dp_rx_tcp_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_rx_tcp_pkt_enabled(void)
{
	return __qdf_trace_dp_rx_tcp_pkt_enabled();
}

/**
 * qdf_trace_dp_rx_tcp_pkt() - Trace tcp packet in rx direction
 * @nbuf: pointer to network buffer
 * @tcp_seq_num: TCP sequence number
 * @tcp_ack_num: TCP acknowlegment number
 * @srcport: TCP source port
 * @dstport: TCP destination port
 * @latency: latency in milliseconds
 *
 * Return: None
 */
static inline
void qdf_trace_dp_rx_tcp_pkt(qdf_nbuf_t nbuf, uint32_t tcp_seq_num,
			     uint32_t tcp_ack_num, uint16_t srcport,
			     uint16_t dstport, uint64_t latency)
{
	__qdf_trace_dp_rx_tcp_pkt(nbuf, tcp_seq_num, tcp_ack_num,
				  srcport, dstport, latency);
}

/**
 * qdf_trace_dp_tx_comp_tcp_pkt_enabled() - Get the dp_tx_comp_tcp_pkt
 *  tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_tx_comp_tcp_pkt_enabled(void)
{
	return __qdf_trace_dp_tx_comp_tcp_pkt_enabled();
}

/**
 * qdf_trace_dp_tx_comp_tcp_pkt() - Trace tcp packet in tx completion
 * @nbuf: pointer to network buffer
 * @tcp_seq_num: TCP sequence number
 * @tcp_ack_num: TCP acknowlegment number
 * @srcport: TCP source port
 * @dstport: TCP destination port
 * @latency: latency in milliseconds
 *
 * Return: None
 */
static inline
void qdf_trace_dp_tx_comp_tcp_pkt(qdf_nbuf_t nbuf, uint32_t tcp_seq_num,
				  uint32_t tcp_ack_num, uint16_t srcport,
				  uint16_t dstport, uint64_t latency)
{
	__qdf_trace_dp_tx_comp_tcp_pkt(nbuf, tcp_seq_num, tcp_ack_num, srcport,
				       dstport, latency);
}

/**
 * qdf_trace_dp_rx_udp_pkt_enabled() - Get the dp_rx_udp_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_rx_udp_pkt_enabled(void)
{
	return __qdf_trace_dp_rx_udp_pkt_enabled();
}

/**
 * qdf_trace_dp_rx_udp_pkt() - Trace udp packet in rx direction
 * @nbuf: pointer to network buffer
 * @ip_id: ip identification field
 * @srcport: UDP source port
 * @dstport: UDP destination port
 * @latency: latency in milliseconds
 *
 * Return: None
 */
static inline
void qdf_trace_dp_rx_udp_pkt(qdf_nbuf_t nbuf, uint16_t ip_id,
			     uint16_t srcport, uint16_t dstport,
			     uint64_t latency)
{
	__qdf_trace_dp_rx_udp_pkt(nbuf, ip_id, srcport, dstport, latency);
}

/**
 * qdf_trace_dp_tx_comp_udp_pkt_enabled() - Get the dp_tx_comp_udp_pkt
 *  tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_tx_comp_udp_pkt_enabled(void)
{
	return __qdf_trace_dp_tx_comp_udp_pkt_enabled();
}

/**
 * qdf_trace_dp_tx_comp_udp_pkt() - Trace udp packet in tx completion
 * @nbuf: pointer to network buffer
 * @ip_id: ip identification field
 * @srcport: UDP source port
 * @dstport: UDP destination port
 * @latency: latency in milliseconds
 *
 * Return: None
 */
static inline
void qdf_trace_dp_tx_comp_udp_pkt(qdf_nbuf_t nbuf, uint16_t ip_id,
				  uint16_t srcport, uint16_t dstport,
				  uint64_t latency)
{
	__qdf_trace_dp_tx_comp_udp_pkt(nbuf, ip_id, srcport, dstport, latency);
}

/**
 * qdf_trace_dp_rx_pkt_enabled() - Get the dp_rx_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_rx_pkt_enabled(void)
{
	return __qdf_trace_dp_rx_pkt_enabled();
}

/**
 * qdf_trace_dp_rx_pkt() - Trace non-tcp/udp packet in rx direction
 * @nbuf: pointer to network buffer
 * @ether_type: type in ethernet header
 * @latency: latency in milliseconds
 *
 * Return: None
 */
static inline
void qdf_trace_dp_rx_pkt(qdf_nbuf_t nbuf, uint16_t ether_type,
			 uint64_t latency)
{
	__qdf_trace_dp_rx_pkt(nbuf, ether_type, latency);
}

/**
 * qdf_trace_dp_tx_comp_pkt_enabled() - Get the dp_tx_comp_pkt tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_tx_comp_pkt_enabled(void)
{
	return __qdf_trace_dp_tx_comp_pkt_enabled();
}

/**
 * qdf_trace_dp_tx_comp_pkt() - Trace non-tcp/udp packet in rx direction
 * @nbuf: pointer to network buffer
 * @ether_type: type in ethernet header
 * @latency: latency in milliseconds
 *
 * Return: None
 */
static inline
void qdf_trace_dp_tx_comp_pkt(qdf_nbuf_t nbuf, uint16_t ether_type,
			      uint64_t latency)
{
	__qdf_trace_dp_tx_comp_pkt(nbuf, ether_type, latency);
}

/**
 * qdf_trace_dp_del_reg_write_enabled() - Get the dp_del_reg_write tracepoint
 *  enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_del_reg_write_enabled(void)
{
	return __qdf_trace_dp_del_reg_write_enabled();
}

/**
 * qdf_trace_dp_del_reg_write() - Trace delayed register writes
 * @srng_id: srng id
 * @enq_val: enqueue value
 * @deq_val: dequeue value
 * @enq_time: enqueue time in qtimer ticks
 * @deq_time: dequeue time in qtimer ticks
 *
 * Return: None
 */
static inline
void qdf_trace_dp_del_reg_write(uint8_t srng_id, uint32_t enq_val,
				uint32_t deq_val, uint64_t enq_time,
				uint64_t deq_time)
{
	__qdf_trace_dp_del_reg_write(srng_id, enq_val, deq_val, enq_time,
				     deq_time);
}

/**
 * qdf_trace_dp_ce_tasklet_sched_lat_enabled() - Get the dp_ce_tasklet_sched_lat
 *  tracepoint enabled or disabled state
 *
 * Return: True if the tracepoint is enabled else false
 */
static inline
bool qdf_trace_dp_ce_tasklet_sched_latency_enabled(void)
{
	return __qdf_trace_dp_ce_tasklet_sched_latency_enabled();
}

/**
 * qdf_trace_dp_ce_tasklet_sched_latency() - Trace ce tasklet scheduling
 *  latency
 * @ce_id: ce id
 * @sched_latency: ce tasklet sched latency in nanoseconds
 *
 * Return: None
 */
static inline void
qdf_trace_dp_ce_tasklet_sched_latency(uint8_t ce_id, uint64_t sched_latency)
{
	__qdf_trace_dp_ce_tasklet_sched_latency(ce_id, sched_latency);
}
#endif /* _QDF_TRACEPOINT_H */
