/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#ifndef __HAL_RX_FLOW_DEFINES_H
#define __HAL_RX_FLOW_DEFINES_H

#define HAL_FST_HASH_KEY_SIZE_BYTES 40
#define HAL_OFFSET(block, field) block ## _ ## field ## _OFFSET

#define HAL_RX_FST_ENTRY_SIZE (NUM_OF_DWORDS_RX_FLOW_SEARCH_ENTRY * 4)

/**
 * struct hal_rx_flow - Rx Flow parameters to be sent to HW
 * @tuple_info: Rx Flow 5-tuple (src & dest IP, src & dest ports, L4 protocol)
 * @reo_destination_handler: REO destination for this flow
 * @reo_destination_indication: REO indication for this flow
 * @fse_metadata: Flow metadata or tag passed to HW for marking packets
 */
struct hal_rx_flow {
	struct hal_flow_tuple_info tuple_info;
	uint8_t reo_destination_handler;
	uint8_t reo_destination_indication;
	uint32_t fse_metadata;
};

/**
 * struct hal_rx_fst - HAL RX Flow search table context
 * @base_vaddr: Virtual Base address of HW FST
 * @base_paddr: Physical Base address of HW FST
 * @key: Pointer to 320-bit Key read from cfg
 * @shifted_key: Pointer to left-shifted 320-bit Key used for Toeplitz Hash
 * @max_entries : Max number of entries in flow searchh  table
 * @max_skid_length : Max search length if there is hash collision
 * @hash_mask: Hash mask to apply to index into FST
 * @key_cache: Toepliz Key Cache configured key
 * @add_flow_count: Add flow count
 * @del_flow_count: Delete flow count
 */
struct hal_rx_fst {
	uint8_t *base_vaddr;
	qdf_dma_addr_t base_paddr;
	uint8_t *key;
#ifndef WLAN_SUPPORT_RX_FISA
	uint8_t  shifted_key[HAL_FST_HASH_KEY_SIZE_BYTES];
	uint32_t key_cache[HAL_FST_HASH_KEY_SIZE_BYTES][1 << 8];
#endif
	uint16_t max_entries;
	uint16_t max_skid_length;
	uint16_t hash_mask;
	uint32_t add_flow_count;
	uint32_t del_flow_count;
};

#endif /* HAL_RX_FLOW_DEFINES_H */
