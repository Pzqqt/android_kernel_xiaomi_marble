/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <qdf_pkt_add_timestamp.h>

static struct dp_pkt_add_ts_info dp_pkt_ts_info;
static int dp_pkt_add_timestamp;

int qdf_set_dp_pkt_add_ts_info(enum qdf_pkt_supported_proto proto,
			       uint16_t port, uint16_t offset)
{
	if (dp_pkt_ts_info.current_index >= NUM_DP_PKT_TIMESTAMP_SUPPORT) {
		qdf_err("Only %d number of protocol supported",
			NUM_DP_PKT_TIMESTAMP_SUPPORT);
		return -EINVAL;
	}

	dp_pkt_ts_info.enable_protocol_bitmap |= (1 << proto);
	dp_pkt_ts_info.proto_info[dp_pkt_ts_info.current_index].proto = proto;
	dp_pkt_ts_info.proto_info[dp_pkt_ts_info.current_index].port = port;
	dp_pkt_ts_info.proto_info[dp_pkt_ts_info.current_index++].offset =
									offset;
	dp_pkt_add_timestamp = 1;
	return 0;
}

void qdf_clear_dp_pkt_add_ts_info(void)
{
	dp_pkt_add_timestamp = 0;
	qdf_mem_zero(&dp_pkt_ts_info, sizeof(dp_pkt_ts_info));
}

static
const char *qdf_get_proto_str(enum qdf_pkt_supported_proto proto)
{
	switch (proto) {
	case QDF_PKT_PROTO_TCP:
		return "TCP";
	case QDF_PKT_PROTO_UDP:
		return "UDP";
	default:
		return "Invalid";
	}
}

int qdf_show_dp_pkt_add_ts_info(char *buf, size_t size)
{
	int i;
	int cnt = 0;

	qdf_debug("dp_pkt_add_timestamp %d", dp_pkt_add_timestamp);
	qdf_debug("enable_protocol_bitmap %x",
		  dp_pkt_ts_info.enable_protocol_bitmap);
	qdf_debug("current_index %d", dp_pkt_ts_info.current_index);

	cnt += scnprintf(buf + cnt, size - cnt, "dp_pkt_add_timestamp %d\n",
			 dp_pkt_add_timestamp);
	for (i = 0; i < dp_pkt_ts_info.current_index; i++) {
		qdf_debug("proto %d port %d offset %d",
			  dp_pkt_ts_info.proto_info[i].proto,
			  dp_pkt_ts_info.proto_info[i].port,
			  dp_pkt_ts_info.proto_info[i].offset);
		if (size - cnt <= 0)
			continue;
		cnt += scnprintf(buf + cnt, size - cnt,
				 "Protocol: %s Destination Port %d Offset %d\n",
				 qdf_get_proto_str(
					 dp_pkt_ts_info.proto_info[i].proto),
				 dp_pkt_ts_info.proto_info[i].port,
				 dp_pkt_ts_info.proto_info[i].offset);
	}
	return cnt;
}

bool qdf_is_dp_pkt_timestamp_enabled(void)
{
	return dp_pkt_add_timestamp;
}

static inline
uint32_t qdf_get_tcp_offset(qdf_nbuf_t nbuf, uint16_t offset)
{
	uint16_t ip_header_len, tcp_header_len, tcp_header_off;
	uint8_t *skb_data = (uint8_t *)qdf_nbuf_data(nbuf);

	ip_header_len = ((uint8_t)(*(uint8_t *)
				(skb_data + QDF_NBUF_TRAC_IPV4_OFFSET)) &
			QDF_NBUF_TRAC_IPV4_HEADER_MASK) << 2;
	tcp_header_off = QDF_NBUF_TRAC_IPV4_OFFSET + ip_header_len;
	tcp_header_len = ((uint8_t)(*(uint8_t *)
				(skb_data + tcp_header_off +
				 QDF_NBUF_TRAC_TCP_HEADER_LEN_OFFSET))) >> 2;
	return tcp_header_off + tcp_header_len + offset;
}

static inline
uint32_t qdf_get_udp_offset(qdf_nbuf_t nbuf, uint16_t offset)
{
	uint16_t ip_header_len, udp_header_len;
	uint8_t *skb_data = (uint8_t *)qdf_nbuf_data(nbuf);

	ip_header_len = ((uint8_t)(*(uint8_t *)
				(skb_data + QDF_NBUF_TRAC_IPV4_OFFSET)) &
			QDF_NBUF_TRAC_IPV4_HEADER_MASK) << 2;
	udp_header_len = 8;
	return  QDF_NBUF_TRAC_IPV4_OFFSET + ip_header_len +
		udp_header_len + offset;
}

static inline
void qdf_add_ts(qdf_nbuf_t nbuf, uint32_t offset,
		enum qdf_pkt_timestamp_index index, uint64_t time,
		enum qdf_pkt_supported_proto proto)
{
	struct ts *ts_ptr;
	struct ts_info *ts_info;
	uint32_t total_offset;

	if (proto == QDF_PKT_PROTO_TCP)
		total_offset = qdf_get_tcp_offset(nbuf, offset);
	else if (proto == QDF_PKT_PROTO_UDP)
		total_offset = qdf_get_udp_offset(nbuf, offset);
	else
		return;

	if (qdf_nbuf_len(nbuf) < total_offset + sizeof(struct ts))
		return;

	ts_ptr = (struct ts *)(qdf_nbuf_data(nbuf) + total_offset);
	ts_info = &ts_ptr->ts_info[index];

	ts_info->sec = time / 1000000;
	ts_info->usec = time % 1000000;
}

void qdf_add_dp_pkt_timestamp(qdf_nbuf_t nbuf,
			      enum qdf_pkt_timestamp_index index, uint64_t time)
{
	int i;
	uint16_t port;
	uint32_t offset;

	if (dp_pkt_ts_info.enable_protocol_bitmap & QDF_PKT_PROTO_TCP_BIT) {
		if (qdf_nbuf_is_ipv4_tcp_pkt(nbuf)) {
			port =
			    QDF_SWAP_U16(qdf_nbuf_data_get_tcp_dst_port(nbuf));
			for (i = 0; i < dp_pkt_ts_info.current_index; i++) {
				offset =  dp_pkt_ts_info.proto_info[i].offset;
				if (dp_pkt_ts_info.proto_info[i].proto ==
				    QDF_PKT_PROTO_TCP &&
				    dp_pkt_ts_info.proto_info[i].port == port) {
					qdf_add_ts(nbuf, offset, index, time,
						   QDF_PKT_PROTO_TCP);
					break;
				}
			}
			return;
		}
	}

	if (dp_pkt_ts_info.enable_protocol_bitmap & QDF_PKT_PROTO_UDP_BIT) {
		if (qdf_nbuf_is_ipv4_udp_pkt(nbuf)) {
			port =
			    QDF_SWAP_U16(qdf_nbuf_data_get_tcp_dst_port(nbuf));
			for (i = 0; i < dp_pkt_ts_info.current_index; i++) {
				offset =  dp_pkt_ts_info.proto_info[i].offset;
				if (dp_pkt_ts_info.proto_info[i].proto ==
				    QDF_PKT_PROTO_UDP &&
				    dp_pkt_ts_info.proto_info[i].port == port) {
					qdf_add_ts(nbuf, offset, index, time,
						   QDF_PKT_PROTO_UDP);
					break;
				}
			}
			return;
		}
	}
}
