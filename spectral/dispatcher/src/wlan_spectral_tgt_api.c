/*
 * Copyright (c) 2011,2017 The Linux Foundation. All rights reserved.
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

#include <wlan_spectral_tgt_api.h>
#include "../../core/spectral_cmn_api_i.h"

int tgt_send_phydata(struct wlan_objmgr_pdev *pdev,
		     struct sock *sock, qdf_nbuf_t nbuf)
{
	return netlink_broadcast(sock, nbuf, 0, 1, GFP_ATOMIC);
}

void *tgt_get_target_handle(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_spectral *ps;

	if (!pdev) {
		spectral_err("PDEV is NULL!\n");
		return NULL;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(
		pdev,
		 WLAN_UMAC_COMP_SPECTRAL);
	if (!ps) {
		spectral_err("PDEV SPECTRAL object is NULL!\n");
		return NULL;
	}
	return ps->psptrl_target_handle;
}
