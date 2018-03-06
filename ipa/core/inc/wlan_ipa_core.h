/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef _WLAN_IPA_CORE_H_
#define _WLAN_IPA_CORE_H_

#ifdef IPA_OFFLOAD

#include "wlan_ipa_priv.h"
#include "wlan_ipa_public_struct.h"

/**
 * wlan_ipa_is_enabled() - Is IPA enabled?
 * @ipa_cfg: IPA config
 *
 * Return: true if IPA is enabled, false otherwise
 */
static inline bool wlan_ipa_is_enabled(struct wlan_ipa_config *ipa_cfg)
{
	return WLAN_IPA_IS_CONFIG_ENABLED(ipa_cfg, WLAN_IPA_ENABLE_MASK);
}

/**
 * wlan_ipa_uc_is_enabled() - Is IPA UC enabled?
 * @ipa_cfg: IPA config
 *
 * Return: true if IPA UC is enabled, false otherwise
 */
static inline bool wlan_ipa_uc_is_enabled(struct wlan_ipa_config *ipa_cfg)
{
	return WLAN_IPA_IS_CONFIG_ENABLED(ipa_cfg, WLAN_IPA_UC_ENABLE_MASK);
}

/**
 * wlan_ipa_is_rm_enabled() - Is IPA RM enabled?
 * @ipa_cfg: IPA config
 *
 * Return: true if IPA RM is enabled, false otherwise
 */
static inline bool wlan_ipa_is_rm_enabled(struct wlan_ipa_config *ipa_cfg)
{
	return WLAN_IPA_IS_CONFIG_ENABLED(ipa_cfg, WLAN_IPA_RM_ENABLE_MASK);
}

/**
 * wlan_ipa_setup - IPA initialize and setup
 * @ipa_ctx: IPA priv obj
 * @ipa_cfg: IPA config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_setup(struct wlan_ipa_priv *ipa_ctx,
			  struct wlan_ipa_config *ipa_cfg);

/**
 * wlan_ipa_get_obj_context - Get IPA OBJ context
 *
 * Return: IPA context
 */
struct wlan_ipa_priv *wlan_ipa_get_obj_context(void);

/**
 * wlan_ipa_cleanup - IPA cleanup
 * @ipa_ctx: IPA priv obj
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_cleanup(struct wlan_ipa_priv *ipa_ctx);

/**
 * wlan_ipa_uc_enable_pipes() - Enable IPA uC pipes
 * @ipa_ctx: IPA context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_uc_enable_pipes(struct wlan_ipa_priv *ipa_ctx);

/**
 * wlan_ipa_uc_disable_pipes() - Disable IPA uC pipes
 * @ipa_ctx: IPA context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_uc_disable_pipes(struct wlan_ipa_priv *ipa_ctx);

#ifndef CONFIG_IPA_WDI_UNIFIED_API
/**
 * wlan_ipa_wdi_rm_request_resource() - IPA WDI request resource
 * @ipa_ctx: IPA context
 * @res_name: IPA RM resource name
 *
 * Return: 0 on success, negative errno on error
 */
static inline
int wlan_ipa_wdi_rm_request_resource(struct wlan_ipa_priv *ipa_ctx,
				     qdf_ipa_rm_resource_name_t res_name)
{
	return qdf_ipa_rm_request_resource(res_name);
}

/**
 * wlan_ipa_wdi_rm_release_resource() - IPA WDI release resource
 * @ipa_ctx: IPA context
 * @res_name: IPA RM resource name
 *
 * Return: 0 on success, negative errno on error
 */
static inline
int wlan_ipa_wdi_rm_release_resource(struct wlan_ipa_priv *ipa_ctx,
				     qdf_ipa_rm_resource_name_t res_name)
{
	return qdf_ipa_rm_release_resource(res_name);
}

/**
 * wlan_ipa_wdi_rm_request() - Request resource from IPA
 * @ipa_ctx: IPA context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_wdi_rm_request(struct wlan_ipa_priv *ipa_ctx);

/**
 * wlan_ipa_wdi_rm_try_release() - Attempt to release IPA resource
 * @ipa_ctx: IPA context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_wdi_rm_try_release(struct wlan_ipa_priv *ipa_ctx);

/**
 * wlan_ipa_wdi_setup_rm() - Setup IPA resource management
 * @ipa_ctx: IPA context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_wdi_setup_rm(struct wlan_ipa_priv *ipa_ctx);

/**
 * wlan_ipa_wdi_destroy_rm() - Destroy IPA resources
 * @ipa_ctx: IPA context
 *
 * Destroys all resources associated with the IPA resource manager
 *
 * Return: None
 */
void wlan_ipa_wdi_destroy_rm(struct wlan_ipa_priv *ipa_ctx);

#else /* CONFIG_IPA_WDI_UNIFIED_API */

static inline int wlan_ipa_wdi_rm_request_resource(
			struct wlan_ipa_priv *ipa_ctx,
			qdf_ipa_rm_resource_name_t res_name)
{
	return 0;
}

static inline int wlan_ipa_wdi_rm_release_resource(
			struct wlan_ipa_priv *ipa_ctx,
			qdf_ipa_rm_resource_name_t res_name)
{
	return 0;
}

static inline QDF_STATUS wlan_ipa_wdi_setup_rm(struct wlan_ipa_priv *ipa_ctx)
{
	return 0;
}

static inline int wlan_ipa_wdi_destroy_rm(struct wlan_ipa_priv *ipa_ctx)
{
	return 0;
}

static inline QDF_STATUS wlan_ipa_wdi_rm_request(struct wlan_ipa_priv *ipa_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wlan_ipa_wdi_rm_try_release(struct wlan_ipa_priv
						     *ipa_ctx)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* CONFIG_IPA_WDI_UNIFIED_API */

#endif /* IPA_OFFLOAD */
#endif /* _WLAN_IPA_CORE_H_ */
