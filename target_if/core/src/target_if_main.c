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
#ifdef WLAN_PMO_ENABLE
#include "target_if_pmo_main.h"
#endif
#ifdef WLAN_ATF_ENABLE
#include "target_if_atf.h"
#endif
#ifdef WLAN_SA_API_ENABLE
#include "target_if_sa_api.h"
#endif
#ifdef WLAN_CONV_SPECTRAL_ENABLE
#include "target_if_spectral.h"
#endif
#include <target_if_reg.h>
#include <target_if_scan.h>
#ifdef DFS_COMPONENT_ENABLE
#include <target_if_dfs.h>
#endif

#ifdef CONVERGED_P2P_ENABLE
#include "target_if_p2p.h"
#endif

#ifdef WIFI_POS_CONVERGED
#include "target_if_wifi_pos.h"
#endif

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
#include "target_if_nan.h"
#endif /* WLAN_FEATURE_NAN_CONVERGENCE */
#ifdef CONVERGED_TDLS_ENABLE
#include "target_if_tdls.h"
#endif
#ifdef QCA_SUPPORT_SON
#include <target_if_son.h>
#endif

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
	g_target_if_ctx->service_ready_cb = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	qdf_spinlock_destroy(&g_target_if_ctx->lock);
	qdf_mem_free(g_target_if_ctx);
	g_target_if_ctx = NULL;

	return QDF_STATUS_SUCCESS;
}

#ifndef WLAN_ATF_ENABLE
static void target_if_atf_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_ATF_ENABLE */

#ifndef WLAN_SA_API_ENABLE
static void target_if_sa_api_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_SA_API_ENABLE */

#ifdef WIFI_POS_CONVERGED
static void target_if_wifi_pos_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_wifi_pos_register_tx_ops(tx_ops);
}
#else
static void target_if_wifi_pos_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif
#ifdef QCA_SUPPORT_SON
static void target_if_son_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_son_register_tx_ops(tx_ops);
	return;
}
#else
static void target_if_son_tx_ops_register(
			struct wlan_lmac_if_tx_ops *tx_ops)
{
	return;
}
#endif

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
static void target_if_nan_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_nan_register_tx_ops(tx_ops);
}
#else
static void target_if_nan_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_FEATURE_NAN_CONVERGENCE */

#ifdef CONVERGED_TDLS_ENABLE
static void target_if_tdls_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_tdls_register_tx_ops(tx_ops);
}
#else
static void target_if_tdls_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* CONVERGED_TDLS_ENABLE */

#ifdef DFS_COMPONENT_ENABLE
static void target_if_dfs_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_register_dfs_tx_ops(tx_ops);
}
#else
static void target_if_dfs_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* DFS_COMPONENT_ENABLE */

#ifdef WLAN_CONV_SPECTRAL_ENABLE
static void target_if_sptrl_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_sptrl_register_tx_ops(tx_ops);
}
#else
static void target_if_sptrl_tx_ops_register(
				struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

static
QDF_STATUS target_if_register_umac_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	/* call regulatory callback to register tx ops */
	target_if_register_regulatory_tx_ops(tx_ops);

	/* call umac callback to register legacy tx ops */
	wlan_lmac_if_umac_tx_ops_register(tx_ops);

	/* Register scan tx ops */
	target_if_register_scan_tx_ops(&tx_ops->scan);

	target_if_atf_tx_ops_register(tx_ops);

	target_if_sa_api_tx_ops_register(tx_ops);

	target_if_wifi_pos_tx_ops_register(tx_ops);

	target_if_nan_tx_ops_register(tx_ops);

	target_if_dfs_tx_ops_register(tx_ops);

	target_if_son_tx_ops_register(tx_ops);

	target_if_tdls_tx_ops_register(tx_ops);
	/* Converged UMAC components to register their TX-ops here */
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_PMO_ENABLE
/**
 * target_if_pmo_register_tx_ops_req() - Wrapper API to register pmo tx ops.
 * @tx_ops: psoc's lmac if transmit operations
 *
 * Return: None
 */
static void target_if_pmo_register_tx_ops_req(
		struct wlan_lmac_if_tx_ops *tx_ops)
{
	target_if_pmo_register_tx_ops(tx_ops);
}
#else
/**
 * target_if_pmo_register_tx_ops_req() - Dummy API to register pmo tx ops.
 * @tx_ops: psoc's lmac if transmit operations
 *
 * Return: None
 */
static void target_if_pmo_register_tx_ops_req(
		struct wlan_lmac_if_tx_ops *tx_ops)
{
}
#endif /* WLAN_PMO_ENABLE */

QDF_STATUS target_if_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	/* Converged UMAC components to register their TX-ops */
	target_if_register_umac_tx_ops(tx_ops);

	/* Components parallel to UMAC to register their TX-ops here */
	target_if_pmo_register_tx_ops_req(tx_ops);
	target_if_sptrl_tx_ops_register(tx_ops);

#ifdef CONVERGED_P2P_ENABLE
	/* Converged UMAC components to register P2P TX-ops */
	target_if_p2p_register_tx_ops(tx_ops);
#endif
#ifdef CONVERGED_TDLS_ENABLE
	target_if_tdls_register_tx_ops(tx_ops);
#endif

	return QDF_STATUS_SUCCESS;
}

wmi_legacy_service_ready_callback
target_if_get_psoc_legacy_service_ready_cb(void)
{
	wmi_legacy_service_ready_callback service_ready_cb;

	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	if (g_target_if_ctx->service_ready_cb)
		service_ready_cb = g_target_if_ctx->service_ready_cb;
	else
		service_ready_cb = NULL;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return service_ready_cb;
}
EXPORT_SYMBOL(target_if_get_psoc_legacy_service_ready_cb);

QDF_STATUS target_if_register_legacy_service_ready_cb(
	wmi_legacy_service_ready_callback service_ready_cb)
{
	qdf_spin_lock_bh(&g_target_if_ctx->lock);
	g_target_if_ctx->service_ready_cb = service_ready_cb;
	qdf_spin_unlock_bh(&g_target_if_ctx->lock);

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(target_if_register_legacy_service_ready_cb);
