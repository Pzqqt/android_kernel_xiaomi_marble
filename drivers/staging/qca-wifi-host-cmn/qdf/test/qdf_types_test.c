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
#include "qdf_trace.h"
#include "qdf_types.h"
#include "qdf_types_test.h"

#define WHITESPACE "\t\n\r \x20"

#define ut_bool_pass(str, exp) __ut_bool(str, QDF_STATUS_SUCCESS, exp)
#define ut_bool_fail(str) __ut_bool(str, QDF_STATUS_E_FAILURE, false)

static uint32_t
__ut_bool(const char *str, QDF_STATUS exp_status, bool exp_value)
{
	bool value;
	QDF_STATUS status = qdf_bool_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_bool_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (value != exp_value) {
		qdf_nofl_alert("FAIL: qdf_bool_parse(\"%s\") -> %s; expected %s",
			       str, value ? "true" : "false",
			       exp_value ? "true" : "false");
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_bool_parse(void)
{
	uint32_t errors = 0;

	errors += ut_bool_pass("1", true);
	errors += ut_bool_pass("y", true);
	errors += ut_bool_pass("Y", true);
	errors += ut_bool_pass("0", false);
	errors += ut_bool_pass("n", false);
	errors += ut_bool_pass("N", false);
	errors += ut_bool_pass(WHITESPACE "1" WHITESPACE, true);

	errors += ut_bool_fail("true");
	errors += ut_bool_fail("false");
	errors += ut_bool_fail("日本");

	return errors;
}

#define ut_int32_pass(str, exp) __ut_int32(str, QDF_STATUS_SUCCESS, exp)
#define ut_int32_fail(str, exp_status) __ut_int32(str, exp_status, 0)

static uint32_t
__ut_int32(const char *str, QDF_STATUS exp_status, int32_t exp_value)
{
	int32_t value;
	QDF_STATUS status = qdf_int32_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_int32_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (value != exp_value) {
		qdf_nofl_alert("FAIL: qdf_int32_parse(\"%s\") -> %d; expected %d",
			       str, value, exp_value);
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_int32_parse(void)
{
	uint32_t errors = 0;

	errors += ut_int32_pass("1", 1);
	errors += ut_int32_pass("+1", 1);
	errors += ut_int32_pass("-1", -1);
	errors += ut_int32_pass(WHITESPACE "1" WHITESPACE, 1);
	errors += ut_int32_fail("1;", QDF_STATUS_E_FAILURE);
	errors += ut_int32_pass(" 2147483647", 2147483647);
	errors += ut_int32_fail(" 2147483648", QDF_STATUS_E_RANGE);
	errors += ut_int32_pass("-2147483648", -2147483647 - 1);
	errors += ut_int32_fail("-2147483649", QDF_STATUS_E_RANGE);
	errors += ut_int32_fail("日本", QDF_STATUS_E_FAILURE);

	return errors;
}

#define ut_int64_pass(str, exp) __ut_int64(str, QDF_STATUS_SUCCESS, exp)
#define ut_int64_fail(str, exp_status) __ut_int64(str, exp_status, 0)

static uint32_t
__ut_int64(const char *str, QDF_STATUS exp_status, int64_t exp_value)
{
	int64_t value;
	QDF_STATUS status = qdf_int64_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_int64_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (value != exp_value) {
		qdf_nofl_alert("FAIL: qdf_int64_parse(\"%s\") -> %lld; expected %lld",
			       str, value, exp_value);
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_int64_parse(void)
{
	uint32_t errors = 0;

	errors += ut_int64_pass("1", 1);
	errors += ut_int64_pass("+1", 1);
	errors += ut_int64_pass("-1", -1);
	errors += ut_int64_pass(WHITESPACE "1" WHITESPACE, 1);
	errors += ut_int64_fail("1;", QDF_STATUS_E_FAILURE);
	errors += ut_int64_pass(" 9223372036854775807", 9223372036854775807ll);
	errors += ut_int64_fail(" 9223372036854775808", QDF_STATUS_E_RANGE);
	errors += ut_int64_pass("-9223372036854775808",
				-9223372036854775807ll - 1);
	errors += ut_int64_fail("-9223372036854775809", QDF_STATUS_E_RANGE);
	errors += ut_int64_fail("日本", QDF_STATUS_E_FAILURE);

	return errors;
}

#define ut_uint16_array_pass(str, max_size, exp_arr, exp_arr_size) \
__ut_uint16_array(str, QDF_STATUS_SUCCESS, max_size, exp_arr, exp_arr_size)

#define ut_uint16_array_fail(str, max_size, exp_status, exp_arr, exp_arr_size)\
__ut_uint16_array(str, exp_status, max_size, exp_arr, exp_arr_size)

static uint32_t
__ut_uint16_array(const char *str, QDF_STATUS exp_status,
		  uint8_t max_array_size, uint16_t *exp_array,
		  uint8_t exp_array_size)
{
	uint16_t parsed_array[10];
	qdf_size_t parsed_array_size;
	QDF_STATUS status;
	uint8_t i;

	status = qdf_uint16_array_parse(str, parsed_array, max_array_size,
					&parsed_array_size);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_uint16_array_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (parsed_array_size != exp_array_size) {
		qdf_nofl_alert("FAIL: qdf_uint16_array_parse(\"%s\") -> parsed_array_size %zu; exp_array_size %d",
			       str, parsed_array_size, exp_array_size);
		return 1;
	}

	for (i = 0; i < exp_array_size; i++)
		if (parsed_array[i] != exp_array[i]) {
			qdf_nofl_alert("FAIL: qdf_uint16_array_parse(\"%s\") -> parsed_array[%d] %d; exp_array[%d] %d",
				       str, i, parsed_array[i], i,
				       exp_array[i]);
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_uint16_array_parse(void)
{
	uint32_t errors = 0;
	uint16_t exp_array_value[10] = {
			1, 10, 2412, 2417, 100, 65535, 0, 5486, 5180, 9999};

	errors += ut_uint16_array_pass(
			"1, 10, 2412, 2417, 100, 65535, 0, 5486, 5180, 9999",
			10, exp_array_value, 10);
	errors += ut_uint16_array_pass(
		"+1, +10, +2412, +2417, +100, +65535, 0, +5486, +5180, +9999",
		10, exp_array_value, 10);
	errors += ut_uint16_array_fail("1;", 10, QDF_STATUS_E_FAILURE,
				       exp_array_value, 0);
	/* Out of range test where 65536 is out of range */
	errors += ut_uint16_array_fail(
			"1, 10, 2412, 2417, 100, 65536, 0, 5486, 5180, 9999",
			10, QDF_STATUS_E_RANGE, exp_array_value, 0);
	errors += ut_uint16_array_fail(
		"-1, -10, -2412, -2417, -100, -65535, 0, -5486, -5180, -9999",
		10, QDF_STATUS_E_RANGE, exp_array_value, 0);
	errors += ut_uint16_array_fail(
			"1, 10, 2412, 2417, 100, 日本, 0, 5486, 5180, 9999",
			10, QDF_STATUS_E_FAILURE, exp_array_value, 0);

	return errors;
}

#define ut_uint32_array_pass(str, max_size, exp_arr, exp_arr_size) \
__ut_uint32_array(str, QDF_STATUS_SUCCESS, max_size, exp_arr, exp_arr_size)

#define ut_uint32_array_fail(str, max_size, exp_status, exp_arr, exp_arr_size)\
__ut_uint32_array(str, exp_status, max_size, exp_arr, exp_arr_size)

static uint32_t
__ut_uint32_array(const char *str, QDF_STATUS exp_status,
		  uint8_t max_array_size, uint32_t *exp_array,
		  uint8_t exp_array_size)
{
	uint32_t parsed_array[10];
	qdf_size_t parsed_array_size;
	QDF_STATUS status;
	uint8_t i;

	status = qdf_uint32_array_parse(str, parsed_array, max_array_size,
					&parsed_array_size);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_uint32_array_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (parsed_array_size != exp_array_size) {
		qdf_nofl_alert("FAIL: qdf_uint32_array_parse(\"%s\") -> parsed_array_size %zu; exp_array_size %d",
			       str, parsed_array_size, exp_array_size);
		return 1;
	}

	for (i = 0; i < exp_array_size; i++)
		if (parsed_array[i] != exp_array[i]) {
			qdf_nofl_alert("FAIL: qdf_uint32_array_parse(\"%s\") -> parsed_array[%d] %d; exp_array[%d] %d",
				       str, i, parsed_array[i], i,
				       exp_array[i]);
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_uint32_array_parse(void)
{
	uint32_t errors = 0;
	uint32_t exp_array_value[10] = { 1, 100, 9997, 899965, 65536, 0,
					 4294967295U, 268435456U,
					 2164184149U, 999999999U};

	errors += ut_uint32_array_pass(
		  "1, 100, 9997, 899965, 65536, 0, 4294967295, 268435456, 2164184149, 999999999",
		  10, exp_array_value, 10);
	errors += ut_uint32_array_pass(
		  "+1, +100, +9997, +899965, +65536, 0, +4294967295, +268435456, +2164184149, +999999999",
		  10, exp_array_value, 10);
	errors += ut_uint32_array_fail("1;", 10, QDF_STATUS_E_FAILURE,
				       exp_array_value, 0);
	/* Out of range test where 4294967296 is out of range */
	errors += ut_uint32_array_fail(
		  "1, 100, 9997, 899965, 65536, 0, 4294967296, 268435456, 2164184149, 999999999",
		  10, QDF_STATUS_E_RANGE, exp_array_value, 0);
	errors += ut_uint32_array_fail(
		  "-1, -100, -9997, -899965, -65536, 0, -4294967295, -268435456, -2164184149, -999999999",
		  10, QDF_STATUS_E_RANGE, exp_array_value, 0);
	errors += ut_uint32_array_fail(
			"1, 100, 9997, 899965, 65536, 日本, 0, 4294967295, 268435456, 999999999",
			10, QDF_STATUS_E_FAILURE, exp_array_value, 0);

	return errors;
}

#define ut_uint32_pass(str, exp) __ut_uint32(str, QDF_STATUS_SUCCESS, exp)
#define ut_uint32_fail(str, exp_status) __ut_uint32(str, exp_status, 0)

static uint32_t
__ut_uint32(const char *str, QDF_STATUS exp_status, uint32_t exp_value)
{
	uint32_t value;
	QDF_STATUS status = qdf_uint32_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_uint32_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (value != exp_value) {
		qdf_nofl_alert("FAIL: qdf_uint32_parse(\"%s\") -> %d; expected %d",
			       str, value, exp_value);
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_uint32_parse(void)
{
	uint32_t errors = 0;

	errors += ut_uint32_pass("1", 1);
	errors += ut_uint32_pass("+1", 1);
	errors += ut_uint32_pass(WHITESPACE "1" WHITESPACE, 1);
	errors += ut_uint32_fail("1;", QDF_STATUS_E_FAILURE);
	errors += ut_uint32_pass("4294967295", 4294967295U);
	errors += ut_uint32_fail("4294967296", QDF_STATUS_E_RANGE);
	errors += ut_uint32_pass(" 0", 0);
	errors += ut_uint32_fail("-1", QDF_STATUS_E_RANGE);
	errors += ut_uint32_fail("日本", QDF_STATUS_E_FAILURE);

	return errors;
}

#define ut_uint64_pass(str, exp) __ut_uint64(str, QDF_STATUS_SUCCESS, exp)
#define ut_uint64_fail(str, exp_status) __ut_uint64(str, exp_status, 0)

static uint32_t
__ut_uint64(const char *str, QDF_STATUS exp_status, uint64_t exp_value)
{
	uint64_t value;
	QDF_STATUS status = qdf_uint64_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_uint64_parse(\"%s\") -> status %d; expected status %d",
			       str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (value != exp_value) {
		qdf_nofl_alert("FAIL: qdf_uint64_parse(\"%s\") -> %llu; expected %llu",
			       str, value, exp_value);
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_uint64_parse(void)
{
	uint32_t errors = 0;

	errors += ut_uint64_pass("1", 1);
	errors += ut_uint64_pass("+1", 1);
	errors += ut_uint64_pass(WHITESPACE "1" WHITESPACE, 1);
	errors += ut_uint64_fail("1;", QDF_STATUS_E_FAILURE);
	errors += ut_uint64_pass("18446744073709551615",
				 18446744073709551615ull);
	errors += ut_uint64_fail("18446744073709551616", QDF_STATUS_E_RANGE);
	errors += ut_uint64_pass(" 0", 0);
	errors += ut_uint64_fail("-1", QDF_STATUS_E_RANGE);
	errors += ut_uint64_fail("日本", QDF_STATUS_E_FAILURE);

	return errors;
}

static uint32_t qdf_types_ut_int_formats_parse(void)
{
	uint32_t errors = 0;

	errors += ut_uint64_pass("0b01", 1);
	errors += ut_uint64_pass("0o01234567", 342391);
	errors += ut_uint64_pass("0123456789", 123456789);
	errors += ut_uint64_pass("0x0123456789abcdef", 81985529216486895ll);

	errors += ut_uint64_fail("0b012", QDF_STATUS_E_FAILURE);
	errors += ut_uint64_fail("0o012345678", QDF_STATUS_E_FAILURE);
	errors += ut_uint64_fail("0123456789a", QDF_STATUS_E_FAILURE);
	errors += ut_uint64_fail("0x0123456789abcdefg", QDF_STATUS_E_FAILURE);

	return errors;
}

#define ut_mac_pass(str, exp) __ut_mac(str, #str, QDF_STATUS_SUCCESS, &(exp))
#define ut_mac_fail(str) __ut_mac(str, #str, QDF_STATUS_E_FAILURE, NULL)

static uint32_t
__ut_mac(const char *str, const char *display_str, QDF_STATUS exp_status,
	 struct qdf_mac_addr *exp_value)
{
	struct qdf_mac_addr value;
	QDF_STATUS status = qdf_mac_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_mac_parse(%s) -> status %d; expected status %d",
			       display_str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (qdf_mem_cmp(&value, exp_value, sizeof(value))) {
		qdf_nofl_alert("FAIL: qdf_mac_parse(%s) -> " QDF_FULL_MAC_FMT
			       "; expected " QDF_FULL_MAC_FMT,
			       display_str,
			       QDF_FULL_MAC_REF(value.bytes),
			       QDF_FULL_MAC_REF(exp_value->bytes));
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_mac_parse(void)
{
	uint32_t errors = 0;
	struct qdf_mac_addr addr_aabbccddeeff = { {
		0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff } };
	struct qdf_mac_addr addr_0123456789ab = { {
		0x01, 0x23, 0x45, 0x67, 0x89, 0xab } };

	errors += ut_mac_fail("");
	errors += ut_mac_fail("test");
	errors += ut_mac_fail("¥円");
	errors += ut_mac_pass("aabbccddeeff", addr_aabbccddeeff);
	errors += ut_mac_pass("AABBCCDDEEFF", addr_aabbccddeeff);
	errors += ut_mac_fail("aa:bbccddeeff");
	errors += ut_mac_fail("aabbccddee:ff");
	errors += ut_mac_pass("aa:bb:cc:dd:ee:ff", addr_aabbccddeeff);
	errors += ut_mac_pass("01:23:45:67:89:ab", addr_0123456789ab);
	errors += ut_mac_fail("01:23:45:67:89:ab:cd:ef");
	errors += ut_mac_fail("01:23:45\0:67:89:ab");
	errors += ut_mac_pass(WHITESPACE "01:23:45:67:89:ab" WHITESPACE,
			      addr_0123456789ab);
	errors += ut_mac_pass("01:23:45:67:89:ab\n", addr_0123456789ab);
	errors += ut_mac_fail("01:23:45:67:89:ab\t ,");

	return errors;
}

#define ut_ipv4_pass(str, exp) __ut_ipv4(str, #str, QDF_STATUS_SUCCESS, &(exp))
#define ut_ipv4_fail(str) __ut_ipv4(str, #str, QDF_STATUS_E_FAILURE, NULL)

static uint32_t
__ut_ipv4(const char *str, const char *display_str, QDF_STATUS exp_status,
	  struct qdf_ipv4_addr *exp_value)
{
	struct qdf_ipv4_addr value;
	QDF_STATUS status = qdf_ipv4_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_ipv4_parse(%s) -> status %d; expected status %d",
			       display_str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (qdf_mem_cmp(&value, exp_value, sizeof(value))) {
		qdf_nofl_alert("FAIL: qdf_ipv4_parse(%s) -> " QDF_IPV4_ADDR_STR
			       "; expected " QDF_IPV4_ADDR_STR,
			       display_str,
			       QDF_IPV4_ADDR_ARRAY(value.bytes),
			       QDF_IPV4_ADDR_ARRAY(exp_value->bytes));
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_ipv4_parse(void)
{
	uint32_t errors = 0;
	struct qdf_ipv4_addr addr_0000 = { { 0, 0, 0, 0 } };
	struct qdf_ipv4_addr addr_127001 = { { 127, 0, 0, 1 } };
	struct qdf_ipv4_addr addr_0112123 = { { 0, 1, 12, 123 } };
	struct qdf_ipv4_addr addr_255255255255 = { { 255, 255, 255, 255 } };

	errors += ut_ipv4_fail("");
	errors += ut_ipv4_fail("test");
	errors += ut_ipv4_fail("¥円");
	errors += ut_ipv4_pass("0.0.0.0", addr_0000);
	errors += ut_ipv4_pass("127.0.0.1", addr_127001);
	errors += ut_ipv4_pass("255.255.255.255", addr_255255255255);
	errors += ut_ipv4_fail(".0.0.1");
	errors += ut_ipv4_fail("127.0.0.");
	errors += ut_ipv4_fail("abc.123.123.123");
	errors += ut_ipv4_fail("256.0.0.0");
	errors += ut_ipv4_pass("0.1.12.123", addr_0112123);
	errors += ut_ipv4_pass(WHITESPACE "0.1.12.123" WHITESPACE,
			       addr_0112123);
	errors += ut_ipv4_fail("0.1.12\0.123");
	errors += ut_ipv4_fail("0.1.12.123 ,");

	return errors;
}

#define ut_ipv6_pass(str, exp) __ut_ipv6(str, #str, QDF_STATUS_SUCCESS, &(exp))
#define ut_ipv6_fail(str) __ut_ipv6(str, #str, QDF_STATUS_E_FAILURE, NULL)

static uint32_t
__ut_ipv6(const char *str, const char *display_str, QDF_STATUS exp_status,
	  struct qdf_ipv6_addr *exp_value)
{
	struct qdf_ipv6_addr value;
	QDF_STATUS status = qdf_ipv6_parse(str, &value);

	if (status != exp_status) {
		qdf_nofl_alert("FAIL: qdf_ipv6_parse(%s) -> status %d; expected status %d",
			       display_str, status, exp_status);
		return 1;
	}

	if (QDF_IS_STATUS_ERROR(status))
		return 0;

	if (qdf_mem_cmp(&value, exp_value, sizeof(value))) {
		qdf_nofl_alert("FAIL: qdf_ipv6_parse(%s) -> " QDF_IPV6_ADDR_STR
			       "; expected " QDF_IPV6_ADDR_STR,
			       display_str,
			       QDF_IPV6_ADDR_ARRAY(value.bytes),
			       QDF_IPV6_ADDR_ARRAY(exp_value->bytes));
		return 1;
	}

	return 0;
}

static uint32_t qdf_types_ut_ipv6_parse(void)
{
	uint32_t errors = 0;
	struct qdf_ipv6_addr addr_00000000000000000000000000000000 = { {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	} };
	struct qdf_ipv6_addr addr_00000000000000000000000000000001 = { {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	} };
	struct qdf_ipv6_addr addr_00010000000000000000000000000000 = { {
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	} };
	struct qdf_ipv6_addr addr_0123456789abcdefabcdef0123456789 = { {
		0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
		0xab, 0xcd, 0xef, 0x01, 0x23, 0x45, 0x67, 0x89,
	} };
	struct qdf_ipv6_addr addr_20010db885a3000000008a2e03707334 = { {
		0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00,
		0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34,
	} };
	struct qdf_ipv6_addr addr_ff020000000000000000000000000001 = { {
		0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	} };
	struct qdf_ipv6_addr addr_00000000000000000000ffffc0000280 = { {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0xff, 0xff, 0xc0, 0x00, 0x02, 0x80,
	} };
	struct qdf_ipv6_addr addr_00010000000000000000000000000001 = { {
		0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	} };

	errors += ut_ipv6_fail("");
	errors += ut_ipv6_fail("test");
	errors += ut_ipv6_fail("¥円");
	errors += ut_ipv6_pass("::",
			       addr_00000000000000000000000000000000);
	errors += ut_ipv6_pass("::0",
			       addr_00000000000000000000000000000000);
	errors += ut_ipv6_pass("0:0:0:0:0:0:0:0",
			       addr_00000000000000000000000000000000);
	errors += ut_ipv6_pass("::1",
			       addr_00000000000000000000000000000001);
	errors += ut_ipv6_pass("1::",
			       addr_00010000000000000000000000000000);
	errors += ut_ipv6_pass("0:0:0:0:0:0:0:1",
			       addr_00000000000000000000000000000001);
	errors += ut_ipv6_pass("0123:4567:89ab:cdef:ABCD:EF01:2345:6789",
			       addr_0123456789abcdefabcdef0123456789);
	errors += ut_ipv6_fail("::0123:4567:89ab:cdef:ABCD:EF01:2345:6789");
	errors += ut_ipv6_fail("0123:4567:89ab:cdef:ABCD:EF01:2345:6789::");
	errors += ut_ipv6_pass("2001:0db8:85a3:0000:0000:8a2e:0370:7334",
			       addr_20010db885a3000000008a2e03707334);
	errors += ut_ipv6_pass("2001:db8:85a3:0:0:8a2e:370:7334",
			       addr_20010db885a3000000008a2e03707334);
	errors += ut_ipv6_pass("2001:db8:85a3::8a2e:370:7334",
			       addr_20010db885a3000000008a2e03707334);
	errors += ut_ipv6_pass("ff02::1",
			       addr_ff020000000000000000000000000001);
	errors += ut_ipv6_pass("::ffff:c000:0280",
			       addr_00000000000000000000ffffc0000280);
	errors += ut_ipv6_fail(":0:0:0:0:0:0:1");
	errors += ut_ipv6_fail(":0:0::0:0:1");
	errors += ut_ipv6_fail("0:0:0:0:0:0:0:");
	errors += ut_ipv6_fail("0:0:0::0:0:");
	errors += ut_ipv6_fail("0:0::0:0::0:0");
	errors += ut_ipv6_fail("xyz::zyx");
	errors += ut_ipv6_pass(WHITESPACE "1::1" WHITESPACE,
			       addr_00010000000000000000000000000001);
	errors += ut_ipv6_fail("1\0::1");
	errors += ut_ipv6_fail("1::1 ,");
	errors += ut_ipv6_fail("abcd");

	return errors;
}

uint32_t qdf_types_unit_test(void)
{
	uint32_t errors = 0;

	errors += qdf_types_ut_bool_parse();
	errors += qdf_types_ut_int32_parse();
	errors += qdf_types_ut_int64_parse();
	errors += qdf_types_ut_uint32_parse();
	errors += qdf_types_ut_uint64_parse();
	errors += qdf_types_ut_int_formats_parse();
	errors += qdf_types_ut_mac_parse();
	errors += qdf_types_ut_ipv4_parse();
	errors += qdf_types_ut_ipv6_parse();
	errors += qdf_types_ut_uint16_array_parse();
	errors += qdf_types_ut_uint32_array_parse();

	return errors;
}

