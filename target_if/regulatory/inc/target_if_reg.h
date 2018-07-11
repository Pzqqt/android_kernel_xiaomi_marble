/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * This file contains regulatory target interface
 */
#ifndef __TARGET_IF_REG_H__
#define __TARGET_IF_REG_H__

/**
 * target_if_register_regulatory_tx_ops() - register regulatory tx ops
 *
 * @tx_ops: tx_ops pointer
 * Return: Success or Failure
 */
QDF_STATUS target_if_register_regulatory_tx_ops(struct wlan_lmac_if_tx_ops
						*tx_ops);

/**
 * target_if_reg_set_offloaded_info() - populate regulatory offloaded info
 *
 * @psoc: psoc pointer
 * Return: Success or Failure
 */
QDF_STATUS target_if_reg_set_offloaded_info(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_if_regulatory_modify_freq_range() - Modify low and high freq ranges based
 * on wireless mode.
 */
QDF_STATUS tgt_if_regulatory_modify_freq_range(struct wlan_objmgr_psoc *psoc);
#endif /* __TARGET_IF_REG_H__ */
