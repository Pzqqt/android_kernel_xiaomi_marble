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

/**
 * DOC: qdf_pkt_add_timestamp.h
 * This file defines HLOS agnostic functions providing external interface
 * for adding timestamp in packet payload.
 */

#if  !defined(_QDF_PKT_ADD_TS_H)
#define _QDF_PKT_ADD_TS_H

#include  <qdf_nbuf.h>
#include  <qdf_types.h>
#include  <qdf_trace.h>

/**
 * enum qdf_pkt_supported_proto - supported protocol for timestamp
 * @QDF_PKT_PROTO_INVAL - invalid
 * @QDF_PKT_PROTO_TCP - tcp protocol
 * @QDF_PKT_PROTO_UDP - udp protocol
 * @QDF_PKT_PROTO_MAX - max, keep it at last
 */
enum qdf_pkt_supported_proto {
	QDF_PKT_PROTO_INVAL,
	QDF_PKT_PROTO_TCP,
	QDF_PKT_PROTO_UDP,
	QDF_PKT_PROTO_MAX
};

/**
 * enum qdf_pkt_timestamp_index - index of different timestamp
 * @QDF_PKT_TX_DRIVER_ENTRY - tx driver entry timestamp
 * @QDF_PKT_TX_DRIVER_EXIT - tx driver exit timestamp
 * @QDF_PKT_RX_DRIVER_ENTRY - rx driver entry timestamp
 * @QDF_PKT_RX_DRIVER_EXIT - rx driver exit timestamp
 * @QDF_PKT_TIMESTAMP_MAX - maximum index, keep it at last
 */
enum qdf_pkt_timestamp_index {
	QDF_PKT_TX_DRIVER_ENTRY,
	QDF_PKT_TX_DRIVER_EXIT,
	QDF_PKT_RX_DRIVER_ENTRY,
	QDF_PKT_RX_DRIVER_EXIT,
	QDF_PKT_TIMESTAMP_MAX
};

#ifdef CONFIG_DP_PKT_ADD_TIMESTAMP

#define NUM_DP_PKT_TIMESTAMP_SUPPORT 4

struct ts_info {
	uint64_t sec;
	uint64_t usec;
} qdf_packed;

struct ts {
	struct ts_info ts_info[QDF_PKT_TIMESTAMP_MAX];
} qdf_packed;

#define QDF_PKT_PROTO_TCP_BIT	(1 << QDF_PKT_PROTO_TCP)
#define QDF_PKT_PROTO_UDP_BIT	(1 << QDF_PKT_PROTO_UDP)

struct dp_pkt_proto_info {
	enum qdf_pkt_supported_proto proto;
	uint16_t port;
	uint16_t offset;
};

struct dp_pkt_add_ts_info {
	uint8_t current_index;
	uint16_t enable_protocol_bitmap;
	struct dp_pkt_proto_info proto_info[NUM_DP_PKT_TIMESTAMP_SUPPORT];
};

/**
 * qdf_set_dp_pkt_add_ts_info() - set protocol/port and offset info
 *
 * @proto: protocol to timestamp
 * @port: destination port of protocol
 * @offset: offset in payload
 *
 * Return: 0 for success
 */
int qdf_set_dp_pkt_add_ts_info(enum qdf_pkt_supported_proto proto,
			       uint16_t port, uint16_t offset);

/**
 * qdf_clear_dp_pkt_add_ts_info() - clear all timestamp info
 *
 * Return: none
 */
void qdf_clear_dp_pkt_add_ts_info(void);

/**
 * qdf_show_dp_pkt_add_ts_info() - Update buffer with configured information
 *
 * @buf: buffer pointer to get information
 * @size: size of the buffer
 *
 * Return: number of bytes update in buffer
 */
int qdf_show_dp_pkt_add_ts_info(char *buf, size_t size);

/**
 * qdf_add_dp_pkt_timestamp() - add timestamp in data payload
 *
 * @nbuf: network buffer
 * @index: this decides offset in payload
 * @time: timestamp to update
 *
 * Return: none
 */
void qdf_add_dp_pkt_timestamp(qdf_nbuf_t nbuf,
			      enum qdf_pkt_timestamp_index index,
			      uint64_t time);

/**
 * qdf_is_dp_pkt_timestamp_enabled() - check if packet timestamping is enabled
 *
 * Return: true/false
 */
bool qdf_is_dp_pkt_timestamp_enabled(void);
#else

static inline
int qdf_set_dp_pkt_add_ts_info(enum qdf_pkt_supported_proto proto,
			       uint16_t port, uint16_t offset)
{
	return 0;
}

static inline
void qdf_clear_dp_pkt_add_ts_info(void)
{
}

static inline
int qdf_show_dp_pkt_add_ts_info(char *buf, size_t size)
{
	return 0;
}

static inline
void qdf_add_dp_pkt_timestamp(qdf_nbuf_t nbuf,
			      enum qdf_pkt_timestamp_index index, uint64_t time)
{
}

static inline
bool qdf_is_dp_pkt_timestamp_enabled(void)
{
	return false;
}

#endif
#endif
