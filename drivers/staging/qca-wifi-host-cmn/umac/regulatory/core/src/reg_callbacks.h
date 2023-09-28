/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: reg_callbks.h
 * This file provides prototypes of reg callbacks.
 */

#ifndef __REG_CALLBKS_H_
#define __REG_CALLBKS_H_

#ifdef CONFIG_REG_CLIENT
/**
 * reg_register_chan_change_callback() - Register channel change callbacks
 * @psoc: Pointer to psoc
 * @cbk: Pointer to callback function
 * @arg: List of arguments
 */
void reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
				       reg_chan_change_callback cbk, void *arg);

/**
 * reg_unregister_chan_change_callback() - Unregister channel change callbacks
 * @psoc: Pointer to psoc
 * @cbk: Pointer to callback function
 */
void reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					 reg_chan_change_callback cbk);

/**
 * reg_notify_sap_event() - Notify regulatory domain for sap event
 * @pdev: The physical dev to set the band for
 * @sap_state: true for sap start else false
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_notify_sap_event(struct wlan_objmgr_pdev *pdev,
				bool sap_state);

/**
 * reg_send_scheduler_msg_sb() - Start scheduler to call list of south bound
 * callbacks registered whenever current chan list changes.
 * @psoc: Pointer to PSOC structure.
 * @pdev: Pointer to PDEV structure.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_send_scheduler_msg_sb(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev);

/**
 * reg_send_scheduler_msg_nb() - Start scheduler to call list of north bound
 * @psoc: Pointer to global psoc structure.
 * @pdev: Pointer to global pdev structure.
 */
QDF_STATUS reg_send_scheduler_msg_nb(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev);
#else
static inline void reg_register_chan_change_callback(
		struct wlan_objmgr_psoc *psoc, reg_chan_change_callback cbk,
		void *arg)
{
}

static inline void reg_unregister_chan_change_callback(
		struct wlan_objmgr_psoc *psoc, reg_chan_change_callback cbk)
{
}

static inline QDF_STATUS reg_send_scheduler_msg_sb(
		struct wlan_objmgr_psoc *psoc, struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_send_scheduler_msg_nb(
		struct wlan_objmgr_psoc *psoc, struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
