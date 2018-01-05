/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#ifndef _TARGET_IF_DIRECT_BUF_RX_MAIN_H_
#define _TARGET_IF_DIRECT_BUF_RX_MAIN_H_

#include "qdf_types.h"
#include "qdf_status.h"

struct wlan_objmgr_psoc;
struct wlan_lmac_if_tx_ops;
struct direct_buf_rx_data;

#define DBR_RING_BASE_ALIGN 8
#define DBR_EVENT_TIMEOUT_IN_MS 1
#define DBR_NUM_RESP_PER_EVENT 1

/**
 * enum DBR_MODULE - Enum containing the modules supporting direct buf rx
 * @DBR_MODULE_SPECTRAL: Module ID for Spectral
 * @DBR_MODULE_MAX: Max module ID
 */
enum DBR_MODULE {
	DBR_MODULE_SPECTRAL = 1,
	DBR_MODULE_MAX,
};

/**
 * struct direct_buf_rx_info - direct buffer rx operation info struct
 * @cookie: SW cookie used to get the virtual address
 * @paddr: Physical address pointer for DMA operation
 * @vaddr: Virtual address pointer
 * @offset: Offset of aligned address from unaligned
 */
struct direct_buf_rx_buf_info {
	uint32_t cookie;
	qdf_dma_addr_t paddr;
	void *vaddr;
	uint8_t offset;
};

/**
 * struct direct_buf_rx_ring_cfg - DMA ring config parameters
 * @num_ptr: Depth or the number of physical address pointers in the ring
 * @ring_alloc_size: Size of the HAL ring
 * @base_paddr_unaligned: base physical addr unaligned
 * @base_vaddr_unaligned: base virtual addr unaligned
 * @base_paddr_aligned: base physical addr aligned
 * @base_vaddr_aligned: base virtual addr unaligned
 * @head_idx_addr: head index addr
 * @tail_idx_addr: tail index addr
 * @srng: HAL srng context
 */
struct direct_buf_rx_ring_cfg {
	uint32_t num_ptr;
	uint32_t ring_alloc_size;
	qdf_dma_addr_t base_paddr_unaligned;
	void *base_vaddr_unaligned;
	qdf_dma_addr_t base_paddr_aligned;
	void *base_vaddr_aligned;
	qdf_dma_addr_t head_idx_addr;
	qdf_dma_addr_t tail_idx_addr;
	void *srng;
	uint32_t buf_size;
};

/**
 * struct direct_buf_rx_ring_cap - DMA ring capabilities
 * @ring_elems_min: Minimum number of pointers in the ring
 * @min_buf_size: Minimum size of each buffer entry in the ring
 * @min_buf_align: Minimum alignment of the addresses in the ring
 */
struct direct_buf_rx_ring_cap {
	uint32_t ring_elems_min;
	uint32_t min_buf_size;
	uint32_t min_buf_align;
};

/**
 * struct direct_buf_rx_module_param - DMA module param
 * @mod_id: Module ID
 * @dbr_ring_cap: Pointer to direct buf rx ring capabilities struct
 * @dbr_ring_cfg: Pointer to direct buf rx ring config struct
 * @dbr_buf_pool: Pointer to direct buf rx buffer pool struct
 * @dbr_rsp_handler: Pointer to direct buf rx response handler for the module
 */
struct direct_buf_rx_module_param {
	enum DBR_MODULE mod_id;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_buf_info *dbr_buf_pool;
	int (*dbr_rsp_handler)(struct wlan_objmgr_pdev *pdev,
			       struct direct_buf_rx_data *dbr_data);
};

/**
 * struct direct_buf_rx_pdev_obj - Direct Buf RX pdev object struct
 * @num_modules: Number of modules registered to DBR for the pdev
 * @dbr_mod_param: Pointer to direct buf rx module param struct
 */
struct direct_buf_rx_pdev_obj {
	uint32_t num_modules;
	struct direct_buf_rx_module_param *dbr_mod_param;
};

/**
 * struct direct_buf_rx_psoc_obj - Direct Buf RX psoc object struct
 * @hal_soc: Opaque HAL SOC handle
 * @osdev: QDF os device handle
 */
struct direct_buf_rx_psoc_obj {
	void *hal_soc;
	qdf_device_t osdev;
};

/**
 * target_if_direct_buf_rx_register_events() - Register WMI events to direct
 *                                             buffer rx module
 * @psoc: pointer to psoc object
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_register_events(
				struct wlan_objmgr_psoc *psoc);

/**
 * target_if_direct_buf_rx_unregister_events() - Unregister WMI events to direct
 *                                               buffer rx module
 * @psoc: pointer to psoc object
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_unregister_events(
				struct wlan_objmgr_psoc *psoc);

/**
 * target_if_direct_buf_rx_pdev_create_handler() - Handler to be invoked for
 *                                                 direct buffer rx module
 *                                                 during pdev object create
 * @pdev: pointer to pdev object
 * @data: pointer to data
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_pdev_create_handler(
				struct wlan_objmgr_pdev *pdev, void *data);

/**
 * target_if_direct_buf_rx_pdev_destroy_handler() - Handler to be invoked for
 *                                                  direct buffer rx module
 *                                                  during pdev object destroy
 * @pdev: pointer to pdev object
 * @data: pointer to data
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_pdev_destroy_handler(
				struct wlan_objmgr_pdev *pdev, void *data);

/**
 * target_if_direct_buf_rx_psoc_create_handler() - Handler invoked for
 *                                                 direct buffer rx module
 *                                                 during attach
 * @pdev: pointer to psoc object
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_psoc_create_handler(
				struct wlan_objmgr_psoc *psoc, void *data);

/**
 * target_if_direct_buf_rx_psoc_destroy_handler() - Handler invoked for
 *                                                  direct buffer rx module
 *                                                  during detach
 * @pdev: pointer to psoc object
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_psoc_destroy_handler(
				struct wlan_objmgr_psoc *psoc, void *data);

/**
 * target_if_deinit_dbr_ring() - Function to deinitialize buffers and ring
 *                               allocated for direct buffer rx module
 * @pdev: pointer to pdev object
 * @dbr_pdev_obj: pointer to direct buffer rx module pdev obj
 * @mod_id: module id indicating the module using direct buffer rx framework
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_deinit_dbr_ring(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
				enum DBR_MODULE mod_id);
/**
 * target_if_direct_buf_rx_module_register() - Function to register to direct
 *                                             buffer rx module
 * @pdev: pointer to pdev object
 * @mod_id: module id indicating the module using direct buffer rx framework
 * @dbr_rsp_handler: function pointer pointing to the response handler to be
 *                   invoked for the module registering to direct buffer rx
 *                   module
 *
 * Return: QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_module_register(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id,
			int (*dbr_rsp_handler)(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_data *dbr_data));

#endif /* _TARGET_IF_DIRECT_BUF_RX_MAIN_H_ */
