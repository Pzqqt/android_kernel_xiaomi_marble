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
 * DOC: Declare public API related to the wlan ipa called by north bound
 */

#ifndef _WLAN_IPA_UCFG_API_H_
#define _WLAN_IPA_UCFG_API_H_

#include "wlan_ipa_public_struct.h"
#include "wlan_ipa_obj_mgmt_api.h"
#include "wlan_objmgr_pdev_obj.h"

#ifdef IPA_OFFLOAD

/**
 * ucfg_ipa_is_present() - get IPA hw status
 *
 * ipa_uc_reg_rdyCB is not directly designed to check
 * ipa hw status. This is an undocumented function which
 * has confirmed with IPA team.
 *
 * Return: true - ipa hw present
 *         false - ipa hw not present
 */
bool ucfg_ipa_is_present(void);

/**
 * ucfg_ipa_update_config() - Update IPA component config
 *
 * Return: None
 */
void ucfg_ipa_update_config(struct wlan_ipa_config *config);

/**
 * ucfg_ipa_set_dp_handle() - register DP handle
 * @psoc: psoc handle
 * @dp_soc: data path soc handle
 *
 * Return: None
 */
void ucfg_ipa_set_dp_handle(struct wlan_objmgr_psoc *psoc,
			       void *dp_soc);

/**
 * ucfg_ipa_set_txrx_handle() - register pdev txrx handler
 * @psoc: psoc handle
 * @txrx_handle: data path pdev txrx handle
 *
 * Return: None
 */
void ucfg_ipa_set_txrx_handle(struct wlan_objmgr_psoc *psoc,
			      void *txrx_handle);

/**
 * ucfg_ipa_set_perf_level() - Set IPA perf level
 * @pdev: Pdev obj handle
 * @tx_packets: Number of packets transmitted in the last sample period
 * @rx_packets: Number of packets received in the last sample period
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ucfg_ipa_set_perf_level(struct wlan_objmgr_pdev *pdev,
				   uint64_t tx_packets, uint64_t rx_packets);

#else

static inline bool ucfg_ipa_is_present(void)
{
	return false;
}

static inline void ucfg_ipa_update_config(struct wlan_ipa_config *config)
{
}

static inline
QDF_STATUS ucfg_ipa_set_dp_handle(struct wlan_objmgr_psoc *psoc,
				     void *dp_soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_set_txrx_handle(struct wlan_objmgr_psoc *psoc,
				    void *txrx_handle)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_set_perf_level(struct wlan_objmgr_pdev *pdev,
				   uint64_t tx_packets, uint64_t rx_packets)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */
#endif /* _WLAN_IPA_UCFG_API_H_ */
