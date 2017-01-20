/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This target interface shall be used
 *      to communicate with target using WMI.
 */
#ifndef _WLAN_TARGET_IF_H_
#define _WLAN_TARGET_IF_H_

#include "qdf_types.h"
#include "qdf_util.h"
#include "wlan_objmgr_psoc_obj.h"

/* ASCII "TGT\0" */
#define TGT_MAGIC 0x54575400

#define target_if_log(level, args...) \
		QDF_TRACE(QDF_MODULE_ID_TARGET_IF, level, ## args)
#define target_if_logfl(level, format, args...) \
		target_if_log(level, FL(format), ## args)

#define target_if_fatal(format, args...) \
		target_if_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define target_if_err(format, args...) \
		target_if_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define target_if_warn(format, args...) \
		target_if_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define target_if_info(format, args...) \
		target_if_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define target_if_debug(format, args...) \
		target_if_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define TARGET_IF_ENTER() target_if_logfl(QDF_TRACE_LEVEL_INFO, "enter")
#define TARGET_IF_EXIT() target_if_logfl(QDF_TRACE_LEVEL_INFO, "exit")

#define GET_WMI_HDL_FROM_PSOC(psoc) (psoc->tgt_if_handle)

typedef struct wlan_objmgr_psoc *(*get_psoc_handle_callback)(
			void *scn_handle);

/**
 * struct target_if_ctx - target_interface context
 * @magic: magic for target if ctx
 * @get_wmi_handle:  function pointer to get wmi handle
 * @lock: spin lock for protecting the ctx
 */
struct target_if_ctx {
	uint32_t magic;
	get_psoc_handle_callback get_psoc_hdl_cb;
	qdf_spinlock_t lock;
};

/**
 * target_if_open() - target_if open
 * @get_wmi_handle: function pointer to get wmi handle
 *
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_open(get_psoc_handle_callback psoc_hdl_cb);

/**
 * target_if_close() - Close target_if
 * @scn_handle: scn handle
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS target_if_close(void);

/**
 * wlan_get_tgt_if_ctx() -Get target if ctx
 *
 * Return: target if ctx
 */
struct target_if_ctx *target_if_get_ctx(void);

/**
 * target_if_get_psoc_from_scn_hdl() - get psoc from scn handle
 * @scn_handle: scn handle
 *
 * This API is generally used while processing wmi event.
 * In wmi event SCN handle will be passed by wmi hence
 * using this API we can get psoc from scn handle.
 *
 * Return: index for matching scn handle
 */
struct wlan_objmgr_psoc *target_if_get_psoc_from_scn_hdl(void *scn_handle);

/** target_if_register_tx_ops() - register tx_ops
 * @tx_ops: tx_ops structure
 *
 * This function is to be used by components to populate
 * the OL function pointers (tx_ops) required by the component
 * for UMAC-LMAC interaction, with the appropriate handler
 *
 * Return: QDF STATUS
 */
QDF_STATUS target_if_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

#endif

