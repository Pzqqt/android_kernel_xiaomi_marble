/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2018-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#include <linux/version.h>
#undef TRACE_SYSTEM
#define TRACE_SYSTEM rmnet
#undef TRACE_INCLUDE_PATH
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,5,0)
#ifdef CONFIG_ARCH_SDXNIGHTJAR
#define TRACE_INCLUDE_PATH ../../../../../../../datarmnet/core
#else
#define TRACE_INCLUDE_PATH ../../../../../../../src/datarmnet/core
#endif /* endif LINUX_VERSION_CODE < KERNEL_VERSION(5,5,0) */
#else
#define TRACE_INCLUDE_PATH ../../../../vendor/qcom/opensource/datarmnet/core
#endif
#define TRACE_INCLUDE_FILE rmnet_trace

#if !defined(_TRACE_RMNET_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RMNET_H

#include <linux/skbuff.h>
#include <linux/tracepoint.h>
#include <linux/timekeeping.h>
/*****************************************************************************/
/* Trace events for rmnet module */
/*****************************************************************************/
DECLARE_EVENT_CLASS(rmnet_skb_time_template,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb),

	TP_STRUCT__entry(
		__field(void *,	skbaddr)
		__field(unsigned int, len)
		__string(name,	skb->dev->name)
		__field(u64,	qtime)

	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->len = skb->len;
		__assign_str(name, skb->dev->name);
		__entry->qtime = ktime_get_raw_ns();
	),

	TP_printk("dev=%s skbaddr=%pK len=%u UTC time %ld",
		  __get_str(name), __entry->skbaddr, __entry->len,
		   __entry->qtime)
);

DEFINE_EVENT
	(rmnet_skb_time_template, rmnet_skb_ip_route_entry,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb)
);

DEFINE_EVENT
	(rmnet_skb_time_template, rmnet_skb_ip_route_exit,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb)
);

DEFINE_EVENT
	(rmnet_skb_time_template, rmnet_skb_egress_entry,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb)
);

DEFINE_EVENT
	(rmnet_skb_time_template, rmnet_skb_egress_exit,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb)
);


TRACE_EVENT(rmnet_xmit_skb,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb),

	TP_STRUCT__entry(
		__string(dev_name, skb->dev->name)
		__field(unsigned int, len)
	),

	TP_fast_assign(
		__assign_str(dev_name, skb->dev->name);
		__entry->len = skb->len;
	),

	TP_printk("dev_name=%s len=%u", __get_str(dev_name), __entry->len)
);

DECLARE_EVENT_CLASS
	(rmnet_mod_template,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2),

	 TP_STRUCT__entry(__field(u8, func)
			  __field(u8, evt)
			  __field(u32, uint1)
			  __field(u32, uint2)
			  __field(u64, ulong1)
			  __field(u64, ulong2)
			  __field(void *, ptr1)
			  __field(void *, ptr2)
	 ),

	 TP_fast_assign(__entry->func = func;
			__entry->evt = evt;
			__entry->uint1 = uint1;
			__entry->uint2 = uint2;
			__entry->ulong1 = ulong1;
			__entry->ulong2 = ulong2;
			__entry->ptr1 = ptr1;
			__entry->ptr2 = ptr2;
	 ),

TP_printk("fun:%u ev:%u u1:%u u2:%u ul1:%llu ul2:%llu p1:0x%pK p2:0x%pK",
	  __entry->func, __entry->evt,
	  __entry->uint1, __entry->uint2,
	  __entry->ulong1, __entry->ulong2,
	  __entry->ptr1, __entry->ptr2)
)

DEFINE_EVENT
	(rmnet_mod_template, rmnet_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

TRACE_EVENT(print_skb_gso,

	TP_PROTO(struct sk_buff *skb, __be16 src, __be16 dest,
		 u16 ip_proto, u16 xport_proto, const char *saddr, const char *daddr),

	TP_ARGS(skb, src, dest, ip_proto, xport_proto, saddr, daddr),

	TP_STRUCT__entry(
		__field(void *, skbaddr)
		__field(int, len)
		__field(int, data_len)
		__field(__be16, src)
		__field(__be16, dest)
		__field(u16, ip_proto)
		__field(u16, xport_proto)
		__string(saddr, saddr)
		__string(daddr, daddr)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->len = skb->len;
		__entry->data_len = skb->data_len;
		__entry->src = src;
		__entry->dest = dest;
		__entry->ip_proto = ip_proto;
		__entry->xport_proto = xport_proto;
		__assign_str(saddr, saddr);
		__assign_str(daddr, daddr);
	),

	TP_printk("GSO: skbaddr=%pK, len=%d, data_len=%d, [%s][%s] src=%s %u dest=%s %u",
		__entry->skbaddr, __entry->len, __entry->data_len,
		__entry->ip_proto == htons(ETH_P_IP) ? "IPv4" : "IPv6",
		__entry->xport_proto == IPPROTO_TCP ? "TCP" : "UDP",
		__get_str(saddr), be16_to_cpu(__entry->src),
		__get_str(daddr), be16_to_cpu(__entry->dest))
);

DECLARE_EVENT_CLASS(print_icmp,

	TP_PROTO(struct sk_buff *skb, u16 ip_proto, u8 type, __be16 sequence,
		const char *saddr, const char *daddr),

	TP_ARGS(skb, ip_proto, type, sequence, saddr, daddr),

	TP_STRUCT__entry(
		__field(void *, skbaddr)
		__field(int, len)
		__field(u16, ip_proto)
		__field(u8, type)
		__field(__be16, sequence)
		__string(saddr, saddr)
		__string(daddr, daddr)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->len = skb->len;
		__entry->ip_proto = ip_proto;
		__entry->type = type;
		__entry->sequence = sequence;
		__assign_str(saddr, saddr);
		__assign_str(daddr, daddr);
	),

	TP_printk("ICMP: skbaddr=%pK, len=%d, [%s] type=%u sequence=%u source=%s dest=%s",
		__entry->skbaddr, __entry->len,
		__entry->ip_proto == htons(ETH_P_IP) ? "IPv4" : "IPv6",
		__entry->type, be16_to_cpu(__entry->sequence), __get_str(saddr),
		__get_str(daddr))
);

DEFINE_EVENT
	(print_icmp, print_icmp_tx,

	TP_PROTO(struct sk_buff *skb, u16 ip_proto, u8 type, __be16 sequence,
		const char *saddr, const char *daddr),

	TP_ARGS(skb, ip_proto, type, sequence, saddr, daddr)
);

DEFINE_EVENT
	(print_icmp, print_icmp_rx,

	TP_PROTO(struct sk_buff *skb, u16 ip_proto, u8 type, __be16 sequence,
		const char *saddr, const char *daddr),

	TP_ARGS(skb, ip_proto, type, sequence, saddr, daddr)
);

DECLARE_EVENT_CLASS(print_tcp,

	TP_PROTO(struct sk_buff *skb, const char *saddr, const char *daddr,
		 struct tcphdr *tp),

	TP_ARGS(skb, saddr, daddr, tp),

	TP_STRUCT__entry(
		__field(void *, skbaddr)
		__field(int, len)
		__string(saddr, saddr)
		__string(daddr, daddr)
		__field(__be16, source)
		__field(__be16, dest)
		__field(__be32, seq)
		__field(__be32, ack_seq)
		__field(u8, syn)
		__field(u8, ack)
		__field(u8, fin)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->len = skb->len;
		__assign_str(saddr, saddr);
		__assign_str(daddr, daddr);
		__entry->source = tp->source;
		__entry->dest = tp->dest;
		__entry->seq = tp->seq;
		__entry->ack_seq = tp->ack_seq;
		__entry->syn = tp->syn;
		__entry->ack = tp->ack;
		__entry->fin = tp->fin;
	),

	TP_printk("TCP: skbaddr=%pK, len=%d source=%s %u dest=%s %u seq=%u ack_seq=%u syn=%u ack=%u fin=%u",
		__entry->skbaddr, __entry->len,
		__get_str(saddr), be16_to_cpu(__entry->source),
		__get_str(daddr), be16_to_cpu(__entry->dest),
		be32_to_cpu(__entry->seq), be32_to_cpu(__entry->ack_seq),
		!!__entry->syn, !!__entry->ack, !!__entry->fin)
);

DEFINE_EVENT
	(print_tcp, print_tcp_tx,

	TP_PROTO(struct sk_buff *skb, const char *saddr, const char *daddr,
		 struct tcphdr *tp),

	TP_ARGS(skb, saddr, daddr, tp)
);

DEFINE_EVENT
	(print_tcp, print_tcp_rx,

	TP_PROTO(struct sk_buff *skb, const char *saddr, const char *daddr,
		 struct tcphdr *tp),

	TP_ARGS(skb, saddr, daddr, tp)
);

DECLARE_EVENT_CLASS(print_udp,

	TP_PROTO(struct sk_buff *skb, const char *saddr, const char *daddr,
		 struct udphdr *uh),

	TP_ARGS(skb, saddr, daddr, uh),

	TP_STRUCT__entry(
		__field(void *, skbaddr)
		__field(int, len)
		__string(saddr, saddr)
		__string(daddr, daddr)
		__field(__be16, source)
		__field(__be16, dest)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->len = skb->len;
		__assign_str(saddr, saddr);
		__assign_str(daddr, daddr);
		__entry->source = uh->source;
		__entry->dest = uh->dest;
	),

	TP_printk("UDP: skbaddr=%pK, len=%d source=%s %u dest=%s %u",
		__entry->skbaddr, __entry->len,
		__get_str(saddr), be16_to_cpu(__entry->source),
		__get_str(daddr), be16_to_cpu(__entry->dest))
);

DEFINE_EVENT
	(print_udp, print_udp_tx,

	TP_PROTO(struct sk_buff *skb, const char *saddr, const char *daddr,
		 struct udphdr *uh),

	TP_ARGS(skb, saddr, daddr, uh)
);

DEFINE_EVENT
	(print_udp, print_udp_rx,

	TP_PROTO(struct sk_buff *skb, const char *saddr, const char *daddr,
		 struct udphdr *uh),

	TP_ARGS(skb, saddr, daddr, uh)
);

/*****************************************************************************/
/* Trace events for rmnet_perf module */
/*****************************************************************************/
DEFINE_EVENT
	(rmnet_mod_template, rmnet_perf_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_perf_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_perf_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)

);

/*****************************************************************************/
/* Trace events for rmnet_shs module */
/*****************************************************************************/
DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)
);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)
);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)
);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_wq_low,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)
);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_wq_high,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)
);

DEFINE_EVENT
	(rmnet_mod_template, rmnet_shs_wq_err,

	 TP_PROTO(u8 func, u8 evt, u32 uint1, u32 uint2,
		  u64 ulong1, u64 ulong2, void *ptr1, void *ptr2),

	 TP_ARGS(func, evt, uint1, uint2, ulong1, ulong2, ptr1, ptr2)
);

DECLARE_EVENT_CLASS
	(rmnet_freq_template,

	 TP_PROTO(u8 core, u32 newfreq),

	 TP_ARGS(core, newfreq),

	 TP_STRUCT__entry(__field(u8, core)
			  __field(u32, newfreq)
	 ),

	 TP_fast_assign(__entry->core = core;
			__entry->newfreq = newfreq;
	 ),

TP_printk("freq policy core:%u freq floor :%u",
	  __entry->core, __entry->newfreq)

);

DEFINE_EVENT
	(rmnet_freq_template, rmnet_freq_boost,

	 TP_PROTO(u8 core, u32 newfreq),

	 TP_ARGS(core, newfreq)
);

DEFINE_EVENT
	(rmnet_freq_template, rmnet_freq_reset,

	 TP_PROTO(u8 core, u32 newfreq),

	 TP_ARGS(core, newfreq)
);

TRACE_EVENT
	(rmnet_freq_update,

	 TP_PROTO(u8 core, u32 lowfreq, u32 highfreq),

	 TP_ARGS(core, lowfreq, highfreq),

	 TP_STRUCT__entry(__field(u8, core)
			  __field(u32, lowfreq)
			  __field(u32, highfreq)
	 ),

	 TP_fast_assign(__entry->core = core;
			__entry->lowfreq = lowfreq;
			__entry->highfreq = highfreq;

	 ),

TP_printk("freq policy update core:%u policy freq floor :%u freq ceil :%u",
	  __entry->core, __entry->lowfreq, __entry->highfreq)
);
#endif /* _TRACE_RMNET_H */

#include <trace/define_trace.h>
