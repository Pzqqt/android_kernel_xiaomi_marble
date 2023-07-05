/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: target_if_reg.h
 * This file contains regulatory target interfaces.
 */
#ifndef __TARGET_IF_REG_H__
#define __TARGET_IF_REG_H__

/**
 * tgt_if_regulatory_modify_freq_range() - Modify low and high freq ranges based
 * on wireless mode.
 * @psoc: psoc pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS tgt_if_regulatory_modify_freq_range(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_register_regulatory_tx_ops() - register regulatory tx ops
 *
 * @tx_ops: tx_ops pointer
 * Return: Success or Failure
 */
QDF_STATUS target_if_register_regulatory_tx_ops(
		struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_reg_set_offloaded_info() - populate regulatory offloaded info
 *
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS target_if_reg_set_offloaded_info(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_reg_set_6ghz_info() - populate 6ghz enablement info
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS target_if_reg_set_6ghz_info(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_reg_set_5dot9_ghz_info() - populate 5.9ghz enablement info
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS target_if_reg_set_5dot9_ghz_info(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_reg_is_reg_cc_ext_event_host_supported() - Populate if reg_cc_ext
 * event is supported by host.
 * @psoc: psoc pointer
 *
 * Return: True if host supports, false otherwise.
 */
bool
target_if_reg_is_reg_cc_ext_event_host_supported(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_regulatory_get_rx_ops() - Get regdb rx ops
 * @psoc: pointer to psoc object
 *
 * Return: Reg rx_ops
 */
struct wlan_lmac_if_reg_rx_ops *
target_if_regulatory_get_rx_ops(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_regulatory_set_ext_tpc() - populate FW support of new WMI command
 * for TPC
 * @psoc: psoc pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS target_if_regulatory_set_ext_tpc(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_regulatory_get_tx_ops() - Get regdb tx ops
 * @psoc: pointer to psoc object
 *
 * Return: Reg tx_ops
 */
struct wlan_lmac_if_reg_tx_ops *
target_if_regulatory_get_tx_ops(struct wlan_objmgr_psoc *psoc);

#if defined(CONFIG_BAND_6GHZ)
/**
 * target_if_reg_set_lower_6g_edge_ch_info() - populate lower 6ghz edge channel
 * enablement info
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS
target_if_reg_set_lower_6g_edge_ch_info(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_reg_set_disable_upper_6g_edge_ch_info() - populate upper 6ghz
 * edge channel disablement info
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS
target_if_reg_set_disable_upper_6g_edge_ch_info(struct wlan_objmgr_psoc *psoc);
#else
static inline QDF_STATUS
target_if_reg_set_lower_6g_edge_ch_info(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_E_FAILURE;
}

static inline QDF_STATUS
target_if_reg_set_disable_upper_6g_edge_ch_info(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_E_FAILURE;
}
#endif
#endif /* __TARGET_IF_REG_H__ */
