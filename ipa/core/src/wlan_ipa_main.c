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
 * DOC: contains ipa component main function definitions
 */

#include "wlan_ipa_main.h"
#include "wlan_ipa_core.h"
#include "wlan_ipa_tgt_api.h"

static struct wlan_ipa_config *g_ipa_config;
static bool g_ipa_hw_support;

bool ipa_check_hw_present(void)
{
	/* Check if ipa hw is enabled */
	if (WLAN_IPA_CHECK_HW() != -EPERM) {
		g_ipa_hw_support = true;
		return true;
	} else {
		return false;
	}
}

QDF_STATUS ipa_config_mem_alloc(void)
{
	struct wlan_ipa_config *ipa_cfg;

	ipa_cfg = qdf_mem_malloc(sizeof(*ipa_cfg));
	if (!ipa_cfg) {
		ipa_err("Failed to allocate memory for ipa config");
		return QDF_STATUS_E_NOMEM;
	}

	g_ipa_config = ipa_cfg;

	return QDF_STATUS_SUCCESS;
}

void ipa_config_mem_free(void)
{
	if (!g_ipa_config) {
		ipa_err("IPA config already freed");
		return;
	}

	qdf_mem_free(g_ipa_config);
	g_ipa_config = NULL;
}

bool ipa_is_hw_support(void)
{
	return g_ipa_hw_support;
}

void ipa_config_update(struct wlan_ipa_config *config)
{
	if (!g_ipa_config) {
		ipa_err("IPA config already freed");
		return;
	}

	qdf_mem_copy(g_ipa_config, config, sizeof(*g_ipa_config));
}

bool ipa_config_is_enabled(void)
{
	return wlan_ipa_is_enabled(g_ipa_config);
}

QDF_STATUS ipa_obj_setup(struct wlan_ipa_priv *ipa_ctx)
{
	return wlan_ipa_setup(ipa_ctx, g_ipa_config);
}

QDF_STATUS ipa_obj_cleanup(struct wlan_ipa_priv *ipa_ctx)
{
	return wlan_ipa_cleanup(ipa_ctx);
}

QDF_STATUS ipa_send_uc_offload_enable_disable(struct wlan_objmgr_pdev *pdev,
				struct ipa_uc_offload_control_params *req)
{
	return tgt_ipa_uc_offload_enable_disable(pdev, req);
}

void ipa_set_dp_handle(struct wlan_objmgr_psoc *psoc, void *dp_soc)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ipa_priv *ipa_obj;

	if (!g_ipa_hw_support) {
		ipa_info("ipa hw not present");
		return;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, 0,
					  WLAN_IPA_ID);

	if (!pdev) {
		ipa_err("Failed to get pdev handle");
		return;
	}

	ipa_obj = ipa_pdev_get_priv_obj(pdev);
	if (!ipa_obj) {
		ipa_err("IPA object is NULL");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_IPA_ID);
		return;
	}

	ipa_obj->dp_soc = dp_soc;
	wlan_objmgr_pdev_release_ref(pdev, WLAN_IPA_ID);
}

void ipa_set_txrx_handle(struct wlan_objmgr_psoc *psoc, void *txrx_handle)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_ipa_priv *ipa_obj;

	if (!g_ipa_hw_support) {
		ipa_info("ipa hw not present");
		return;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, 0,
					  WLAN_IPA_ID);

	if (!pdev) {
		ipa_err("Failed to get pdev handle");
		return;
	}

	ipa_obj = ipa_pdev_get_priv_obj(pdev);
	if (!ipa_obj) {
		ipa_err("IPA object is NULL");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_IPA_ID);
		return;
	}

	ipa_obj->dp_pdev = txrx_handle;
	wlan_objmgr_pdev_release_ref(pdev, WLAN_IPA_ID);
}
