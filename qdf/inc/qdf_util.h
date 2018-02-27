/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_util.h
 * This file defines utility functions.
 */

#ifndef _QDF_UTIL_H
#define _QDF_UTIL_H

#include <i_qdf_util.h>

#ifdef QCA_CONFIG_SMP
#define QDF_MAX_AVAILABLE_CPU	8
#else
#define QDF_MAX_AVAILABLE_CPU	1
#endif

typedef __qdf_wait_queue_head_t qdf_wait_queue_head_t;
/**
 * qdf_is_fw_down_callback - callback to query if fw is down
 *
 * Return: true if fw is down and false if fw is not down
 */
typedef bool (*qdf_is_fw_down_callback)(void);

/**
 * qdf_unlikely - Compiler-dependent macro denoting code likely to execute
 * @_expr: expression to be checked
 */
#define qdf_unlikely(_expr)     __qdf_unlikely(_expr)

/**
 * qdf_likely - Compiler-dependent macro denoting code unlikely to execute
 * @_expr: expression to be checked
 */
#define qdf_likely(_expr)       __qdf_likely(_expr)

/**
 * qdf_mb - read + write memory barrier.
 */
#define qdf_mb()                 __qdf_mb()

/**
 * qdf_ioread32 - read a register
 * @offset: register address
 */
#define qdf_ioread32(offset)            __qdf_ioread32(offset)
/**
 * qdf_iowrite32 - write a register
 * @offset: register address
 * @value: value to write (32bit value)
 */
#define qdf_iowrite32(offset, value)    __qdf_iowrite32(offset, value)

/**
 * qdf_assert - assert "expr" evaluates to false.
 */
#ifdef QDF_DEBUG
#define qdf_assert(expr)         __qdf_assert(expr)
#else
#define qdf_assert(expr)
#endif /* QDF_DEBUG */

/**
 * qdf_assert_always - alway assert "expr" evaluates to false.
 */
#define qdf_assert_always(expr)  __qdf_assert(expr)

/**
 * qdf_target_assert_always - alway target assert "expr" evaluates to false.
 */
#define qdf_target_assert_always(expr)  __qdf_target_assert(expr)

/**
 * QDF_MAX - get maximum of two values
 * @_x: 1st arguement
 * @_y: 2nd arguement
 */
#define QDF_MAX(_x, _y) (((_x) > (_y)) ? (_x) : (_y))

/**
 * QDF_MIN - get minimum of two values
 * @_x: 1st arguement
 * @_y: 2nd arguement
 */
#define QDF_MIN(_x, _y) (((_x) < (_y)) ? (_x) : (_y))

/**
 * qdf_status_to_os_return - returns the status to OS.
 * @status: enum QDF_STATUS
 *
 * returns: int status success/failure
 */
static inline int qdf_status_to_os_return(QDF_STATUS status)
{
	return __qdf_status_to_os_return(status);
}

/**
 * qdf_set_bit() - set bit in address
 * @nr: bit number to be set
 * @addr: address buffer pointer
 *
 * Return: none
 */
#define qdf_set_bit(nr, addr)    __qdf_set_bit(nr, addr)

/**
 * qdf_clear_bit() - clear bit in address
 * @nr: bit number to be clear
 * @addr: address buffer pointer
 *
 * Return: none
 */
#define qdf_clear_bit(nr, addr)    __qdf_clear_bit(nr, addr)

/**
 * qdf_test_bit() - test bit position in address
 * @nr: bit number to be tested
 * @addr: address buffer pointer
 *
 * Return: none
 */
#define qdf_test_bit(nr, addr)    __qdf_test_bit(nr, addr)

/**
 * qdf_test_and_clear_bit() - test and clear bit position in address
 * @nr: bit number to be tested
 * @addr: address buffer pointer
 *
 * Return: none
 */
#define qdf_test_and_clear_bit(nr, addr)    __qdf_test_and_clear_bit(nr, addr)

/**
 * qdf_find_first_bit() - find first bit position in address
 * @addr: address buffer pointer
 * @nbits: number of bits
 *
 * Return: position first set bit in addr
 */
#define qdf_find_first_bit(addr, nbits)    __qdf_find_first_bit(addr, nbits)

#define qdf_wait_queue_interruptible(wait_queue, condition) \
		__qdf_wait_queue_interruptible(wait_queue, condition)

/**
 * qdf_wait_queue_timeout() - wait for specified time on given condition
 * @wait_queue: wait queue to wait on
 * @condition: condition to wait on
 * @timeout: timeout value in jiffies
 *
 * Return: 0 if condition becomes false after timeout
 *         1 or remaining jiffies, if condition becomes true during timeout
 */
#define qdf_wait_queue_timeout(wait_queue, condition, timeout) \
			__qdf_wait_queue_timeout(wait_queue, \
						condition, timeout)


#define qdf_init_waitqueue_head(_q) __qdf_init_waitqueue_head(_q)

#define qdf_wake_up_interruptible(_q) __qdf_wake_up_interruptible(_q)

/**
 * qdf_wake_up() - wakes up sleeping waitqueue
 * @wait_queue: wait queue, which needs wake up
 *
 * Return: none
 */
#define qdf_wake_up(_q) __qdf_wake_up(_q)

#define qdf_wake_up_completion(_q) __qdf_wake_up_completion(_q)

/**
 * qdf_container_of - cast a member of a structure out to the containing
 * structure
 * @ptr: the pointer to the member.
 * @type: the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 */
#define qdf_container_of(ptr, type, member) \
	 __qdf_container_of(ptr, type, member)

/**
 * qdf_is_pwr2 - test input value is power of 2 integer
 * @value: input integer
 */
#define QDF_IS_PWR2(value) (((value) ^ ((value)-1)) == ((value) << 1) - 1)

/**
 * qdf_roundup() - roundup the input value
 * @x: value to roundup
 * @y: input value rounded to multiple of this
 *
 * Return: rounded value
 */
#define qdf_roundup(x, y) __qdf_roundup(x, y)

/**
 * qdf_is_macaddr_equal() - compare two QDF MacAddress
 * @mac_addr1: Pointer to one qdf MacAddress to compare
 * @mac_addr2: Pointer to the other qdf MacAddress to compare
 *
 * This function returns a bool that tells if a two QDF MacAddress'
 * are equivalent.
 *
 * Return: true if the MacAddress's are equal
 * not true if the MacAddress's are not equal
 */
static inline bool qdf_is_macaddr_equal(struct qdf_mac_addr *mac_addr1,
					struct qdf_mac_addr *mac_addr2)
{
	return __qdf_is_macaddr_equal(mac_addr1, mac_addr2);
}


/**
 * qdf_is_macaddr_zero() - check for a MacAddress of all zeros.
 * @mac_addr: pointer to the struct qdf_mac_addr to check.
 *
 * This function returns a bool that tells if a MacAddress is made up of
 * all zeros.
 *
 * Return: true if the MacAddress is all Zeros
 * false if the MacAddress is not all Zeros.
 */
static inline bool qdf_is_macaddr_zero(struct qdf_mac_addr *mac_addr)
{
	struct qdf_mac_addr zero_mac_addr = QDF_MAC_ADDR_ZERO_INIT;

	return qdf_is_macaddr_equal(mac_addr, &zero_mac_addr);
}

/**
 * qdf_zero_macaddr() - zero out a MacAddress
 * @mac_addr: pointer to the struct qdf_mac_addr to zero.
 *
 * This function zeros out a QDF MacAddress type.
 *
 * Return: none
 */
static inline void qdf_zero_macaddr(struct qdf_mac_addr *mac_addr)
{
	__qdf_zero_macaddr(mac_addr);
}


/**
 * qdf_is_macaddr_group() - check for a MacAddress is a 'group' address
 * @mac_addr1: pointer to the qdf MacAddress to check
 *
 * This function returns a bool that tells if a the input QDF MacAddress
 * is a "group" address. Group addresses have the 'group address bit' turned
 * on in the MacAddress. Group addresses are made up of Broadcast and
 * Multicast addresses.
 *
 * Return: true if the input MacAddress is a Group address
 * false if the input MacAddress is not a Group address
 */
static inline bool qdf_is_macaddr_group(struct qdf_mac_addr *mac_addr)
{
	return mac_addr->bytes[0] & 0x01;
}


/**
 * qdf_is_macaddr_broadcast() - check for a MacAddress is a broadcast address
 * @mac_addr: Pointer to the qdf MacAddress to check
 *
 * This function returns a bool that tells if a the input QDF MacAddress
 * is a "broadcast" address.
 *
 * Return: true if the input MacAddress is a broadcast address
 * flase if the input MacAddress is not a broadcast address
 */
static inline bool qdf_is_macaddr_broadcast(struct qdf_mac_addr *mac_addr)
{
	struct qdf_mac_addr broadcast_mac_addr = QDF_MAC_ADDR_BCAST_INIT;
	return qdf_is_macaddr_equal(mac_addr, &broadcast_mac_addr);
}

/**
 * qdf_copy_macaddr() - copy a QDF MacAddress
 * @dst_addr: pointer to the qdf MacAddress to copy TO (the destination)
 * @src_addr: pointer to the qdf MacAddress to copy FROM (the source)
 *
 * This function copies a QDF MacAddress into another QDF MacAddress.
 *
 * Return: none
 */
static inline void qdf_copy_macaddr(struct qdf_mac_addr *dst_addr,
				    struct qdf_mac_addr *src_addr)
{
	*dst_addr = *src_addr;
}

/**
 * qdf_set_macaddr_broadcast() - set a QDF MacAddress to the 'broadcast'
 * @mac_addr: pointer to the qdf MacAddress to set to broadcast
 *
 * This function sets a QDF MacAddress to the 'broadcast' MacAddress. Broadcast
 * MacAddress contains all 0xFF bytes.
 *
 * Return: none
 */
static inline void qdf_set_macaddr_broadcast(struct qdf_mac_addr *mac_addr)
{
	__qdf_set_macaddr_broadcast(mac_addr);
}

/**
 * qdf_set_u16() - Assign 16-bit unsigned value to a byte array base on CPU's
 * endianness.
 * @ptr: Starting address of a byte array
 * @value: The value to assign to the byte array
 *
 * Caller must validate the byte array has enough space to hold the vlaue
 *
 * Return: The address to the byte after the assignment. This may or may not
 * be valid. Caller to verify.
 */
static inline uint8_t *qdf_set_u16(uint8_t *ptr, uint16_t value)
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
 * qdf_get_u16() - Retrieve a 16-bit unsigned value from a byte array base on
 * CPU's endianness.
 * @ptr: Starting address of a byte array
 * @value: Pointer to a caller allocated buffer for 16 bit value. Value is to
 * assign to this location.
 *
 * Caller must validate the byte array has enough space to hold the vlaue
 *
 * Return: The address to the byte after the assignment. This may or may not
 * be valid. Caller to verify.
 */
static inline uint8_t *qdf_get_u16(uint8_t *ptr, uint16_t *value)
{
#if defined(ANI_BIG_BYTE_ENDIAN)
	*value = (((uint16_t) (*ptr << 8)) | ((uint16_t) (*(ptr + 1))));
#else
	*value = (((uint16_t) (*(ptr + 1) << 8)) | ((uint16_t) (*ptr)));
#endif
	return ptr + 2;
}

/**
 * qdf_get_u32() - retrieve a 32-bit unsigned value from a byte array base on
 * CPU's endianness.
 * @ptr: Starting address of a byte array
 * @value: Pointer to a caller allocated buffer for 32 bit value. Value is to
 * assign to this location.
 *
 * Caller must validate the byte array has enough space to hold the vlaue
 *
 * Return: The address to the byte after the assignment. This may or may not
 * be valid. Caller to verify.
 */
static inline uint8_t *qdf_get_u32(uint8_t *ptr, uint32_t *value)
{
#if defined(ANI_BIG_BYTE_ENDIAN)
	*value = ((uint32_t) (*(ptr) << 24) |
		   (uint32_t) (*(ptr + 1) << 16) |
		   (uint32_t) (*(ptr + 2) << 8) | (uint32_t) (*(ptr + 3)));
#else
	*value = ((uint32_t) (*(ptr + 3) << 24) |
		   (uint32_t) (*(ptr + 2) << 16) |
		   (uint32_t) (*(ptr + 1) << 8) | (uint32_t) (*(ptr)));
#endif
	return ptr + 4;
}

/**
 * qdf_ntohs - Convert a 16-bit value from network byte order to host byte order
 */
#define qdf_ntohs(x)                         __qdf_ntohs(x)

/**
 * qdf_ntohl - Convert a 32-bit value from network byte order to host byte order
 */
#define qdf_ntohl(x)                         __qdf_ntohl(x)

/**
 * qdf_htons - Convert a 16-bit value from host byte order to network byte order
 */
#define qdf_htons(x)                         __qdf_htons(x)

/**
 * qdf_htonl - Convert a 32-bit value from host byte order to network byte order
 */
#define qdf_htonl(x)                         __qdf_htonl(x)

/**
 * qdf_cpu_to_le16 - Convert a 16-bit value from CPU byte order to
 * little-endian byte order
 *
 * @x: value to be converted
 */
#define qdf_cpu_to_le16(x)                   __qdf_cpu_to_le16(x)

/**
 * qdf_cpu_to_le32 - Convert a 32-bit value from CPU byte order to
 * little-endian byte order
 *
 * @x: value to be converted
 */
#define qdf_cpu_to_le32(x)                   __qdf_cpu_to_le32(x)

/**
 * qdf_cpu_to_le64 - Convert a 64-bit value from CPU byte order to
 * little-endian byte order
 *
 * @x: value to be converted
 */
#define qdf_cpu_to_le64(x)                   __qdf_cpu_to_le64(x)

/**
 * qdf_le16_to_cpu - Convert a 16-bit value from little-endian byte order
 * to CPU byte order
 *
 * @x: value to be converted
 */
#define qdf_le16_to_cpu(x)                   __qdf_le16_to_cpu(x)

/**
 * qdf_le32_to_cpu - Convert a 32-bit value from little-endian byte
 * order to CPU byte order
 *
 * @x: value to be converted
 */
#define qdf_le32_to_cpu(x)                   __qdf_le32_to_cpu(x)

/**
 * qdf_le64_to_cpu - Convert a 64-bit value from little-endian byte
 * order to CPU byte order
 *
 * @x: value to be converted
 */
#define qdf_le64_to_cpu(x)                   __qdf_le64_to_cpu(x)

/**
 * qdf_cpu_to_be16 - Convert a 16-bit value from CPU byte order to
 * big-endian byte order
 *
 * @x: value to be converted
 */
#define qdf_cpu_to_be16(x)                   __qdf_cpu_to_be16(x)

/**
 * qdf_cpu_to_be32 - Convert a 32-bit value from CPU byte order to
 * big-endian byte order
 *
 * @x: value to be converted
 */
#define qdf_cpu_to_be32(x)                   __qdf_cpu_to_be32(x)

/**
 * qdf_cpu_to_be64 - Convert a 64-bit value from CPU byte order to
 * big-endian byte order
 *
 * @x: value to be converted
 */
#define qdf_cpu_to_be64(x)                   __qdf_cpu_to_be64(x)


/**
 * qdf_be16_to_cpu - Convert a 16-bit value from big-endian byte order
 * to CPU byte order
 *
 * @x: value to be converted
 */
#define qdf_be16_to_cpu(x)                   __qdf_be16_to_cpu(x)

/**
 * qdf_be32_to_cpu - Convert a 32-bit value from big-endian byte order
 * to CPU byte order
 *
 * @x: value to be converted
 */
#define qdf_be32_to_cpu(x)                   __qdf_be32_to_cpu(x)

/**
 * qdf_be64_to_cpu - Convert a 64-bit value from big-endian byte order
 * to CPU byte order
 *
 * @x: value to be converted
 */
#define qdf_be64_to_cpu(x)                   __qdf_be64_to_cpu(x)

/**
 * qdf_function - replace with the name of the current function
 */
#define qdf_function             __qdf_function

/**
 * qdf_min - minimum of two numbers
 */
#define qdf_min(a, b)   __qdf_min(a, b)

/**
 * qdf_get_pwr2() - get next power of 2 integer from input value
 * @value: input value to find next power of 2 integer
 *
 * Get next power of 2 integer from input value
 *
 * Return: Power of 2 integer
 */
static inline int qdf_get_pwr2(int value)
{
	int log2;

	if (QDF_IS_PWR2(value))
		return value;

	log2 = 0;
	while (value) {
		value >>= 1;
		log2++;
	}
	return 1 << log2;
}

static inline
int qdf_get_cpu(void)
{
	return __qdf_get_cpu();
}

/**
 * qdf_device_init_wakeup() - allow a device to wake up the aps system
 * @qdf_dev: the qdf device context
 * @enable: enable/disable the device as a wakup source
 *
 * Return: 0 or errno
 */
static inline int qdf_device_init_wakeup(qdf_device_t qdf_dev, bool enable)
{
	return __qdf_device_init_wakeup(qdf_dev, enable);
}

static inline
uint64_t qdf_get_totalramsize(void)
{
	return __qdf_get_totalramsize();
}

/**
 * qdf_get_lower_32_bits() - get lower 32 bits from an address.
 * @addr: address
 *
 * This api returns the lower 32 bits of an address.
 *
 * Return: lower 32 bits.
 */
static inline
uint32_t qdf_get_lower_32_bits(qdf_dma_addr_t addr)
{
	return __qdf_get_lower_32_bits(addr);
}

/**
 * qdf_get_upper_32_bits() - get upper 32 bits from an address.
 * @addr: address
 *
 * This api returns the upper 32 bits of an address.
 *
 * Return: upper 32 bits.
 */
static inline
uint32_t qdf_get_upper_32_bits(qdf_dma_addr_t addr)
{
	return __qdf_get_upper_32_bits(addr);
}

/**
 * qdf_rounddown_pow_of_two() - Round down to nearest power of two
 * @n: number to be tested
 *
 * Test if the input number is power of two, and return the nearest power of two
 *
 * Return: number rounded down to the nearest power of two
 */
static inline
unsigned long qdf_rounddown_pow_of_two(unsigned long n)
{
	return __qdf_rounddown_pow_of_two(n);
}

/**
 * qdf_set_dma_coherent_mask() - set max number of bits allowed in dma addr
 * @dev: device pointer
 * @addr_bits: max number of bits allowed in dma address
 *
 * This API sets the maximum allowed number of bits in the dma address.
 *
 * Return: 0 - success, non zero - failure
 */
static inline
int qdf_set_dma_coherent_mask(struct device *dev, uint8_t addr_bits)
{
	return __qdf_set_dma_coherent_mask(dev, addr_bits);
}

/**
 * qdf_do_div() - wrapper function for kernel macro(do_div).
 * @dividend: Dividend value
 * @divisor : Divisor value
 *
 * Return: Quotient
 */
static inline
uint64_t qdf_do_div(uint64_t dividend, uint32_t divisor)
{
	return __qdf_do_div(dividend, divisor);
}

/**
 * qdf_do_div_rem() - wrapper function for kernel macro(do_div)
 *                    to get remainder.
 * @dividend: Dividend value
 * @divisor : Divisor value
 *
 * Return: remainder
 */
static inline
uint64_t qdf_do_div_rem(uint64_t dividend, uint32_t divisor)
{
	return __qdf_do_div_rem(dividend, divisor);
}

/**
 * qdf_get_random_bytes() - returns nbytes bytes of random
 * data
 *
 * Return: random bytes of data
 */
static inline
void qdf_get_random_bytes(void *buf, int nbytes)
{
	return __qdf_get_random_bytes(buf, nbytes);
}

/**
 * qdf_register_fw_down_callback() - API to register fw down callback
 * @is_fw_down: callback to query if fw is down or not
 *
 * Return: none
 */
void qdf_register_fw_down_callback(qdf_is_fw_down_callback *is_fw_down);
#endif /*_QDF_UTIL_H*/
