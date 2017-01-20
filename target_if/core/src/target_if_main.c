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
 * DOC: API for interacting with target interface.
 *
 */

#include "target_if.h"

static struct target_if_ctx *g_target_if_ctx;

struct target_if_ctx *target_if_get_ctx()
{
	return g_target_if_ctx;
}

struct wlan_objmgr_psoc *target_if_get_psoc_from_scn_hdl(void *scn_handle)
{
	struct wlan_objmgr_psoc *psoc;

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	if (scn_handle && g_target_if_ctx->get_psoc_hdl_cb)
		psoc = g_target_if_ctx->get_psoc_hdl_cb(scn_handle);
	else
		psoc = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return psoc;
}
EXPORT_SYMBOL(target_if_get_psoc_from_scn_hdl);

QDF_STATUS target_if_open(get_psoc_handle_callback psoc_hdl_cb)
{
	g_target_if_ctx = qdf_mem_malloc(sizeof(*g_target_if_ctx));
	if (!g_target_if_ctx) {
		target_if_err("Cannot allocate target if ctx");
		QDF_ASSERT(0);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&g_target_if_ctx->lock);

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->magic = TGT_MAGIC;
	g_target_if_ctx->get_psoc_hdl_cb = psoc_hdl_cb;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(target_if_open);

QDF_STATUS target_if_close(void)
{
	if (!g_target_if_ctx) {
		QDF_ASSERT(0);
		target_if_err("target if ctx is null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->magic = 0;
	g_target_if_ctx->get_psoc_hdl_cb = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	qdf_spinlock_destroy(&g_target_if_ctx->lock);
	qdf_mem_free(g_target_if_ctx);
	g_target_if_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(target_if_close);

QDF_STATUS target_if_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(target_if_register_tx_ops);


