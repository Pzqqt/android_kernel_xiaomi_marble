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
 * wlan_ipa_setup - IPA initialize and setup
 * @ipa_ctx: IPA priv obj
 * @ipa_cfg: IPA config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_setup(struct wlan_ipa_priv *ipa_ctx,
			  struct wlan_ipa_config *ipa_cfg);

/**
 * wlan_ipa_cleanup - IPA cleanup
 * @ipa_ctx: IPA priv obj
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_ipa_cleanup(struct wlan_ipa_priv *ipa_ctx);
#endif /* IPA_OFFLOAD */
#endif /* _WLAN_IPA_CORE_H_ */
