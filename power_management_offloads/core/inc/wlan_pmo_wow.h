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
 * DOC: Declare API's for wow pattern addition and deletion in fwr
 */

#ifndef _WLAN_PMO_WOW_H_
#define _WLAN_PMO_WOW_H_

#include "wlan_pmo_main.h"
#include "wlan_pmo_wow_public_struct.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_common_public_struct.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

/**
 * DOC: wlan_pmo_wowl
 *
 * This module houses all the logic for WOW(wake on wireless) in
 * PMO(Power Management and Offload).
 *
 * It provides the following APIs
 *
 * - Ability to enable/disable following WoWL modes
 *  1) Magic packet (MP) mode
 *  2) Pattern Byte Matching (PBM) mode
 * - Ability to add/remove patterns for PBM
 *
 * A Magic Packet is a packet that contains 6 0xFFs followed by 16
 * contiguous copies of the receiving NIC's Ethernet address. There is
 * no API to configure Magic Packet Pattern.
 *
 * Wakeup pattern (used for PBM) is defined as following:
 * struct
 * {
 *  U8  PatternSize;                  // Non-Zero pattern size
 *  U8  PatternMaskSize;              // Non-zero pattern mask size
 *  U8  PatternMask[PatternMaskSize]; // Pattern mask
 *  U8  Pattern[PatternSize];         // Pattern
 * } hdd_wowl_ptrn_t;
 *
 * PatternSize and PatternMaskSize indicate size of the variable
 * length Pattern and PatternMask. PatternMask indicates which bytes
 * of an incoming packet should be compared with corresponding bytes
 * in the pattern.
 *
 * Maximum allowed pattern size is 128 bytes. Maximum allowed
 * PatternMaskSize is 16 bytes.
 *
 * Maximum number of patterns that can be configured is 8
 *
 * PMO will add following 2 commonly used patterns for PBM by default:
 *  1) ARP Broadcast Pattern
 *  2) Unicast Pattern
 *
 * However note that WoWL will not be enabled by default by PMO. WoWL
 * needs to enabled explcitly by exercising the iwpriv command.
 *
 * PMO will expose an API that accepts patterns as Hex string in the
 * following format:
 * "PatternSize:PatternMaskSize:PatternMask:Pattern"
 *
 * Multiple patterns can be specified by deleimiting each pattern with
 * the ';' token:
 * "PatternSize1:PatternMaskSize1:PatternMask1:Pattern1;PatternSize2:..."
 *
 * Patterns can be configured dynamically via iwpriv cmd or statically
 * via qcom_cfg.ini file
 *
 * PBM (when enabled) can perform filtering on unicast data or
 * broadcast data or both. These configurations are part of factory
 * default (cfg.dat) and the default behavior is to perform filtering
 * on both unicast and data frames.
 *
 * MP filtering (when enabled) is performed ALWAYS on both unicast and
 * broadcast data frames.
 *
 * Management frames are not subjected to WoWL filtering and are
 * discarded when WoWL is enabled.
 *
 * Whenever a patern match succeeds, RX path is restored and packets
 * (both management and data) will be pushed to the host from that
 * point onwards.  Therefore, exit from WoWL is implicit and happens
 * automatically when the first packet match succeeds.
 *
 * WoWL works on top of BMPS. So when WoWL is requested, SME will
 * attempt to put the device in BMPS mode (if not already in BMPS). If
 * attempt to BMPS fails, request for WoWL will be rejected.
 */

/**
 * pmo_get_and_increment_wow_default_ptrn() -Get and increment wow default ptrn
 * @vdev_ctx: pmo vdev priv ctx
 *
 * API to get and increment wow default ptrn
 *
 * Return: current wow default ptrn count
 */
static inline uint8_t pmo_get_and_increment_wow_default_ptrn(
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	uint8_t count;

	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	count = vdev_ctx->num_wow_default_patterns++;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);

	return count;
}

/**
 * pmo_increment_wow_default_ptrn() -increment wow default ptrn
 * @vdev_ctx: pmo vdev priv ctx
 *
 * API to increment wow default ptrn
 *
 * Return: None
 */
static inline void pmo_increment_wow_default_ptrn(
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->num_wow_default_patterns++;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

/**
 * pmo_decrement_wow_default_ptrn() -decrement wow default ptrn
 * @vdev_ctx: pmo vdev priv ctx
 *
 * API to decrement wow default ptrn
 *
 * Return: None
 */
static inline void pmo_decrement_wow_default_ptrn(
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->num_wow_default_patterns--;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

/**
 * pmo_increment_wow_user_ptrn() -increment wow user ptrn
 * @vdev_ctx: pmo vdev priv ctx
 *
 * API to increment wow user ptrn
 *
 * Return: None
 */
static inline void pmo_increment_wow_user_ptrn(
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->num_wow_user_patterns++;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

/**
 * pmo_decrement_wow_user_ptrn() -decrement wow user ptrn
 * @vdev_ctx: pmo vdev priv ctx
 *
 * API to decrement wow user ptrn
 *
 * Return: None
 */
static inline void pmo_decrement_wow_user_ptrn(
		struct pmo_vdev_priv_obj *vdev_ctx)
{
	qdf_spin_lock_bh(&vdev_ctx->pmo_vdev_lock);
	vdev_ctx->num_wow_user_patterns--;
	qdf_spin_unlock_bh(&vdev_ctx->pmo_vdev_lock);
}

void pmo_dump_wow_ptrn(struct pmo_wow_add_pattern *ptrn);

/**
 * pmo_core_add_wow_pattern() - Function which will add the WoWL pattern to be
 *			 used when PBM filtering is enabled
 * @vdev: pointer to the vdev
 * @ptrn: pointer to the pattern string to be added
 *
 * Return: false if any errors encountered, QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS pmo_core_add_wow_pattern(struct wlan_objmgr_vdev *vdev,
		const char *ptrn);

/**
 * pmo_core_del_wow_pattern() - Function which will delete the WoWL pattern
 * @vdev: pointer to the vdev
 * @ptrn: pointer to the pattern string to be added
 *
 * Return: error if any errors encountered, QDF_STATUS_SUCCESS otherwise
 */
QDF_STATUS pmo_core_del_wow_pattern(struct wlan_objmgr_vdev *vdev,
		const char *ptrn);

/**
 * pmo_core_wow_enter() - store enable/disable status for pattern
 * @wma: wma handle
 * @info: wow parameters
 *
 * Records pattern enable/disable status locally. This choice will
 * take effect when the driver enter into suspend state.
 *
 * Return: QDF status
 */
QDF_STATUS pmo_core_wow_enter(struct wlan_objmgr_vdev *vdev,
		struct pmo_wow_enter_params *wow_enter_param);

/**
 * pmo_core_wow_exit() - clear all wma states
 * @wma: wma handle
 * @info: wow params
 *
 * Return: QDF status
 */
QDF_STATUS pmo_core_wow_exit(struct wlan_objmgr_vdev *vdev);

/**
 * pmo_core_enable_wakeup_event() -  enable wow wakeup events
 * @psoc: objmgr psoc
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 *
 * Return: none
 */
void pmo_core_enable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t bitmap);

/**
 * pmo_core_disable_wakeup_event() -  disable wow wakeup events
 * @psoc: objmgr psoc
 * @vdev_id: vdev id
 * @bitmap: Event bitmap
 *
 * Return: none
 */
void pmo_core_disable_wakeup_event(struct wlan_objmgr_psoc *psoc,
	uint32_t vdev_id, uint32_t bitmap);

#endif /* end  of _WLAN_PMO_WOW_H_ */
