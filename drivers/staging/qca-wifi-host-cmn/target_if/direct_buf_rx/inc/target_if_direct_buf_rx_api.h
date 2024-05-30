/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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

#ifndef _TARGET_IF_DIRECT_BUF_RX_API_H_
#define _TARGET_IF_DIRECT_BUF_RX_API_H_

#include "qdf_nbuf.h"
#include "qdf_atomic.h"
#include "wmi_unified_api.h"

/**
 * enum DBR_MODULE - Enum containing the modules supporting direct buf rx
 * @DBR_MODULE_SPECTRAL: Module ID for Spectral
 * @DBR_MODULE_CFR: Module ID for CFR
 * @DBR_MODULE_MAX: Max module ID
 */
enum DBR_MODULE {
	DBR_MODULE_SPECTRAL = 0,
	DBR_MODULE_CFR      = 1,
	DBR_MODULE_MAX,
};

#ifdef DIRECT_BUF_RX_ENABLE
#ifdef WLAN_DEBUGFS
#ifdef DIRECT_BUF_RX_DEBUG
/* Base debugfs entry for DBR module */
extern qdf_dentry_t dbr_debugfs_entry;
#endif /* DIRECT_BUF_RX_DEBUG */
#endif /* WLAN_DEBUGFS */

#define direct_buf_rx_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define direct_buf_rx_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define direct_buf_rx_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define direct_buf_rx_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define direct_buf_rx_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define direct_buf_rx_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define direct_buf_rx_enter() \
	QDF_TRACE_ENTER(QDF_MODULE_ID_DIRECT_BUF_RX, "enter")
#define direct_buf_rx_exit() \
	QDF_TRACE_EXIT(QDF_MODULE_ID_DIRECT_BUF_RX, "exit")

#define directbuf_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define directbuf_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define directbuf_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define directbuf_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_DIRECT_BUF_RX, params)
#define directbuf_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_DIRECT_BUF_RX, params)

#define DBR_MAX_CHAINS      (8)

struct wlan_objmgr_psoc;
struct wlan_lmac_if_tx_ops;

#ifdef WMI_DBR_SUPPORT
/**
 * struct direct_buf_rx_data - direct buffer rx data
 * @dbr_len: Length of the buffer DMAed
 * @vaddr: Virtual address of the buffer that has DMAed data
 * @cookie: Cookie for the buffer rxed from target
 * @paddr: physical address of buffer corresponding to vaddr
 * @meta_data_valid: Indicates that metadata is valid
 * @meta_data: Meta data
 */
struct direct_buf_rx_data {
	size_t dbr_len;
	void *vaddr;
	uint32_t cookie;
	qdf_dma_addr_t paddr;
	bool meta_data_valid;
	struct direct_buf_rx_metadata meta_data;
};
#endif

/**
 * struct dbr_module_config - module configuration for dbr
 * @num_resp_per_event: Number of events to be packed together
 * @event_timeout_in_ms: Timeout until which multiple events can be packed
 */
struct dbr_module_config {
	uint32_t num_resp_per_event;
	uint32_t event_timeout_in_ms;
};

/**
 * direct_buf_rx_init() - Function to initialize direct buf rx module
 *
 * Return: QDF status of operation
 */
QDF_STATUS direct_buf_rx_init(void);

/**
 * direct_buf_rx_deinit() - Function to deinitialize direct buf rx module
 *
 * Return: QDF status of operation
 */
QDF_STATUS direct_buf_rx_deinit(void);

/**
 * direct_buf_rx_target_attach() - Attach hal_soc,osdev in direct buf rx psoc obj
 * @psoc: pointer to psoc object
 * @hal_soc: Opaque HAL SOC handle
 * @osdev: QDF os device handle
 *
 * Return: QDF status of operation
 */
QDF_STATUS direct_buf_rx_target_attach(struct wlan_objmgr_psoc *psoc,
				void *hal_soc, qdf_device_t osdev);

/**
 * target_if_direct_buf_rx_register_tx_ops() - Register tx ops for direct buffer
 *                                             rx module
 * @tx_ops: pointer to lmac interface tx ops
 *
 * Return: None
 */
void target_if_direct_buf_rx_register_tx_ops(
				struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_dbr_cookie_lookup() - Function to retrieve cookie from
 *                                 buffer address(paddr)
 * @pdev: pointer to pdev object
 * @mod_id: module id indicating the module using direct buffer rx framework
 * @paddr: Physical address of buffer for which cookie info is required
 * @cookie: cookie will be returned in this param
 * @srng_id: srng ID
 *
 * Return: QDF status of operation
 */
QDF_STATUS target_if_dbr_cookie_lookup(struct wlan_objmgr_pdev *pdev,
				       uint8_t mod_id, qdf_dma_addr_t paddr,
				       uint32_t *cookie, uint8_t srng_id);

/**
 * target_if_dbr_buf_release() - Notify direct buf that a previously provided
 *                               buffer can be released.
 * @pdev: pointer to pdev object
 * @mod_id: module id indicating the module using direct buffer rx framework
 * @paddr: Physical address of buffer for which cookie info is required
 * @cookie: cookie value corresponding to the paddr
 * @srng_id: srng ID
 *
 * Return: QDF status of operation
 */
QDF_STATUS target_if_dbr_buf_release(struct wlan_objmgr_pdev *pdev,
				     uint8_t mod_id, qdf_dma_addr_t paddr,
				     uint32_t cookie, uint8_t srng_id);

/**
 * target_if_dbr_update_pdev_for_hw_mode_change() - Update DBR object in pdev
 * structure for hw mode change
 * @pdev: pointer to pdev object
 * @phy_idx: Phy index
 */
QDF_STATUS target_if_dbr_update_pdev_for_hw_mode_change(
		struct wlan_objmgr_pdev *pdev, int phy_idx);
#else /* DIRECT_BUF_RX_ENABLE*/

static inline QDF_STATUS
target_if_dbr_cookie_lookup(struct wlan_objmgr_pdev *pdev,
			    uint8_t mod_id, qdf_dma_addr_t paddr,
			    uint32_t *cookie, uint8_t srng_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_dbr_buf_release(struct wlan_objmgr_pdev *pdev,
			  uint8_t mod_id, qdf_dma_addr_t paddr,
			  uint32_t cookie, uint8_t srng_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_dbr_update_pdev_for_hw_mode_change(
		struct wlan_objmgr_pdev *pdev, int phy_idx)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DIRECT_BUF_RX_ENABLE */
#endif /* _TARGET_IF_DIRECT_BUF_RX_API_H_ */
