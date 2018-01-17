/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#include "qdf_mem.h"
#include "qdf_status.h"
#include "qdf_trace.h"
#include "qdf_types.h"

static QDF_STATUS qdf_consume_char(const char **str, char c)
{
	if (*str[0] != c)
		return QDF_STATUS_E_FAILURE;

	(*str)++;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_whitespace(const char **str, char *c)
{
	switch (*str[0]) {
	case ' ':
	case '\t':
	case '\n':
	case '\r':
		*c = *str[0];
		(*str)++;
		return QDF_STATUS_SUCCESS;
	default:
		return QDF_STATUS_E_FAILURE;
	}
}

static void qdf_skip_whitespace(const char **str)
{
	QDF_STATUS status;
	char c;

	do {
		status = qdf_consume_whitespace(str, &c);
	} while (QDF_IS_STATUS_SUCCESS(status));
}

static QDF_STATUS qdf_consume_dec(const char **str, uint8_t *out_digit)
{
	uint8_t c = *str[0];

	if (c >= '0' && c <= '9')
		*out_digit = c - '0';
	else
		return QDF_STATUS_E_FAILURE;

	(*str)++;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_octet_dec(const char **str, uint8_t *out_octet)
{
	uint8_t len = 0;
	uint16_t octet = 0;
	int i;

	/* consume up to 3 decimal digits */
	for (i = 0; i < 3; i++) {
		uint8_t digit;

		if (QDF_IS_STATUS_ERROR(qdf_consume_dec(str, &digit)))
			break;

		len++;
		octet = octet * 10 + digit;
	}

	/* require at least 1 digit */
	if (!len)
		return QDF_STATUS_E_FAILURE;

	if (octet > 255) {
		(*str) -= len;
		return QDF_STATUS_E_FAILURE;
	}

	*out_octet = octet;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_hex(const char **str, uint8_t *out_nibble)
{
	uint8_t c = *str[0];

	if (c >= '0' && c <= '9')
		*out_nibble = c - '0';
	else if (c >= 'a' && c <= 'f')
		*out_nibble = c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		*out_nibble = c - 'A' + 10;
	else
		return QDF_STATUS_E_FAILURE;

	(*str)++;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_hex_pair(const char **str, uint8_t *out_byte)
{
	QDF_STATUS status;
	uint8_t hi, low;

	status = qdf_consume_hex(str, &hi);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = qdf_consume_hex(str, &low);
	if (QDF_IS_STATUS_ERROR(status)) {
		(*str)--;
		return status;
	}

	*out_byte = hi << 4 | low;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_hextet(const char **str, uint16_t *out_hextet)
{
	uint8_t len = 0;
	uint16_t hextet = 0;
	int i;

	/* consume up to 4 hex digits */
	for (i = 0; i < 4; i++) {
		uint8_t digit;

		if (QDF_IS_STATUS_ERROR(qdf_consume_hex(str, &digit)))
			break;

		len++;
		hextet = (hextet << 4) + digit;
	}

	/* require at least 1 digit */
	if (!len)
		return QDF_STATUS_E_FAILURE;

	/* no need to check for overflow */

	*out_hextet = hextet;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_mac_parse(const char *mac_str, struct qdf_mac_addr *out_addr)
{
	QDF_STATUS status;
	struct qdf_mac_addr addr;
	bool colons;
	int i;

	QDF_BUG(mac_str);
	if (!mac_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_addr);
	if (!out_addr)
		return QDF_STATUS_E_INVAL;

	qdf_skip_whitespace(&mac_str);

	/* parse leading hex pair */
	status = qdf_consume_hex_pair(&mac_str, &addr.bytes[0]);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	/* dynamically detect colons */
	colons = mac_str[0] == ':';

	for (i = 1; i < QDF_MAC_ADDR_SIZE; i++) {
		/* ensure colon separator if previously detected */
		if (colons) {
			status = qdf_consume_char(&mac_str, ':');
			if (QDF_IS_STATUS_ERROR(status))
				return status;
		}

		/* parse next hex pair */
		status = qdf_consume_hex_pair(&mac_str, &addr.bytes[i]);
		if (QDF_IS_STATUS_ERROR(status))
			return status;
	}

	qdf_skip_whitespace(&mac_str);
	if (mac_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	*out_addr = addr;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_ipv4_parse(const char *ipv4_str, struct qdf_ipv4_addr *out_addr)
{
	QDF_STATUS status;
	struct qdf_ipv4_addr addr;
	int i;

	QDF_BUG(ipv4_str);
	if (!ipv4_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_addr);
	if (!out_addr)
		return QDF_STATUS_E_INVAL;

	qdf_skip_whitespace(&ipv4_str);

	/* parse leading octet */
	status = qdf_consume_octet_dec(&ipv4_str, &addr.bytes[0]);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	for (i = 1; i < QDF_IPV4_ADDR_SIZE; i++) {
		/* ensure dot separator */
		status = qdf_consume_char(&ipv4_str, '.');
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		/* parse next octet */
		status = qdf_consume_octet_dec(&ipv4_str, &addr.bytes[i]);
		if (QDF_IS_STATUS_ERROR(status))
			return status;
	}

	qdf_skip_whitespace(&ipv4_str);
	if (ipv4_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	*out_addr = addr;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_ipv6_parse(const char *ipv6_str, struct qdf_ipv6_addr *out_addr)
{
	QDF_STATUS status;
	struct qdf_ipv6_addr addr;
	int8_t zero_comp = -1;
	uint8_t hextets_found = 0;

	QDF_BUG(ipv6_str);
	if (!ipv6_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_addr);
	if (!out_addr)
		return QDF_STATUS_E_INVAL;

	qdf_skip_whitespace(&ipv6_str);

	/* check for leading zero-compression ("::") */
	status = qdf_consume_char(&ipv6_str, ':');
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = qdf_consume_char(&ipv6_str, ':');
		if (QDF_IS_STATUS_SUCCESS(status))
			zero_comp = 0;
		else
			return QDF_STATUS_E_FAILURE;
	}

	while (hextets_found < QDF_IPV6_ADDR_HEXTET_COUNT) {
		uint16_t hextet;

		/* parse hextet */
		status = qdf_consume_hextet(&ipv6_str, &hextet);
		if (QDF_IS_STATUS_ERROR(status)) {
			/* we must end with hextet or zero compression */
			if (hextets_found != zero_comp)
				return QDF_STATUS_E_FAILURE;

			break;
		}

		addr.bytes[hextets_found * 2] = hextet >> 8;
		addr.bytes[hextets_found * 2 + 1] = hextet;
		hextets_found++;

		/* parse ':' char */
		status = qdf_consume_char(&ipv6_str, ':');
		if (QDF_IS_STATUS_ERROR(status))
			break;

		/* check for zero compression ("::") */
		status = qdf_consume_char(&ipv6_str, ':');
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/* only one zero compression is allowed */
			if (zero_comp >= 0)
				return QDF_STATUS_E_FAILURE;

			zero_comp = hextets_found;
		}
	}

	/* we must have max hextets or a zero compression */
	if (hextets_found < QDF_IPV6_ADDR_HEXTET_COUNT && zero_comp == -1)
		return QDF_STATUS_E_FAILURE;

	qdf_skip_whitespace(&ipv6_str);
	if (ipv6_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	/* shift lower hextets if zero compressed */
	if (zero_comp >= 0) {
		uint8_t shift = QDF_IPV6_ADDR_HEXTET_COUNT - hextets_found;
		void *to = &addr.bytes[(zero_comp + shift) * 2];
		void *from = &addr.bytes[zero_comp * 2];

		qdf_mem_move(to, from, (hextets_found - zero_comp) * 2);
		qdf_mem_set(from, shift * 2, 0);
	}

	*out_addr = addr;

	return QDF_STATUS_SUCCESS;
}

