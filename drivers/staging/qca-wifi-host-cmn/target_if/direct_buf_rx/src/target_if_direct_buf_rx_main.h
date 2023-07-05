/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <target_if_direct_buf_rx_api.h>

struct wlan_objmgr_psoc;
struct wlan_lmac_if_tx_ops;
struct direct_buf_rx_data;

#define DBR_RING_BASE_ALIGN 8

#ifdef DBR_MULTI_SRNG_ENABLE
#define DBR_SRNG_NUM 2
#define dbr_get_pdev_id(srng_id, pdev_id) (srng_id)
#else
#define DBR_SRNG_NUM 1
#define dbr_get_pdev_id(srng_id, pdev_id) (pdev_id)
#endif

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
 * enum DBR_RING_DEBUG_EVENT - DMA ring debug event
 * @DBR_RING_DEBUG_EVENT_NONE: Not a real value, just a place holder for
 * no entry
 * @DBR_RING_DEBUG_EVENT_RX: DBR Rx event
 * @DBR_RING_DEBUG_EVENT_REPLENISH_RING: DBR replenish event
 * @DBR_RING_DEBUG_EVENT_MAX: Not a real value, just a place holder for max
 */
enum DBR_RING_DEBUG_EVENT {
	DBR_RING_DEBUG_EVENT_NONE = 0,
	DBR_RING_DEBUG_EVENT_RX,
	DBR_RING_DEBUG_EVENT_REPLENISH_RING,
	DBR_RING_DEBUG_EVENT_MAX,
};

#define DIRECT_BUF_RX_MAX_RING_DEBUG_ENTRIES (1024)
/**
 * struct direct_buf_rx_ring_debug_entry - DBR ring debug entry
 * @head_idx: Head index of the DMA ring
 * @tail_idx: Tail index of the DMA ring
 * @timestamp: Timestamp at the time of logging
 * @event: Name of the event
 */
struct direct_buf_rx_ring_debug_entry {
	uint32_t head_idx;
	uint32_t tail_idx;
	uint64_t timestamp;
	enum DBR_RING_DEBUG_EVENT event;
};

#ifdef WLAN_DEBUGFS
/**
 * struct dbr_debugfs_priv - Private data for DBR ring debugfs
 * @dbr_pdev_obj: Pointer to the pdev obj of Direct buffer rx module
 * @mod_id: Pointer to the registered module ID
 * @srng_id: srng ID
 */
struct dbr_debugfs_priv {
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj;
	enum DBR_MODULE mod_id;
	uint8_t srng_id;
};
#endif

/**
 * struct direct_buf_rx_ring_debug - DMA ring debug of a module
 * @entries: Pointer to the array of ring debug entries
 * @ring_debug_idx: Current index in the array of ring debug entries
 * @num_ring_debug_entries: Total ring debug entries
 * @debugfs_entry: Debugfs entry for this ring
 * @debugfs_priv: Debugfs ops for this ring
 */
struct direct_buf_rx_ring_debug {
	struct direct_buf_rx_ring_debug_entry *entries;
	uint32_t ring_debug_idx;
	uint32_t num_ring_debug_entries;
#ifdef WLAN_DEBUGFS
	qdf_dentry_t debugfs_entry;
	struct qdf_debugfs_fops *debugfs_fops;
#endif
};

/**
 * struct direct_buf_rx_module_debug - Debug of a module subscribed to DBR
 * @dbr_ring_debug: Array of ring debug structers corresponding to each srng
 * @poisoning_enabled: Whether buffer poisoning is enabled for this module
 * @poison_value: Value with which buffers should be poisoned
 * @debugfs_entry: Debugfs entry for this module
 */
struct direct_buf_rx_module_debug {
	struct direct_buf_rx_ring_debug dbr_ring_debug[DBR_SRNG_NUM];
	bool poisoning_enabled;
	uint32_t poison_value;
#ifdef WLAN_DEBUGFS
	qdf_dentry_t debugfs_entry;
#endif
};

/**
 * struct direct_buf_rx_module_param - DMA module param
 * @mod_id: Module ID
 * @pdev_id: pdev ID
 * @dbr_config: Pointer to dirct buf rx module configuration struct
 * @dbr_ring_cap: Pointer to direct buf rx ring capabilities struct
 * @dbr_ring_cfg: Pointer to direct buf rx ring config struct
 * @dbr_buf_pool: Pointer to direct buf rx buffer pool struct
 * @dbr_rsp_handler: Pointer to direct buf rx response handler for the module
 */
struct direct_buf_rx_module_param {
	enum DBR_MODULE mod_id;
	uint8_t pdev_id;
	uint8_t srng_id;
	struct dbr_module_config dbr_config;
	struct direct_buf_rx_ring_cap *dbr_ring_cap;
	struct direct_buf_rx_ring_cfg *dbr_ring_cfg;
	struct direct_buf_rx_buf_info *dbr_buf_pool;
	bool (*dbr_rsp_handler)(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_data *dbr_data);
};

/**
 * struct direct_buf_rx_pdev_obj - Direct Buf RX pdev object struct
 * @num_modules: Number of modules registered to DBR for the pdev
 * @dbr_mod_param: Pointer to direct buf rx module param struct
 * @dbr_mod_debug: Pointer to the array of DBR module debug structures
 * @debugfs_entry: DBR debugfs entry of this radio
 */
struct direct_buf_rx_pdev_obj {
	uint32_t num_modules;
	struct direct_buf_rx_module_param (*dbr_mod_param)[DBR_SRNG_NUM];
#ifdef DIRECT_BUF_RX_DEBUG
	struct direct_buf_rx_module_debug *dbr_mod_debug;
#ifdef WLAN_DEBUGFS
	qdf_dentry_t debugfs_entry;
#endif
#endif
};

/**
 * struct direct_buf_rx_psoc_obj - Direct Buf RX psoc object struct
 * @hal_soc: Opaque HAL SOC handle
 * @osdev: QDF os device handle
 * @dbr_pdev_objs: array of DBR pdev objects
 * @mem_list: list for holding the large memories during the entire
 *  PSOC lifetime
 * @mem_list_lock: spin lock for the memory list
 */
struct direct_buf_rx_psoc_obj {
	void *hal_soc;
	qdf_device_t osdev;
	struct direct_buf_rx_pdev_obj *dbr_pdev_obj[WLAN_UMAC_MAX_PDEVS];
#if defined(DBR_HOLD_LARGE_MEM) && defined(CNSS_MEM_PRE_ALLOC)
	qdf_list_t mem_list[WLAN_UMAC_MAX_PDEVS];
	qdf_spinlock_t mem_list_lock;
#endif
};

/**
 * struct module_ring_params - Direct buf ring params for module
 * @num_bufs: Number of buffers alloted to this module
 * @buf_size: size of buffers
 */
struct module_ring_params {
	uint32_t num_bufs;
	uint32_t buf_size;
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
 * target_if_direct_buf_rx_print_ring_stat() - Print ring status for each
 *                                             module in the pdev
 * @pdev: pointer to pdev object
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_print_ring_stat(
				struct wlan_objmgr_pdev *pdev);

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
 * @srng_id: srng ID
 *
 * Return : QDF status of operation
 */
QDF_STATUS target_if_deinit_dbr_ring(struct wlan_objmgr_pdev *pdev,
				struct direct_buf_rx_pdev_obj *dbr_pdev_obj,
				enum DBR_MODULE mod_id, uint8_t srng_id);
/**
 * target_if_direct_buf_rx_module_register() - Function to register to direct
 *                                             buffer rx module
 * @pdev: pointer to pdev object
 * @mod_id: module id indicating the module using direct buffer rx framework
 * @dbr_config: dbr module configuration params
 * @dbr_rsp_handler: function pointer pointing to the response handler to be
 *                   invoked for the module registering to direct buffer rx
 *                   module
 *
 * Return: QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_module_register(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id,
			struct dbr_module_config *dbr_config,
			bool (*dbr_rsp_handler)
			     (struct wlan_objmgr_pdev *pdev,
			      struct direct_buf_rx_data *dbr_data));

/**
 * target_if_direct_buf_rx_module_unregister() - Function to unregister to
 *                                               direct buffer rx module
 * @pdev: pointer to pdev object
 * @mod_id: module id indicating the module using direct buffer rx framework
 *
 * Return: QDF status of operation
 */
QDF_STATUS target_if_direct_buf_rx_module_unregister(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id);

/**
 * target_if_direct_buf_rx_get_ring_params() - Function to get ring parameters
 *                                             for module_id
 * @pdev: pointer to pdev object
 * @module_ring_params: pointer to store ring params
 * @mod_id: module idindicating module using direct buffer rx framework
 * @srng_id: srng ID
 */
QDF_STATUS
target_if_direct_buf_rx_get_ring_params(struct wlan_objmgr_pdev *pdev,
					struct module_ring_params *param,
					uint8_t mod_id, uint8_t srng_id);

/**
 * target_if_dbr_start_ring_debug() - Start DBR ring debug
 * @pdev: pointer to pdev object
 * @mod_id: module ID indicating the module using direct buffer rx framework
 * @num_ring_debug_entries: Size of the ring debug entries
 */
QDF_STATUS target_if_dbr_start_ring_debug(struct wlan_objmgr_pdev *pdev,
					  uint8_t mod_id,
					  uint32_t num_ring_debug_entries);

/**
 * target_if_dbr_stop_ring_debug() - Stop DBR ring debug
 * @pdev: pointer to pdev object
 * @mod_id: module ID indicating the module using direct buffer rx framework
 */
QDF_STATUS target_if_dbr_stop_ring_debug(struct wlan_objmgr_pdev *pdev,
					 uint8_t mod_id);

/**
 * target_if_dbr_start_buffer_poisoning() - Start DBR buffer poisoning
 * @pdev: pointer to pdev object
 * @mod_id: module ID indicating the module using direct buffer rx framework
 * @value: Value with which buffers should be poisoned
 *
 * Only those buffers which are going to be mapped to the device after this
 * API call are guaranteed to be poisoned. If user wants all the buffers in
 * the ring to be poisoned from their creation time then this API should be
 * called before module's registration to the DBR.
 *
 */
QDF_STATUS target_if_dbr_start_buffer_poisoning(struct wlan_objmgr_pdev *pdev,
						uint8_t mod_id, uint32_t value);

/**
 * target_if_dbr_stop_buffer_poisoning() - Stop DBR buffer poisoning
 * @pdev: pointer to pdev object
 * @mod_id: module ID indicating the module using direct buffer rx framework
 */
QDF_STATUS target_if_dbr_stop_buffer_poisoning(struct wlan_objmgr_pdev *pdev,
					       uint8_t mod_id);
#endif /* _TARGET_IF_DIRECT_BUF_RX_MAIN_H_ */
