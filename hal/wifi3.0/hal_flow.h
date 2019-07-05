/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
#ifndef __HAL_FLOW_H
#define __HAL_FLOW_H

#define HAL_SET_FLD_SM(block, field, value) \
	(((value) << (block ## _ ## field ## _LSB)) & \
	 (block ## _ ## field ## _MASK))

#define HAL_SET_FLD_MS(block, field, value) \
	(((value) & (block ## _ ## field ## _MASK)) >> \
	 (block ## _ ## field ## _LSB))

#define HAL_CLR_FLD(desc, block, field) \
do { \
	uint32_t val; \
	typeof(desc) desc_ = desc; \
	val = *((uint32_t *)((uint8_t *)(desc_) + \
		HAL_OFFSET(block, field))); \
	val &= ~(block ## _ ## field ## _MASK); \
	HAL_SET_FLD(desc_, block, field) = val; \
} while (0)

#define HAL_GET_FLD(desc, block, field) \
	    ((*((uint32_t *)((uint8_t *)(desc) + HAL_OFFSET(block, field))) & \
	    (block ## _ ## field ## _MASK)) >> (block ## _ ## field ## _LSB))

/**
 * struct hal_flow_tuple_info - Hal Flow 5-tuple
 * @dest_ip_127_96: Destination IP address bits 96-127
 * @dest_ip_95_64: Destination IP address bits 64-95
 * @dest_ip_63_32: Destination IP address bits 32-63
 * @dest_ip_31_0: Destination IP address bits 0-31
 * @src_ip_127_96: Source IP address bits 96-127
 * @src_ip_95_64: Source IP address bits 64-95
 * @src_ip_63_32: Source IP address bits 32-63
 * @src_ip_31_0: Source IP address bits 0-31
 * @dest_port: Destination Port
 * @src_port: Source Port
 * @l4_protocol: Layer-4 protocol type (TCP/UDP)
 */
struct hal_flow_tuple_info {
	uint32_t dest_ip_127_96;
	uint32_t dest_ip_95_64;
	uint32_t dest_ip_63_32;
	uint32_t dest_ip_31_0;
	uint32_t src_ip_127_96;
	uint32_t src_ip_95_64;
	uint32_t src_ip_63_32;
	uint32_t src_ip_31_0;
	uint16_t dest_port;
	uint16_t src_port;
	uint16_t l4_protocol;
};

/**
 * key_bitwise_shift_left() - Bitwise left shift (in place) an array of bytes
 * @key: Pointer to array to key bytes
 * @len: size of array (number of key bytes)
 * @shift: number of shift operations to be performed
 *
 * Return:
 */
static inline void
key_bitwise_shift_left(uint8_t *key, int len, int shift)
{
	int i;
	int next;

	while (shift--) {
		for (i = len - 1; i >= 0 ; i--) {
			if (i > 0)
				next = (key[i - 1] & 0x80 ? 1 : 0);
			else
				next = 0;
			key[i] = (key[i] << 1) | next;
		}
	}
}

/**
 * key_reverse() - Reverse the key buffer from MSB to LSB
 * @dest: pointer to the destination key
 * @src: pointer to the source key which should be shifted
 * @len: size of key in bytes
 *
 * Return:
 */
static inline void
key_reverse(uint8_t *dest, uint8_t *src, int len)
{
	int i, j;

	for (i = 0, j = len  - 1; i < len; i++, j--)
		dest[i] = src[j];
}
#endif /* HAL_FLOW_H */
