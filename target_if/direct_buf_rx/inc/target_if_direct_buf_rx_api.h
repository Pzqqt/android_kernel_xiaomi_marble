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

#ifndef _TARGET_IF_DIRECT_BUF_RX_API_H_
#define _TARGET_IF_DIRECT_BUF_RX_API_H_

#include "qdf_nbuf.h"
#include "qdf_atomic.h"

#define direct_buf_rx_log(level, args...) \
		QDF_TRACE(QDF_MODULE_ID_DIRECT_BUF_RX, level, ## args)
#define direct_buf_rx_logfl(level, format, args...) \
		direct_buf_rx_log(level, FL(format), ## args)
#define direct_buf_alert(format, args...) \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define direct_buf_rx_err(format, args...) \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define direct_buf_rx_warn(format, args...) \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define direct_buf_rx_notice(format, args...) \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define direct_buf_rx_info(format, args...) \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_INFO_HIGH, format, ## args)
#define direct_buf_rx_debug(format, args...) \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)
#define direct_buf_rx_enter() \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_DEBUG, "enter")
#define direct_buf_rx_exit() \
		direct_buf_rx_logfl(QDF_TRACE_LEVEL_DEBUG, "exit")

#define DBR_MAX_CHAINS      (8)

struct wlan_objmgr_psoc;
struct wlan_lmac_if_tx_ops;

/**
 * struct direct_buf_rx_data - direct buffer rx data
 * @dbr_len: Length of the buffer DMAed
 * @vaddr: Virtual address of the buffer that has DMAed data
 * @meta_data_valid: Indicates that metadata is valid
 * @meta_data: Meta data
 */
struct direct_buf_rx_data {
	size_t dbr_len;
	void *vaddr;
	bool meta_data_valid;
	struct direct_buf_rx_metadata meta_data;
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

#endif /* _TARGET_IF_DIRECT_BUF_RX_API_H_ */
