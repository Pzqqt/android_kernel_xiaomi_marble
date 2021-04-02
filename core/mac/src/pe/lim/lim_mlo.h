/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : lim_mlo.h
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#if !defined(LIM_MLO_H)
#define LIM_MLO_H

#include "ani_global.h"
#include "lim_session.h"

#ifdef WLAN_FEATURE_11BE_MLO

/**
 * lim_update_partner_link_info - Update partner link information
 *
 * This function is triggered from mlo mgr
 *
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS_SUCCESS on successful update link info else failure.
 */
QDF_STATUS lim_partner_link_info_change(struct wlan_objmgr_vdev *vdev);

/**
 * lim_get_max_simultaneous_link_num() - Get max simultaneous link num
 *                                       It is max vdev number for sap
 * @session: pe session
 *
 * Return: max simultaneous link num
 */
uint8_t lim_get_max_simultaneous_link_num(struct pe_session *session);

/**
 * lim_mlo_free_vdev_ref() - release vdev reference
 * @vdev: vdev obj
 *
 * Return: void
 */
void lim_mlo_release_vdev_ref(struct wlan_objmgr_vdev *vdev);

/**
 * pe_find_partner_session_by_link_id() - Get partner session by link id
 * @session: pe session
 * @link_id: link id
 *
 * Return: partner session
 */
struct pe_session *pe_find_partner_session_by_link_id(
		struct pe_session *session, uint8_t link_id);

/**
 * lim_get_mlo_vdev_list() - Get mlo vdev list
 * @session: pe session
 * @vdev_count: vdev count
 * @wlan_vdev_list: vdev list
 *
 * Return: void
 */
void lim_get_mlo_vdev_list(struct pe_session *session, uint16_t *vdev_count,
			   struct wlan_objmgr_vdev **wlan_vdev_list);
#endif
#endif
