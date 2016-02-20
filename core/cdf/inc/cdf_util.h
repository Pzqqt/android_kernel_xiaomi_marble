/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: cdf_util.h
 *
 * This file defines utility functions.
 */

#ifndef _CDF_UTIL_H
#define _CDF_UTIL_H

#include <i_cdf_util.h>

/**
 * cdf_unlikely - Compiler-dependent macro denoting code likely to execute
 * @_expr: expression to be checked
 */
#define cdf_unlikely(_expr)     __cdf_unlikely(_expr)

/**
 * cdf_likely - Compiler-dependent macro denoting code unlikely to execute
 * @_expr: expression to be checked
 */
#define cdf_likely(_expr)       __cdf_likely(_expr)

static inline int cdf_status_to_os_return(QDF_STATUS status)
{
	return __cdf_status_to_os_return(status);
}

/**
 * cdf_assert - assert "expr" evaluates to false
 * @expr: assert expression
 */
#ifdef CDF_OS_DEBUG
#define cdf_assert(expr)         __cdf_assert(expr)
#else
#define cdf_assert(expr)
#endif /* CDF_OS_DEBUG */

/**
 * @cdf_assert_always- alway assert "expr" evaluates to false
 * @expr: assert expression
 */
#define cdf_assert_always(expr)  __cdf_assert(expr)

/**
 * cdf_os_cpu_to_le64 - Convert a 64-bit value from CPU byte order to
 *			little-endian byte order
 * @x: value to be converted
 */
#define cdf_os_cpu_to_le64(x)                   __cdf_os_cpu_to_le64(x)

/**
 * cdf_le16_to_cpu - Convert a 16-bit value from little-endian byte order
 *			to CPU byte order
 * @x: value to be converted
 */
#define cdf_le16_to_cpu(x)                   __cdf_le16_to_cpu(x)

/**
 * cdf_le32_to_cpu - Convert a 32-bit value from little-endian byte order to
 *			CPU byte order
 * @x: value to be converted
 */
#define cdf_le32_to_cpu(x)                   __cdf_le32_to_cpu(x)

/**
 * cdf_in_interrupt - returns true if in interrupt context
 */
#define cdf_in_interrupt          in_interrupt

/**
 * cdf_container_of - cast a member of a structure out to the containing
 *                    structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define cdf_container_of(ptr, type, member) \
	 __cdf_container_of(ptr, type, member)

/**
 * cdf_is_pwr2 - test input value is power of 2 integer
 *
 * @value: input integer
 *
 */
#define CDF_IS_PWR2(value) (((value) ^ ((value)-1)) == ((value) << 1) - 1)

/**
 * cdf_is_macaddr_equal() - compare two CDF MacAddress
 * @pMacAddr1: Pointer to one cdf MacAddress to compare
 * @pMacAddr2: Pointer to the other cdf MacAddress to compare
 *
 * This function returns a bool that tells if a two CDF MacAddress'
 * are equivalent.
 *
 * Return: true if the MacAddress's are equal
 *	not true if the MacAddress's are not equal
 */
static inline bool cdf_is_macaddr_equal(struct qdf_mac_addr *pMacAddr1,
					struct qdf_mac_addr *pMacAddr2)
{
	return 0 == memcmp(pMacAddr1, pMacAddr2, QDF_MAC_ADDR_SIZE);
}

/**
 * cdf_is_macaddr_zero() - check for a MacAddress of all zeros.
 * @pMacAddr - pointer to the struct qdf_mac_addr to check.
 *
 * This function returns a bool that tells if a MacAddress is made up of
 * all zeros.
 *
 *
 * Return:  true if the MacAddress is all Zeros
 *	false if the MacAddress is not all Zeros.
 *
 */
static inline bool cdf_is_macaddr_zero(struct qdf_mac_addr *pMacAddr)
{
	struct qdf_mac_addr zeroMacAddr = QDF_MAC_ADDR_ZERO_INITIALIZER;

	return cdf_is_macaddr_equal(pMacAddr, &zeroMacAddr);
}

/**
 * cdf_zero_macaddr() - zero out a MacAddress
 * @pMacAddr: pointer to the struct qdf_mac_addr to zero.
 *
 * This function zeros out a CDF MacAddress type.
 *
 * Return: nothing
 */
static inline void cdf_zero_macaddr(struct qdf_mac_addr *pMacAddr)
{
	memset(pMacAddr, 0, QDF_MAC_ADDR_SIZE);
}

/**
 * cdf_is_macaddr_group() - check for a MacAddress is a 'group' address
 * @pMacAddr1: pointer to the cdf MacAddress to check
 *
 * This function returns a bool that tells if a the input CDF MacAddress
 * is a "group" address.  Group addresses have the 'group address bit' turned
 * on in the MacAddress.  Group addresses are made up of Broadcast and
 * Multicast addresses.
 *
 * Return:  true if the input MacAddress is a Group address
 *	false if the input MacAddress is not a Group address
 */
static inline bool cdf_is_macaddr_group(struct qdf_mac_addr *pMacAddr)
{
	return pMacAddr->bytes[0] & 0x01;
}

/**
 * cdf_is_macaddr_broadcast() - check for a MacAddress is a broadcast address
 *
 * This function returns a bool that tells if a the input CDF MacAddress
 * is a "broadcast" address.
 *
 * @pMacAddr: Pointer to the cdf MacAddress to check
 *
 * Return:  true if the input MacAddress is a broadcast address
 *	false if the input MacAddress is not a broadcast address
 */
static inline bool cdf_is_macaddr_broadcast(struct qdf_mac_addr *pMacAddr)
{
	struct qdf_mac_addr broadcastMacAddr =
					QDF_MAC_ADDR_BROADCAST_INITIALIZER;

	return cdf_is_macaddr_equal(pMacAddr, &broadcastMacAddr);
}

/**
 * cdf_copy_macaddr() - copy a CDF MacAddress
 * @pDst - pointer to the cdf MacAddress to copy TO (the destination)
 * @pSrc - pointer to the cdf MacAddress to copy FROM (the source)
 *
 * This function copies a CDF MacAddress into another CDF MacAddress.
 *
 *
 * Return: nothing
 */
static inline void cdf_copy_macaddr(struct qdf_mac_addr *pDst,
				    struct qdf_mac_addr *pSrc)
{
	*pDst = *pSrc;
}

/**
 * cdf_set_macaddr_broadcast() - set a CDF MacAddress to the 'broadcast'
 * @pMacAddr: pointer to the cdf MacAddress to set to broadcast
 *
 * This function sets a CDF MacAddress to the 'broadcast' MacAddress. Broadcast
 * MacAddress contains all 0xFF bytes.
 *
 * Return: nothing
 */
static inline void cdf_set_macaddr_broadcast(struct qdf_mac_addr *pMacAddr)
{
	memset(pMacAddr, 0xff, QDF_MAC_ADDR_SIZE);
}

#if defined(ANI_LITTLE_BYTE_ENDIAN)

/**
 * i_cdf_htonl() - convert from host byte order to network byte order
 * @ul: input to be converted
 *
 * Return: converted network byte order
 */
static inline unsigned long i_cdf_htonl(unsigned long ul)
{
	return ((ul & 0x000000ff) << 24) |
		((ul & 0x0000ff00) << 8) |
		((ul & 0x00ff0000) >> 8) | ((ul & 0xff000000) >> 24);
}

/**
 * i_cdf_ntohl() - convert network byte order to host byte order
 * @ul: input to be converted
 *
 * Return: converted host byte order
 */
static inline unsigned long i_cdf_ntohl(unsigned long ul)
{
	return i_cdf_htonl(ul);
}

#endif

/**
 * cdf_set_u16() - Assign 16-bit unsigned value to a byte array base on CPU's
 *			endianness.
 * @ptr: Starting address of a byte array
 * @value: The value to assign to the byte array
 *
 * Caller must validate the byte array has enough space to hold the vlaue
 *
 * Return: The address to the byte after the assignment. This may or may not
 *	be valid. Caller to verify.
 */
static inline uint8_t *cdf_set_u16(uint8_t *ptr, uint16_t value)
{
#if defined(ANI_BIG_BYTE_ENDIAN)
	*(ptr) = (uint8_t) (value >> 8);
	*(ptr + 1) = (uint8_t) (value);
#else
	*(ptr + 1) = (uint8_t) (value >> 8);
	*(ptr) = (uint8_t) (value);
#endif

	return ptr + 2;
}

/**
 * cdf_get_u16() - Retrieve a 16-bit unsigned value from a byte array base on
 *			CPU's endianness.
 * @ptr: Starting address of a byte array
 * @pValue: Pointer to a caller allocated buffer for 16 bit value. Value is to
 *		assign to this location.
 *
 * Caller must validate the byte array has enough space to hold the vlaue
 *
 * Return: The address to the byte after the assignment. This may or may not
 *	be valid. Caller to verify.
 */
static inline uint8_t *cdf_get_u16(uint8_t *ptr, uint16_t *pValue)
{
#if defined(ANI_BIG_BYTE_ENDIAN)
	*pValue = (((uint16_t) (*ptr << 8)) | ((uint16_t) (*(ptr + 1))));
#else
	*pValue = (((uint16_t) (*(ptr + 1) << 8)) | ((uint16_t) (*ptr)));
#endif

	return ptr + 2;
}

/**
 * cdf_get_u32() - retrieve a 32-bit unsigned value from a byte array base on
 *			CPU's endianness.
 * @ptr: Starting address of a byte array
 * @pValue: Pointer to a caller allocated buffer for 32 bit value. Value is to
 *		assign to this location.
 *
 * Caller must validate the byte array has enough space to hold the vlaue
 *
 * Return: The address to the byte after the assignment. This may or may not
 *		be valid. Caller to verify.
 */
static inline uint8_t *cdf_get_u32(uint8_t *ptr, uint32_t *pValue)
{
#if defined(ANI_BIG_BYTE_ENDIAN)
	*pValue = ((uint32_t) (*(ptr) << 24) |
		   (uint32_t) (*(ptr + 1) << 16) |
		   (uint32_t) (*(ptr + 2) << 8) | (uint32_t) (*(ptr + 3)));
#else
	*pValue = ((uint32_t) (*(ptr + 3) << 24) |
		   (uint32_t) (*(ptr + 2) << 16) |
		   (uint32_t) (*(ptr + 1) << 8) | (uint32_t) (*(ptr)));
#endif
	return ptr + 4;
}

/**
 * cdf_get_pwr2() - get next power of 2 integer from input value
 * @value: input value to find next power of 2 integer
 *
 * Get next power of 2 integer from input value
 *
 * Return: Power of 2 integer
 */
static inline int cdf_get_pwr2(int value)
{
	int log2;
	if (CDF_IS_PWR2(value))
		return value;

	log2 = 0;
	while (value) {
		value >>= 1;
		log2++;
	}
	return 1 << log2;
}

#endif /*_CDF_UTIL_H*/
