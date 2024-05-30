/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains CoAP structure definitions
 */

#ifndef _WLAN_COAP_PUBLIC_STRUCTS_H_
#define _WLAN_COAP_PUBLIC_STRUCTS_H_
#include <qdf_types.h>

/**
 * struct coap_offload_reply_param - parameters to enable CoAP offload reply
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 * @cache_timeout: the cached packet expire timeout in ms
 * @src_ip_v4: source IPv4 address for sending reply message
 * @src_udp_port: source udp port for sending reply message
 * @dest_ip_v4: destination IPv4 address to match recived CoAP message
 * @dest_ip_v4_is_bc: indicate whether the destination address is broadcast
 * address or not
 * @dest_udp_port: destination UDP port to match recived CoAP message
 * @verify_offset: UDP payload offset to match recived CoAP message
 * @verify_len: UDP payload length to match recived CoAP message
 * @verify: pointer to binary data to match recived CoAP message
 * @coapmsg_len: CoAP reply message length
 * @coapmsg: pointer to CoAP reply message
 */
struct coap_offload_reply_param {
	uint32_t vdev_id;
	uint32_t pattern_id;
	uint32_t cache_timeout;
	uint32_t src_ip_v4;
	uint16_t src_udp_port;
	uint32_t dest_ip_v4;
	bool dest_ip_v4_is_bc;
	uint16_t dest_udp_port;
	uint32_t verify_offset;
	uint32_t verify_len;
	uint8_t *verify;
	uint32_t coapmsg_len;
	uint8_t *coapmsg;
};

/**
 * struct coap_offload_periodic_tx_param - parameters to enable CoAP offload
 * periodic tranmitting
 * @vdev_id: vdev id
 * @pattern_id: pattern id
 * @src_ip_v4: IPv4 address for sending CoAP message
 * @src_udp_port: source udp port for sending CoAP message
 * @dest_ip_v4: destination IPv4 address for sending CoAP message
 * @dest_ip_v4_is_bc: indicate whether the destination address is broadcast
 * address or not
 * @dest_udp_port: destination UDP port for sending CoAP message
 * @timeout: the periorid to send keepalive message in ms
 * @coapmsg_len: keeplive CoAP message length
 * @coapmsg: pointer to keeplive CoAP message
 */
struct coap_offload_periodic_tx_param {
	uint32_t vdev_id;
	uint32_t pattern_id;
	uint32_t src_ip_v4;
	uint16_t src_udp_port;
	uint32_t dest_ip_v4;
	bool dest_ip_v4_is_bc;
	uint16_t dest_udp_port;
	uint32_t timeout;
	uint32_t coapmsg_len;
	uint8_t *coapmsg;
};

/**
 * struct coap_buf_node - CoAP message info entry
 * @node: List entry element
 * @tsf: TSF of the CoAP meesage
 * @src_ip: source IPv4 address of the CoAP message
 * @len: length of the payload
 * @payload: pointer to buffer holding UDP payload of the CoAP message
 */
struct coap_buf_node {
	qdf_list_node_t node;
	uint64_t tsf;
	uint32_t src_ip;
	uint32_t len;
	uint8_t *payload;
};

/**
 * struct coap_buf_info - info of the cached CoAP messages
 * @vdev_id: vdev id
 * @req_id: request id
 * @more_info: flag to indicate whether there are more cached messages
 * @info_list: list to hold cached CoAP messages
 */
struct coap_buf_info {
	uint8_t vdev_id;
	uint32_t req_id;
	bool more_info;
	qdf_list_t info_list;
};

/**
 * typedef coap_cache_get_callback() - callback for getting cached CoAP messages
 * @context: context for getting cached CoAP messages
 * @info: pointer to info of the cached CoAP messages
 */
typedef void (*coap_cache_get_callback)(void *context,
					struct coap_buf_info *info);
#endif
