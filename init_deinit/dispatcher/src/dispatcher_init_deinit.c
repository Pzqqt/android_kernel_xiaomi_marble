/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "cfg_dispatcher.h"
#include <qdf_types.h>
#include <qdf_trace.h>
#include <qdf_threads.h>
#include <qdf_module.h>
#include <dispatcher_init_deinit.h>
#include <scheduler_api.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_ftm_init_deinit_api.h>
#include <wlan_mgmt_txrx_utils_api.h>
#include <wlan_serialization_api.h>
#include "wlan_psoc_mlme_api.h"
#include <include/wlan_mlme_cmn.h>
#ifdef WLAN_ATF_ENABLE
#include <wlan_atf_utils_api.h>
#endif
#ifdef QCA_SUPPORT_SON
#include <wlan_son_pub.h>
#endif
#ifdef WLAN_SA_API_ENABLE
#include <wlan_sa_api_utils_api.h>
#endif
#ifdef WIFI_POS_CONVERGED
#include "wifi_pos_api.h"
#endif /* WIFI_POS_CONVERGED */
#include <wlan_reg_services_api.h>
#ifdef WLAN_CONV_CRYPTO_SUPPORTED
#include "wlan_crypto_main.h"
#endif
#ifdef DFS_COMPONENT_ENABLE
#include <wlan_dfs_init_deinit_api.h>
#endif

#ifdef WLAN_OFFCHAN_TXRX_ENABLE
#include <wlan_offchan_txrx_api.h>
#endif

#ifdef WLAN_SUPPORT_SPLITMAC
#include <wlan_splitmac.h>
#endif
#ifdef WLAN_CONV_SPECTRAL_ENABLE
#include <wlan_spectral_utils_api.h>
#endif
#ifdef WLAN_SUPPORT_FILS
#include <wlan_fd_utils_api.h>
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
#include <wlan_green_ap_api.h>
#endif

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_cp_stats_utils_api.h>
#endif

#ifdef WLAN_CFR_ENABLE
#include <wlan_cfr_utils_api.h>
#endif

#ifdef FEATURE_COEX
#include <wlan_coex_utils_api.h>
#endif

#ifdef DCS_INTERFERENCE_DETECTION
#include <wlan_dcs_init_deinit_api.h>
#endif

#ifdef WLAN_FEATURE_INTERFACE_MGR
#include <wlan_if_mgr_main.h>
#endif

/**
 * DOC: This file provides various init/deinit trigger point for new
 * components.
 */

/* All new components needs to replace their dummy init/deinit
 * psoc_open, psco_close, psoc_enable and psoc_disable APIs once
 * their actual handlers are ready
 */

struct dispatcher_spectral_ops ops_spectral;

#ifdef WLAN_CFR_ENABLE
static QDF_STATUS dispatcher_init_cfr(void)
{
	return wlan_cfr_init();
}

static QDF_STATUS dispatcher_deinit_cfr(void)
{
	return wlan_cfr_deinit();
}

static QDF_STATUS dispatcher_cfr_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	return wlan_cfr_pdev_open(pdev);
}

static QDF_STATUS dispatcher_cfr_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return wlan_cfr_pdev_close(pdev);
}
#else
static QDF_STATUS dispatcher_init_cfr(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_cfr(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_cfr_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_cfr_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

#endif

#ifdef QCA_SUPPORT_CP_STATS
static QDF_STATUS dispatcher_init_cp_stats(void)
{
	return wlan_cp_stats_init();
}

static QDF_STATUS dispatcher_deinit_cp_stats(void)
{
	return wlan_cp_stats_deinit();
}

static QDF_STATUS cp_stats_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_cp_stats_open(psoc);
}

static QDF_STATUS cp_stats_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_cp_stats_close(psoc);
}

static QDF_STATUS cp_stats_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_cp_stats_enable(psoc);
}

static QDF_STATUS cp_stats_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_cp_stats_disable(psoc);
}
#else
static QDF_STATUS dispatcher_init_cp_stats(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_cp_stats(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cp_stats_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cp_stats_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cp_stats_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cp_stats_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef DCS_INTERFERENCE_DETECTION
static QDF_STATUS dispatcher_init_dcs(void)
{
	return wlan_dcs_init();
}

static QDF_STATUS dispatcher_deinit_dcs(void)
{
	return wlan_dcs_deinit();
}

static QDF_STATUS dcs_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_dcs_enable(psoc);
}

static QDF_STATUS dcs_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_dcs_disable(psoc);
}

static QDF_STATUS dcs_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_dcs_psoc_open(psoc);
}

static QDF_STATUS dcs_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS dispatcher_init_dcs(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_dcs(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dcs_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dcs_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dcs_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dcs_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined QCA_SUPPORT_SON && QCA_SUPPORT_SON >= 1
static QDF_STATUS dispatcher_init_son(void)
{
	return wlan_son_init();
}
static QDF_STATUS son_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_son_psoc_open(psoc);
}
static QDF_STATUS dispatcher_deinit_son(void)
{
	return wlan_son_deinit();
}

static QDF_STATUS son_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_son_psoc_close(psoc);
}
#else
static QDF_STATUS dispatcher_init_son(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_son(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS son_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS son_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* END of QCA_SUPPORT_SON */

static QDF_STATUS dispatcher_regulatory_init(void)
{
	return wlan_regulatory_init();
}

static QDF_STATUS dispatcher_regulatory_deinit(void)
{
	return wlan_regulatory_deinit();
}

static QDF_STATUS dispatcher_regulatory_psoc_open(struct wlan_objmgr_psoc
						  *psoc)
{
	return regulatory_psoc_open(psoc);
}

static QDF_STATUS dispatcher_regulatory_psoc_close(struct wlan_objmgr_psoc
						   *psoc)
{
	return regulatory_psoc_close(psoc);
}

#ifdef WLAN_CONV_SPECTRAL_ENABLE
#ifdef SPECTRAL_MODULIZED_ENABLE
QDF_STATUS
dispatcher_register_spectral_ops_handler(struct dispatcher_spectral_ops *sops)
{
	qdf_mem_copy(&ops_spectral, sops,
		     qdf_min(sizeof(*sops), sizeof(ops_spectral)));

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(dispatcher_register_spectral_ops_handler);

static QDF_STATUS dispatcher_spectral_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	return ops_spectral.spectral_pdev_open_handler(pdev);
}

static QDF_STATUS dispatcher_spectral_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS spectral_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return ops_spectral.spectral_psoc_open_handler(psoc);
}

static QDF_STATUS spectral_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return ops_spectral.spectral_psoc_close_handler(psoc);
}

static QDF_STATUS spectral_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return ops_spectral.spectral_psoc_enable_handler(psoc);
}

static QDF_STATUS spectral_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return ops_spectral.spectral_psoc_disable_handler(psoc);
}
#else
static QDF_STATUS dispatcher_spectral_pdev_open(struct wlan_objmgr_pdev
						  *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_spectral_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS spectral_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_spectral_psoc_open(psoc);
}

static QDF_STATUS spectral_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_spectral_psoc_close(psoc);
}

static QDF_STATUS spectral_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_spectral_psoc_enable(psoc);
}

static QDF_STATUS spectral_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_spectral_psoc_disable(psoc);
}
#endif
#else
static QDF_STATUS dispatcher_spectral_pdev_open(struct wlan_objmgr_pdev
						  *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_spectral_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS spectral_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS spectral_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS spectral_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS spectral_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS dispatcher_regulatory_pdev_open(struct wlan_objmgr_pdev
						  *pdev)
{
	return regulatory_pdev_open(pdev);
}

static QDF_STATUS dispatcher_regulatory_pdev_close(struct wlan_objmgr_pdev
						  *pdev)
{
	return regulatory_pdev_close(pdev);
}

#ifdef WLAN_SA_API_ENABLE
static QDF_STATUS dispatcher_init_sa_api(void)
{
	return wlan_sa_api_init();
}

static QDF_STATUS dispatcher_deinit_sa_api(void)
{
	return wlan_sa_api_deinit();
}

static QDF_STATUS sa_api_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_sa_api_enable(psoc);
}

static QDF_STATUS sa_api_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_sa_api_disable(psoc);
}
#else
static QDF_STATUS dispatcher_init_sa_api(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_sa_api(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS sa_api_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS sa_api_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* END of WLAN_SA_API_ENABLE */


#ifdef WLAN_ATF_ENABLE
static QDF_STATUS dispatcher_init_atf(void)
{
	return wlan_atf_init();
}

static QDF_STATUS dispatcher_deinit_atf(void)
{
	return wlan_atf_deinit();
}

static QDF_STATUS atf_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_atf_open(psoc);
}

static QDF_STATUS atf_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_atf_close(psoc);
}

static QDF_STATUS atf_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_atf_enable(psoc);
}

static QDF_STATUS atf_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_atf_disable(psoc);
}
#else
static QDF_STATUS dispatcher_init_atf(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_atf(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS atf_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS atf_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS atf_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS atf_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* END of WLAN_ATF_ENABLE */

#ifdef WLAN_CONV_CRYPTO_SUPPORTED
static QDF_STATUS dispatcher_init_crypto(void)
{
	return wlan_crypto_init();
}

static QDF_STATUS dispatcher_deinit_crypto(void)
{
	return wlan_crypto_deinit();
}
#else
static QDF_STATUS dispatcher_init_crypto(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_crypto(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* END of WLAN_CONV_CRYPTO_SUPPORTED */

#ifdef WIFI_POS_CONVERGED
static QDF_STATUS dispatcher_init_wifi_pos(void)
{
	return wifi_pos_init();
}

static QDF_STATUS dispatcher_deinit_wifi_pos(void)
{
	return wifi_pos_deinit();
}

static QDF_STATUS dispatcher_wifi_pos_enable(struct wlan_objmgr_psoc *psoc)
{
	return wifi_pos_psoc_enable(psoc);
}

static QDF_STATUS dispatcher_wifi_pos_disable(struct wlan_objmgr_psoc *psoc)
{
	return wifi_pos_psoc_disable(psoc);
}
#else
static QDF_STATUS dispatcher_init_wifi_pos(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_wifi_pos(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_wifi_pos_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_wifi_pos_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef DFS_COMPONENT_ENABLE
static QDF_STATUS dispatcher_init_dfs(void)
{
	return dfs_init();
}

static QDF_STATUS dispatcher_deinit_dfs(void)
{
	return dfs_deinit();
}

static QDF_STATUS dispatcher_dfs_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wifi_dfs_psoc_enable(psoc);
}

static QDF_STATUS dispatcher_dfs_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wifi_dfs_psoc_disable(psoc);
}
#else
static QDF_STATUS dispatcher_init_dfs(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_deinit_dfs(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_dfs_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_dfs_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_OFFCHAN_TXRX_ENABLE
static QDF_STATUS dispatcher_offchan_txrx_init(void)
{
	return wlan_offchan_txrx_init();
}

static QDF_STATUS dispatcher_offchan_txrx_deinit(void)
{
	return wlan_offchan_txrx_deinit();
}
#else
static QDF_STATUS dispatcher_offchan_txrx_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_offchan_txrx_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif /*WLAN_OFFCHAN_TXRX_ENABLE*/

#ifdef WLAN_SUPPORT_SPLITMAC
static QDF_STATUS dispatcher_splitmac_init(void)
{
	return wlan_splitmac_init();
}

static QDF_STATUS dispatcher_splitmac_deinit(void)
{
	return wlan_splitmac_deinit();
}
#else
static QDF_STATUS dispatcher_splitmac_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_splitmac_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif  /* WLAN_SUPPORT_SPLITMAC */

#ifdef WLAN_CONV_SPECTRAL_ENABLE
#ifndef SPECTRAL_MODULIZED_ENABLE
static QDF_STATUS dispatcher_spectral_init(void)
{
	return wlan_spectral_init();
}

static QDF_STATUS dispatcher_spectral_deinit(void)
{
	return wlan_spectral_deinit();
}
#else
static QDF_STATUS dispatcher_spectral_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_spectral_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#else
static QDF_STATUS dispatcher_spectral_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_spectral_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef DIRECT_BUF_RX_ENABLE
static QDF_STATUS dispatcher_dbr_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		qdf_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (tx_ops->dbr_tx_ops.direct_buf_rx_register_events)
		return tx_ops->dbr_tx_ops.direct_buf_rx_register_events(psoc);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_dbr_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		qdf_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (tx_ops->dbr_tx_ops.direct_buf_rx_unregister_events)
		return tx_ops->dbr_tx_ops.direct_buf_rx_unregister_events(psoc);

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS dispatcher_dbr_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_dbr_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* DIRECT_BUF_RX_ENABLE */

#ifdef WLAN_SUPPORT_GREEN_AP
static QDF_STATUS dispatcher_green_ap_init(void)
{
	return wlan_green_ap_init();
}

static QDF_STATUS dispatcher_green_ap_pdev_open(
				struct wlan_objmgr_pdev *pdev)
{
	return wlan_green_ap_pdev_open(pdev);
}

/* Only added this for symmetry */
static QDF_STATUS dispatcher_green_ap_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_green_ap_deinit(void)
{
	return wlan_green_ap_deinit();
}
#else
static QDF_STATUS dispatcher_green_ap_init(void)
{
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS dispatcher_green_ap_pdev_open(
				struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

/* Only added this for symmetry */
static QDF_STATUS dispatcher_green_ap_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_green_ap_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_SUPPORT_FILS
static QDF_STATUS dispatcher_fd_init(void)
{
	return wlan_fd_init();
}

static QDF_STATUS dispatcher_fd_deinit(void)
{
	return wlan_fd_deinit();
}

static QDF_STATUS fd_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_fd_enable(psoc);
}

static QDF_STATUS fd_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return wlan_fd_disable(psoc);
}
#else
static QDF_STATUS dispatcher_fd_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_fd_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS fd_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS fd_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_SUPPORT_FILS */

#ifdef WLAN_FEATURE_INTERFACE_MGR
static QDF_STATUS dispatcher_if_mgr_init(void)
{
	return wlan_if_mgr_init();
}

static QDF_STATUS dispatcher_if_mgr_deinit(void)
{
	return wlan_if_mgr_deinit();
}
#else
static QDF_STATUS dispatcher_if_mgr_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS dispatcher_if_mgr_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef FEATURE_COEX
static QDF_STATUS dispatcher_coex_init(void)
{
	return wlan_coex_init();
}

static QDF_STATUS dispatcher_coex_deinit(void)
{
	return wlan_coex_deinit();
}

static QDF_STATUS dispatcher_coex_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return wlan_coex_psoc_open(psoc);
}

static QDF_STATUS dispatcher_coex_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return wlan_coex_psoc_close(psoc);
}
#else
static inline QDF_STATUS dispatcher_coex_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dispatcher_coex_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
dispatcher_coex_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
dispatcher_coex_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_COEX */

QDF_STATUS dispatcher_init(void)
{
	if (QDF_STATUS_SUCCESS != wlan_objmgr_global_obj_init())
		goto out;

	if (QDF_STATUS_SUCCESS != wlan_mgmt_txrx_init())
		goto mgmt_txrx_init_fail;

	if (QDF_STATUS_SUCCESS != ucfg_scan_init())
		goto ucfg_scan_init_fail;

	if (QDF_STATUS_SUCCESS != wlan_serialization_init())
		goto serialization_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_crypto())
		goto crypto_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_cp_stats())
		goto cp_stats_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_dcs())
		goto dcs_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_atf())
		goto atf_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_sa_api())
		goto sa_api_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_wifi_pos())
		goto wifi_pos_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_dfs())
		goto dfs_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_regulatory_init())
		goto regulatory_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_offchan_txrx_init())
		goto offchan_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_son())
		goto son_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_splitmac_init())
		goto splitmac_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_fd_init())
		goto fd_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_green_ap_init())
		goto green_ap_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_ftm_init())
		goto ftm_init_fail;

	if (QDF_IS_STATUS_ERROR(cfg_dispatcher_init()))
		goto cfg_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_spectral_init())
		goto spectral_init_fail;

	if (QDF_STATUS_SUCCESS != wlan_cmn_mlme_init())
		goto cmn_mlme_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_init_cfr())
		goto cfr_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_coex_init())
		goto coex_init_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_if_mgr_init())
		goto ifmgr_init_fail;

	/*
	 * scheduler INIT has to be the last as each component's
	 * initialization has to happen first and then at the end
	 * scheduler needs to start accepting the service.
	 */
	if (QDF_STATUS_SUCCESS != scheduler_init())
		goto scheduler_init_fail;

	return QDF_STATUS_SUCCESS;

scheduler_init_fail:
	dispatcher_if_mgr_deinit();
ifmgr_init_fail:
	dispatcher_coex_deinit();
coex_init_fail:
	dispatcher_deinit_cfr();
cfr_init_fail:
	wlan_cmn_mlme_deinit();
cmn_mlme_init_fail:
	dispatcher_spectral_deinit();
spectral_init_fail:
	cfg_dispatcher_deinit();
cfg_init_fail:
	dispatcher_ftm_deinit();
ftm_init_fail:
	dispatcher_green_ap_deinit();
green_ap_init_fail:
	dispatcher_fd_deinit();
fd_init_fail:
	dispatcher_splitmac_deinit();
splitmac_init_fail:
	dispatcher_deinit_son();
son_init_fail:
	dispatcher_offchan_txrx_deinit();
offchan_init_fail:
	dispatcher_regulatory_deinit();
regulatory_init_fail:
	dispatcher_deinit_dfs();
dfs_init_fail:
	dispatcher_deinit_wifi_pos();
wifi_pos_init_fail:
	dispatcher_deinit_sa_api();
sa_api_init_fail:
	dispatcher_deinit_atf();
atf_init_fail:
	dispatcher_deinit_dcs();
dcs_init_fail:
	dispatcher_deinit_cp_stats();
cp_stats_init_fail:
	dispatcher_deinit_crypto();
crypto_init_fail:
	wlan_serialization_deinit();
serialization_init_fail:
	ucfg_scan_deinit();
ucfg_scan_init_fail:
	wlan_mgmt_txrx_deinit();
mgmt_txrx_init_fail:
	wlan_objmgr_global_obj_deinit();

out:
	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(dispatcher_init);

QDF_STATUS dispatcher_deinit(void)
{
	QDF_STATUS status;

	QDF_BUG(QDF_STATUS_SUCCESS == scheduler_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_if_mgr_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_coex_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_cfr());

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_cmn_mlme_deinit());

	status = cfg_dispatcher_deinit();
	QDF_BUG(QDF_IS_STATUS_SUCCESS(status));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_ftm_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_green_ap_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_fd_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_spectral_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_splitmac_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_son());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_offchan_txrx_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_regulatory_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_dfs());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_wifi_pos());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_sa_api());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_atf());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_cp_stats());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_dcs());

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_deinit_crypto());

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_serialization_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == ucfg_scan_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_mgmt_txrx_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_objmgr_global_obj_deinit());

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(dispatcher_deinit);

QDF_STATUS dispatcher_enable(void)
{
	QDF_STATUS status;

	status = scheduler_enable();

	return status;
}
qdf_export_symbol(dispatcher_enable);

QDF_STATUS dispatcher_disable(void)
{
	QDF_BUG(QDF_IS_STATUS_SUCCESS(scheduler_disable()));

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(dispatcher_disable);

QDF_STATUS dispatcher_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	if (QDF_STATUS_SUCCESS != wlan_mgmt_txrx_psoc_open(psoc))
		goto out;

	if (QDF_STATUS_SUCCESS != ucfg_scan_psoc_open(psoc))
		goto scan_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != cp_stats_psoc_open(psoc))
		goto cp_stats_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != atf_psoc_open(psoc))
		goto atf_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_regulatory_psoc_open(psoc))
		goto regulatory_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != son_psoc_open(psoc))
		goto psoc_son_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_ftm_psoc_open(psoc))
		goto ftm_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_coex_psoc_open(psoc))
		goto coex_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != dcs_psoc_open(psoc))
		goto dcs_psoc_open_fail;

	status = spectral_psoc_open(psoc);
	if (status != QDF_STATUS_SUCCESS && status != QDF_STATUS_COMP_DISABLED)
		goto spectral_psoc_open_fail;

	if (QDF_IS_STATUS_ERROR(mlme_psoc_open(psoc)))
		goto mlme_psoc_open_fail;

	return QDF_STATUS_SUCCESS;

mlme_psoc_open_fail:
	spectral_psoc_close(psoc);
spectral_psoc_open_fail:
	dcs_psoc_close(psoc);
dcs_psoc_open_fail:
	dispatcher_coex_psoc_close(psoc);
coex_psoc_open_fail:
	dispatcher_ftm_psoc_close(psoc);
ftm_psoc_open_fail:
	son_psoc_close(psoc);
psoc_son_fail:
	regulatory_psoc_close(psoc);
regulatory_psoc_open_fail:
	atf_psoc_close(psoc);
atf_psoc_open_fail:
	cp_stats_psoc_close(psoc);
cp_stats_psoc_open_fail:
	ucfg_scan_psoc_close(psoc);
scan_psoc_open_fail:
	wlan_mgmt_txrx_psoc_close(psoc);

out:
	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(dispatcher_psoc_open);

QDF_STATUS dispatcher_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	QDF_BUG(QDF_STATUS_SUCCESS == mlme_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dcs_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_coex_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_ftm_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == son_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_regulatory_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == atf_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == cp_stats_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == ucfg_scan_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_mgmt_txrx_psoc_close(psoc));

	status = spectral_psoc_close(psoc);
	QDF_BUG((status == QDF_STATUS_SUCCESS) ||
		(status == QDF_STATUS_COMP_DISABLED));

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(dispatcher_psoc_close);

QDF_STATUS dispatcher_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	if (QDF_STATUS_SUCCESS != wlan_serialization_psoc_enable(psoc))
		goto out;

	if (QDF_STATUS_SUCCESS != ucfg_scan_psoc_enable(psoc))
		goto serialization_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != sa_api_psoc_enable(psoc))
		goto sa_api_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != cp_stats_psoc_enable(psoc))
		goto cp_stats_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != dcs_psoc_enable(psoc))
		goto dcs_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != atf_psoc_enable(psoc))
		goto atf_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_wifi_pos_enable(psoc))
		goto wifi_pos_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_dfs_psoc_enable(psoc))
		goto wifi_dfs_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != fd_psoc_enable(psoc))
		goto fd_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != dispatcher_dbr_psoc_enable(psoc))
		goto dbr_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != wlan_mlme_psoc_enable(psoc))
		goto mlme_psoc_enable_fail;

	status = spectral_psoc_enable(psoc);
	if (status != QDF_STATUS_SUCCESS && status != QDF_STATUS_COMP_DISABLED)
		goto spectral_psoc_enable_fail;

	return QDF_STATUS_SUCCESS;

spectral_psoc_enable_fail:
	wlan_mlme_psoc_disable(psoc);
mlme_psoc_enable_fail:
	dispatcher_dbr_psoc_disable(psoc);
dbr_psoc_enable_fail:
	fd_psoc_disable(psoc);
fd_psoc_enable_fail:
	dispatcher_dfs_psoc_disable(psoc);
wifi_dfs_psoc_enable_fail:
	dispatcher_wifi_pos_disable(psoc);
wifi_pos_psoc_enable_fail:
	atf_psoc_disable(psoc);
atf_psoc_enable_fail:
	dcs_psoc_disable(psoc);
dcs_psoc_enable_fail:
	cp_stats_psoc_disable(psoc);
cp_stats_psoc_enable_fail:
	sa_api_psoc_disable(psoc);
sa_api_psoc_enable_fail:
	ucfg_scan_psoc_disable(psoc);
serialization_psoc_enable_fail:
	wlan_serialization_psoc_disable(psoc);
out:
	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(dispatcher_psoc_enable);

QDF_STATUS dispatcher_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_STATUS status;

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_mlme_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_dbr_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == fd_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_dfs_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_wifi_pos_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == atf_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == cp_stats_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == sa_api_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == ucfg_scan_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_serialization_psoc_disable(psoc));

	status = spectral_psoc_disable(psoc);
	QDF_BUG((status == QDF_STATUS_SUCCESS) ||
		(status == QDF_STATUS_COMP_DISABLED));

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(dispatcher_psoc_disable);

QDF_STATUS dispatcher_pdev_open(struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS status;

	if (QDF_STATUS_SUCCESS != dispatcher_regulatory_pdev_open(pdev))
		goto regulatory_pdev_open_fail;

	status = dispatcher_spectral_pdev_open(pdev);
	if (status != QDF_STATUS_SUCCESS && status != QDF_STATUS_COMP_DISABLED)
		goto spectral_pdev_open_fail;

	status = dispatcher_cfr_pdev_open(pdev);
	if (status != QDF_STATUS_SUCCESS && status != QDF_STATUS_COMP_DISABLED)
		goto cfr_pdev_open_fail;

	if (QDF_STATUS_SUCCESS != wlan_mgmt_txrx_pdev_open(pdev))
		goto mgmt_txrx_pdev_open_fail;

	if (QDF_IS_STATUS_ERROR(dispatcher_green_ap_pdev_open(pdev)))
		goto green_ap_pdev_open_fail;

	return QDF_STATUS_SUCCESS;

green_ap_pdev_open_fail:
	wlan_mgmt_txrx_pdev_close(pdev);
mgmt_txrx_pdev_open_fail:
	dispatcher_cfr_pdev_close(pdev);
cfr_pdev_open_fail:
	dispatcher_spectral_pdev_close(pdev);
spectral_pdev_open_fail:
	dispatcher_regulatory_pdev_close(pdev);
regulatory_pdev_open_fail:

	return QDF_STATUS_E_FAILURE;
}
qdf_export_symbol(dispatcher_pdev_open);

QDF_STATUS dispatcher_pdev_close(struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS status;

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_green_ap_pdev_close(pdev));

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_mgmt_txrx_pdev_close(pdev));

	status = dispatcher_cfr_pdev_close(pdev);
	QDF_BUG((QDF_STATUS_SUCCESS == status) ||
		(QDF_STATUS_COMP_DISABLED == status));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_spectral_pdev_close(pdev));

	QDF_BUG(QDF_STATUS_SUCCESS == dispatcher_regulatory_pdev_close(pdev));

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(dispatcher_pdev_close);
