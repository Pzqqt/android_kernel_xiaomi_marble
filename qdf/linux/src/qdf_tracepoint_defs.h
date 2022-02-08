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
 * DOC: qdf_tracepoint_defs.h
 * This file provides OS abstraction for function tracing.
 */

#if  !defined(_QDF_TRACEPOINT_DEFS_H) || defined(TRACE_HEADER_MULTI_READ)
#define _QDF_TRACEPOINT_DEFS_H

#include <linux/tracepoint.h>
#include <linux/skbuff.h>

#undef TRACE_SYSTEM
#define TRACE_SYSTEM wlan

#ifndef WLAN_TRACEPOINTS
#undef TRACE_EVENT
#define TRACE_EVENT(name, proto, args, tstruct, assign, print) \
	static inline void trace_##name(proto) {}              \
	static inline bool trace_##name##_enabled(void)        \
	{                                                      \
		return false;                                  \
	}

#undef DECLARE_EVENT_CLASS
#define DECLARE_EVENT_CLASS(name, proto, args, tstruct, assign, print)

#undef DEFINE_EVENT
#define DEFINE_EVENT(evt_class, name, proto, args)      \
	static inline void trace_##name(proto) {}       \
	static inline bool trace_##name##_enabled(void) \
	{                                               \
		return false;                           \
	}
#endif /* WLAN_TRACEPOINTS */

DECLARE_EVENT_CLASS(dp_trace_tcp_pkt_class,
		    TP_PROTO(struct sk_buff *skb, uint32_t tcp_seq_num,
			     uint32_t tcp_ack_num, uint16_t srcport,
			     uint16_t dstport, uint64_t tdelta),
		    TP_ARGS(skb, tcp_seq_num, tcp_ack_num, srcport,
			    dstport, tdelta),
		    TP_STRUCT__entry(
			__field(void *, skb)
			__field(uint32_t, tcp_seq_num)
			__field(uint32_t, tcp_ack_num)
			__field(uint16_t, srcport)
			__field(uint16_t, dstport)
			__field(uint64_t, tdelta)
		    ),
		    TP_fast_assign(
			__entry->skb = skb;
			__entry->tcp_seq_num = tcp_seq_num;
			__entry->tcp_ack_num = tcp_ack_num;
			__entry->srcport = srcport;
			__entry->dstport = dstport;
			__entry->tdelta = tdelta;
		    ),
		    TP_printk("skb=%pK seqnum=%u acknum=%u srcport=%u dstport=%u latency(ms)=%llu",
			      __entry->skb, __entry->tcp_seq_num,
			      __entry->tcp_ack_num, __entry->srcport,
			      __entry->dstport, __entry->tdelta)
);

DEFINE_EVENT(dp_trace_tcp_pkt_class, dp_rx_tcp_pkt,
	     TP_PROTO(struct sk_buff *skb, uint32_t tcp_seq_num,
		      uint32_t tcp_ack_num, uint16_t srcport,
		      uint16_t dstport, uint64_t tdelta),
	     TP_ARGS(skb, tcp_seq_num, tcp_ack_num, srcport, dstport, tdelta)
);

DEFINE_EVENT(dp_trace_tcp_pkt_class, dp_tx_comp_tcp_pkt,
	     TP_PROTO(struct sk_buff *skb, uint32_t tcp_seq_num,
		      uint32_t tcp_ack_num, uint16_t srcport,
		      uint16_t dstport, uint64_t tdelta),
	     TP_ARGS(skb, tcp_seq_num, tcp_ack_num, srcport, dstport, tdelta)
);

DECLARE_EVENT_CLASS(dp_trace_udp_pkt_class,
		    TP_PROTO(struct sk_buff *skb, uint16_t ip_id,
			     uint16_t srcport, uint16_t dstport,
			     uint64_t tdelta),
		    TP_ARGS(skb, ip_id, srcport, dstport, tdelta),
		    TP_STRUCT__entry(
			__field(void *, skb)
			__field(uint16_t, ip_id)
			__field(uint16_t, srcport)
			__field(uint16_t, dstport)
			__field(uint64_t, tdelta)
		    ),
		    TP_fast_assign(
			__entry->skb = skb;
			__entry->ip_id = ip_id;
			__entry->srcport = srcport;
			__entry->dstport = dstport;
			__entry->tdelta = tdelta;
		    ),
		    TP_printk("skb=%pK ip_id=%u srcport=%u dstport=%d latency(ms)=%llu",
			      __entry->skb, __entry->ip_id,
			      __entry->srcport, __entry->dstport,
			      __entry->tdelta)
);

DEFINE_EVENT(dp_trace_udp_pkt_class, dp_rx_udp_pkt,
	     TP_PROTO(struct sk_buff *skb, uint16_t ip_id, uint16_t srcport,
		      uint16_t dstport, uint64_t tdelta),
	     TP_ARGS(skb, ip_id, srcport, dstport, tdelta)
);

DEFINE_EVENT(dp_trace_udp_pkt_class, dp_tx_comp_udp_pkt,
	     TP_PROTO(struct sk_buff *skb, uint16_t ip_id, uint16_t srcport,
		      uint16_t dstport, uint64_t tdelta),
	     TP_ARGS(skb, ip_id, srcport, dstport, tdelta)
);

DECLARE_EVENT_CLASS(dp_trace_generic_ip_pkt_class,
		    TP_PROTO(struct sk_buff *skb, uint8_t ip_proto,
			     uint16_t ip_id, uint32_t trans_hdr_4_bytes,
			     uint64_t tdelta),
		    TP_ARGS(skb, ip_proto, ip_id,
			    trans_hdr_4_bytes, tdelta),
		    TP_STRUCT__entry(
			__field(void *, skb)
			__field(uint8_t, ip_proto)
			__field(uint16_t, ip_id)
			__field(uint32_t, trans_hdr_4_bytes)
			__field(uint64_t, tdelta)
		    ),
		    TP_fast_assign(
			__entry->skb = skb;
			__entry->ip_proto = ip_proto;
			__entry->ip_id = ip_id;
			__entry->trans_hdr_4_bytes = trans_hdr_4_bytes;
			__entry->tdelta = tdelta;
		    ),
		    TP_printk("skb=%pK ip_proto=0x%x ip_id=0x%x, transport_hdr[4]:0x%08x, latency(ms)=%llu",
			      __entry->skb, __entry->ip_proto,
			      __entry->ip_id,  __entry->trans_hdr_4_bytes,
			      __entry->tdelta)
);

DEFINE_EVENT(dp_trace_generic_ip_pkt_class, dp_rx_generic_ip_pkt,
	     TP_PROTO(struct sk_buff *skb, uint8_t ip_proto,
		      uint16_t ip_id, uint32_t trans_hdr_4_bytes,
		      uint64_t tdelta),
	     TP_ARGS(skb, ip_proto, ip_id,
		     trans_hdr_4_bytes,
		     tdelta)
);

DEFINE_EVENT(dp_trace_generic_ip_pkt_class, dp_tx_comp_generic_ip_pkt,
	     TP_PROTO(struct sk_buff *skb, uint8_t ip_proto,
		      uint16_t ip_id, uint32_t trans_hdr_4_bytes,
		      uint64_t tdelta),
	     TP_ARGS(skb, ip_proto, ip_id,
		     trans_hdr_4_bytes,
		     tdelta)
);

DECLARE_EVENT_CLASS(dp_trace_pkt_class,
		    TP_PROTO(struct sk_buff *skb, uint16_t ether_type,
			     uint64_t tdelta),
		    TP_ARGS(skb, ether_type, tdelta),
		    TP_STRUCT__entry(
			__field(void *, skb)
			__field(uint16_t, ether_type)
			__field(uint64_t, tdelta)
		    ),
		    TP_fast_assign(
			__entry->skb = skb;
			__entry->ether_type = ether_type;
			__entry->tdelta = tdelta;
		    ),
		    TP_printk("skb=%pK ether_type=0x%x latency(ms)=%llu",
			      __entry->skb, __entry->ether_type,
			      __entry->tdelta)
);

DEFINE_EVENT(dp_trace_pkt_class, dp_rx_pkt,
	     TP_PROTO(struct sk_buff *skb, uint16_t ether_type,
		      uint64_t tdelta),
	     TP_ARGS(skb, ether_type, tdelta)
);

DEFINE_EVENT(dp_trace_pkt_class, dp_tx_comp_pkt,
	     TP_PROTO(struct sk_buff *skb, uint16_t ether_type,
		      uint64_t tdelta),
	     TP_ARGS(skb, ether_type, tdelta)
);

TRACE_EVENT(dp_del_reg_write,
	    TP_PROTO(uint8_t srng_id, uint32_t enq_val, uint32_t deq_val,
		     uint64_t enq_time, uint64_t deq_time),
	    TP_ARGS(srng_id, enq_val, deq_val, enq_time, deq_time),
	    TP_STRUCT__entry(
		__field(uint8_t, srng_id)
		__field(uint32_t, enq_val)
		__field(uint32_t, deq_val)
		__field(uint64_t, enq_time)
		__field(uint64_t, deq_time)
	    ),
	    TP_fast_assign(
		__entry->srng_id = srng_id;
		__entry->enq_val = enq_val;
		__entry->deq_val = deq_val;
		__entry->enq_time = enq_time;
		__entry->deq_time = deq_time;
	    ),
	    TP_printk("srng_id=%u enq_val=%u deq_val=%u enq_time=0x%llx deq_time=0x%llx",
		      __entry->srng_id, __entry->enq_val, __entry->deq_val,
		      __entry->enq_time, __entry->deq_time)
);

TRACE_EVENT(dp_ce_tasklet_sched_latency,
	    TP_PROTO(uint8_t ce_id, uint64_t sched_latency),
	    TP_ARGS(ce_id, sched_latency),
	    TP_STRUCT__entry(
		__field(uint8_t, ce_id)
		__field(uint64_t, sched_latency)
	    ),
	    TP_fast_assign(
		__entry->ce_id = ce_id;
		__entry->sched_latency = sched_latency;
	    ),
	    TP_printk("ce_id=%u latency(ns)=%llu", __entry->ce_id,
		      __entry->sched_latency)
);
#endif /* _QDF_TRACEPOINT_DEFS_H */

/* Below should be outside the protection */
#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE qdf_tracepoint_defs
#include <trace/define_trace.h>
