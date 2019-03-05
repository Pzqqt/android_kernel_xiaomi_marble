/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_CFR_TGT_API_H_
#define _WLAN_CFR_TGT_API_H_

#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_cmn.h>
#include <qdf_types.h>

/* tgt layer has APIs in application, to access functions in target
 * through tx_ops.
 */

/**
 * tgt_cfr_init_pdev() - API that registers CFR to handlers.
 * @pdev: pointer to pdev_object
 *
 * Return: success/failure of init
 */
int tgt_cfr_init_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_deinit_pdev() - API that de-registers CFR to handlers.
 * @pdev: pointer to pdev_object
 *
 * Return: success/failure of de-init
 */
int tgt_cfr_deinit_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_get_target_type() - API to determine target type.
 * @psoc: pointer to psoc_object
 *
 * Return: enum value of target type
 */
int tgt_cfr_get_target_type(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_cfr_start_capture() - API to start cfr capture on a peer.
 * @pdev: pointer to pdev_object
 * @peer: pointer to peer_object
 * @cfr_params: pointer to config cfr_params
 *
 * Return: success/failure of start capture
 */
int tgt_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_peer *peer,
			  struct cfr_capture_params *cfr_params);

/**
 * tgt_cfr_stop_capture() - API to stop cfr capture on a peer.
 * @pdev: pointer to pdev_object
 * @peer: pointer to peer_object
 *
 * Return: success/failure of stop capture
 */
int tgt_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			 struct wlan_objmgr_peer *peer);

/**
 * tgt_cfr_enable_cfr_timer() - API to enable cfr timer
 * @pdev: pointer to pdev_object
 * @cfr_timer: Amount of time this timer has to run. If 0, it disables timer.
 *
 * Return: success/failure of timer enable
 */
int
tgt_cfr_enable_cfr_timer(struct wlan_objmgr_pdev *pdev, uint32_t cfr_timer);

/**
 * tgt_cfr_support_set() - API to set cfr support
 * @psoc: pointer to psoc_object
 * @value: value to be set
 */
void tgt_cfr_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value);

/**
 * tgt_cfr_info_send() - API to send cfr info
 * @pdev: pointer to pdev_object
 * @head: pointer to cfr info head
 * @hlen: head len
 * @data: pointer to cfr info data
 * @dlen: data len
 * @tail: pointer to cfr info tail
 * @tlen: tail len
 *
 * Return: success/failure of cfr info send
 */
uint32_t tgt_cfr_info_send(struct wlan_objmgr_pdev *pdev, void *head,
			   size_t hlen, void *data, size_t dlen, void *tail,
			   size_t tlen);

#endif
