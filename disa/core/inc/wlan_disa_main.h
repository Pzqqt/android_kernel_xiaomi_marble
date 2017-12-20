/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: declare various api which shall be used by
 * DISA user configuration and target interface
 */

#ifndef _WLAN_DISA_MAIN_H_
#define _WLAN_DISA_MAIN_H_

#include "wlan_disa_public_struct.h"
#include "wlan_disa_obj_mgmt_public_struct.h"
#include "wlan_disa_priv.h"
#include "wlan_disa_objmgr.h"

#define disa_log(level, args...) QDF_TRACE(QDF_MODULE_ID_DISA, level, ## args)
#define disa_logfl(level, format, args...) disa_log(level, FL(format), ## args)

#define disa_fatal(format, args...) \
		disa_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define disa_err(format, args...) \
		disa_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define disa_warn(format, args...) \
		disa_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define disa_info(format, args...) \
		disa_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define disa_debug(format, args...) \
		disa_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define DISA_ENTER() disa_debug("enter")
#define DISA_EXIT() disa_debug("exit")

/**
 * disa_allocate_ctx() - Api to allocate disa ctx
 *
 * Helper function to allocate disa ctx
 *
 * Return: Success or failure.
 */
QDF_STATUS disa_allocate_ctx(void);

/**
 * disa_free_ctx() - to free disa context
 *
 * Helper function to free disa context
 *
 * Return: None.
 */
void disa_free_ctx(void);

/**
 * disa_get_context() - to get disa context
 *
 * Helper function to get disa context
 *
 * Return: disa context.
 */
struct wlan_disa_ctx *disa_get_context(void);

/**
 * disa_core_encrypt_decrypt_req() - Form encrypt/decrypt request
 * @psoc: objmgr psoc object
 * @req: DISA encrypt/decrypt request parameters
 *
 * Return: QDF status success or failure
 */
QDF_STATUS disa_core_encrypt_decrypt_req(struct wlan_objmgr_psoc *psoc,
		struct disa_encrypt_decrypt_req_params *req,
		encrypt_decrypt_resp_callback cb,
		void *cookie);

#endif /* end  of _WLAN_DISA_MAIN_H_ */
