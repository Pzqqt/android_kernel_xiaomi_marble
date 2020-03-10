/*
 * Copyright (c) 2016-2018, 2020 The Linux Foundation. All rights reserved.
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

#ifndef _WMI_UNIFIED_DBR_PARAM_H_
#define _WMI_UNIFIED_DBR_PARAM_H_

#define WMI_HOST_DBR_RING_ADDR_LO_S 0
#define WMI_HOST_DBR_RING_ADDR_LO_M 0xffffffff
#define WMI_HOST_DBR_RING_ADDR_LO \
	(WMI_HOST_DBR_RING_ADDR_LO_M << WMI_HOST_DBR_RING_ADDR_LO_S)

#define WMI_HOST_DBR_RING_ADDR_LO_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_RING_ADDR_LO)
#define WMI_HOST_DBR_RING_ADDR_LO_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_RING_ADDR_LO)

#define WMI_HOST_DBR_RING_ADDR_HI_S 0
#define WMI_HOST_DBR_RING_ADDR_HI_M 0xf
#define WMI_HOST_DBR_RING_ADDR_HI \
	(WMI_HOST_DBR_RING_ADDR_HI_M << WMI_HOST_DBR_RING_ADDR_HI_S)

#define WMI_HOST_DBR_RING_ADDR_HI_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_RING_ADDR_HI)
#define WMI_HOST_DBR_RING_ADDR_HI_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_RING_ADDR_HI)

#define WMI_HOST_DBR_DATA_ADDR_LO_S 0
#define WMI_HOST_DBR_DATA_ADDR_LO_M 0xffffffff
#define WMI_HOST_DBR_DATA_ADDR_LO \
	(WMI_HOST_DBR_DATA_ADDR_LO_M << WMI_HOST_DBR_DATA_ADDR_LO_S)

#define WMI_HOST_DBR_DATA_ADDR_LO_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_DATA_ADDR_LO)
#define WMI_HOST_DBR_DATA_ADDR_LO_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_DATA_ADDR_LO)

#define WMI_HOST_DBR_DATA_ADDR_HI_S 0
#define WMI_HOST_DBR_DATA_ADDR_HI_M 0xf
#define WMI_HOST_DBR_DATA_ADDR_HI \
	(WMI_HOST_DBR_DATA_ADDR_HI_M << WMI_HOST_DBR_DATA_ADDR_HI_S)

#define WMI_HOST_DBR_DATA_ADDR_HI_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_DATA_ADDR_HI)
#define WMI_HOST_DBR_DATA_ADDR_HI_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_DATA_ADDR_HI)

#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_S 12
#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_M 0x7ffff
#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA \
	(WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_M << \
	 WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_S)

#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_GET(dword) \
		WMI_HOST_F_MS(dword, WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA)
#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_SET(dword, val) \
		WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA)

#define WMI_HOST_MAX_NUM_CHAINS 8

/**
 * struct direct_buf_rx_rsp: direct buffer rx response structure
 *
 * @pdev_id: Index of the pdev for which response is received
 * @mod_mod: Index of the module for which respone is received
 * @num_buf_release_entry: Number of buffers released through event
 * @dbr_entries: Pointer to direct buffer rx entry struct
 */
struct direct_buf_rx_rsp {
	uint32_t pdev_id;
	uint32_t mod_id;
	uint32_t num_buf_release_entry;
	uint32_t num_meta_data_entry;
	struct direct_buf_rx_entry *dbr_entries;
};

/**
 * struct direct_buf_rx_cfg_req: direct buffer rx config request structure
 *
 * @pdev_id: Index of the pdev for which response is received
 * @mod_id: Index of the module for which respone is received
 * @base_paddr_lo: Lower 32bits of ring base address
 * @base_paddr_hi: Higher 32bits of ring base address
 * @head_idx_paddr_lo: Lower 32bits of head idx register address
 * @head_idx_paddr_hi: Higher 32bits of head idx register address
 * @tail_idx_paddr_lo: Lower 32bits of tail idx register address
 * @tail_idx_paddr_hi: Higher 32bits of tail idx register address
 * @buf_size: Size of the buffer for each pointer in the ring
 * @num_elems: Number of pointers allocated and part of the source ring
 */
struct direct_buf_rx_cfg_req {
	uint32_t pdev_id;
	uint32_t mod_id;
	uint32_t base_paddr_lo;
	uint32_t base_paddr_hi;
	uint32_t head_idx_paddr_lo;
	uint32_t head_idx_paddr_hi;
	uint32_t tail_idx_paddr_hi;
	uint32_t tail_idx_paddr_lo;
	uint32_t buf_size;
	uint32_t num_elems;
	uint32_t event_timeout_ms;
	uint32_t num_resp_per_event;
};

/**
 * struct direct_buf_rx_metadata: direct buffer metadata
 *
 * @noisefloor: noisefloor
 * @reset_delay: reset delay
 */
struct direct_buf_rx_metadata {
	int32_t noisefloor[WMI_HOST_MAX_NUM_CHAINS];
	uint32_t reset_delay;
};

/**
 * struct direct_buf_rx_entry: direct buffer rx release entry structure
 *
 * @addr_lo: LSB 32-bits of the buffer
 * @addr_hi: MSB 32-bits of the buffer
 * @len: Length of the buffer
 */
struct direct_buf_rx_entry {
	uint32_t paddr_lo;
	uint32_t paddr_hi;
	uint32_t len;
};

#endif /* _WMI_UNIFIED_DBR_PARAM_H_ */
