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
 * IPA user configuration and target interface
 */

#ifndef _WLAN_IPA_MAIN_H_
#define _WLAN_IPA_MAIN_H_

#ifdef IPA_OFFLOAD

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_ipa_public_struct.h>
#include <wlan_ipa_priv.h>

#define ipa_log(level, args...) QDF_TRACE(QDF_MODULE_ID_IPA, level, ## args)
#define ipa_logfl(level, format, args...) ipa_log(level, FL(format), ## args)

#define ipa_fatal(format, args...) \
		ipa_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define ipa_err(format, args...) \
		ipa_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define ipa_warn(format, args...) \
		ipa_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define ipa_info(format, args...) \
		ipa_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define ipa_debug(format, args...) \
		ipa_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define IPA_ENTER() ipa_debug("enter")
#define IPA_EXIT() ipa_debug("exit")

/**
 * ipa_check_hw_present() - get IPA hw status
 *
 * ipa_uc_reg_rdyCB is not directly designed to check
 * ipa hw status. This is an undocumented function which
 * has confirmed with IPA team.
 *
 * Return: true - ipa hw present
 *         false - ipa hw not present
 */
bool ipa_check_hw_present(void);

/**
 * wlan_get_pdev_ipa_obj() - private API to get ipa pdev object
 * @pdev: pdev object
 *
 * Return: ipa object
 */
static inline struct wlan_ipa_priv *
ipa_pdev_get_priv_obj(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_ipa_priv *pdev_obj;

	pdev_obj = (struct wlan_ipa_priv *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_IPA);

	return pdev_obj;
}

/**
 * ipa_is_hw_support() - Is IPA HW support?
 *
 * Return: true if IPA HW  is present or false otherwise
 */
bool ipa_is_hw_support(void);

/**
 * ipa_config_mem_alloc() - IPA config allocation
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ipa_config_mem_alloc(void);

/**
 * ipa_config_mem_free() - IPA config mem free
 *
 * Return: None
 */
void ipa_config_mem_free(void);

/**
 * ipa_config_update() - IPA component config update
 * @config: IPA config
 *
 * Return: None
 */
void ipa_config_update(struct wlan_ipa_config *config);

/**
 * ipa_config_is_enabled() - Is IPA config enabled?
 *
 * Return: true if IPA is enabled in IPA config
 */
bool ipa_config_is_enabled(void);

/**
 * ipa_obj_setup() - IPA obj initialization and setup
 * @ipa_ctx: IPA obj context
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ipa_obj_setup(struct wlan_ipa_priv *ipa_ctx);

/**
 * ipa_obj_cleanup() - IPA obj cleanup
 * @ipa_ctx: IPA obj context
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ipa_obj_cleanup(struct wlan_ipa_priv *ipa_ctx);

/**
 * ipa_send_uc_offload_enable_disable() - wdi enable/disable notify to fw
 * @pdev: objmgr pdev object
 * @req: ipa offload control request
 *
 * Return: QDF status success or failure
 */
QDF_STATUS ipa_send_uc_offload_enable_disable(struct wlan_objmgr_pdev *pdev,
				struct ipa_uc_offload_control_params *req);

/**
 * ipa_set_dp_handle() - set dp soc handle
 * @psoc: psoc handle
 * @dp_soc: dp soc handle
 *
 * Return: None
 */
void ipa_set_dp_handle(struct wlan_objmgr_psoc *psoc, void *dp_soc);

/**
 * ipa_set_txrx_handle() - set dp txrx handle
 * @psoc: psoc handle
 * @txrx_handle: dp txrx handle
 *
 * Return: None
 */
void ipa_set_txrx_handle(struct wlan_objmgr_psoc *psoc, void *txrx_handle);
#endif /* IPA_OFFLOAD */
#endif /* end  of _WLAN_IPA_MAIN_H_ */
