/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
#include "qdf_module.h"
#include "qdf_status.h"
#include "qdf_str.h"
#include "qdf_trace.h"
#include "qdf_types.h"

const char *qdf_opmode_str(const enum QDF_OPMODE opmode)
{
	switch (opmode) {
	case QDF_STA_MODE:
		return "STA";
	case QDF_SAP_MODE:
		return "SAP";
	case QDF_P2P_CLIENT_MODE:
		return "P2P Client";
	case QDF_P2P_GO_MODE:
		return "P2P GO";
	case QDF_FTM_MODE:
		return "FTM";
	case QDF_IBSS_MODE:
		return "IBSS";
	case QDF_MONITOR_MODE:
		return "Monitor";
	case QDF_P2P_DEVICE_MODE:
		return "P2P Device";
	case QDF_OCB_MODE:
		return "OCB";
	case QDF_EPPING_MODE:
		return "EPPing";
	case QDF_QVIT_MODE:
		return "QVIT";
	case QDF_NDI_MODE:
		return "NDI";
	case QDF_WDS_MODE:
		return "WDS";
	case QDF_BTAMP_MODE:
		return "BTAMP";
	case QDF_AHDEMO_MODE:
		return "AHDEMO";
	case QDF_TDLS_MODE:
		return "TDLS";
	case QDF_NAN_DISC_MODE:
		return "NAN";
	default:
		return "Invalid operating mode";
	}
}

static QDF_STATUS qdf_consume_char(const char **str, char c)
{
	if ((*str)[0] != c)
		return QDF_STATUS_E_FAILURE;

	(*str)++;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_dec(const char **str, uint8_t *out_digit)
{
	uint8_t c = (*str)[0];

	if (c >= '0' && c <= '9')
		*out_digit = c - '0';
	else
		return QDF_STATUS_E_FAILURE;

	(*str)++;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS qdf_consume_hex(const char **str, uint8_t *out_nibble)
{
	uint8_t c = (*str)[0];

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

static QDF_STATUS qdf_consume_radix(const char **str, uint8_t *out_radix)
{
	if ((*str)[0] == '0') {
		switch ((*str)[1]) {
		case 'b':
			*out_radix = 2;
			*str += 2;
			break;
		case 'o':
			*out_radix = 8;
			*str += 2;
			break;
		case 'x':
			*out_radix = 16;
			*str += 2;
			break;
		default:
			*out_radix = 10;
			break;
		}

		return QDF_STATUS_SUCCESS;
	}

	if (*str[0] >= '0' && *str[0] <= '9') {
		*out_radix = 10;
		return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

static QDF_STATUS
__qdf_int_parse_lazy(const char **int_str, uint64_t *out_int, bool *out_negate)
{
	QDF_STATUS status;
	bool negate = false;
	uint8_t radix;
	uint8_t digit;
	uint64_t value = 0;
	uint64_t next_value;
	const char *str = *int_str;

	str = qdf_str_left_trim(str);

	status = qdf_consume_char(&str, '-');
	if (QDF_IS_STATUS_SUCCESS(status))
		negate = true;
	else
		qdf_consume_char(&str, '+');

	status = qdf_consume_radix(&str, &radix);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	while (QDF_IS_STATUS_SUCCESS(qdf_consume_hex(&str, &digit))) {
		if (digit >= radix)
			return QDF_STATUS_E_FAILURE;

		next_value = value * radix + digit;
		if (next_value < value)
			return QDF_STATUS_E_RANGE;

		value = next_value;
	}

	*int_str = str;
	*out_negate = negate;
	*out_int = value;

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
qdf_int_parse(const char *int_str, uint64_t *out_int, bool *out_negate)
{
	QDF_STATUS status;
	bool negate;
	uint64_t value;

	QDF_BUG(int_str);
	if (!int_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_int);
	if (!out_int)
		return QDF_STATUS_E_INVAL;

	status = __qdf_int_parse_lazy(&int_str, &value, &negate);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	int_str = qdf_str_left_trim(int_str);
	if (int_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	*out_negate = negate;
	*out_int = value;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS qdf_int32_parse(const char *int_str, int32_t *out_int)
{
	QDF_STATUS status;
	int64_t value;

	status = qdf_int64_parse(int_str, &value);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	if ((int32_t)value != value)
		return QDF_STATUS_E_RANGE;

	*out_int = value;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_int32_parse);

QDF_STATUS qdf_uint32_parse(const char *int_str, uint32_t *out_int)
{
	QDF_STATUS status;
	uint64_t value;

	status = qdf_uint64_parse(int_str, &value);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	if ((uint32_t)value != value)
		return QDF_STATUS_E_RANGE;

	*out_int = value;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_uint32_parse);

QDF_STATUS qdf_int64_parse(const char *int_str, int64_t *out_int)
{
	QDF_STATUS status;
	bool negate = false;
	uint64_t value = 0;
	int64_t signed_value;

	status = qdf_int_parse(int_str, &value, &negate);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	if (negate) {
		signed_value = -value;
		if (signed_value > 0)
			return QDF_STATUS_E_RANGE;
	} else {
		signed_value = value;
		if (signed_value < 0)
			return QDF_STATUS_E_RANGE;
	}

	*out_int = signed_value;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_int64_parse);

QDF_STATUS qdf_uint64_parse(const char *int_str, uint64_t *out_int)
{
	QDF_STATUS status;
	bool negate = false;
	uint64_t value = 0;

	status = qdf_int_parse(int_str, &value, &negate);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	if (negate)
		return QDF_STATUS_E_RANGE;

	*out_int = value;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_uint64_parse);

QDF_STATUS qdf_bool_parse(const char *bool_str, bool *out_bool)
{
	bool value;

	QDF_BUG(bool_str);
	if (!bool_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_bool);
	if (!out_bool)
		return QDF_STATUS_E_INVAL;

	bool_str = qdf_str_left_trim(bool_str);

	switch (bool_str[0]) {
	case '1':
	case 'y':
	case 'Y':
		value = true;
		break;
	case '0':
	case 'n':
	case 'N':
		value = false;
		break;
	default:
		return QDF_STATUS_E_FAILURE;
	}

	bool_str++;
	bool_str = qdf_str_left_trim(bool_str);
	if (bool_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	*out_bool = value;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_bool_parse);

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

	mac_str = qdf_str_left_trim(mac_str);

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

	mac_str = qdf_str_left_trim(mac_str);
	if (mac_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	*out_addr = addr;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_mac_parse);

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

	ipv4_str = qdf_str_left_trim(ipv4_str);

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

	ipv4_str = qdf_str_left_trim(ipv4_str);
	if (ipv4_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	*out_addr = addr;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_ipv4_parse);

static inline void qdf_ipv6_apply_zero_comp(struct qdf_ipv6_addr *addr,
					    uint8_t hextets,
					    uint8_t zero_comp_index)
{
	/* Given the following hypothetical ipv6 address:
	 * |---------------------------------------|
	 * | 01 | ab | cd | ef |    |    |    |    |
	 * |---------------------------------------|
	 *           ^--- zero_comp_index (2)
	 * from -----^
	 * to ---------------------------^
	 * |    hextets (4)    |
	 *                     |   zero comp size  |
	 *           | to move |
	 *
	 * We need to apply the zero compression such that we get:
	 * |---------------------------------------|
	 * | 01 | ab | 00 | 00 | 00 | 00 | cd | ef |
	 * |---------------------------------------|
	 *           |     zero comp     |
	 *                               |  moved  |
	 */

	size_t zero_comp_size = (QDF_IPV6_ADDR_HEXTET_COUNT - hextets) * 2;
	size_t bytes_to_move = (hextets - zero_comp_index) * 2;
	uint8_t *from = &addr->bytes[zero_comp_index * 2];
	uint8_t *to = from + zero_comp_size;

	if (bytes_to_move)
		qdf_mem_move(to, from, bytes_to_move);

	qdf_mem_zero(from, to - from);
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

	ipv6_str = qdf_str_left_trim(ipv6_str);

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

	ipv6_str = qdf_str_left_trim(ipv6_str);
	if (ipv6_str[0] != '\0')
		return QDF_STATUS_E_FAILURE;

	/* we must have max hextets or a zero compression, but not both */
	if (hextets_found < QDF_IPV6_ADDR_HEXTET_COUNT) {
		if (zero_comp < 0)
			return QDF_STATUS_E_FAILURE;

		qdf_ipv6_apply_zero_comp(&addr, hextets_found, zero_comp);
	} else if (zero_comp > -1) {
		return QDF_STATUS_E_FAILURE;
	}

	*out_addr = addr;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_ipv6_parse);

QDF_STATUS qdf_uint32_array_parse(const char *in_str, uint32_t *out_array,
				  qdf_size_t array_size, qdf_size_t *out_size)
{
	QDF_STATUS status;
	bool negate;
	qdf_size_t size = 0;
	uint64_t value;

	QDF_BUG(in_str);
	if (!in_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_array);
	if (!out_array)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_size);
	if (!out_size)
		return QDF_STATUS_E_INVAL;

	while (size < array_size) {
		status = __qdf_int_parse_lazy(&in_str, &value, &negate);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		if ((uint32_t)value != value || negate)
			return QDF_STATUS_E_RANGE;

		in_str = qdf_str_left_trim(in_str);

		switch (in_str[0]) {
		case ',':
			out_array[size++] = value;
			in_str++;
			break;
		case '\0':
			out_array[size++] = value;
			*out_size = size;
			return QDF_STATUS_SUCCESS;
		default:
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(qdf_uint32_array_parse);

QDF_STATUS qdf_uint16_array_parse(const char *in_str, uint16_t *out_array,
				  qdf_size_t array_size, qdf_size_t *out_size)
{
	QDF_STATUS status;
	bool negate;
	qdf_size_t size = 0;
	uint64_t value;

	QDF_BUG(in_str);
	if (!in_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_array);
	if (!out_array)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_size);
	if (!out_size)
		return QDF_STATUS_E_INVAL;

	while (size < array_size) {
		status = __qdf_int_parse_lazy(&in_str, &value, &negate);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		if ((uint16_t)value != value || negate)
			return QDF_STATUS_E_RANGE;

		in_str = qdf_str_left_trim(in_str);

		switch (in_str[0]) {
		case ',':
			out_array[size++] = value;
			in_str++;
			break;
		case '\0':
			out_array[size++] = value;
			*out_size = size;
			return QDF_STATUS_SUCCESS;
		default:
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(qdf_uint16_array_parse);

QDF_STATUS qdf_uint8_array_parse(const char *in_str, uint8_t *out_array,
				 qdf_size_t array_size, qdf_size_t *out_size)
{
	QDF_STATUS status;
	bool negate;
	qdf_size_t size = 0;
	uint64_t value;

	QDF_BUG(in_str);
	if (!in_str)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_array);
	if (!out_array)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(out_size);
	if (!out_size)
		return QDF_STATUS_E_INVAL;

	while (size < array_size) {
		status = __qdf_int_parse_lazy(&in_str, &value, &negate);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		if ((uint8_t)value != value || negate)
			return QDF_STATUS_E_RANGE;

		in_str = qdf_str_left_trim(in_str);

		switch (in_str[0]) {
		case ',':
			out_array[size++] = value;
			in_str++;
			break;
		case '\0':
			out_array[size++] = value;
			*out_size = size;
			return QDF_STATUS_SUCCESS;
		default:
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(qdf_uint8_array_parse);
